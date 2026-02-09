// cm_tool.cpp (C++23)
// Championship Manager DB helper:
//  - Reads index.dat (skips 8-byte header) and finds blocks
//  - Reads club.dat (581-byte fixed records) and prints clubs / squads
//  - Reads staff.dat "people" block (fileType=6) and resolves names via:
//      first_names.dat / second_names.dat / common_names.dat
//
// IMPORTANT FIXES vs your previous version:
//  1) staff.dat blocks are selected by fileType (6/9/10/22), not "first three by offset".
//  2) TNames layout is correct: Name[51] at offset 0, ID at offset 51 (NOT name at +4).
//  3) TStaff layout is correct per the C# model: size 0x6E (110). Player at 0x61, NonPlayer at 0x69.
//
// Build:
//   clang++ -std=c++23 -O2 -Wall -Wextra cm_tool.cpp -o cm_tool
//
// Run:
//   ./cm_tool Input/Data --club-find ajax
//   ./cm_tool Input/Data --club-id 244
//   ./cm_tool Input/Data --staff-dump 89856

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace fs = std::filesystem;

// ======================================================
// TStaff (from provided C# Pack=1):
// Total size = 0x6E (110)
// ======================================================
static constexpr size_t STAFF_REC_SIZE        = 0x6E; // 110 bytes
static constexpr size_t OFF_STAFF_ID          = 0x00;
static constexpr size_t OFF_STAFF_FIRSTNAME   = 0x04;
static constexpr size_t OFF_STAFF_SECONDNAME  = 0x08;
static constexpr size_t OFF_STAFF_COMMONNAME  = 0x0C;

// Not actually pointers; these are IDs into player/nonplayer blocks.
static constexpr size_t OFF_STAFF_PLAYER_ID   = 0x61;
static constexpr size_t OFF_STAFF_PREFS_ID    = 0x65;
static constexpr size_t OFF_STAFF_NONPLAYER_ID= 0x69;

// ======================================================
// Helpers
// ======================================================
static std::vector<std::uint8_t> read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open file: " + p.string());
    f.seekg(0, std::ios::end);
    std::streamsize n = f.tellg();
    f.seekg(0, std::ios::beg);
    if (n < 0) throw std::runtime_error("Bad file size: " + p.string());
    std::vector<std::uint8_t> buf(static_cast<size_t>(n));
    if (!f.read(reinterpret_cast<char*>(buf.data()), n)) {
        throw std::runtime_error("Failed to read file: " + p.string());
    }
    return buf;
}

static std::uint32_t u32le_raw(const std::uint8_t* p) {
    return (std::uint32_t)p[0]
         | ((std::uint32_t)p[1] << 8)
         | ((std::uint32_t)p[2] << 16)
         | ((std::uint32_t)p[3] << 24);
}
static std::int32_t i32le_raw(const std::uint8_t* p) { return (std::int32_t)u32le_raw(p); }

static std::uint32_t u32le(const std::vector<std::uint8_t>& b, size_t off) {
    if (off + 4 > b.size()) return 0;
    return u32le_raw(b.data() + off);
}
static std::int32_t i32le(const std::vector<std::uint8_t>& b, size_t off) {
    if (off + 4 > b.size()) return 0;
    return i32le_raw(b.data() + off);
}

static std::string read_cstr_fixed(const std::vector<std::uint8_t>& b, size_t off, size_t maxLen) {
    std::string s;
    s.reserve(maxLen);
    for (size_t i = 0; i < maxLen && (off + i) < b.size(); ++i) {
        char c = (char)b[off + i];
        if (c == '\0') break;
        s.push_back(c);
    }
    return s;
}

static std::string rtrim_spaces(std::string s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) s.pop_back();
    return s;
}

static std::string to_lower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}
static bool icontains(std::string_view hay, std::string_view needle) {
    return to_lower(std::string(hay)).find(to_lower(std::string(needle))) != std::string::npos;
}

static void hex_dump(const std::vector<std::uint8_t>& v, size_t off, size_t len, size_t bytesPerLine = 16) {
    size_t end = std::min(off + len, v.size());
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);

    for (size_t i = off; i < end; i += bytesPerLine) {
        std::cout << "  +" << std::setw(4) << std::setfill('0') << std::hex << (i - off) << "  ";
        size_t lineEnd = std::min(i + bytesPerLine, end);
        for (size_t j = i; j < lineEnd; ++j) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)v[j] << " ";
        }
        std::cout << "\n";
    }

    std::cout.copyfmt(oldState);
}

static std::string fixed_cstr_to_string(const char* buf, size_t n) {
    size_t len = 0;
    while (len < n && buf[len] != '\0') ++len;
    return std::string(buf, buf + len);
}

static int count_non_minus_one(const int32_t* arr, size_t n) {
    int c = 0;
    for (size_t i = 0; i < n; ++i) if (arr[i] != -1) ++c;
    return c;
}

static std::string pro_status_to_string(uint8_t v) {
    switch (v) { case 1: return "pro"; case 2: return "semi"; case 3: return "amtr"; default: return "unk"; }
}

// ======================================================
// index.dat (TIndex in C#):
// Name[51], FileType(int), Count(int), Offset(int), Version(int)
// C# reads objects from indexData[8..] => 8-byte header
// ======================================================
struct IndexEntry {
    std::string filename;
    std::uint32_t fileType{};
    std::uint32_t count{};
    std::uint32_t offset{};
    std::uint32_t version{};
};

static std::vector<IndexEntry> parse_index_dat(const std::vector<std::uint8_t>& idx) {
    constexpr size_t HEADER = 8;
    constexpr size_t REC = 51 + 4 + 4 + 4 + 4;

    if (idx.size() < HEADER) throw std::runtime_error("index.dat too small");

    std::vector<IndexEntry> out;
    for (size_t off = HEADER; off + REC <= idx.size(); off += REC) {
        IndexEntry e;
        e.filename = rtrim_spaces(read_cstr_fixed(idx, off, 51));
        e.fileType = u32le(idx, off + 51);
        e.count    = u32le(idx, off + 55);
        e.offset   = u32le(idx, off + 59);
        e.version  = u32le(idx, off + 63);
        if (!e.filename.empty()) out.push_back(std::move(e));
    }
    return out;
}

static std::optional<IndexEntry> find_entry(const std::vector<IndexEntry>& entries,
                                            const std::string& name,
                                            std::optional<uint32_t> fileType = std::nullopt) {
    for (const auto& e : entries) {
        if (e.filename != name) continue;
        if (fileType && e.fileType != *fileType) continue;
        return e;
    }
    return std::nullopt;
}

static IndexEntry require_entry(const std::vector<IndexEntry>& entries,
                                const std::string& name,
                                std::optional<uint32_t> fileType = std::nullopt) {
    auto e = find_entry(entries, name, fileType);
    if (!e) {
        if (fileType) throw std::runtime_error("Missing entry: " + name + " fileType=" + std::to_string(*fileType));
        throw std::runtime_error("Missing entry: " + name);
    }
    return *e;
}

// ======================================================
// Names tables (TNames in C#):
//  - Name[51] at offset 0
//  - ID (int) at offset 51
//  - Nation (int) at offset 55
//  - Count (sbyte) at offset 59
// Total = 60 bytes
// ======================================================
struct NamesTable {
    std::vector<std::string> byIndex; // record order
    std::unordered_map<std::uint32_t, std::string> byIdField;
    size_t recSize{0};
};

static NamesTable load_names_table(const fs::path& filePath, uint32_t countFromIndex) {
    auto b = read_file(filePath);
    if (countFromIndex == 0) throw std::runtime_error(filePath.filename().string() + ": count is 0 in index.dat");
    if (b.empty()) throw std::runtime_error(filePath.filename().string() + ": file empty");

    if (b.size() % (size_t)countFromIndex != 0) {
        std::cerr << "[warn] " << filePath.filename().string()
                  << " size " << b.size() << " not divisible by count " << countFromIndex
                  << " (will use floor(size/count))\n";
    }

    size_t recSize = b.size() / (size_t)countFromIndex;
    if (recSize < 60) {
        std::cerr << "[warn] " << filePath.filename().string()
                  << " recSize=" << recSize << " (expected 60 for TNames)\n";
    }

    NamesTable t;
    t.recSize = recSize;
    t.byIndex.reserve(countFromIndex);
    t.byIdField.reserve(countFromIndex);

    constexpr size_t NAME_OFF = 0;
    constexpr size_t NAME_LEN = 51;
    constexpr size_t ID_OFF   = 51;

    for (size_t i = 0; i < (size_t)countFromIndex; ++i) {
        size_t off = i * recSize;
        if (off + recSize > b.size()) break;

        std::string name = rtrim_spaces(read_cstr_fixed(b, off + NAME_OFF, std::min(NAME_LEN, recSize)));
        std::uint32_t idField = (off + ID_OFF + 4 <= b.size()) ? u32le(b, off + ID_OFF) : 0;

        t.byIndex.push_back(name);
        if (!name.empty()) t.byIdField.emplace(idField, name);
    }
    return t;
}

static std::string resolve_name_id(const NamesTable& t, int32_t id) {
    if (id < 0) return {};

    // Try ID-field match (most robust)
    if (auto it = t.byIdField.find((std::uint32_t)id); it != t.byIdField.end())
        return it->second;

    // Try 0-based index
    if ((size_t)id < t.byIndex.size() && !t.byIndex[(size_t)id].empty())
        return t.byIndex[(size_t)id];

    // Try 1-based index
    if (id > 0 && (size_t)(id - 1) < t.byIndex.size() && !t.byIndex[(size_t)(id - 1)].empty())
        return t.byIndex[(size_t)(id - 1)];

    return {};
}

// ======================================================
// club.dat record (581 bytes) (matches C# TClub)
// ======================================================
#pragma pack(push, 1)
struct ClubRecord581 {
    int32_t id;
    std::array<char, 51> long_name;
    uint8_t long_name_gender;
    std::array<char, 26> short_name;
    uint8_t short_name_gender;
    int32_t nation_id;
    int32_t division_id;
    int32_t last_division_id;
    uint8_t last_position;
    int32_t reserve_division_id;
    uint8_t professional_status;
    int32_t bank_balance;
    int32_t stadium_id;
    uint8_t owns_stadium;
    int32_t reserve_stadium_id;
    uint8_t match_day;
    int32_t avg_attendance;
    int32_t min_attendance;
    int32_t max_attendance;
    uint8_t training_facilities;
    uint16_t reputation;
    uint8_t is_plc;
    int32_t home_shirt_fg;
    int32_t home_shirt_bg;
    int32_t away_shirt_fg;
    int32_t away_shirt_bg;
    int32_t third_shirt_fg;
    int32_t third_shirt_bg;
    std::array<int32_t, 3> liked_staff;
    std::array<int32_t, 3> disliked_staff;
    std::array<int32_t, 3> rival_clubs;
    int32_t chairman_staff_id;

    std::array<int32_t, 3> directors;
    int32_t manager_staff_id;
    int32_t assistant_manager_staff_id;

    std::array<int32_t, 50> playing_squad;
    std::array<int32_t, 5> coaches;
    std::array<int32_t, 7> scouts;
    std::array<int32_t, 3> physios;

    int32_t euro_flag;
    uint8_t euro_seeding;
    std::array<int32_t, 20> current_squad;

    std::array<int32_t, 4> tactics;
    int32_t current_tactics;
    uint8_t is_linked;
};
#pragma pack(pop)

static_assert(sizeof(ClubRecord581) == 581, "ClubRecord581 must be 581 bytes");

static std::vector<ClubRecord581> read_club_dat(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + p.string());
    in.seekg(0, std::ios::end);
    std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0) throw std::runtime_error("Bad club.dat size");
    size_t count = static_cast<size_t>(size / 581);
    std::vector<ClubRecord581> clubs(count);

    for (size_t i = 0; i < count; ++i) {
        ClubRecord581 c{};
        in.read(reinterpret_cast<char*>(&c), sizeof(c));
        if (!in) throw std::runtime_error("Read error club record " + std::to_string(i));
        clubs[i] = c;
    }
    return clubs;
}

// ======================================================
// staff(people) lite (based on the C# TStaff offsets)
// ======================================================
struct StaffLite {
    int32_t id{-1};
    int32_t firstNameRef{-1};
    int32_t secondNameRef{-1};
    int32_t commonNameRef{-1};
    int32_t playerId{-1};
    int32_t nonPlayerId{-1};
    int32_t prefsId{-1};
};

static StaffLite parse_staff_lite(const std::vector<std::uint8_t>& rec) {
    StaffLite s{};
    if (rec.size() < STAFF_REC_SIZE) return s;

    s.id           = i32le_raw(rec.data() + OFF_STAFF_ID);
    s.firstNameRef = i32le_raw(rec.data() + OFF_STAFF_FIRSTNAME);
    s.secondNameRef= i32le_raw(rec.data() + OFF_STAFF_SECONDNAME);
    s.commonNameRef= i32le_raw(rec.data() + OFF_STAFF_COMMONNAME);

    s.playerId     = i32le_raw(rec.data() + OFF_STAFF_PLAYER_ID);
    s.prefsId      = i32le_raw(rec.data() + OFF_STAFF_PREFS_ID);
    s.nonPlayerId  = i32le_raw(rec.data() + OFF_STAFF_NONPLAYER_ID);
    return s;
}

static std::string resolve_staff_name(const StaffLite& s,
                                      const NamesTable& firstNames,
                                      const NamesTable& secondNames,
                                      const NamesTable& commonNames) {
    std::string common = resolve_name_id(commonNames, s.commonNameRef);
    if (!common.empty()) return common;

    std::string fn = resolve_name_id(firstNames, s.firstNameRef);
    std::string sn = resolve_name_id(secondNames, s.secondNameRef);

    if (fn.empty() && sn.empty()) return "<unknown>";
    if (fn.empty()) return sn;
    if (sn.empty()) return fn;
    return fn + " " + sn;
}

// ======================================================
// CLI
// ======================================================
struct Args {
    fs::path dataDir = fs::path("Input") / "Data";
    std::optional<std::string> clubFind;
    std::optional<int32_t> clubId;
    std::optional<int32_t> staffDump;
};

static void usage() {
    std::cout <<
        "Usage:\n"
        "  cm_tool [DataDir] [--club-find TEXT] [--club-id N] [--staff-dump StaffID]\n";
}

static Args parse_args(int argc, char** argv) {
    Args a;
    int i = 1;

    if (i < argc && std::string_view(argv[i]).rfind("--", 0) != 0) {
        a.dataDir = fs::path(argv[i]);
        ++i;
    }

    while (i < argc) {
        std::string key = argv[i++];
        if (key == "--club-find") {
            if (i >= argc) throw std::runtime_error("--club-find needs a value");
            a.clubFind = std::string(argv[i++]);
        } else if (key == "--club-id") {
            if (i >= argc) throw std::runtime_error("--club-id needs a value");
            a.clubId = (int32_t)std::stol(argv[i++]);
        } else if (key == "--staff-dump") {
            if (i >= argc) throw std::runtime_error("--staff-dump needs a value");
            a.staffDump = (int32_t)std::stol(argv[i++]);
        } else if (key == "--help" || key == "-h") {
            usage();
            std::exit(0);
        } else {
            throw std::runtime_error("Unknown arg: " + key);
        }
    }
    return a;
}

// ======================================================
// Main
// ======================================================
int main(int argc, char** argv) {
    try {

        Args args = parse_args(argc, argv);

        auto indexDat = read_file(args.dataDir / "index.dat");
        auto entries  = parse_index_dat(indexDat);

        auto staffDat = read_file(args.dataDir / "staff.dat");
        auto clubs    = read_club_dat(args.dataDir / "club.dat");

        // staff.dat blocks by fileType (matches the C# example)
        IndexEntry staffPeople = require_entry(entries, "staff.dat", 6);
        IndexEntry staffNonP   = require_entry(entries, "staff.dat", 9);
        IndexEntry staffPlayer = require_entry(entries, "staff.dat", 10);
        auto staffPrefsOpt     = find_entry(entries, "staff.dat", 22);

        // Validate staff people block size vs expected record size (110)
        // We don't *need* next-block math; we can validate using offsets if we can find the end.
        auto next_staff_offset = [&](uint32_t curOffset) -> size_t {
            size_t end = staffDat.size();
            for (const auto& e : entries) {
                if (e.filename != "staff.dat") continue;
                if (e.offset > curOffset) end = std::min(end, (size_t)e.offset);
            }
            return end;
        };

        size_t peopleEnd = next_staff_offset(staffPeople.offset);
        size_t peopleBytes = peopleEnd - (size_t)staffPeople.offset;
        size_t expectedPeopleBytes = (size_t)staffPeople.count * STAFF_REC_SIZE;
        if (staffPeople.count > 0 && peopleBytes != expectedPeopleBytes) {
            std::cerr << "[warn] people block bytes=" << peopleBytes
                      << " but count*110=" << expectedPeopleBytes
                      << " (index offsets/counts or version mismatch?)\n";
        }

        // names counts from index.dat
        IndexEntry firstEntry  = require_entry(entries, "first_names.dat");
        IndexEntry secondEntry = require_entry(entries, "second_names.dat");
        IndexEntry commonEntry = require_entry(entries, "common_names.dat");

        NamesTable firstNames  = load_names_table(args.dataDir / "first_names.dat",  firstEntry.count);
        NamesTable secondNames = load_names_table(args.dataDir / "second_names.dat", secondEntry.count);
        NamesTable commonNames = load_names_table(args.dataDir / "common_names.dat", commonEntry.count);

        std::cout << "Loaded:\n";
        std::cout << "  clubs:     " << clubs.size() << " (club.dat)\n";
        std::cout << "  staff.dat: " << staffDat.size() << " bytes\n";
        std::cout << "Staff blocks:\n";
        std::cout << "  people (type=6)  offset=" << staffPeople.offset << " count=" << staffPeople.count << "\n";
        std::cout << "  nonP   (type=9)  offset=" << staffNonP.offset   << " count=" << staffNonP.count   << "\n";
        std::cout << "  player (type=10) offset=" << staffPlayer.offset << " count=" << staffPlayer.count << "\n";
        if (staffPrefsOpt) {
            std::cout << "  prefs  (type=22) offset=" << staffPrefsOpt->offset << " count=" << staffPrefsOpt->count << "\n";
        }
        std::cout << "Names recSize:\n";
        std::cout << "  first="  << firstNames.recSize  << " second=" << secondNames.recSize << " common=" << commonNames.recSize << "\n\n";

        auto get_staff_people_record = [&](int32_t staffId) -> std::optional<std::vector<std::uint8_t>> {
            if (staffId < 0 || (uint32_t)staffId >= staffPeople.count) return std::nullopt;
            size_t off = (size_t)staffPeople.offset + (size_t)staffId * STAFF_REC_SIZE;
            if (off + STAFF_REC_SIZE > staffDat.size()) return std::nullopt;
            return std::vector<std::uint8_t>(staffDat.begin() + (long)off, staffDat.begin() + (long)(off + STAFF_REC_SIZE));
        };

        // Staff dump
        if (args.staffDump) {
            int32_t sid = *args.staffDump;
            auto recOpt = get_staff_people_record(sid);
            if (!recOpt) throw std::runtime_error("staffId out of range for people block");
            auto rec = *recOpt;

            StaffLite s = parse_staff_lite(rec);
            std::cout << "STAFF DUMP staffId=" << sid << "\n";
            std::cout << "  idField=" << s.id
                      << " firstRef=" << s.firstNameRef
                      << " secondRef=" << s.secondNameRef
                      << " commonRef=" << s.commonNameRef
                      << " playerId=" << s.playerId
                      << " nonPlayerId=" << s.nonPlayerId
                      << " prefsId=" << s.prefsId
                      << "\n";
            std::cout << "  name=" << resolve_staff_name(s, firstNames, secondNames, commonNames) << "\n";
            std::cout << "  record bytes (first 128):\n";
            hex_dump(rec, 0, std::min<size_t>(128, rec.size()));
            return 0;
        }

        auto print_club_full_with_squad = [&](const ClubRecord581& c) {
            std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
            std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());

            std::cout << "\n================ CLUB " << c.id << " ================\n";
            std::cout << "Short name : " << sn << "\n";
            std::cout << "Long name  : " << ln << "\n";
            std::cout << "Nation ID  : " << c.nation_id << "\n";
            std::cout << "Division ID: " << c.division_id << "\n";
            std::cout << "Reputation : " << c.reputation << "\n";
            std::cout << "Bank       : " << c.bank_balance << "\n";
            std::cout << "Pro status : " << pro_status_to_string(c.professional_status) << "\n";
            std::cout << "Squad count: " << count_non_minus_one(c.playing_squad.data(), c.playing_squad.size()) << "\n\n";

            std::cout << "Playing squad (staffId -> name -> playerId/nonPlayerId):\n";
            for (size_t i = 0; i < c.playing_squad.size(); ++i) {
                int32_t sid = c.playing_squad[i];
                if (sid == -1) continue;

                auto recOpt = get_staff_people_record(sid);
                if (!recOpt) continue;

                StaffLite s = parse_staff_lite(*recOpt);
                std::string name = resolve_staff_name(s, firstNames, secondNames, commonNames);

                std::cout << "  [" << std::setw(2) << i << "] "
                          << "staffId=" << std::setw(7) << sid
                          << "  name=" << std::setw(30) << std::left << name
                          << "  playerId=" << std::setw(7) << std::right << s.playerId
                          << "  nonPlayerId=" << std::setw(7) << s.nonPlayerId
                          << "\n";
            }

            std::cout << "============================================\n";
        };

        auto print_club_row = [&](const ClubRecord581& c) {
            std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
            std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());
            std::cout << std::left
                      << std::setw(6)  << c.id
                      << std::setw(28) << (sn.empty() ? "-" : sn.substr(0, 27))
                      << std::setw(22) << (ln.empty() ? "-" : ln.substr(0, 21))
                      << std::setw(8)  << c.nation_id
                      << std::setw(8)  << c.division_id
                      << std::setw(8)  << c.reputation
                      << std::setw(12) << c.bank_balance
                      << std::setw(6)  << pro_status_to_string(c.professional_status)
                      << "\n";
        };

        // club-id
        if (args.clubId) {
            int32_t cid = *args.clubId;
            auto it = std::ranges::find_if(clubs, [&](const ClubRecord581& c) { return c.id == cid; });
            if (it == clubs.end()) throw std::runtime_error("No club with that ID");
            print_club_full_with_squad(*it);
            return 0;
        }

        // club-find
        if (args.clubFind) {
            std::string needle = *args.clubFind;
            std::vector<const ClubRecord581*> matches;
            for (const auto& c : clubs) {
                std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
                std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());
                if (icontains(sn, needle) || icontains(ln, needle)) matches.push_back(&c);
            }

            std::cout << "Matches for \"" << needle << "\": " << matches.size() << "\n\n";
            std::cout << std::left
                      << std::setw(6)  << "ID"
                      << std::setw(28) << "ShortName"
                      << std::setw(22) << "LongName"
                      << std::setw(8)  << "Nation"
                      << std::setw(8)  << "Div"
                      << std::setw(8)  << "Rep"
                      << std::setw(12) << "Bank"
                      << std::setw(6)  << "Pro"
                      << "\n";
            std::cout << std::string(6+28+22+8+8+8+12+6, '-') << "\n";
            for (auto* pc : matches) print_club_row(*pc);

            if (matches.size() == 1) {
                std::cout << "\nOnly one match -> printing full club:\n";
                print_club_full_with_squad(*matches.front());
            }
            return 0;
        }

        // default list
        std::cout << "First 40 clubs:\n";
        std::cout << std::left
                  << std::setw(6)  << "ID"
                  << std::setw(28) << "ShortName"
                  << std::setw(22) << "LongName"
                  << std::setw(8)  << "Nation"
                  << std::setw(8)  << "Div"
                  << std::setw(8)  << "Rep"
                  << std::setw(12) << "Bank"
                  << std::setw(6)  << "Pro"
                  << "\n";
        std::cout << std::string(6+28+22+8+8+8+12+6, '-') << "\n";
        for (size_t i = 0; i < std::min<size_t>(40, clubs.size()); ++i) print_club_row(clubs[i]);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n[error] " << e.what() << "\n\n";
        usage();
        return 1;
    }
}

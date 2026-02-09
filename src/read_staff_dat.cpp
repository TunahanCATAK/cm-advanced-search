#include <algorithm>
#include <array>
#include <cctype>
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

namespace fs = std::filesystem;

// ---------------------------
// Binary helpers
// ---------------------------
static std::vector<std::uint8_t> read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open file: " + p.string());
    f.seekg(0, std::ios::end);
    std::streamsize n = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buf(static_cast<size_t>(n));
    if (!f.read(reinterpret_cast<char*>(buf.data()), n)) {
        throw std::runtime_error("Failed to read file: " + p.string());
    }
    return buf;
}

static std::uint32_t u32le(const std::vector<std::uint8_t>& b, size_t off) {
    return (std::uint32_t)b[off + 0]
         | ((std::uint32_t)b[off + 1] << 8)
         | ((std::uint32_t)b[off + 2] << 16)
         | ((std::uint32_t)b[off + 3] << 24);
}
static std::int32_t i32le(const std::vector<std::uint8_t>& b, size_t off) {
    return (std::int32_t)u32le(b, off);
}
static std::string read_cstr_fixed(const std::vector<std::uint8_t>& b, size_t off, size_t maxLen) {
    std::string s;
    s.reserve(maxLen);
    for (size_t i = 0; i < maxLen; ++i) {
        char c = (char)b[off + i];
        if (c == '\0') break;
        s.push_back(c);
    }
    return s;
}

static std::string to_lower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// ---------------------------
// index.dat parsing
// Format from John Locke doc:
// header 8 bytes, then repeating:
// file name 51 bytes, id 4, count 4, offset 4, type/version 4 :contentReference[oaicite:2]{index=2}
// ---------------------------
struct IndexEntry {
    std::string filename;   // e.g. "staff.dat"
    std::uint32_t fileId{};
    std::uint32_t count{};
    std::uint32_t offset{};
    std::uint32_t fileType{};
};

static std::vector<IndexEntry> parse_index_dat(const std::vector<std::uint8_t>& idx) {
    constexpr size_t HEADER = 8;
    constexpr size_t REC = 51 + 4 + 4 + 4 + 4;

    if (idx.size() < HEADER) throw std::runtime_error("index.dat too small");

    std::vector<IndexEntry> out;
    for (size_t off = HEADER; off + REC <= idx.size(); off += REC) {
        IndexEntry e;
        e.filename = read_cstr_fixed(idx, off, 51);
        e.fileId   = u32le(idx, off + 51);
        e.count    = u32le(idx, off + 55);
        e.offset   = u32le(idx, off + 59);
        e.fileType = u32le(idx, off + 63);

        // Skip empty trailing records
        if (e.filename.empty()) continue;
        out.push_back(std::move(e));
    }
    return out;
}

// ---------------------------
// Names files: first_names.dat / second_names.dat / common_names.dat
// 60-byte repeating record per Locke doc :contentReference[oaicite:3]{index=3}
// (We only need id + string)
// ---------------------------
static std::unordered_map<std::uint32_t, std::string>
load_names_map(const fs::path& namesFile) {
    auto b = read_file(namesFile);
    constexpr size_t REC = 60;

    std::unordered_map<std::uint32_t, std::string> m;
    for (size_t off = 0; off + REC <= b.size(); off += REC) {
        std::uint32_t id = u32le(b, off);
        std::string name = read_cstr_fixed(b, off + 4, 51); // includes terminator space per doc
        if (!name.empty()) m.emplace(id, std::move(name));
    }
    return m;
}

// ---------------------------
// staff.dat: contains multiple blocks.
// We use index.dat to locate the 4 sub-blocks for staff.dat:
// - FileType 6  : "people/staff" (TStaff)  (in your C# code: staffBlockIndex)
// - FileType 9  : non-players (TNonPlayer)
// - FileType 10 : players (TPlayer)
// - FileType 22 : preferences (TPreferences)
// The only universally-safe thing: index.dat tells offset + count for each sub-block. :contentReference[oaicite:4]{index=4}
//
// Layout problem: We still need the *record sizes* for TStaff and TPlayer.
// We'll implement reading as "fixed-size records" once you set STAFF_REC_SIZE / PLAYER_REC_SIZE.
// ---------------------------
struct StaffLite {
    std::int32_t id{-1};
    std::int32_t firstNameId{-1};
    std::int32_t secondNameId{-1};
    std::int32_t commonNameId{-1};
    std::int32_t playerPtr{-1};  // ID into player block, or -1
    std::int32_t nonPlayerPtr{-1};
    std::int32_t prefPtr{-1};

    // keep raw bytes for debug printing / later expansion
    std::vector<std::uint8_t> raw;
};

static void hex_dump_line(const std::vector<std::uint8_t>& v, size_t off, size_t len) {
    size_t end = std::min(off + len, v.size());
    for (size_t i = off; i < end; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)v[i] << " ";
    }
    std::cout << std::dec << "\n";
}

// ---- IMPORTANT: adjust these once you confirm C# struct sizes ----
// If you already have the CM.Save.Model structs, set these to Marshal.SizeOf(TStaff/TPlayer).
static constexpr size_t STAFF_REC_SIZE  = 110; // common guess; verify!
static constexpr size_t PLAYER_REC_SIZE =  80; // placeholder; verify!

// Field offsets inside TStaff record (bytes).
// These are ALSO placeholders. You should update them from your C# struct layout.
static constexpr size_t OFF_STAFF_ID         = 0;
static constexpr size_t OFF_STAFF_FIRSTNAME  = 4;
static constexpr size_t OFF_STAFF_SECONDNAME = 8;
static constexpr size_t OFF_STAFF_COMMONNAME = 12;

// Pointers (player/nonplayer/prefs) — placeholders.
// In your C# model: staff.NonPlayer, staff.Player, staff.StaffPreferences
static constexpr size_t OFF_STAFF_NONPLAYER_PTR = 106;
static constexpr size_t OFF_STAFF_PLAYER_PTR    = 98;
static constexpr size_t OFF_STAFF_PREF_PTR      = 64;

static std::vector<StaffLite> load_staff_block(
    const std::vector<std::uint8_t>& staffDat,
    const IndexEntry& staffPeopleEntry
) {
    const size_t base = staffPeopleEntry.offset;
    const size_t totalBytes = (size_t)staffPeopleEntry.count * STAFF_REC_SIZE;
    if (base + totalBytes > staffDat.size()) {
        throw std::runtime_error("staff.dat too small for staff people block (check STAFF_REC_SIZE and index offsets)");
    }

    std::vector<StaffLite> out;
    out.reserve(staffPeopleEntry.count);

    for (size_t i = 0; i < staffPeopleEntry.count; ++i) {
        size_t off = base + i * STAFF_REC_SIZE;
        StaffLite s;
        s.raw.assign(staffDat.begin() + (long)off, staffDat.begin() + (long)(off + STAFF_REC_SIZE));

        s.id          = i32le(staffDat, off + OFF_STAFF_ID);
        s.firstNameId = i32le(staffDat, off + OFF_STAFF_FIRSTNAME);
        s.secondNameId= i32le(staffDat, off + OFF_STAFF_SECONDNAME);
        s.commonNameId= i32le(staffDat, off + OFF_STAFF_COMMONNAME);

        s.nonPlayerPtr= i32le(staffDat, off + OFF_STAFF_NONPLAYER_PTR);
        s.playerPtr   = i32le(staffDat, off + OFF_STAFF_PLAYER_PTR);
        s.prefPtr     = i32le(staffDat, off + OFF_STAFF_PREF_PTR);

        out.push_back(std::move(s));
    }
    return out;
}

static std::string resolve_staff_name(
    const StaffLite& s,
    const std::unordered_map<std::uint32_t, std::string>& firstNames,
    const std::unordered_map<std::uint32_t, std::string>& secondNames,
    const std::unordered_map<std::uint32_t, std::string>& commonNames
) {
    auto get = [](const auto& m, std::int32_t id) -> std::string {
        if (id < 0) return {};
        auto it = m.find((std::uint32_t)id);
        return it == m.end() ? std::string{} : it->second;
    };

    std::string common = get(commonNames, s.commonNameId);
    if (!common.empty()) return common;

    std::string fn = get(firstNames, s.firstNameId);
    std::string sn = get(secondNames, s.secondNameId);
    if (fn.empty() && sn.empty()) return "<unknown>";
    if (fn.empty()) return sn;
    if (sn.empty()) return fn;
    return fn + " " + sn;
}

static void print_staff_full(
    const StaffLite& s,
    const std::unordered_map<std::uint32_t, std::string>& firstNames,
    const std::unordered_map<std::uint32_t, std::string>& secondNames,
    const std::unordered_map<std::uint32_t, std::string>& commonNames
) {
    std::cout << "--------------------------------------------------\n";
    std::cout << "Staff ID:         " << s.id << "\n";
    std::cout << "Name:             " << resolve_staff_name(s, firstNames, secondNames, commonNames) << "\n";
    std::cout << "FirstNameId:      " << s.firstNameId << "\n";
    std::cout << "SecondNameId:     " << s.secondNameId << "\n";
    std::cout << "CommonNameId:     " << s.commonNameId << "\n";
    std::cout << "PlayerPtr:        " << s.playerPtr << "\n";
    std::cout << "NonPlayerPtr:     " << s.nonPlayerPtr << "\n";
    std::cout << "PreferencesPtr:   " << s.prefPtr << "\n";
    std::cout << "Raw (first 64B):  ";
    hex_dump_line(s.raw, 0, std::min<size_t>(64, s.raw.size()));
}

int main(int argc, char** argv) {
    try {
        // Expected folder structure:
        // Input/Data/index.dat
        // Input/Data/staff.dat
        // Input/Data/first_names.dat
        // Input/Data/second_names.dat
        // Input/Data/common_names.dat
        fs::path dataDir = (argc >= 2) ? fs::path(argv[1]) : fs::path("Input") / "Data";

        const auto indexDat = read_file(dataDir / "index.dat");
        const auto staffDat = read_file(dataDir / "staff.dat");

        auto firstNames  = load_names_map(dataDir / "first_names.dat");
        auto secondNames = load_names_map(dataDir / "second_names.dat");
        auto commonNames = load_names_map(dataDir / "common_names.dat");

        auto entries = parse_index_dat(indexDat);
        // std::cout << "Index entries (" << entries.size() << "):\n";
        // for (const auto& e : entries) {
        //     std::cout
        //         << "  name=\"" << e.filename << "\""
        //         << " type=" << e.fileType
        //         << " id=" << e.fileId
        //         << " count=" << e.count
        //         << " offset=" << e.offset
        //         << "\n";
        // }
        // std::cout << "\n";
        auto findEntry = [&](std::string_view fname, std::optional<std::uint32_t> fileType) -> IndexEntry {
            for (const auto& e : entries) {
                if (e.filename == fname) {
                    if (!fileType || e.fileType == *fileType) return e;
                }
            }
            throw std::runtime_error("index.dat missing entry: " + std::string(fname) +
                                     (fileType ? (" (type=" + std::to_string(*fileType) + ")") : ""));
        };

        // staff.dat blocks (per your C# logic and Locke note about staff.dat being special) :contentReference[oaicite:5]{index=5}
        IndexEntry staffPeople = findEntry("staff.dat", 6);
        IndexEntry staffNonPlayers = findEntry("staff.dat", 9);
        IndexEntry staffPlayers = findEntry("staff.dat", 10);
        IndexEntry staffPrefs = findEntry("staff.dat", 22);

        std::cout << "Loaded index.dat entries: " << entries.size() << "\n";
        std::cout << "staff.dat blocks:\n";
        std::cout << "  people(type=6):    offset=" << staffPeople.offset << " count=" << staffPeople.count << "\n";
        std::cout << "  nonplayers(type=9):offset=" << staffNonPlayers.offset << " count=" << staffNonPlayers.count << "\n";
        std::cout << "  players(type=10):  offset=" << staffPlayers.offset << " count=" << staffPlayers.count << "\n";
        std::cout << "  prefs(type=22):    offset=" << staffPrefs.offset << " count=" << staffPrefs.count << "\n\n";

        auto staff = load_staff_block(staffDat, staffPeople);

        // Simple interactive search by name (case-insensitive substring)
        std::cout << "Enter a staff name substring to search (empty = skip): ";
        std::string q;
        std::getline(std::cin, q);
        q = to_lower(q);

        if (!q.empty()) {
            int shown = 0;
            for (const auto& s : staff) {
                std::string name = resolve_staff_name(s, firstNames, secondNames, commonNames);
                if (to_lower(name).find(q) != std::string::npos) {
                    print_staff_full(s, firstNames, secondNames, commonNames);
                    if (++shown >= 20) {
                        std::cout << "(showing first 20 matches)\n";
                        break;
                    }
                }
            }
            if (shown == 0) std::cout << "No matches.\n";
        }

        // Direct lookup by Staff ID
        std::cout << "\nEnter a Staff ID to print all core fields (-1 = exit): ";
        int sid = -1;
        if (std::cin >> sid && sid >= 0 && (size_t)sid < staff.size()) {
            print_staff_full(staff[(size_t)sid], firstNames, secondNames, commonNames);
        }

        std::cout << "\nDone.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

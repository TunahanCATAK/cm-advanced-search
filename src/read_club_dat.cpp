#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

static std::string fixed_cstr_to_string(const char* buf, size_t n) {
    size_t len = 0;
    while (len < n && buf[len] != '\0') ++len;
    return std::string(buf, buf + len);
}

static std::string to_lower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

static bool icontains(std::string_view haystack, std::string_view needle) {
    std::string h(haystack);
    std::string n(needle);
    h = to_lower(h);
    n = to_lower(n);
    return h.find(n) != std::string::npos;
}

static int count_non_minus_one(const int32_t* arr, size_t n) {
    int c = 0;
    for (size_t i = 0; i < n; ++i) if (arr[i] != -1) ++c;
    return c;
}

static std::string pro_status_to_string(uint8_t v) {
    switch (v) {
        case 1: return "pro";
        case 2: return "semi";
        case 3: return "amtr";
        default: return "unk";
    }
}

template <size_t N>
static void print_int_array(const std::string& label,
                            const std::array<int32_t, N>& a,
                            int perLine = 10,
                            int indent = 2) {
    std::cout << std::string(indent, ' ') << label << " (" << N << "):\n";
    for (size_t i = 0; i < N; ++i) {
        if (i % static_cast<size_t>(perLine) == 0) {
            std::cout << std::string(indent + 2, ' ');
        }
        std::cout << std::setw(6) << a[i] << " ";
        if ((i + 1) % static_cast<size_t>(perLine) == 0 || i + 1 == N) std::cout << "\n";
    }
}

#pragma pack(push, 1)
struct ClubRecord581 {
    int32_t id;                              // 0..3
    std::array<char, 51> long_name;          // 4..54
    uint8_t long_name_gender;                // 55
    std::array<char, 26> short_name;         // 56..81
    uint8_t short_name_gender;               // 82
    int32_t nation_id;                       // 83..86
    int32_t division_id;                     // 87..90
    int32_t last_division_id;                // 91..94
    uint8_t last_position;                   // 95
    int32_t reserve_division_id;             // 96..99
    uint8_t professional_status;             // 100
    int32_t bank_balance;                    // 101..104
    int32_t stadium_id;                      // 105..108
    uint8_t owns_stadium;                    // 109
    int32_t reserve_stadium_id;              // 110..113
    uint8_t match_day;                       // 114
    int32_t avg_attendance;                  // 115..118
    int32_t min_attendance;                  // 119..122
    int32_t max_attendance;                  // 123..126
    uint8_t training_facilities;             // 127
    int16_t reputation;                      // 128..129
    uint8_t is_plc;                          // 130
    int32_t home_shirt_fg;                   // 131..134
    int32_t home_shirt_bg;                   // 135..138
    int32_t away_shirt_fg;                   // 139..142
    int32_t away_shirt_bg;                   // 143..146
    int32_t third_shirt_fg;                  // 147..150
    int32_t third_shirt_bg;                  // 151..154
    std::array<int32_t, 3> liked_staff;      // 155..166
    std::array<int32_t, 3> disliked_staff;   // 167..178
    std::array<int32_t, 3> rival_clubs;      // 179..190
    int32_t chairman_staff_id;               // 191..194

    std::array<int32_t, 3> directors;        // 195..206
    int32_t manager_staff_id;                // 207..210
    int32_t assistant_manager_staff_id;      // 211..214

    std::array<int32_t, 50> playing_squad;   // 215..414
    std::array<int32_t, 5> coaches;          // 415..434
    std::array<int32_t, 7> scouts;           // 435..462
    std::array<int32_t, 3> physios;          // 463..474

    int32_t euro_flag;                       // 475..478
    uint8_t euro_seeding;                    // 479
    std::array<int32_t, 20> current_squad;   // 480..559

    std::array<int32_t, 4> tactics;          // 560..575
    int32_t current_tactics;                 // 576..579
    uint8_t is_linked;                       // 580
};
#pragma pack(pop)

static_assert(sizeof(ClubRecord581) == 581, "ClubRecord581 must be exactly 581 bytes.");
static_assert(alignof(ClubRecord581) == 1, "ClubRecord581 must be packed.");

static std::vector<ClubRecord581> read_club_dat(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + p.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0) throw std::runtime_error("Bad file size.");
    if (size % 581 != 0) {
        std::cerr << "[warn] File size (" << size << ") is not divisible by 581. "
                  << "Parsing will use floor(size/581) records.\n";
    }

    const size_t count = static_cast<size_t>(size / 581);
    std::vector<ClubRecord581> clubs(count);

    for (size_t i = 0; i < count; ++i) {
        ClubRecord581 rec{};
        in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
        clubs[i] = rec;
    }
    return clubs;
}

static void print_compact_table(const std::vector<ClubRecord581>& clubs, size_t maxRows = 40) {
    const size_t N = std::min(maxRows, clubs.size());

    std::cout
        << std::left
        << std::setw(6)  << "ID"
        << std::setw(28) << "ShortName"
        << std::setw(20) << "LongName"
        << std::setw(8)  << "Nation"
        << std::setw(8)  << "Div"
        << std::setw(8)  << "Rep"
        << std::setw(12) << "Bank"
        << std::setw(6)  << "Pro"
        << std::setw(6)  << "Sq"
        << std::setw(6)  << "Cur"
        << "\n";

    std::cout << std::string(6+28+20+8+8+8+12+6+6+6, '-') << "\n";

    for (size_t i = 0; i < N; ++i) {
        const auto& c = clubs[i];
        const std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
        const std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());

        const int squadCount = count_non_minus_one(c.playing_squad.data(), c.playing_squad.size());
        const int curCount   = count_non_minus_one(c.current_squad.data(), c.current_squad.size());

        std::cout
            << std::left
            << std::setw(6)  << c.id
            << std::setw(28) << (sn.empty() ? "-" : sn.substr(0, 27))
            << std::setw(20) << (ln.empty() ? "-" : ln.substr(0, 19))
            << std::setw(8)  << c.nation_id
            << std::setw(8)  << c.division_id
            << std::setw(8)  << c.reputation
            << std::setw(12) << c.bank_balance
            << std::setw(6)  << pro_status_to_string(c.professional_status)
            << std::setw(6)  << squadCount
            << std::setw(6)  << curCount
            << "\n";
    }
}

static void print_full(const ClubRecord581& c) {
    const std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
    const std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());

    std::cout << "\n================ CLUB " << c.id << " ================\n";
    std::cout << "Short name         : " << sn << "\n";
    std::cout << "Short name gender  : " << static_cast<int>(c.short_name_gender) << "\n";
    std::cout << "Long name          : " << ln << "\n";
    std::cout << "Long name gender   : " << static_cast<int>(c.long_name_gender) << "\n\n";

    std::cout << "Nation ID          : " << c.nation_id << "\n";
    std::cout << "Division ID        : " << c.division_id << "\n";
    std::cout << "Last Division ID   : " << c.last_division_id << "\n";
    std::cout << "Last Position      : " << static_cast<int>(c.last_position) << "\n";
    std::cout << "Reserve DivisionID : " << c.reserve_division_id << "\n\n";

    std::cout << "Professional status: " << pro_status_to_string(c.professional_status)
              << " (" << static_cast<int>(c.professional_status) << ")\n";
    std::cout << "Bank balance       : " << c.bank_balance << "\n";
    std::cout << "Reputation         : " << c.reputation << "\n";
    std::cout << "Training facilities: " << static_cast<int>(c.training_facilities) << "\n";
    std::cout << "PLC                : " << static_cast<int>(c.is_plc) << "\n\n";

    std::cout << "Stadium ID         : " << c.stadium_id << "\n";
    std::cout << "Owns stadium       : " << static_cast<int>(c.owns_stadium) << "\n";
    std::cout << "Reserve stadium ID : " << c.reserve_stadium_id << "\n";
    std::cout << "Match day          : " << static_cast<int>(c.match_day) << "\n";
    std::cout << "Avg attendance     : " << c.avg_attendance << "\n";
    std::cout << "Min attendance     : " << c.min_attendance << "\n";
    std::cout << "Max attendance     : " << c.max_attendance << "\n\n";

    std::cout << "Shirts:\n";
    std::cout << "  Home  fg/bg      : " << c.home_shirt_fg  << " / " << c.home_shirt_bg  << "\n";
    std::cout << "  Away  fg/bg      : " << c.away_shirt_fg  << " / " << c.away_shirt_bg  << "\n";
    std::cout << "  Third fg/bg      : " << c.third_shirt_fg << " / " << c.third_shirt_bg << "\n\n";

    std::cout << "Chairman staff ID  : " << c.chairman_staff_id << "\n";
    std::cout << "Manager staff ID   : " << c.manager_staff_id << "\n";
    std::cout << "Asst manager staff : " << c.assistant_manager_staff_id << "\n\n";

    print_int_array("Liked staff IDs", c.liked_staff, 3);
    print_int_array("Disliked staff IDs", c.disliked_staff, 3);
    print_int_array("Rival club IDs", c.rival_clubs, 3);

    print_int_array("Directors", c.directors, 3);

    std::cout << "\nSquads (counts exclude -1):\n";
    std::cout << "  playing_squad count: " << count_non_minus_one(c.playing_squad.data(), c.playing_squad.size()) << "\n";
    std::cout << "  current_squad count: " << count_non_minus_one(c.current_squad.data(), c.current_squad.size()) << "\n\n";

    print_int_array("Playing squad (staff IDs)", c.playing_squad, 10);
    print_int_array("Current squad (staff IDs)", c.current_squad, 10);

    print_int_array("Coaches", c.coaches, 5);
    print_int_array("Scouts", c.scouts, 7);
    print_int_array("Physios", c.physios, 3);

    std::cout << "\nEurope:\n";
    std::cout << "  euro_flag         : " << c.euro_flag << "\n";
    std::cout << "  euro_seeding      : " << static_cast<int>(c.euro_seeding) << "\n\n";

    print_int_array("Tactics", c.tactics, 4, 4);
    std::cout << "Current tactics     : " << c.current_tactics << "\n";
    std::cout << "Is linked           : " << static_cast<int>(c.is_linked) << "\n";
    std::cout << "============================================\n\n";
}

struct Args {
    fs::path file = "club.dat";
    std::optional<std::string> find;
    std::optional<int32_t> id;
};

static void usage() {
    std::cout <<
        "Usage:\n"
        "  read_club_dat [club.dat] [--find TEXT] [--id N]\n\n"
        "Examples:\n"
        "  read_club_dat club.dat\n"
        "  read_club_dat club.dat --find ajax\n"
        "  read_club_dat club.dat --id 123\n";
}

static Args parse_args(int argc, char** argv) {
    Args a;
    int i = 1;

    // first positional (file) if present and not a flag
    if (i < argc && std::string_view(argv[i]).rfind("--", 0) != 0) {
        a.file = fs::path(argv[i]);
        ++i;
    }

    while (i < argc) {
        std::string key = argv[i++];
        if (key == "--find") {
            if (i >= argc) throw std::runtime_error("--find needs a value");
            a.find = std::string(argv[i++]);
        } else if (key == "--id") {
            if (i >= argc) throw std::runtime_error("--id needs a value");
            a.id = static_cast<int32_t>(std::stol(argv[i++]));
        } else if (key == "--help" || key == "-h") {
            usage();
            std::exit(0);
        } else {
            throw std::runtime_error("Unknown arg: " + key);
        }
    }
    return a;
}

int main(int argc, char** argv) {
    try {
        const Args args = parse_args(argc, argv);
        auto clubs = read_club_dat(args.file);

        std::cout << "Loaded " << clubs.size() << " clubs from: " << args.file << "\n";

        if (args.id.has_value()) {
            auto it = std::ranges::find_if(clubs, [&](const ClubRecord581& c) { return c.id == *args.id; });
            if (it == clubs.end()) {
                std::cerr << "[error] No club found with ID=" << *args.id << "\n";
                return 1;
            }
            print_full(*it);
            return 0;
        }

        if (args.find.has_value()) {
            const std::string needle = *args.find;
            std::vector<const ClubRecord581*> matches;
            matches.reserve(64);

            for (const auto& c : clubs) {
                const std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
                const std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());
                if (icontains(sn, needle) || icontains(ln, needle)) {
                    matches.push_back(&c);
                }
            }

            std::cout << "Matches for \"" << needle << "\": " << matches.size() << "\n\n";

            // print matches as compact rows
            std::cout
                << std::left
                << std::setw(6)  << "ID"
                << std::setw(28) << "ShortName"
                << std::setw(20) << "LongName"
                << std::setw(8)  << "Nation"
                << std::setw(8)  << "Div"
                << std::setw(8)  << "Rep"
                << std::setw(12) << "Bank"
                << std::setw(6)  << "Pro"
                << "\n";
            std::cout << std::string(6+28+20+8+8+8+12+6, '-') << "\n";

            for (const auto* pc : matches) {
                const auto& c = *pc;
                const std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
                const std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());
                std::cout
                    << std::left
                    << std::setw(6)  << c.id
                    << std::setw(28) << (sn.empty() ? "-" : sn.substr(0, 27))
                    << std::setw(20) << (ln.empty() ? "-" : ln.substr(0, 19))
                    << std::setw(8)  << c.nation_id
                    << std::setw(8)  << c.division_id
                    << std::setw(8)  << c.reputation
                    << std::setw(12) << c.bank_balance
                    << std::setw(6)  << pro_status_to_string(c.professional_status)
                    << "\n";
            }

            // If exactly one match, auto-print full detail (nice UX)
            if (matches.size() == 1) {
                std::cout << "\nOnly one match -> printing full record:\n";
                print_full(*matches.front());
            } else {
                std::cout << "\nTip: run with --id <ID> to inspect one club.\n";
            }
            return 0;
        }

        // default
        print_compact_table(clubs, 40);
        std::cout << "\nTip: use --find \"ajax\" or --id 123\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n[error] " << e.what() << "\n\n";
        usage();
        return 1;
    }
}

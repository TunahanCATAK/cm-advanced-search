#pragma once
#include <array> 
#include <cstdint>
#include <iostream> 
#include <ostream>
#include <string_view>

#include "entity.h"

#pragma pack(push, 1)
struct Club : public Entity {
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

template <class Arr>
inline void print_int_array(std::ostream& out,
                     std::string_view label,
                     const Arr& arr,
                     std::size_t per_line,
                     std::size_t max_lines = 5)
{
    out << label << ":\n  ";

    std::size_t printed = 0;
    std::size_t line = 0;

    for (std::size_t i = 0; i < arr.size(); ++i)
    {
        if (arr[i] == -1) continue;

        out << arr[i] << ' ';
        ++printed;

        if (printed % per_line == 0)
        {
            out << "\n  ";
            ++line;
            if (line >= max_lines) break;
        }
    }

    if (printed == 0) out << "(none)";
    out << "\n\n";
}

inline std::ostream& operator<<(std::ostream &out, const Club &c)
{
    const std::string sn = fixed_cstr_to_string(c.short_name.data(), c.short_name.size());
    const std::string ln = fixed_cstr_to_string(c.long_name.data(), c.long_name.size());

    out << "\n================ CLUB " << c.id << " ================\n";
    out << "Short name         : " << sn << "\n";
    out << "Short name gender  : " << static_cast<int>(c.short_name_gender) << "\n";
    out << "Long name          : " << ln << "\n";
    out << "Long name gender   : " << static_cast<int>(c.long_name_gender) << "\n\n";

    out << "Nation ID          : " << c.nation_id << "\n";
    out << "Division ID        : " << c.division_id << "\n";
    out << "Last Division ID   : " << c.last_division_id << "\n";
    out << "Last Position      : " << static_cast<int>(c.last_position) << "\n";
    out << "Reserve DivisionID : " << c.reserve_division_id << "\n\n";

    out << "Professional status: " << static_cast<int>(c.professional_status) << "\n";
    out << "Bank balance       : " << c.bank_balance << "\n";
    out << "Reputation         : " << c.reputation << "\n";
    out << "Training facilities: " << static_cast<int>(c.training_facilities) << "\n";
    out << "PLC                : " << static_cast<int>(c.is_plc) << "\n\n";

    out << "Stadium ID         : " << c.stadium_id << "\n";
    out << "Owns stadium       : " << static_cast<int>(c.owns_stadium) << "\n";
    out << "Reserve stadium ID : " << c.reserve_stadium_id << "\n";
    out << "Match day          : " << static_cast<int>(c.match_day) << "\n";
    out << "Avg attendance     : " << c.avg_attendance << "\n";
    out << "Min attendance     : " << c.min_attendance << "\n";
    out << "Max attendance     : " << c.max_attendance << "\n\n";

    out << "Shirts:\n";
    out << "  Home  fg/bg      : " << c.home_shirt_fg  << " / " << c.home_shirt_bg  << "\n";
    out << "  Away  fg/bg      : " << c.away_shirt_fg  << " / " << c.away_shirt_bg  << "\n";
    out << "  Third fg/bg      : " << c.third_shirt_fg << " / " << c.third_shirt_bg << "\n\n";

    out << "Chairman staff ID  : " << c.chairman_staff_id << "\n";
    out << "Manager staff ID   : " << c.manager_staff_id << "\n";
    out << "Asst manager staff : " << c.assistant_manager_staff_id << "\n\n";

    print_int_array(out, "Liked staff IDs", c.liked_staff, 3);
    print_int_array(out, "Disliked staff IDs", c.disliked_staff, 3);
    print_int_array(out, "Rival club IDs", c.rival_clubs, 3);

    print_int_array(out, "Directors", c.directors, 3);

    out << "\nSquads (counts exclude -1):\n";
    out << "  playing_squad count: "
        << count_non_minus_one(c.playing_squad.data(), c.playing_squad.size()) << "\n";
    out << "  current_squad count: "
        << count_non_minus_one(c.current_squad.data(), c.current_squad.size()) << "\n\n";

    print_int_array(out, "Playing squad (staff IDs)", c.playing_squad, 10);
    print_int_array(out, "Current squad (staff IDs)", c.current_squad, 10);

    print_int_array(out, "Coaches", c.coaches, 5);
    print_int_array(out, "Scouts", c.scouts, 7);
    print_int_array(out, "Physios", c.physios, 3);

    out << "\nEurope:\n";
    out << "  euro_flag         : " << c.euro_flag << "\n";
    out << "  euro_seeding      : " << static_cast<int>(c.euro_seeding) << "\n\n";

    print_int_array(out, "Tactics", c.tactics, 4, 4);
    out << "Current tactics     : " << c.current_tactics << "\n";
    out << "Is linked           : " << static_cast<int>(c.is_linked) << "\n";
    out << "============================================\n\n";

    return out;
}
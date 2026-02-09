#pragma once
#include <cstdint>


#pragma pack(push, 1)
struct CMDate
{
    std::int16_t Day;      // 0x0
    std::int16_t Year;     // 0x2
    std::int32_t LeapYear; // 0x4  (C# int)

    // Total: 8 bytes
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Staff
{
    std::int32_t id;                    // 0..3
    std::int32_t FirstName;             // 4..7
    std::int32_t SecondName;            // 8..11
    std::int32_t CommonName;            // 12..15

    // 0x10
    CMDate DateOfBirth;          // 16..23

    // 0x18
    std::uint16_t YearOfBirth;          // 24..25

    // 0x1A
    std::int32_t Nation;                // 26..29

    // 0x1E
    std::int32_t SecondNation;          // 30..33

    // 0x22
    std::uint8_t IntApps;               // 34

    // 0x23
    std::uint8_t IntGoals;              // 35

    // 0x24
    std::int32_t NationalJob;           // 36..39

    // 0x28
    std::uint8_t JobForNation;

    // 0x29
    CMDate DateJoinedNation;     // 0x29..0x30

    // 0x31
    CMDate DateExpiresNation;    // 0x31..0x38

    // 0x39
    std::int32_t ClubJob;

    // 0x3D
    std::uint8_t JobForClub;

    // 0x3E
    CMDate DateJoinedClub;       // 0x3E..0x45

    // 0x46
    CMDate DateExpiresClub;      // 0x46..0x4D

    // 0x4E
    std::int32_t Wage;

    // 0x52
    std::int32_t Value;

    // 0x56
    std::uint8_t Adaptability;
    // 0x57
    std::uint8_t Ambition;
    // 0x58
    std::uint8_t Determination;
    // 0x59
    std::uint8_t Loyality;        // (typo preserved from C# comment)
    // 0x5A
    std::uint8_t Pressure;
    // 0x5B
    std::uint8_t Professionalism;
    // 0x5C
    std::uint8_t Sportsmanship;
    // 0x5D
    std::uint8_t Temperament;
    // 0x5E
    std::uint8_t PlayingSquad;
    // 0x5F
    std::uint8_t Classification;
    // 0x60
    std::uint8_t ClubValuation;

    // 0x61
    std::int32_t Player;

    // 0x65
    std::int32_t StaffPreferences;

    // 0x69
    std::int32_t NonPlayer;

    // 0x6D
    std::uint8_t SquadSelectedFor;

    // Total: 0x6E (110 bytes)
};

#pragma pack(pop)


inline std::ostream& operator<<(std::ostream& os, const CMDate& d)
{
    // CM date stores "Day = day-of-year - 1" in your C# FromDateTime().
    // We print it as Year + (Day+1) so you can interpret quickly.
    os << "{dayOfYear=" << (static_cast<int>(d.Day) + 1)
       << ", year=" << d.Year
       << ", leap=" << d.LeapYear << "}";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Staff& s)
{
    // ensure uint8_t prints as numbers, not chars
    auto u8 = [](std::uint8_t v) { return static_cast<unsigned>(v); };

    os << "Staff {\n"
       << "  ID=" << s.id << "\n"
       << "  FirstName=" << s.FirstName
       << ", SecondName=" << s.SecondName
       << ", CommonName=" << s.CommonName << "\n"
       << "  DateOfBirth=" << s.DateOfBirth
       << ", YearOfBirth=" << s.YearOfBirth << "\n"
       << "  Nation=" << s.Nation
       << ", SecondNation=" << s.SecondNation << "\n"
       << "  IntApps=" << u8(s.IntApps)
       << ", IntGoals=" << u8(s.IntGoals) << "\n"
       << "  NationalJob=" << s.NationalJob
       << ", JobForNation=" << u8(s.JobForNation) << "\n"
       << "  DateJoinedNation=" << s.DateJoinedNation
       << ", DateExpiresNation=" << s.DateExpiresNation << "\n"
       << "  ClubJob=" << s.ClubJob
       << ", JobForClub=" << u8(s.JobForClub) << "\n"
       << "  DateJoinedClub=" << s.DateJoinedClub
       << ", DateExpiresClub=" << s.DateExpiresClub << "\n"
       << "  Wage=" << s.Wage
       << ", Value=" << s.Value << "\n"
       << "  Personality:"
       << " Adaptability=" << u8(s.Adaptability)
       << " Ambition=" << u8(s.Ambition)
       << " Determination=" << u8(s.Determination)
       << " Loyality=" << u8(s.Loyality)
       << " Pressure=" << u8(s.Pressure)
       << " Professionalism=" << u8(s.Professionalism)
       << " Sportsmanship=" << u8(s.Sportsmanship)
       << " Temperament=" << u8(s.Temperament) << "\n"
       << "  Flags:"
       << " PlayingSquad=" << u8(s.PlayingSquad)
       << " Classification=" << u8(s.Classification)
       << " ClubValuation=" << u8(s.ClubValuation) << "\n"
       << "  Pointers/Refs:"
       << " Player=" << s.Player
       << " StaffPreferences=" << s.StaffPreferences
       << " NonPlayer=" << s.NonPlayer
       << " SquadSelectedFor=" << u8(s.SquadSelectedFor) << "\n"
       << "}";
    return os;
}
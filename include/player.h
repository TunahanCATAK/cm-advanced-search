#include <cstdint>

#pragma pack(push, 1)
struct Player : public Entity
{
    // 0x00
    std::int32_t  id;                  // 0x00..0x03

    // 0x04
    std::uint8_t  SquadNumber;         // 0x04

    // 0x05
    std::int16_t  CurrentAbility;      // 0x05..0x06

    // 0x07
    std::int16_t  PotentialAbility;    // 0x07..0x08

    // 0x09
    std::uint16_t HomeReputation;      // 0x09..0x0A

    // 0x0B
    std::uint16_t CurrentReputation;   // 0x0B..0x0C

    // 0x0D
    std::uint16_t WorldReputation;     // 0x0D..0x0E

    // 0x0F..0x44 (all sbyte => int8_t)
    std::int8_t   Goalkeeper;          // 0x0F
    std::int8_t   Sweeper;             // 0x10
    std::int8_t   Defender;            // 0x11
    std::int8_t   DefensiveMidfielder; // 0x12
    std::int8_t   Midfielder;          // 0x13
    std::int8_t   AttackingMidfielder; // 0x14
    std::int8_t   Attacker;            // 0x15
    std::int8_t   WingBack;            // 0x16
    std::int8_t   RightSide;           // 0x17
    std::int8_t   LeftSide;            // 0x18
    std::int8_t   Central;             // 0x19
    std::int8_t   FreeRole;            // 0x1A
    std::int8_t   Acceleration;        // 0x1B
    std::int8_t   Aggression;          // 0x1C
    std::int8_t   Agility;             // 0x1D
    std::int8_t   Anticipation;        // 0x1E
    std::int8_t   Balance;             // 0x1F
    std::int8_t   Bravery;             // 0x20
    std::int8_t   Consistency;         // 0x21
    std::int8_t   Corners;             // 0x22
    std::int8_t   Crossing;            // 0x23
    std::int8_t   Decisions;           // 0x24
    std::int8_t   Dirtiness;           // 0x25
    std::int8_t   Dribbling;           // 0x26
    std::int8_t   Finishing;           // 0x27
    std::int8_t   Flair;               // 0x28
    std::int8_t   FreeKicks;           // 0x29
    std::int8_t   Handling;            // 0x2A
    std::int8_t   Heading;             // 0x2B
    std::int8_t   ImportantMatches;    // 0x2C
    std::int8_t   InjuryProneness;     // 0x2D
    std::int8_t   Jumping;             // 0x2E
    std::int8_t   Leadership;          // 0x2F
    std::int8_t   LeftFoot;            // 0x30
    std::int8_t   LongShots;           // 0x31
    std::int8_t   Marking;             // 0x32
    std::int8_t   Movement;            // 0x33
    std::int8_t   NaturalFitness;      // 0x34
    std::int8_t   OneOnOnes;           // 0x35
    std::int8_t   PlayerPace;          // 0x36
    std::int8_t   Passing;             // 0x37
    std::int8_t   Penalties;           // 0x38
    std::int8_t   Positioning;         // 0x39
    std::int8_t   Reflexes;            // 0x3A
    std::int8_t   RightFoot;           // 0x3B
    std::int8_t   Stamina;             // 0x3C
    std::int8_t   Strength;            // 0x3D
    std::int8_t   Tackling;            // 0x3E
    std::int8_t   Teamwork;            // 0x3F
    std::int8_t   Technique;           // 0x40
    std::int8_t   ThrowIns;            // 0x41
    std::int8_t   Versatility;         // 0x42
    std::int8_t   Vision;              // 0x43
    std::int8_t   WorkRate;            // 0x44

    // 0x45
    std::uint8_t  PlayerMorale;        // 0x45

    // Total: 0x46 (70 bytes)
};
#pragma pack(pop)
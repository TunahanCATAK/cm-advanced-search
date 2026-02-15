#include <cstdint>

#include "entity.h"

#pragma pack(push, 1)
struct NonPlayer : public Entity
{
    std::int32_t id; 
    std::int16_t CurrentAbility;        // 0x00..0x01
    std::int16_t PotentialAbility;      // 0x02..0x03
    std::int16_t HomeReputation;        // 0x04..0x05  (v0x02 char->short)
    std::int16_t CurrentReputation;     // 0x06..0x07  (v0x02 char->short)
    std::int16_t WorldReputation;       // 0x08..0x09  (v0x02 char->short)

    // Attributes (1 byte each)
    std::uint8_t Attacking;             // 0x0A
    std::uint8_t Business;              // 0x0B
    std::uint8_t Coaching;              // 0x0C
    std::uint8_t CoachingGks;           // 0x0D
    std::uint8_t CoachingTechnique;     // 0x0E
    std::uint8_t Directness;            // 0x0F
    std::uint8_t Discipline;            // 0x10
    std::uint8_t FreeRoles;             // 0x11
    std::uint8_t Interference;          // 0x12
    std::uint8_t Judgement;             // 0x13
    std::uint8_t JudgingPotential;      // 0x14
    std::uint8_t ManHandling;           // 0x15
    std::uint8_t Marking;               // 0x16
    std::uint8_t Motivating;            // 0x17
    std::uint8_t Offside;               // 0x18
    std::uint8_t Patience;              // 0x19
    std::uint8_t Physiotherapy;         // 0x1A
    std::uint8_t Pressing;              // 0x1B
    std::uint8_t Resources;             // 0x1C
    std::uint8_t Tactics;               // 0x1D
    std::uint8_t Youngsters;            // 0x1E

    // Positions / role suitability (4 bytes each)
    std::int32_t Goalkeeper;            // 0x1F..0x22
    std::int32_t Sweeper;               // 0x23..0x26
    std::int32_t Defender;              // 0x27..0x2A
    std::int32_t DefensiveMidfielder;   // 0x2B..0x2E
    std::int32_t Midfielder;            // 0x2F..0x32
    std::int32_t AttackingMidfielder;   // 0x33..0x36
    std::int32_t Attacker;              // 0x37..0x3A
    std::int32_t WingBack;              // 0x3B..0x3E

    std::uint8_t FormationPreferred;    // 0x3F
};
#pragma pack(pop)
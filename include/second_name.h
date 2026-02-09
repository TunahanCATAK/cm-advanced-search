#include <array>
#include <cstdint>

#include "entity.h"

#pragma pack(push, 1)
struct SecondName : public Entity
{
    std::array<std::uint8_t, 51> Name; // 51 bytes
    std::int32_t id;                   // C# int -> 32-bit signed
    std::int32_t Nation;               // C# int -> 32-bit signed
    std::int8_t  Count;                // C# sbyte -> 8-bit signed
};
#pragma pack(pop)

inline std::string name_as_string(const SecondName& n)
{
    const auto* p = reinterpret_cast<const char*>(n.Name.data());
    size_t len = 0;
    while (len < n.Name.size() && p[len] != '\0') ++len;
    return std::string(p, p + len);
}
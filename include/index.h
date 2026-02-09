#pragma once

#pragma pack(push, 1)
struct Index {
    std::array<char, 51> file_name;         // 0..50
    int32_t id;                             // 51..53
    uint32_t table_size;                    // 54..57
    uint32_t offset;                        // 58..61
    uint32_t version;                       // 62..65
    // total 66 bytes
};
#pragma pack(pop)

inline std::ostream& operator<<(std::ostream& os, const Index& idx)
{
    // Safely convert fixed char array to string_view
    const auto end = std::find(idx.file_name.begin(),
                               idx.file_name.end(),
                               '\0');

    std::string_view fileName(idx.file_name.data(),
                              std::distance(idx.file_name.begin(), end));

    os << "Index {\n"
       << "  file_name  : \"" << fileName << "\"\n"
       << "  id         : " << idx.id << "\n"
       << "  table_size : " << idx.table_size << "\n"
       << "  offset     : " << idx.offset << "\n"
       << "  version    : " << idx.version << "\n"
       << "}";

    return os;
}
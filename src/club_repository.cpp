#include <fstream>
#include <iomanip>
#include <iostream> 
#include <algorithm>

#include "club_repository.h"

// index offset and lazy deserialize. 
// normally, in this implementation one time indexing offsets should be done 
// and then the deserialization for only the offset 
// think about thread safety as well
// additionally add a small LRU cache 
ClubRepository::ClubRepository(const std::filesystem::path& tableName): m_tablePath(tableName) 
{
    std::ifstream in(m_tablePath, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + m_tablePath.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0) throw std::runtime_error("Bad file size.");
    if (size % sizeof(Club) != 0) {
        std::cerr << "[warn] File size (" << size << ") is not divisible by 581. "
                  << "Parsing will use floor(size/581) records.\n";
    }

    const size_t count = static_cast<size_t>(size / sizeof(Club));
    m_clubs.resize(count);

    for (size_t i = 0; i < count; ++i) {
        Club rec{};
        in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
        m_clubs[i] = rec;
    }
}

std::optional<Club> ClubRepository::GetById(int id) const 
{
    auto it = std::ranges::find_if(m_clubs, [&](const auto& club){ return id == club.id; });
    if (it == m_clubs.end())
        return std::nullopt;

    return *it;
}

std::optional<Club> ClubRepository::GetByName(const std::string& name) const
{
    auto it = std::ranges::find_if(m_clubs, [&](const auto& club){ return name == fixed_cstr_to_string(club.short_name.data(), club.short_name.size()); });
    if (it == m_clubs.end())
        return std::nullopt;

    return *it;
} 

static std::string to_lower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

std::optional<std::vector<Club>> ClubRepository::SearchByName(const std::string& name) const
{
    std::vector<Club> res; 

    for (const auto& club : m_clubs) 
    {
        auto clubName = to_lower(fixed_cstr_to_string(club.short_name.data(), club.short_name.size()));

        if (clubName.contains(to_lower(name)))
        {
            res.push_back(club);
        }
    }

    if (res.size() == 0) 
        return std::nullopt;

    return res;

}
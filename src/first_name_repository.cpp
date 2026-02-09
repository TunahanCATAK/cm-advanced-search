#include <fstream>
#include <iomanip>
#include <iostream> 
#include <algorithm>

#include "first_name_repository.h"

// index offset and lazy deserialize. 
// normally, in this implementation one time indexing offsets should be done 
// and then the deserialization for only the offset 
// think about thread safety as well
// additionally add a small LRU cache 
FirstNameRepository::FirstNameRepository(const std::filesystem::path& tableName): m_tablePath(tableName) 
{
    std::ifstream in(m_tablePath, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + m_tablePath.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0) throw std::runtime_error("Bad file size.");
    if (size % sizeof(FirstName) != 0) {
        std::cerr << "[warn] File size (" << size << ") is not divisible by 581. "
                  << "Parsing will use floor(size/581) records.\n";
    }

    const size_t count = static_cast<size_t>(size / sizeof(FirstName));
    m_firstNames.resize(count);

    for (size_t i = 0; i < count; ++i) {
        FirstName rec{};
        in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
        m_firstNames[i] = rec;
    }
}

std::optional<std::string> FirstNameRepository::GetById(int id) const 
{
    auto it = std::ranges::find_if(m_firstNames, [&](const auto& firstName){ return id == firstName.id; });
    if (it == m_firstNames.end())
        return std::nullopt;

    return name_as_string(*it);
}
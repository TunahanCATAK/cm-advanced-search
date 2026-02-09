#include <fstream>
#include <iomanip>
#include <iostream> 
#include <algorithm>

#include "index_repository.h"

// index offset and lazy deserialize. 
// normally, in this implementation one time indexing offsets should be done 
// and then the deserialization for only the offset 
// think about thread safety as well
// additionally add a small LRU cache 
IndexRepository::IndexRepository(const std::filesystem::path& tableName): m_tablePath(tableName) 
{
    std::cout << "Index size = " << sizeof(Index) << std::endl;

    std::ifstream in(m_tablePath, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + m_tablePath.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);
    in.seekg(8, std::ios::cur);   // skip header

    if (size < 0) throw std::runtime_error("Bad file size.");
    if (size % sizeof(Index) != 0) {
        std::cerr << "[warn] File size (" << size << ") is not divisible by 66. "
                  << "Parsing will use floor(size/66) records.\n";
    }

    const size_t count = static_cast<size_t>(size / sizeof(Index));
    m_indexes.resize(count);

    for (size_t i = 0; i < count; ++i) {
        Index rec{};
        in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
        m_indexes[i] = rec;
    }
}

std::optional<std::vector<Index>> IndexRepository::GetAll() const
{
    if (m_indexes.size() > 0)
        return m_indexes; 
    
    return std::nullopt;

}
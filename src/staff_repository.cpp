#include <fstream>
#include <iomanip>
#include <iostream> 
#include <algorithm>
#include <iterator>

#include "staff_repository.h"

// index offset and lazy deserialize. 
// normally, in this implementation one time indexing offsets should be done 
// and then the deserialization for only the offset 
// think about thread safety as well
// additionally add a small LRU cache 
StaffRepository::StaffRepository(const std::filesystem::path& tableName): m_tablePath(tableName) 
{
    std::cout << sizeof(Staff) << std::endl;
    std::ifstream in(m_tablePath, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open: " + m_tablePath.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0) throw std::runtime_error("Bad file size.");
    // if (size % sizeof(Staff) != 0) {
    //     std::cerr << "[warn] File size (" << size << ") is not divisible by 581. "
    //               << "Parsing will use floor(size/581) records.\n";
    // }

    //const size_t count = static_cast<size_t>(size / sizeof(Staff));
    const size_t count = 132724; //TODO: calculated manually by hand. 
    m_staffs.resize(count);

    for (size_t i = 0; i < count; ++i) {
        Staff rec = read_staff_le(in);
        if (!in) throw std::runtime_error("Read error at record " + std::to_string(i));
        m_staffs[i] = rec;

        // Staff rec{};
        // in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        // if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
        // m_staffs[i] = rec;
    }

    std::cout << "==[0]===\n";
    std::cout << m_staffs[0] << std::endl;
    std::cout << "======\n";


    std::cout << "==[1]===\n";
    std::cout << m_staffs[1] << std::endl;
    std::cout << "======\n";

    std::cout << "==[2]===\n";
    std::cout << m_staffs[2] << std::endl;
    std::cout << "======\n";

    std::cout << "==[3]===\n";
    std::cout << m_staffs[3] << std::endl;
    std::cout << "======\n";
}

std::optional<Staff> StaffRepository::GetById(int id) const 
{
    auto it = std::ranges::find_if(m_staffs, [&](const auto& staff){ return id == staff.id; });
    if (it == m_staffs.end())
        return std::nullopt;

    std::cout << "Found at index: " << std::distance(m_staffs.begin(), it) << std::endl;
    
    return *it;
}

std::optional<Staff> StaffRepository::GetByName(const std::string& name) const
{

    return std::nullopt;

} 

static std::string to_lower(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

std::optional<std::vector<Staff>> StaffRepository::SearchByName(const std::string& name) const
{

    return std::nullopt;

}
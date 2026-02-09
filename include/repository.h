#include <vector> 
#include <optional>
#include <string> 
#include <filesystem> 
#include <fstream>
#include <iomanip>
#include <iostream> 
#include <algorithm>
#include <concepts> 
#include <type_traits> 

#include "entity.h"

template <typename T> 
concept HasId = requires(T entity){ {entity.id};};

template <typename T> 
requires(std::derived_from<T, Entity> && HasId<T>)
class Repository {

public: 
    explicit Repository(const std::filesystem::path& tableName, size_t offset = 0, size_t max_size = 0) 
    {
        std::ifstream in(tableName, std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open: " + tableName.string());

        std::streamoff size;
        if (max_size != 0)
        {
            size = max_size * sizeof(T);
        }
        else 
        {
            in.seekg(0, std::ios::end);
            size = in.tellg();
            in.seekg(offset, std::ios::beg); // skip header by offset
        }

        if (size < 0) throw std::runtime_error("Bad file size.");
        if (size % sizeof(T) != 0) {
            std::cerr << "[warn] File size (" << size << ") is not divisible by size of the type. "
                    << "Parsing will use floor(size/size of the type) records.\n";
        }

        const size_t count = static_cast<size_t>(size / sizeof(T));
        m_list.resize(count);

        for (size_t i = 0; i < count; ++i) {
            T rec{};
            in.read(reinterpret_cast<char*>(&rec), sizeof(rec));
            if (!in) throw std::runtime_error("Read error while reading record " + std::to_string(i));
            m_list[i] = rec;
        }
    }

    std::optional<T> GetById(int id) const 
    {
        auto it = std::ranges::find_if(m_list, [&](const auto& item){ return id == item.id; });
        if (it == m_list.end())
            return std::nullopt;

        return *it;
    }

    std::optional<std::vector<T>> GetAll() const 
    {
        if (m_list.size() > 0)
            return m_list; 
        
        return std::nullopt;
    }

    // TODO: find a way to implement a find_if kind function instead of searchByName. 
    // With this way, client has the flexibility of searching with a lambda function.
    // std::optional<std::vector<T>> SearchByName(const std::string& name) const
    // {
    //     std::vector<T> res; 

    //     for (const auto& item : m_list) 
    //     {
    //         auto itemName = to_lower(fixed_cstr_to_string(item.short_name.data(), item.short_name.size()));

    //         if (itemName.contains(to_lower(name)))
    //         {
    //             res.push_back(item);
    //         }
    //     }

    //     if (res.size() == 0) 
    //         return std::nullopt;

    //     return res;
    // }


private:  
    std::vector<T> m_list;

    // static std::string to_lower(std::string s) {
    //     for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    //     return s;
    // }

};
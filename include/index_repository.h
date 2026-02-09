#pragma once
#include <vector> 
#include <optional>
#include <string> 
#include <filesystem> 

#include "index.h"

class IndexRepository {

public: 
    explicit IndexRepository(const std::filesystem::path& tableName);

    std::optional<std::vector<Index>> GetAll() const; 

private: 
    std::filesystem::path m_tablePath; 
    std::vector<Index> m_indexes;

};
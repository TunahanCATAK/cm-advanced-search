#pragma once
#include <vector> 
#include <optional>
#include <string> 
#include <filesystem> 

#include "first_name.h"

class FirstNameRepository {

public: 
    explicit FirstNameRepository(const std::filesystem::path& tableName);

    std::optional<std::string> GetById(int id) const; 

private: 
    std::filesystem::path m_tablePath; 
    std::vector<FirstName> m_firstNames;

};
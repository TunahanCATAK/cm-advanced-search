#pragma once
#include <vector> 
#include <optional>
#include <string> 
#include <filesystem> 

#include "club.h"

class ClubRepository {

public: 
    explicit ClubRepository(const std::filesystem::path& tableName);

    std::optional<Club> GetById(int id) const; 
    std::optional<Club> GetByName(const std::string& name) const; 

    std::optional<std::vector<Club>> SearchByName(const std::string& name) const;

private: 
    std::filesystem::path m_tablePath; 
    std::vector<Club> m_clubs;

};
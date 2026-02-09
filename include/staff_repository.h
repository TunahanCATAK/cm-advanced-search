#pragma once
#include <vector> 
#include <optional>
#include <string> 
#include <filesystem> 
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <array>
#include <cstddef>
#include <cstdint>

#include "staff.h"

class StaffRepository {

public: 
    explicit StaffRepository(const std::filesystem::path& tableName);

    std::optional<Staff> GetById(int id) const; 
    std::optional<Staff> GetByName(const std::string& name) const; 

    std::optional<std::vector<Staff>> SearchByName(const std::string& name) const;

private: 
    std::filesystem::path m_tablePath; 
    std::vector<Staff> m_staffs;

    template <typename T>
    static void dump_record_bytes(const std::filesystem::path& path, size_t idx)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open file");

        const std::streamoff offset = static_cast<std::streamoff>(idx) * static_cast<std::streamoff>(110);
        in.seekg(0, std::ios::end);
        const std::streamoff size = in.tellg();
        if (offset + static_cast<std::streamoff>(sizeof(T)) > size)
            throw std::runtime_error("Index out of range for this file size");

        in.seekg(offset, std::ios::beg);

        std::array<std::byte, sizeof(T)> buf{};
        in.read(reinterpret_cast<char*>(buf.data()), buf.size());
        if (!in) throw std::runtime_error("Short read");

        std::cout << "Record idx=" << idx
                << " offset=" << offset
                << " size=" << sizeof(T) << " bytes\n";
        hex_dump(buf.data(), buf.size());
    }

    static void hex_dump(const std::byte* data, size_t n, size_t bytes_per_line = 16)
    {
        for (size_t i = 0; i < n; i += bytes_per_line) {
            std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
            for (size_t j = 0; j < bytes_per_line; ++j) {
                if (i + j < n) {
                    auto b = std::to_integer<unsigned>(data[i + j]);
                    std::cout << std::setw(2) << b << ' ';
                } else {
                    std::cout << "   ";
                }
            }
            std::cout << "\n";
        }
        std::cout << std::dec << std::setfill(' ');
    }

    static std::uint32_t read_u32_le(std::istream& in) {
        std::uint8_t b[4];
        in.read(reinterpret_cast<char*>(b), 4);
        if (!in) throw std::runtime_error("read_u32_le failed");
        return (std::uint32_t)b[0]
            | ((std::uint32_t)b[1] << 8)
            | ((std::uint32_t)b[2] << 16)
            | ((std::uint32_t)b[3] << 24);
    }

    static std::int32_t read_i32_le(std::istream& in) {
        return (std::int32_t)read_u32_le(in);
    }

    static std::uint16_t read_u16_le(std::istream& in) {
        std::uint8_t b[2];
        in.read(reinterpret_cast<char*>(b), 2);
        if (!in) throw std::runtime_error("read_u16_le failed");
        return (std::uint16_t)b[0] | ((std::uint16_t)b[1] << 8);
    }

    static std::int16_t read_i16_le(std::istream& in) {
        return (std::int16_t)read_u16_le(in);
    }

    static std::uint8_t read_u8(std::istream& in) {
        std::uint8_t b;
        in.read(reinterpret_cast<char*>(&b), 1);
        if (!in) throw std::runtime_error("read_u8 failed");
        return b;
    }

    static CMDate read_cmdate_le(std::istream& in) {
        CMDate d{};
        d.Day      = read_i16_le(in);
        d.Year     = read_i16_le(in);
        d.LeapYear = read_i32_le(in);
        return d;
    }

    static Staff read_staff_le(std::istream& in) {
        Staff s{};

        s.id         = read_i32_le(in);
        s.FirstName  = read_i32_le(in);
        s.SecondName = read_i32_le(in);
        s.CommonName = read_i32_le(in);

        s.DateOfBirth = read_cmdate_le(in);

        s.YearOfBirth = read_u16_le(in);

        s.Nation       = read_i32_le(in);
        s.SecondNation = read_i32_le(in);

        s.IntApps  = read_u8(in);
        s.IntGoals = read_u8(in);

        s.NationalJob = read_i32_le(in);

        s.JobForNation = read_u8(in);

        s.DateJoinedNation  = read_cmdate_le(in);
        s.DateExpiresNation = read_cmdate_le(in);

        s.ClubJob    = read_i32_le(in);
        s.JobForClub = read_u8(in);

        s.DateJoinedClub  = read_cmdate_le(in);
        s.DateExpiresClub = read_cmdate_le(in);

        s.Wage  = read_i32_le(in);
        s.Value = read_i32_le(in);

        s.Adaptability     = read_u8(in);
        s.Ambition         = read_u8(in);
        s.Determination    = read_u8(in);
        s.Loyality         = read_u8(in);
        s.Pressure         = read_u8(in);
        s.Professionalism  = read_u8(in);
        s.Sportsmanship    = read_u8(in);
        s.Temperament      = read_u8(in);
        s.PlayingSquad     = read_u8(in);
        s.Classification   = read_u8(in);
        s.ClubValuation    = read_u8(in);

        s.Player          = read_i32_le(in);
        s.StaffPreferences= read_i32_le(in);
        s.NonPlayer       = read_i32_le(in);

        s.SquadSelectedFor = read_u8(in);

        return s;
    }

};
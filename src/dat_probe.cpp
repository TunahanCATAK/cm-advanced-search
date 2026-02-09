#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

static std::vector<std::uint8_t> read_all_bytes(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open file: " + path);
    f.seekg(0, std::ios::end);
    std::size_t size = (std::size_t)f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buf(size);
    if (!f.read(reinterpret_cast<char*>(buf.data()), (std::streamsize)size))
        throw std::runtime_error("failed to read file");
    return buf;
}

static std::uint32_t u32le_at(const std::vector<std::uint8_t>& d, std::size_t p) {
    if (p + 4 > d.size()) return 0;
    std::uint32_t v;
    std::memcpy(&v, d.data() + p, 4);
    return v;
}

static void dump_hex(const std::vector<std::uint8_t>& d, std::size_t from, std::size_t n) {
    std::size_t end = std::min(d.size(), from + n);
    for (std::size_t i = from; i < end; i += 16) {
        std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
        for (std::size_t j = 0; j < 16 && i + j < end; ++j) {
            std::cout << std::setw(2) << (int)d[i + j] << " ";
        }
        std::cout << " ";
        for (std::size_t j = 0; j < 16 && i + j < end; ++j) {
            unsigned char c = d[i + j];
            std::cout << (std::isprint(c) ? (char)c : '.');
        }
        std::cout << "\n";
    }
    std::cout << std::dec;
}

static void find_cstrings(const std::vector<std::uint8_t>& d, std::size_t maxHits = 50) {
    std::size_t hits = 0;
    for (std::size_t i = 0; i < d.size() && hits < maxHits; ) {
        if (d[i] >= 32 && d[i] <= 126) {
            std::size_t start = i;
            while (i < d.size() && d[i] >= 32 && d[i] <= 126) i++;
            if (i < d.size() && d[i] == 0) {
                std::size_t len = i - start;
                if (len >= 5) {
                    std::string s(reinterpret_cast<const char*>(d.data() + start), len);
                    std::cout << "str @" << std::hex << start << std::dec << " len=" << len << " : " << s << "\n";
                    hits++;
                }
            }
        }
        i++;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.dat>\n";
        return 1;
    }

    auto d = read_all_bytes(argv[1]);
    std::cout << "File: " << argv[1] << "\n";
    std::cout << "Size: " << d.size() << " bytes\n\n";

    std::cout << "First 256 bytes:\n";
    dump_hex(d, 0, 256);

    std::cout << "\nFirst 32 u32le values:\n";
    for (int i = 0; i < 32; ++i) {
        std::uint32_t v = u32le_at(d, i * 4);
        std::cout << "u32[" << i << "] = " << v << " (0x" << std::hex << v << std::dec << ")\n";
    }

    // Heuristic: look for (count, recordSize) in first 64 bytes
    std::cout << "\nHeuristic candidates (count * recordSize fits file):\n";
    for (int cPos = 0; cPos < 16; ++cPos) {
        std::uint32_t count = u32le_at(d, cPos * 4);
        if (count == 0 || count > 5'000'000) continue;
        for (int sPos = 0; sPos < 16; ++sPos) {
            std::uint32_t recSize = u32le_at(d, sPos * 4);
            if (recSize < 4 || recSize > 4096) continue;
            std::uint64_t bytes = (std::uint64_t)count * recSize;
            if (bytes > 0 && bytes <= d.size()) {
                std::cout << "count=u32[" << cPos << "]=" << count
                          << " recSize=u32[" << sPos << "]=" << recSize
                          << " count*recSize=" << bytes << "\n";
            }
        }
    }

    std::cout << "\nSome C-strings (printable...\\0):\n";
    find_cstrings(d);

    return 0;
}

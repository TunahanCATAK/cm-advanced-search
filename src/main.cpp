#include <iostream>
#include <ranges> 
#include <algorithm> 

#include "club_repository.h"
#include "staff_repository.h"
#include "non_player.h"
#include "first_name_repository.h"
#include "index_repository.h"
#include "player.h"

#include "repository.h"
#include "second_name.h"

int main() {

    // TODO: Move these repo related codes to the repository lib 
    // make a dbset class to set these db related mechanisms.

    constexpr size_t INDEX_HEADER_OFFSET = 8;

    Repository<Index> ir("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/index2.dat", INDEX_HEADER_OFFSET);
    auto indexList = ir.GetAll();
    if (!indexList.has_value())
        return -1;

    std::cout << "Indexes" << std::endl;
    for (const auto& ind : indexList.value())
    {
        std::cout << ind << std::endl;
    }
    std::cout << "===============\n";

    Repository<Club> clubRepository("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/club.dat");
    auto club244 = clubRepository.GetById(245);
    if (club244.has_value())
    {
        Club cb244 = club244.value();

        std::cout << cb244 << "\n";
    }

    // find the staff index: 
    auto staffInd = std::ranges::find_if(
        indexList.value(),
        [](const auto& ind) {
            std::string_view name(ind.file_name.data());
            return name == "staff.dat";
        }
    );
    Repository<Staff> staffRepository("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/staff.dat", 
                                    staffInd->offset, 
                                    staffInd->table_size);
    
    auto y = staffRepository.GetById(89037);
    if (!y.has_value())
        return -1;
    Staff staff = y.value();

    std::cout << staff << std::endl;

    Repository<FirstName> fnr("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/first_names.dat");
    auto z = fnr.GetById(61);
    std::cout << name_as_string(z.value()) << std::endl; 

    z = fnr.GetById(6997);
    std::cout << name_as_string(z.value()) << std::endl; 

    z = fnr.GetById(32052);
    std::cout << name_as_string(z.value()) << std::endl;

    Repository<SecondName> snr("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/second_names.dat");
    auto t = snr.GetById(37055);
    std::cout << name_as_string(t.value()) << std::endl; 

    // find the player index: 
    auto playerInd = std::ranges::find_if(
        indexList.value(),
        [](const auto& ind) {
            std::string_view name(ind.file_name.data());
            return name == "staff.dat" && ind.version == 10;
        }
    );  
    Repository<Player> playerRepository("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/staff.dat", 
                                    playerInd->offset, 
                                    playerInd->table_size);

    std::cout << "Player has been read!\n";

    // find the non-player index: 
    auto nonPlayerInd = std::ranges::find_if(
        indexList.value(),
        [](const auto& ind) {
            std::string_view name(ind.file_name.data());
            return name == "staff.dat" && ind.version == 9;
        }
    );  
    Repository<NonPlayer> nonPlayerRepository("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/staff.dat", 
                                    nonPlayerInd->offset, 
                                    nonPlayerInd->table_size);

    return 0;
}

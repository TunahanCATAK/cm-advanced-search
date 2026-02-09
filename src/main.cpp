#include <iostream>

#include "club_repository.h"
#include "staff_repository.h"
#include "first_name_repository.h"
#include "index_repository.h"

int main() {

    IndexRepository ir("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/index2.dat");
    auto indexList = ir.GetAll();
    if (!indexList.has_value())
        return -1;

    std::cout << "Indexes" << std::endl;
    for (const auto& ind : indexList.value())
    {
        std::cout << ind << std::endl;
    }
    std::cout << "===============\n";



    ClubRepository cr("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/club.dat"); 

    auto x = cr.GetById(244);
    if (!x.has_value())
        return -1;
    Club club = x.value();

    std::cout << club << std::endl;


    // auto y = cr.SearchByName("real"); 
    // if (!y.has_value())
    //     return -1;
    // std::vector<Club> clubs = y.value();

    // for (const auto& club : clubs) {
    //     std::cout << club << std::endl;
    // }

    StaffRepository sr("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/staff.dat");

    // auto y = sr.GetById(132722);
    auto y = sr.GetById(89037);
    if (!y.has_value())
        return -1;
    Staff staff = y.value();

    std::cout << staff << std::endl;


    FirstNameRepository fnr("/Users/tcatak/Documents/repos/cm-advanced-search/data/v2/first_names.dat");
    auto z = fnr.GetById(61);
    std::cout << z.value() << std::endl; 

    z = fnr.GetById(6997);
    std::cout << z.value() << std::endl; 

    z = fnr.GetById(32052);
    std::cout << z.value() << std::endl;


    return 0;
}

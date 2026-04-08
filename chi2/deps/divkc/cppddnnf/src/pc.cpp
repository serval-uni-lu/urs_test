//
// Created by oz on 2/3/22.
//

#include<iostream>

#include "nnf.hpp"
#include "pdac.hpp"


int main(int argc, char** argv) {
    std::string const cnf_path(argv[1]);
    auto pdac = pdac_from_file(cnf_path);

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    std::cout << pc << "\n";

    return 0;
}

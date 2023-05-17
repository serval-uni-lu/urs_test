#include<iostream>
#include<string>

#include "CNF.hpp"
#include "xoshiro512starstar.h"

int main(int argc, char const** argv) {
    if(argc < 3) {
        std::cerr << "too few arguments\nexiting\n";
        exit(1);
    }

    seed();

    std::string const path(argv[1]);
    CNF cnf(path.c_str());

    std::cout << cnf;

    return 0;
}

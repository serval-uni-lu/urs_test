#include<iostream>
#include<string>

#include<cstring>

#include "CNF.hpp"
#include "xoshiro512starstar.h"

uint64_t parse_int(char const * str) {
    errno = 0;
    int64_t n = strtol(str, NULL, 10);
    if(errno != 0 || n < 0) {
        fprintf(stderr, "error while parsing number \"%s\"\nexiting\n", str);
        exit(EXIT_FAILURE);
    }
    return n;
}

int main(int argc, char const** argv) {
    if(argc < 3) {
        std::cerr << "too few arguments\nexiting\n";
        exit(1);
    }

    seed();

    std::string const path(argv[1]);
    CNF cnf(path.c_str());
    uint64_t nb = parse_int(argv[2]);

    uint32_t type = 0;
    if(strcmp("simple", argv[3]))
        type = 0;
    else if(strcmp("imbalance", argv[3]))
        type = 1;


    for(uint64_t i = 0; i < nb; i++) {
        cnf.single_blast(type == 0);
    }

    std::cout << cnf;

    return 0;
}

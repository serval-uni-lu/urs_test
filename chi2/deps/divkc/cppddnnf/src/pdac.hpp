#ifndef PDAC_HPP
#define PDAC_HPP

#include <set>

struct PDAC {
    std::set<Variable> pvar;
    NNF pnnf;
    NNF unnf;
};


PDAC pdac_from_file(std::string const& path);
void appmc(PDAC const& pdac, std::size_t const N, double const alpha, std::size_t const lN, double const epsilon, bool const verbose);
void ksampler(PDAC const& pdac, std::size_t const N, std::size_t const k);

#endif

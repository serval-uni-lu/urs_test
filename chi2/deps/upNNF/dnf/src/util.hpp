#ifndef UTIL_HPP
#define UTIL_HPP

#include "../dep/cadical-rel-2.1.3/src/cadical.hpp"
#include "cnf.hpp"

CaDiCaL::Solver * cnf_to_cadical(CNF const& cnf);

#endif

#include "util.hpp"

CaDiCaL::Solver * cnf_to_cadical(CNF const& cnf) {
    CaDiCaL::Solver * solver = new CaDiCaL::Solver;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            for(Literal const& l : cnf.clause(i)) {
                solver->add(l.to_int());
            }
            solver->add(0);
        }
    }

    return solver;
}

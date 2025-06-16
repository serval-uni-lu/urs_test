#include <iostream>
#include <string>
#include <cmath>

extern "C" {
    #include "cudd.h"
    #include "dddmp.h"
}

#include "CNF.hpp"

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "usage:\ncnf2bdd <DIMACS>\nexiting\n";
        return 1;
    }

    std::string const cnf_path = argv[1];
    std::string const bdd_path = cnf_path + ".bdd";

    CNF cnf(cnf_path.c_str());

    DdManager *manager = Cudd_Init(cnf.nb_vars(), 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

    DdNode *bdd = Cudd_ReadOne(manager);
    Cudd_Ref(bdd);

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        Clause const& clause = cnf[i];

        DdNode *clause_bdd = Cudd_ReadLogicZero(manager); // false
        Cudd_Ref(clause_bdd);

        for(Literal const& l : clause) {
            int var = Variable(l).to_int() - 1;
            DdNode *literal_bdd = Cudd_bddIthVar(manager, var);
            if(l.to_int() < 0) {
                literal_bdd = Cudd_Not(literal_bdd);
            }
            DdNode *temp = Cudd_bddOr(manager, clause_bdd, literal_bdd);
            Cudd_Ref(temp);
            Cudd_RecursiveDeref(manager, clause_bdd);
            clause_bdd = temp;
        }

        DdNode *temp = Cudd_bddAnd(manager, bdd, clause_bdd);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(manager, bdd);
        Cudd_RecursiveDeref(manager, clause_bdd);
        bdd = temp;
    }

    std::cout << "Satisfiability: "
        << (bdd != Cudd_ReadLogicZero(manager) ? "SAT" : "UNSAT") << "\n";

    //DdNode *nodes[] = {bdd};

    //int res = Dddmp_cuddBddArrayStore(
    //    manager,
    //    DDDMP_MODE_BINARY,          // or DDDMP_MODE_BINARY
    //    DDDMP_VARIDS,
    //    NULL,                     // variable names (optional)
    //    NULL,                     // output names (optional)
    //    nodes,
    //    1,
    //    NULL,
    //    bdd_path.c_str(),
    //    NULL
    //);
    char** pvars = new char*[cnf.nb_vars() + 1];
    for(int i = 0; i < cnf.nb_vars(); i++) {
        int bsize = (int) ceil(log10(i + 1)) + 2;
        pvars[i] = new char[bsize];
        snprintf(pvars[i], bsize, "%d", i + 1);
    }
    pvars[cnf.nb_vars()] = NULL;

    int res = Dddmp_cuddBddStore(
            manager,
            NULL,
            bdd,
            pvars, // char** pvars (last char* in array is NULL)
            NULL, //auxids
            DDDMP_MODE_TEXT,
            DDDMP_VARPERMIDS, //varingo
            (char*)bdd_path.c_str(),
            NULL);

    for(int i = 0; i < cnf.nb_vars(); i++) {
        delete[] pvars[i];
    }

    delete[] pvars;

    Cudd_RecursiveDeref(manager, bdd);
    Cudd_Quit(manager);
    return 0;
}

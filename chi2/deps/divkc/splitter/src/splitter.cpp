#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <stack>
#include <algorithm>

#include <cmath>

#include <boost/program_options.hpp>

#include "patoh.h"
#include "CNF.hpp"

#define VIG
// #define CIG

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("nb", po::value<std::size_t>()->default_value(2), "the target number of partitions");

    return desc;
}

void compute_var_hypergraph(CNF const& cnf, std::vector<int> & xpins, std::vector<int> & pins) {
    for(std::size_t j = 0; j < cnf.nb_clauses(); j++) {
        if(cnf.is_active(j)) {
            auto const& cl = cnf.clause(j);

            if(cl.size() > 1) {
                int pos = pins.size();

                for(auto const& i : cl) {
                    pins.push_back(Variable(i).get());
                }
                xpins.push_back(pos);
            }
        }
    }

    xpins.push_back(pins.size());
}

void compute_clause_hypergraph(CNF const& cnf, std::vector<int> & xpins, std::vector<int> & pins) {
    for(int i = 1; i < static_cast<int>(cnf.nb_vars() + 1); i++) {
        std::set<int> ids;
        for(int id : cnf.get_idx(Literal(i))) {
            // if(cnf.is_active(id)) {
                ids.insert(id);
            // }
        }

        for(int id : cnf.get_idx(Literal(-1 * i))) {
            // if(cnf.is_active(id)) {
                ids.insert(id);
            // }
        }

        if(ids.size() > 1) {
            int pos = pins.size();

            for(auto const& id : ids) {
                pins.push_back(id);
            }

            xpins.push_back(pos);
        }
    }

    xpins.push_back(pins.size());
}

std::vector<int> split(CNF const& cnf, int const nb_part, int & cost) {
    PaToH_Parameters pargs;
    // D4 does QUALITY if nb hyperedges >= 200
    PaToH_Initialize_Parameters(&pargs, PATOH_CUTPART, PATOH_SUGPARAM_QUALITY);
    // PaToH_Initialize_Parameters(&pargs, PATOH_CONPART, PATOH_SUGPARAM_DEFAULT);
    pargs._k = nb_part;

    std::vector<int> xpins;
    std::vector<int> pins;
#ifdef VIG
    compute_var_hypergraph(cnf, xpins, pins);
    const int _c = cnf.nb_vars();
#endif
#ifdef CIG
    compute_clause_hypergraph(cnf, xpins, pins);
    const int _c = cnf.nb_clauses();
#endif
    const int _n = xpins.size() - 1;
    const int _nconst = 1;

    std::vector<int> nwghts(_n, 1);
    std::vector<int> cwghts(_c * _nconst, 1);

    PaToH_Alloc(&pargs, _c, _n, _nconst, cwghts.data(), NULL, xpins.data(), pins.data());

    std::vector<int> partvec(_c, -1);
    std::vector<int> pw(pargs._k * _nconst, -1);

    int cut;
    PaToH_Part(&pargs, _c, _n, _nconst, 0, cwghts.data(), NULL, xpins.data(), pins.data(), NULL, partvec.data(), pw.data(), &cut);
    cost = cut;


    PaToH_Free();
    return partvec;
}

int main(int argc, char *argv[]) {
    try {
        po::options_description desc = get_program_options();

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if(! vm.count("cnf")) {
            std::cerr << "ERROR: cnf option not set\n";
            return 1;
        }

        if(! vm.count("nb")) {
            std::cerr << "ERROR: nb option not set\n";
            return 1;
        }

        std::string const path = vm["cnf"].as<std::string>();
        std::size_t const nb_part = vm["nb"].as<std::size_t>();

        int cost;
        CNF cnf(path.c_str());
        //int const nb_part = ceil(cnf.get_nb_vars() / (double)s_size);
        cnf.simplify();

        auto partvec = split(cnf, nb_part, cost);
        std::vector<int> part_sizes(nb_part, 0);

#ifdef VIG
        for(std::size_t i = 0; i < partvec.size(); i++) {
            // std::cout << "v " << (i + 1) << " " << partvec[i] << "\n";
            part_sizes[partvec[i]] += 1;
        }

        // for(int i = 0; i < part_sizes.size(); i++) {
        //     std::cout << "c p " << i << " " << part_sizes[i] << "\n";
        // }

        int nb_c = 0;
        std::set<Variable> vprj;
        for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
            if(cnf.is_active(i)) {
                auto const& cl = cnf.clause(i);

                std::set<int> tmp;
                for(auto const& l : cl) {
                    tmp.insert(partvec[Variable(l).get()]);
                }

                if(tmp.size() > 1) {
                    nb_c += 1;
                    for(auto const& l : cl) {
                        vprj.insert(Variable(l));
                    }
                }
            }
        }

        std::cout << "c p show";
        for(auto const& v : vprj) {
            std::cout << " " << v;
        }
        std::cout << " 0\n";
#endif

#ifdef CIG
        // std::map<int, std::set<Variable> > svmap;
        std::set<Variable> pvar;

        for(int j = 0; j < cnf.nb_clauses(); j++) {
            if(partvec[j] == 0
        }


        // for(int i = 0; i < nb_part; i++) {
        //     std::set<Variable> sv;

        //     for(int j = 0; j < cnf.nb_clauses(); j++) {
        //         if(partvec[j] == i) {
        //             for(auto const& l : cnf.clause(j)) {
        //                 sv.insert(Variable(l));
        //             }
        //         }
        //     }
        //     svmap[i] = sv;

        //     for(auto const& v : sv) {
        //         std::cout << "v " << v << " " << i << "\n";
        //     }

        //     std::cout << "c p " << i << " " << sv.size() << "\n";
        // }
        // std::cout << "--\n\n";
        // for(auto const& v : svmap[0]) {
        //     if(svmap[1].find(v) != svmap[1].end()) {
        //         std::cout << v << "\n";
        //     }
        // }
#endif
        return 0;
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

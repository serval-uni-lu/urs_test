#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>

#include <boost/program_options.hpp>

#include "cnf.hpp"
#include "nnf.hpp"

namespace po = boost::program_options;

using boost::multiprecision::mpf_float;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file");

    return desc;
}

using Cube = LitSet;
using Cubes = std::vector<Cube>;

Cubes read_cubes_from_file(std::string const& path) {
    Cubes cubes;

    std::ifstream f(path);

    std::string oline;
    while(getline(f, oline)) {
        std::string const line = mtrim(oline);

        Cube cube;
        std::stringstream iss(line);

        while(iss) {
            int v;
            iss >> v;
            if(v != 0) {
                cube.insert(Literal(v));
            }
        }

        cubes.push_back(cube);
    }
    return cubes;
}

std::size_t nb_unknown(CNF const& cnf, Cube const& cube) {
    std::size_t nb = 0;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            bool is_sat = false;
            for(Literal const& l : cnf.clause(i)) {
                is_sat = cube.contains(l);

                if(is_sat) {
                    break;
                }
            }

            if(!is_sat) {
                nb += 1;
            }
        }
    }

    return nb;
}

template<typename T>
void set_intersection(std::unordered_set<T> & a, std::unordered_set<T> const& b) {
    for(auto it = a.begin(); it != a.end(); ) {
        if(b.find(*it) != b.end()) {
            it++;
        }
        else {
            it = a.erase(it);
        }
    }
}

std::vector<std::size_t> compute_ign_indices(CNF const& original, CNF const& up) {
    std::vector<std::size_t> res;

    for(std::size_t o_id = 0; o_id < original.nb_clauses(); o_id++) {
        if(original.is_active(o_id)) {
            Clause const& o_cl = original.clause(o_id);

            std::unordered_set<std::size_t> idx = up.get_idx(o_cl[0]);

            for(Literal const l : o_cl) {
                set_intersection(idx, up.get_idx(l));
            }

            bool found = false;
            for(std::size_t const up_id : idx) {
                if(up.is_active(up_id)) {
                    Clause const& up_cl = up.clause(up_id);

                    found = up_cl == o_cl;

                    if(found) {
                        break;
                    }
                }
            }

            if(!found) {
                res.push_back(o_id);
            }
        }
    }

    return res;
}

int main(int argc, char** argv) {
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

        std::string const cnf_path = vm["cnf"].as<std::string>();

        //CNF cnf_ign(cnf_path + ".ign");
        CNF cnf(cnf_path + ".smp");
        CNF cnf_ign(cnf_path + ".ign");
        std::cout << "loading: " << cnf_path + ".smp   nb clauses: " << cnf.nb_active_clauses() << "\n";
        std::cout << "loading: " << cnf_path + ".ign   nb clauses: " << cnf_ign.nb_active_clauses() << "\n";

        NNF const unnf(cnf_path + ".unnf");
        ANNF aunnf(unnf);
        aunnf.annotate_mc();
        mpz_int const unnf_mc = aunnf.mc(NNF::ROOT);

        auto cubes = read_cubes_from_file(cnf_path + ".cubes");

        std::vector<CNF> formulae;
        std::vector<std::vector<std::size_t> > ign_set;

        std::vector<NNF> unnf_formulae;

        bool is_done = false;
        mpz_int max_val = aunnf.mc(NNF::ROOT);
        for(std::size_t i = 0; !is_done; i++) {
            std::string const l_cnf_path = cnf_path + ".smp_split_" + std::to_string(i) + ".up";
            std::string const l_unnf_path = cnf_path + ".smp_split_" + std::to_string(i) + ".unnf";

            is_done = !std::filesystem::exists(l_cnf_path);

            if(!is_done) {
                std::cout << "loading: " << l_cnf_path;
                formulae.emplace_back(l_cnf_path);
                std::cout << "   nb clauses: ";
                std::cout << formulae[formulae.size() - 1].nb_active_clauses();

                ign_set.push_back(compute_ign_indices(cnf, formulae[formulae.size() - 1]));

                std::cout << "   nb ign: ";
                std::cout << ign_set[ign_set.size() - 1].size();

                unnf_formulae.emplace_back(l_unnf_path);
                ANNF launnf(unnf_formulae[unnf_formulae.size() - 1]);
                launnf.annotate_mc();

                std::cout << "   ratio: ";
                std::cout << (launnf.mc(NNF::ROOT) / aunnf.mc(NNF::ROOT));

                max_val = std::max(launnf.mc(NNF::ROOT), max_val);

                std::cout << "\n";
            }
        }
        assert(formulae.size() == ign_set.size());

        std::cout << "\n------------------------------------------------------------\n\n";

        mpz_int mc_sum = 0;
        mpz_int mc_main_sum = 0;

        std::cout << "sz, main, best, bid, mc_bid, mc_ratio\n";
        #pragma omp parallel for
        for(std::size_t cube_id = 0; cube_id < cubes.size(); cube_id++) {
            Cube const& cube = cubes[cube_id];

            std::size_t res = std::numeric_limits<std::size_t>::max();
            std::size_t id = formulae.size();

            mpz_int lowest_mc = max_val;
            std::size_t id_lowest_mc = formulae.size();

            ANNF launnf(unnf);
            launnf.set_assumps(cube);
            launnf.annotate_mc();
            mpf_float mc_ratio = launnf.mc(NNF::ROOT);

            for(std::size_t i = 0; i < formulae.size(); i++) {
                for(std::size_t j = 0; j < cnf.nb_clauses(); j++) {
                    cnf.set_active(j, false);
                }
                for(std::size_t const j : ign_set[i]) {
                    cnf.set_active(j, true);
                }

                std::size_t tmp = nb_unknown(cnf, cube);

                if(tmp < res || id == formulae.size()) {
                    id = i;
                    res = tmp;
                }

                ANNF plaunnf(unnf_formulae[i]);
                plaunnf.set_assumps(cube);
                plaunnf.annotate_mc();

                if(plaunnf.mc(NNF::ROOT) < lowest_mc || id_lowest_mc == formulae.size()) {
                    lowest_mc = plaunnf.mc(NNF::ROOT);
                    id_lowest_mc = i;
                }
            }
            mc_ratio /= lowest_mc;
            #pragma omp critical
            {
                mc_sum += lowest_mc;
                mc_main_sum += launnf.mc(NNF::ROOT);

                std::cout << cube.size();
                std::cout << ", " << nb_unknown(cnf_ign, cube);
                std::cout << ", " << res << ", " << id << ", " << id_lowest_mc << ", " << mc_ratio << "\n";
            }
        }

        std::cout << "unnf " << unnf_mc << "\n";
        std::cout << "sum " << mc_sum << "\n";
        std::cout << "main sum " << mc_main_sum << "\n";

        mpf_float main = unnf_mc;
        main /= mc_sum;
        std::cout << "sum ratio " << main << "\n";
        main = unnf_mc;
        main /= mc_main_sum;
        std::cout << "main sum ratio " << main << "\n";
        main = mc_sum;
        main /= mc_main_sum;
        std::cout << "main sum sum ratio " << main << "\n";
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>
#include <random>
#include <atomic>

#include <boost/program_options.hpp>

#include "cnf.hpp"
#include "nnf.hpp"
#include "xoshiro.hpp"

namespace po = boost::program_options;

using boost::multiprecision::mpf_float;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("k", po::value<std::size_t>()->default_value(1000), "number of cubes to generate")
        ("cnf", po::value<std::string>(), "path to CNF file");

    return desc;
}

struct Cube {
    LitSet lits;
    mpz_int mc;
};

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
                cube.lits.insert(Literal(v));
            }
        }

        cubes.push_back(cube);
    }
    return cubes;
}

std::size_t nb_unknown(CNF const& cnf, LitSet const& cube) {
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

/*
 * generate a random int in [0, R - 1]
 */
template<typename PRNG>
mpz_int gen_random_int(PRNG & g, mpz_int const& R) {
    std::size_t const needed = msb(R) + 1;
    std::size_t const n_limbs = ((needed + 63) / 64);

    mpz_t result;
    mpz_init2(result, n_limbs * 64);
    std::vector<uint64_t> limbs(n_limbs);
    while(true) {

        for(std::size_t i = 0; i < n_limbs; i++) {
            limbs[i] = g();
        }

        std::size_t const extra_bits = n_limbs * 64 - needed;
        if(extra_bits > 0) {
            limbs[n_limbs - 1] &= (uint64_t(1) << (64 - extra_bits)) - 1;
        }

        mpz_import(result, n_limbs, -1, sizeof(uint64_t), 0, 0, limbs.data());

        if(mpz_cmp(result, R.backend().data()) < 0) {
            mpz_int res(result);
            mpz_clear(result);
            return res;
        }
    }
}

LitSet random_assignment(mpz_int id, CNF const& cnf) {
    LitSet result;
    for(int i = 1; i <= cnf.nb_vars(); i++) {
        Literal li(i);

        if(id & 1) {
            li = ~li;
        }
        id >>= 1;

        result.insert(li);
    }
    return result;
}

LitSet random_assignment(mpz_int const id, ANNF const& aunnf) {
    LitSet result;
    aunnf.get_solution(id, result);
    return result;
}

LitSet random_assignment(mpz_int id, Cubes const& cubes, ANNF & aunnf) {
    LitSet result;

    for(Cube const& cube : cubes) {
        assert(id > 0);

        if(id <= cube.mc) {
            aunnf.set_assumps(cube.lits);
            aunnf.annotate_mc();
            aunnf.get_solution(id, result);
            for(Literal const l : cube.lits) {
                assert(result.contains(l));
            }
            return result;
        }

        id -= cube.mc;
    }
    assert(false && "Impossible state reached");
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
        std::size_t const K = vm["k"].as<std::size_t>();

        //CNF cnf_ign(cnf_path + ".ign");
        CNF cnf(cnf_path + ".smp");
        CNF cnf_ign(cnf_path + ".ign");
        std::cout << "loading: " << cnf_path + ".smp   nb clauses: " << cnf.nb_active_clauses() << "\n";
        std::cout << "loading: " << cnf_path + ".ign   nb clauses: " << cnf_ign.nb_active_clauses() << "\n";

        NNF const unnf(cnf_path + ".unnf");
        ANNF aunnf(unnf);
        aunnf.clear_assumps();
        aunnf.annotate_mc();
        mpz_int const unnf_mc = aunnf.mc(NNF::ROOT);

        mpz_int base_mc = 1;
        base_mc <<= cnf.nb_vars();

        std::random_device dev;
        xoshiro256plusplus prng(dev);

        mpf_float unnf_avg = 0;
        mpf_float base_avg = 0;

        #pragma omp parallel for
        for(std::size_t i = 0; i < K; i++) {
            mpz_int unnf_id;
            mpz_int base_id;

            #pragma omp critical (rand)
            {
                unnf_id = gen_random_int(prng, unnf_mc) + 1;
                base_id = gen_random_int(prng, base_mc);
            }

            LitSet c = random_assignment(unnf_id, aunnf);
            std::size_t unnf_nb = nb_unknown(cnf_ign, c);

            c = random_assignment(base_id, cnf);
            std::size_t base_nb = nb_unknown(cnf, c);

            unnf_avg += cnf_ign.nb_clauses() - unnf_nb;
            base_avg += cnf.nb_clauses() - base_nb;
        }

        unnf_avg /= K;
        base_avg /= K;


        mpf_float unnf_p = unnf_avg / cnf_ign.nb_clauses();
        mpf_float base_p = base_avg / cnf.nb_clauses();

        std::cout << "unnf avg: " << unnf_avg << ", " << unnf_p << "\n";
        std::cout << "base avg: " << base_avg << ", " << base_p << "\n";

        mpf_float unnf_amc = unnf_mc;
        for(int i = 0; i < cnf_ign.nb_clauses(); i++) {
            unnf_amc *= unnf_p;
        }

        mpf_float base_amc = base_mc;
        for(int i = 0; i < cnf.nb_clauses(); i++) {
            base_amc *= base_p;
        }

        std::cout << "unnf amc: " << unnf_amc << "\n";
        std::cout << "base amc: " << base_amc << "\n";
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

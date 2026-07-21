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
        ("cubes", "Use cubes")
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

void compute_hist(std::string const& cnf_path, std::size_t const K) {
    CNF cnf(cnf_path + ".smp");
    CNF cnf_ign(cnf_path + ".ign");
    std::cout << "loading: " << cnf_path + ".smp   nb clauses: " << cnf.nb_active_clauses() << "\n";
    std::cout << "loading: " << cnf_path + ".ign   nb clauses: " << cnf_ign.nb_active_clauses() << "\n";

    NNF const unnf(cnf_path + ".unnf");
    ANNF aunnf(unnf);

    std::cout << "computing mc\n";

    aunnf.clear_assumps();
    aunnf.annotate_mc();
    mpz_int const unnf_mc = aunnf.mc(NNF::ROOT);

    std::cout << "computing histogram\n";

    std::vector<std::size_t> hist(500, 0);

    std::random_device dev;
    xoshiro512plusplus prng(dev);

    std::size_t low = cnf_ign.nb_clauses() + 10;
    std::size_t nb_at_low = 0;

    #pragma omp parallel for
    for(std::size_t i = 0; i < K; i++) {
        mpz_int base_id;

        #pragma omp critical (rand)
        {
            base_id = gen_random_int(prng, unnf_mc) + 1;
        }

        LitSet c = random_assignment(base_id, aunnf);
        std::size_t base_nb = nb_unknown(cnf_ign, c);

        #pragma omp critical (hist)
        {
            if(base_nb < hist.size()) {
                hist[base_nb]++;
            }

            low = std::min(low, base_nb);
            if(low < hist.size()) {
                nb_at_low = hist[low];
            }
        }

    }

    std::cout << "header: hlow,nb_at_low,nb_samples\n";
    std::cout << "csv: " << low << "," << nb_at_low << "," << K << "\n";
    std::cout << "writing histogram\n";

    std::ofstream out_file(cnf_path + ".hist.csv");
    out_file << "n, cube\n";
    for(std::size_t i = 0; i < hist.size(); i++) {
        out_file << i;
        out_file << ", " << hist[i];

        out_file << "\n";
    }
    out_file.close();
}

void compute_cubes_hist(std::string const& cnf_path, std::size_t const K) {
    CNF cnf(cnf_path + ".smp");
    CNF cnf_ign(cnf_path + ".ign");
    std::cout << "loading: " << cnf_path + ".smp   nb clauses: " << cnf.nb_active_clauses() << "\n";
    std::cout << "loading: " << cnf_path + ".ign   nb clauses: " << cnf_ign.nb_active_clauses() << "\n";

    NNF const unnf(cnf_path + ".unnf");
    ANNF aunnf(unnf);

    auto cubes = read_cubes_from_file(cnf_path + ".cubes");
    mpz_int sum_mc = 0;

    std::cout << "nb cubes: " << cubes.size() << "\n";
    std::cout << "computing mc\n";

    #pragma omp parallel for
    for(std::size_t cid = 0; cid < cubes.size(); cid++) {
        ANNF launnf(unnf);
        launnf.set_assumps(cubes[cid].lits);
        launnf.annotate_mc();
        cubes[cid].mc = launnf.mc(NNF::ROOT);

        #pragma omp critical
        {
            sum_mc += cubes[cid].mc;
        }
    }

    //aunnf.clear_assumps();
    //aunnf.annotate_mc();
    //mpz_int const unnf_mc = aunnf.mc(NNF::ROOT);

    std::cout << "computing histogram\n";

    std::vector<std::size_t> hist(500, 0);

    std::random_device dev;
    xoshiro512plusplus prng(dev);

    std::size_t low = cnf_ign.nb_clauses() + 10;
    std::size_t nb_at_low = 0;

    #pragma omp parallel for
    for(std::size_t i = 0; i < K; i++) {
        mpz_int sum_id;

        #pragma omp critical (rand)
        {
            sum_id = gen_random_int(prng, sum_mc) + 1;
        }

        ANNF launnf(unnf);
        LitSet c = random_assignment(sum_id, cubes, launnf);
        std::size_t sum_nb = nb_unknown(cnf_ign, c);

        #pragma omp critical (hist)
        {
            if(sum_nb < hist.size()) {
                hist[sum_nb]++;
            }

            low = std::min(low, sum_nb);
            if(low < hist.size()) {
                nb_at_low = hist[low];
            }
        }

    }

    std::cout << "header: chlow,nb_at_cube_low,nb_cube_samples\n";
    std::cout << "csv: " << low << "," << nb_at_low << "," << K << "\n";
    std::cout << "writing histogram\n";

    std::ofstream out_file(cnf_path + ".hist.cube.csv");
    out_file << "n, cube\n";
    for(std::size_t i = 0; i < hist.size(); i++) {
        out_file << i;
        out_file << ", " << hist[i];

        out_file << "\n";
    }
    out_file.close();
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
        bool const CUBES = vm.count("cubes") != 0;

        if(CUBES) {
            compute_cubes_hist(cnf_path, K);
        }
        else {
            compute_hist(cnf_path, K);
        }
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

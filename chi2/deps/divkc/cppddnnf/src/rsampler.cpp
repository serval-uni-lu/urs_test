#include <iostream>
#include <algorithm>
#include <vector>
#include <set>

#include "nnf.hpp"
#include "pdac.hpp"

#include <boost/random/random_device.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
// #include <boost/random/binomial_distribution.hpp>
#include <boost/range/algorithm.hpp>


#include <boost/multiprecision/gmp.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("k", po::value<std::size_t>()->default_value(1000), "buffer size to use")
        ("nb", po::value<std::size_t>()->default_value(10), "the number of solutions to generate");

    return desc;
}

using namespace boost::random;
using boost::multiprecision::mpf_float;
using boost::multiprecision::mpz_int;

void reservoir_heuristic(PDAC const& pdac, std::size_t const N, std::size_t const k) {
    std::cout << "c heuristic based uniformity\n";

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    random_device rng;
    mt19937 mt(rng);
    uniform_int_distribution<mpz_int> ui(1, pc);

    std::set<mpz_int> visited;
    std::vector<mpz_int> reservoir;
    mpz_int tmc = 0;

    #pragma omp parallel for
    for(std::size_t i = 0; i < k; i++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);
        mpz_int l = -1;
        #pragma omp critical
        {
            l = ui(mt);
            while(visited.find(l) != visited.end()) {
                l = ui(mt);
            }
            visited.insert(l);
        }

        path.clear();
        apnnf.get_path(l, path);
        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);

        #pragma omp critical
        {
            tmc += ai;

            if(reservoir.size() < N) {
                while(reservoir.size() < N) {
                    reservoir.push_back(l);
                }
            }
            else {
                uniform_int_distribution<mpz_int> lui(1, tmc);
                for(std::size_t i = 0; i < reservoir.size(); i++) {
                    if(lui(mt) <= ai) {
                        reservoir[i] = l;
                    }
                }
            }
        }

    }

    boost::random_shuffle(reservoir);

    #pragma omp parallel for
    for(std::size_t i = 0; i < reservoir.size(); i++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);
        auto const l = reservoir[i];

        path.clear();
        apnnf.get_path(l, path);
        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);

        uniform_int_distribution<mpz_int> ui(1, ai);
        mpz_int id = -1;
        #pragma omp critical
        id = ui(mt);

        aunnf.get_solution(id, path);

        #pragma omp critical
        {
            for(auto const& l : path) {
                std::cout << l << " ";
            }
            std::cout << "0\n";
        }
    }
}

void reservoir_exact(PDAC const& pdac, std::size_t const N) {
    std::cout << "c true uniformity\n";

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);
    std::size_t const ipc = static_cast<std::size_t>(pc);

    random_device rng;
    mt19937 mt(rng);

    std::vector<mpz_int> reservoir;
    mpz_int tmc = 0;

    #pragma omp parallel for
    for(std::size_t l = 1; l <= ipc; l++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);

        path.clear();
        apnnf.get_path(l, path);
        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);


        #pragma omp critical
        {
            tmc += ai;

            if(reservoir.size() < N) {
                while(reservoir.size() < N) {
                    reservoir.push_back(l);
                }
            }
            else {
                uniform_int_distribution<mpz_int> lui(1, tmc);
                for(std::size_t i = 0; i < reservoir.size(); i++) {
                    if(lui(mt) <= ai) {
                        reservoir[i] = l;
                    }
                }
            }
        }
    }

    boost::random_shuffle(reservoir);

    #pragma omp parallel for
    for(std::size_t i = 0; i < reservoir.size(); i++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);
        auto const l = reservoir[i];

        path.clear();
        apnnf.get_path(l, path);
        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);

        uniform_int_distribution<mpz_int> ui(1, ai);

        mpz_int id = -1;
        #pragma omp critical
        id = ui(mt);

        aunnf.get_solution(id, path);

        #pragma omp critical
        {
            for(auto const& l : path) {
                std::cout << l << " ";
            }
            std::cout << "0\n";
        }
    }
}

void rsampler(PDAC const& pdac, std::size_t const N, std::size_t const k) {
    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    if(pc > k) {
        reservoir_heuristic(pdac, N, k);
    }
    else {
        reservoir_exact(pdac, N);
    }
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
        std::size_t const N = vm["nb"].as<std::size_t>();
        std::size_t const k = vm["k"].as<std::size_t>();

        auto pdac = pdac_from_file(cnf_path);

        rsampler(pdac, N, k);

        return 0;
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

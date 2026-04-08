//
// Created by oz on 4/12/25.
//

#include <iostream>
#include <random>

#include <cmath>

#include "xoshiro.hpp"

#include "nnf.hpp"
#include "pdac.hpp"

#include <boost/program_options.hpp>
#include <boost/multiprecision/gmp.hpp>

namespace po = boost::program_options;
using boost::multiprecision::mpf_float;
using boost::multiprecision::mpz_int;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("nb", po::value<std::size_t>(), "number of samples");

    return desc;
}

/*
 * generate a random int in [0, R - 1]
 */
mpz_int gen_random_int(xoshiro512plusplus & g, mpz_int const& R) {
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

double compute_p(std::size_t const& N, std::size_t const& k) {
    double res;
    res = k;
    res /= N;
    return res;
}

mpz_int compute_geomeric(xoshiro512plusplus & gen, mpz_int const& N, mpz_int const& k, bool const fast) {
    //floor(ln(U) / ln(1 - p))
    //change to use double because 10^6 is likely small enough
    if(!fast) {
        double const p = compute_p(N.convert_to<std::size_t>(), k.convert_to<std::size_t>());
        //double const U = gen_bits(gen, 256);
        std::uniform_real_distribution<double> dist(0, 1);
        double const U = dist(gen);
        double const lp = std::log(1 - p);

        return mpz_int(log(U) / lp);
    }
    else {
        return gen_random_int(gen, N / k);
    }
}

void reservoir_heuristic(PDAC const& pdac, std::size_t const N) {
    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    mpz_int const pc = apnnf.mc(ROOT);

    std::random_device rdevice;
    xoshiro512plusplus prng(rdevice);

    ANNF aunnf = ANNF(pdac.unnf);

    std::vector<std::set<Literal> > reservoir;
    mpz_int cmc = 0;
    mpz_int c_count = 0;
    mpz_int ipath = 1;
    mpz_int isol = 1;

    std::set<Literal> path;
    apnnf.get_path(ipath, path);
    aunnf.set_assumps(path);
    aunnf.annotate_mc();
    cmc += aunnf.mc(ROOT);
    c_count += 1;

    while(ipath <= pc) {
        if(isol > aunnf.mc(ROOT)) {
            isol -= aunnf.mc(ROOT);

            if(c_count < 1000 || isol < (cmc / c_count)) {
                ipath += 1;
                path.clear();

                if(ipath <= pc) {
                    apnnf.get_path(ipath, path);
                    aunnf.set_assumps(path);
                    aunnf.annotate_mc();
                    cmc += aunnf.mc(ROOT);
                    c_count += 1;
                }
            }
            else {
                mpz_int const nb = isol * c_count / cmc;
                ipath += nb;
                isol -= nb * cmc / c_count;

                if(ipath <= pc) {
                    path.clear();
                    apnnf.get_path(ipath, path);
                    aunnf.set_assumps(path);
                    aunnf.annotate_mc();
                    cmc += aunnf.mc(ROOT);
                    c_count += 1;
                }
            }
            continue;
        }

        if(reservoir.size() < N) {
            std::set<Literal> sol;
            aunnf.get_solution(isol, sol);
            reservoir.push_back(sol);

            isol += 1;
            if(reservoir.size() == N) {
                mpz_int const imc = cmc - aunnf.mc(ROOT) + isol + 1;
                isol += compute_geomeric(prng, imc, reservoir.size(), false);
            }
        }
        else {
            std::set<Literal> sol;
            aunnf.get_solution(isol, sol);

            std::uniform_int_distribution<std::size_t> dist(0, reservoir.size() - 1);
            std::size_t const r = dist(prng);
            reservoir[r] = sol;

            mpz_int const imc = cmc - aunnf.mc(ROOT) + isol + 1;
            isol += 1;
            isol += compute_geomeric(prng, imc, reservoir.size(), false);
        }
    }

    for(auto const& ls : reservoir) {
        for(auto const l : ls) {
            std::cout << l << " ";
        }
        std::cout << "0\n";
    }
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

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

        std::string const cnf_path = vm["cnf"].as<std::string>();
        std::size_t const N = vm["nb"].as<std::size_t>();


        auto pdac = pdac_from_file(cnf_path);
        reservoir_heuristic(pdac, N);
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}


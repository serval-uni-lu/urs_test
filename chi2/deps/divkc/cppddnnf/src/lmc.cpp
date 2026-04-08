//
// Created by oz on 2/3/22.
//

#include<iostream>

#include "var.hpp"
#include "nnf.hpp"
#include "pdac.hpp"

#include <fstream>
#include <cmath>
#include <boost/random/random_device.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <boost/multiprecision/gmp.hpp>

using namespace boost::random;
using boost::multiprecision::mpf_float;

mpf_float lmca(PDAC const& pdac, int const N, int const k, double const alpha) {
    mpf_float res = -1;

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    random_device rng;
    mt19937 mt(rng);
    uniform_int_distribution<mpz_int> ui(1, pc);

    #pragma omp parallel for
    for(int i = 0; i < N; i++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);
        mpf_float mean = 0;

        for(int j = 0; j < k; j++) {
            mpz_int l;
            #pragma omp critical
            l = ui(mt);

            path.clear();
            apnnf.get_path(l, path);
            aunnf.set_assumps(path);
            aunnf.annotate_mc();
            mpf_float ai = (aunnf.mc(ROOT) * pc);
            // ai *= pow(2, alpha);
            mean = mean + ((ai - mean) / (j + 1));
        }
        mean *= pow(2, -1 * alpha);

        #pragma omp critical
        if(res == -1 || mean < res) {
            res = mean;
        }
    }
    return res;
}

// mpf_float lmc(PDAC const& pdac, int const N, double const alpha) {
//     mpf_float res = -1;
// 
//     ANNF apnnf = ANNF(pdac.pnnf);
//     apnnf.annotate_pc();
//     auto const pc = apnnf.mc(ROOT);
// 
//     random_device rng;
//     mt19937 mt(rng);
//     uniform_int_distribution<mpz_int> ui(1, pc);
// 
//     #pragma omp parallel for
//     for(int i = 0; i < N; i++) {
//         std::set<Literal> path;
//         ANNF aunnf = ANNF(pdac.unnf);
//         auto l = ui(mt);
// 
//         path.clear();
//         apnnf.get_path(l, path);
//         aunnf.set_assumps(path);
//         aunnf.annotate_mc();
//         mpf_float ai = (aunnf.mc(ROOT) * pc);
//         ai *= pow(2, -1 * alpha);
// 
//         #pragma omp critical
//         if(res == -1 || ai < res) {
//             res = ai;
//         }
//     }
//     return res;
// }

int main(int argc, char** argv) {
    if(argc != 5) {
        std::cerr << "usage:\n";
        std::cerr << "appmc <cnf file> <number of tries> <number of samples per try> <alpha>\n";
        exit(0);
    }
    std::string const cnf_path(argv[1]);
    int const N = std::stoi(argv[2]);
    int const k = std::stoi(argv[3]);
    int const alpha = std::stod(argv[4]);

    auto pdac = pdac_from_file(cnf_path);
    // auto res = lmc(pdac, N, alpha);
    auto res = lmca(pdac, N, k, alpha);

    std::cout << res << "\n";

    return 0;
}

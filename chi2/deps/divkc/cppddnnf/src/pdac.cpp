#include "var.hpp"
#include "nnf.hpp"
#include "pdac.hpp"

#include <fstream>
#include <atomic>
#include <boost/random/random_device.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <boost/multiprecision/gmp.hpp>

#include <boost/math/distributions/normal.hpp>

using namespace boost::random;
using boost::multiprecision::mpf_float;
using boost::math::normal;

PDAC pdac_from_file(std::string const& path) {
    std::set<Variable> pvar;

    std::ifstream f(path + ".proj.log");
    std::string line;
    while(getline(f, line)) {
        if(line.find("c p show ") == 0) {
            std::stringstream stream(line);
            std::string tag;
            stream >> tag >> tag >> tag;
            int v;

            while((stream >> v)) {
                if(v != 0) {
                    pvar.insert(Variable(v));
                }
            }
        }
    }
    f.close();

    PDAC res;

    res.pnnf.init(path + ".pnnf", true, pvar);
    res.unnf.init(path + ".unnf", false, pvar);
    res.pvar = pvar;

    // res.pnnf.annotate_pc();

    return res;
}

void appmc(PDAC const& pdac, std::size_t const N, double const alpha, std::size_t const lN, double const epsilon, bool const verbose) {
    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    random_device rng;
    mt19937 mt(rng);
    uniform_int_distribution<mpz_int> ui(1, pc);

    mpf_float rmean = 0;
    mpf_float rm = 0;

    std::size_t k = 0;
    mpf_float const z = quantile(normal(), 1 - alpha);

    std::cout << "N,Y,Yl,Yh\n";
    std::atomic<bool> done = false;

    #pragma omp parallel for
    for(std::size_t i = 0; i < N; i++) {
        if(done.load()) {
            continue;
        }

        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);
        auto l = ui(mt);

        path.clear();
        apnnf.get_path(l, path);
        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT) * pc;

        #pragma omp critical
        {
            k++;
            mpf_float n_mean = rmean + ((ai - rmean) / k);
            rm = rm + ((ai - rmean) * (ai - n_mean));
            rmean = n_mean;

            if(k > 1) {
                mpf_float S2 = rm / (k - 1);
                mpf_float sd = sqrt(S2) / sqrt(k);

                auto yl = rmean - z * sd;
                auto yh = rmean + z * sd;

                if(verbose || k >= N) {
                    std::cout << k << ", " << rmean << ", " << yl << ", " << yh << "\n";
                }

                if(lN > 0 && k >= lN && rmean / epsilon <= yl && rmean * epsilon >= yh) {
                    done.store(true);

                    if(!verbose) {
                        std::cout << k << ", " << rmean << ", " << yl << ", " << yh << "\n";
                    }
                }
            }
        }
    }

    // std::cout << "pc " << pc << "\n";
    // std::cout << "s " << rmean << "\n";
    // std::cout << "sl " << (rmean - z * sd) << "\n";
    // std::cout << "sh " << (rmean + z * sd) << "\n";
}

// mpz_int exact_mc(PDAC const& pdac) {
//     ANNF apnnf = ANNF(pdac.pnnf);
//     apnnf.annotate_pc();
//     auto const pc = apnnf.mc(ROOT);
// 
//     std::vector<Literal> path;
//     ANNF aunnf = ANNF(pdac.unnf);
//     mpz_int tmc = 0;
// 
//     for(mpz_int i = 1; i <= pc; i++) {
//         path.clear();
//         apnnf.get_path(i, path);
// 
//         aunnf.set_assumps(path);
//         aunnf.annotate_mc();
//         auto const ai = aunnf.mc(ROOT);
// 
//         tmc += ai;
//     }
// 
//     return tmc;
// }
// 
// void exact_uniform_sampling(PDAC const& pdac, int const N) {
//     std::cout << "c true uniformity\n";
// 
//     ANNF apnnf = ANNF(pdac.pnnf);
//     apnnf.annotate_pc();
//     auto const pc = apnnf.mc(ROOT);
//     auto const tmc = exact_mc(pdac);
// 
//     std::cout << "c pc " << pc << "\n";
//     std::cout << "c tmc " << tmc << "\n";
// 
//     std::vector<Literal> path;
//     ANNF aunnf = ANNF(pdac.unnf);
// 
//     random_device rng;
//     mt19937 mt(rng);
//     uniform_int_distribution<mpz_int> ui(1, tmc);
//     for(int i = 0; i < N; i++) {
//         mpz_int id = ui(mt);
//         bool ch = false;
// 
//         for(mpz_int pid = 1; 1 <= pc; pid++) {
//             path.clear();
//             apnnf.get_path(pid, path);
//             aunnf.set_assumps(path);
//             aunnf.annotate_mc();
//             auto const ai = aunnf.mc(ROOT);
// 
//             if(id <= ai) {
//                 aunnf.get_solution(id, path);
//                 std::set<Literal> sol(path.begin(), path.end());
//                 // std::sort(path.begin(), path.end());
//                 for(auto const& l : sol) {
//                     std::cout << l << " ";
//                 }
//                 std::cout << "0\n";
//                 ch = true;
//                 break;
//             }
//             else {
//                 id -= ai;
//             }
//         }
// 
//         if(!ch) {
//             std::cerr << "c ERROR\n";
//         }
//     }
// }
// 
// void heuristic_uniform_sampling(PDAC const& pdac, int const N, int const k) {
//     std::cout << "c heuristic based uniformity\n";
// 
//     ANNF apnnf = ANNF(pdac.pnnf);
//     apnnf.annotate_pc();
//     auto const pc = apnnf.mc(ROOT);
// 
//     std::vector<Literal> path;
//     ANNF aunnf = ANNF(pdac.unnf);
// 
//     random_device rng;
//     mt19937 mt(rng);
//     uniform_int_distribution<mpz_int> ui(1, pc);
// 
//     for(int i = 0; i < N; i++) {
//         std::set<mpz_int> pids;
//         while(pids.size() < k) {
//             pids.insert(ui(mt));
//         }
// 
//         std::vector<mpz_int> vpids(pids.begin(), pids.end());
//         std::vector<mpz_int> mc_vpids;
//         mpz_int tmc = 0;
//         for(std::size_t j = 0; j < vpids.size(); j++) {
//             path.clear();
//             apnnf.get_path(vpids[j], path);
//             aunnf.set_assumps(path);
//             aunnf.annotate_mc();
//             auto const ai = aunnf.mc(ROOT);
// 
//             mc_vpids.push_back(ai);
//             tmc += ai;
//         }
// 
//         uniform_int_distribution<mpz_int> lui(1, tmc);
//         mpz_int id = lui(mt);
//         bool ch = false;
// 
//         for(std::size_t j = 0; j < vpids.size(); j++) {
//             if(id <= mc_vpids[j]) {
//                 path.clear();
//                 apnnf.get_path(vpids[j], path);
//                 aunnf.set_assumps(path);
//                 aunnf.annotate_mc();
// 
//                 aunnf.get_solution(id, path);
//                 std::set<Literal> sol(path.begin(), path.end());
//                 //std::sort(path.begin(), path.end());
//                 for(auto const& l : sol) {
//                     std::cout << l << " ";
//                 }
//                 std::cout << "0\n";
// 
//                 ch = true;
//                 break;
//             }
//             else {
//                 id -= mc_vpids[j];
//             }
//         }
// 
//         if(!ch) {
//             std::cerr << "c ERROR\n";
//         }
//     }
// }
// 
// void ksampler(PDAC const& pdac, int const N, int const k) {
//     ANNF apnnf = ANNF(pdac.pnnf);
//     apnnf.annotate_pc();
//     auto const pc = apnnf.mc(ROOT);
// 
//     if(pc <= k) {
//         exact_uniform_sampling(pdac, N);
//     }
//     else {
//         heuristic_uniform_sampling(pdac, N, k);
//     }
// }

mpz_int exact_mc(PDAC const& pdac) {
    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    std::set<Literal> path;
    ANNF aunnf = ANNF(pdac.unnf);
    mpz_int tmc = 0;

    for(mpz_int i = 1; i <= pc; i++) {
        path.clear();
        apnnf.get_path(i, path);

        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);

        tmc += ai;
    }

    return tmc;
}

void exact_uniform_sampling(PDAC const& pdac, std::size_t const N) {
    std::cout << "c true uniformity\n";

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);
    std::size_t const ipc = static_cast<std::size_t>(pc);

    // auto const tmc = exact_mc(pdac);
    mpz_int tmc = 0;
    std::vector<mpz_int> pmc(ipc, 0);

    #pragma omp parallel for
    for(std::size_t i = 1; i <= ipc; i++) {
        ANNF aunnf = ANNF(pdac.unnf);
        std::set<Literal> path;
        apnnf.get_path(i, path);

        aunnf.set_assumps(path);
        aunnf.annotate_mc();
        auto const ai = aunnf.mc(ROOT);

        #pragma omp critical
        {
            tmc += ai;
        }
        pmc[i - 1] = ai;
    }


    random_device rng;
    mt19937 mt(rng);
    uniform_int_distribution<mpz_int> ui(1, tmc);

    #pragma omp parallel for
    for(std::size_t i = 0; i < N; i++) {
        mpz_int id = 0;
        #pragma omp critical
        id = ui(mt);

        bool ch = false;

        std::set<Literal> lpath;
        ANNF launnf = ANNF(pdac.unnf);

        for(int pid = 1; 1 <= ipc; pid++) {
            if(id <= pmc[pid - 1]) {
                lpath.clear();
                apnnf.get_path(pid, lpath);
                launnf.set_assumps(lpath);
                launnf.annotate_mc();
                launnf.get_solution(id, lpath);
                // std::sort(lpath.begin(), lpath.end());
                #pragma omp critical
                {
                    for(auto const& l : lpath) {
                        std::cout << l << " ";
                    }
                    std::cout << "0\n";
                }
                ch = true;
                break;
            }
            else {
                id -= pmc[pid - 1];
            }

        }

        if(!ch) {
            #pragma omp critical
            std::cerr << "c ERROR\n";
        }
    }
}

void heuristic_uniform_sampling(PDAC const& pdac, std::size_t const N, std::size_t const k) {
    std::cout << "c heuristic based uniformity\n";

    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    random_device rng;

    #pragma omp parallel for
    for(std::size_t i = 0; i < N; i++) {
        std::set<Literal> path;
        ANNF aunnf = ANNF(pdac.unnf);

        mt19937 mt;
        #pragma omp critical
        mt.seed(rng);

        uniform_int_distribution<mpz_int> ui(1, pc);

        std::set<mpz_int> pids;
        while(pids.size() < k) {
            pids.insert(ui(mt));
        }

        std::vector<mpz_int> vpids(pids.begin(), pids.end());
        std::vector<mpz_int> mc_vpids;
        mpz_int tmc = 0;
        for(std::size_t j = 0; j < vpids.size(); j++) {
            path.clear();
            apnnf.get_path(vpids[j], path);
            aunnf.set_assumps(path);
            aunnf.annotate_mc();
            auto const& ai = aunnf.mc(ROOT);

            mc_vpids.push_back(ai);
            tmc += ai;
        }

        uniform_int_distribution<mpz_int> lui(1, tmc);
        mpz_int id = lui(mt);
        bool ch = false;

        for(std::size_t j = 0; j < vpids.size(); j++) {
            if(id <= mc_vpids[j]) {
                path.clear();
                apnnf.get_path(vpids[j], path);
                aunnf.set_assumps(path);
                aunnf.annotate_mc();

                aunnf.get_solution(id, path);
                // std::sort(path.begin(), path.end());
                #pragma omp critical
                {
                    for(auto const& l : path) {
                        std::cout << l << " ";
                    }
                    std::cout << "0\n";
                }

                ch = true;
                break;
            }
            else {
                id -= mc_vpids[j];
            }
        }

        if(!ch) {
            #pragma omp critical
            std::cerr << "c ERROR\n";
        }
    }
}

void ksampler(PDAC const& pdac, std::size_t const N, std::size_t const k) {
    ANNF apnnf = ANNF(pdac.pnnf);
    apnnf.annotate_pc();
    auto const pc = apnnf.mc(ROOT);

    if(pc <= k) {
        exact_uniform_sampling(pdac, N);
    }
    else {
        heuristic_uniform_sampling(pdac, N, k);
    }
}

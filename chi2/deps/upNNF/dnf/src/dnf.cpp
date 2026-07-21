#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <chrono>

#include <boost/program_options.hpp>

#include "cnf.hpp"
#include "compiler.hpp"
#include "heuristics.hpp"

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("p", po::value<double>()->default_value(0.007), "coefficient before resetting the queue in the Merger object")
        ("mintries", po::value<std::size_t>()->default_value(100), "minimum number of tries for to trigger a reset of the queue in the Merger object")
        ("timeout", po::value<std::size_t>()->default_value(3600), "maximum amount of time allowed to generate cubes in seconds")
        ("k", po::value<std::size_t>()->default_value(16), "number of upper bounds to generate")
        ("n", po::value<std::size_t>()->default_value(10000), "maximum number of cubes to generate");

    return desc;
}

std::size_t overlap_size(BitSet<std::size_t> a, BitSet<std::size_t> b ) {
    a.intersect(b);
    return a.size();
}

template<typename T>
void intersection(std::unordered_set<T> & a, std::unordered_set<T> const& b) {
    for(auto it = a.begin(); it != a.end(); ) {
        if(b.find(*it) != b.end()) {
            it++;
        }
        else {
            it = a.erase(it);
        }
    }
}

class Merger {
public:
    struct Tuple {
        std::size_t id1;
        std::size_t id2;
        std::size_t overlap;
    };

    std::size_t next_id;
    double const queue_reset_coefficient;
    std::size_t const queue_reset_minimum_tries;
    std::unordered_map<std::size_t, BitSet<std::size_t> > known;
    std::unordered_map<std::size_t, std::size_t> number_of_merges;
    std::priority_queue<Tuple> queue;

    Merger(std::vector<BitSet<std::size_t> > const& iknown, double const p, std::size_t const mt) : queue_reset_coefficient(p), queue_reset_minimum_tries(mt) {
        for(std::size_t id = 0; id < iknown.size(); id++) {
            known[id] = iknown[id];
            number_of_merges[id] = 1;
        }

        populate_queue();

        next_id = iknown.size();
    }

    void populate_queue() {
        queue = std::priority_queue<Tuple>();

        std::vector<std::size_t> keys;
        keys.reserve(known.size());
        for (const auto& pair : known) {
            keys.push_back(pair.first);
        }

        #pragma omp parallel for
        for(std::size_t i = 0; i < keys.size() - 1; i++) {
            auto const key_i = keys[i];

            for(std::size_t j = i + 1; j < keys.size(); j++) {
                auto const key_j = keys[j];

                auto const overlap = overlap_size(known[key_i], known[key_j]);

                #pragma omp critical (queue)
                {
                    queue.emplace(key_i, key_j, overlap);
                }
            }
        }
    }

    void reset_queue(std::size_t const nb_tries) {
        if(nb_tries > queue_reset_minimum_tries && nb_tries > queue_reset_coefficient * known.size() * known.size()) {
            std::cerr << "queue reset>\n";
            populate_queue();
        }
    }

    void merge_best() {
        std::size_t nb_tries = 0;

        while(queue.size() > 0) {
            nb_tries += 1;

            Tuple const tuple = queue.top();
            queue.pop();

            auto const it1 = known.find(tuple.id1);
            auto const it2 = known.find(tuple.id2);

            if(it1 != known.end() && it2 != known.end()) {
                std::size_t const new_id = next_id;
                next_id++;

                auto res = it1->second;
                res.intersect(it2->second);

                //std::cerr << "merging " << it1->first << " with " << it2->first << " -> " << next_id << " : " << tuple.overlap << "\n";
                number_of_merges[new_id] = number_of_merges[it1->first] + number_of_merges[it2->first];
                number_of_merges.erase(it1->first);
                number_of_merges.erase(it2->first);
                known.erase(it1);
                known.erase(it2);

                std::vector<std::size_t> keys;
                keys.reserve(known.size());
                for (const auto& pair : known) {
                    keys.push_back(pair.first);
                }

                #pragma omp parallel for
                for(std::size_t i = 0; i < keys.size(); i++) {
                    auto const overlap = overlap_size(known[keys[i]], res);

                    #pragma omp critical (queue)
                    {
                        queue.emplace(new_id, keys[i], overlap);
                    }
                }

                known[new_id] = res;
                //std::cerr << "#try> " << nb_tries << " | " << (known.size() * known.size()) << "\n";
                reset_queue(nb_tries);
                return;
            }
        }
        assert(false && "Unexpected state reached");
    }
};

bool operator<(Merger::Tuple const& a, Merger::Tuple const& b) {
    return a.overlap < b.overlap;
}

template<typename H>
std::vector<LitSet > compile_main(H heuristic, CNF const& cnf_ign, CNF const& cnf_smp, std::size_t const N, std::size_t const timeout) {
    Compiler compiler(cnf_ign, cnf_smp, heuristic);
    compiler.init();
    compiler.compile(N, timeout);

    Cubes const& cube_manager = compiler.cube_manager();

    std::cout << "Generated " << cube_manager.nb_cubes() << " cubes\n\n";

    std::vector<LitSet > cubes;

    for(Cubes::Cube const& c : cube_manager) {
        cubes.push_back(c.mlits);
    }

    return cubes;
}

BitSet<std::size_t> compute_known(CNF const& cnf, LitSet const& cube) {
    BitSet<std::size_t> res;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            bool is_sat = false;
            for(Literal const l : cnf.clause(i)) {
                if(cube.contains(l)) {
                    is_sat = true;
                    break;
                }
            }

            if(is_sat) {
                res.insert(i);
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
        std::size_t const N = vm["n"].as<std::size_t>();
        std::size_t const K = vm["k"].as<std::size_t>();
        std::size_t const timeout = vm["timeout"].as<std::size_t>();
        double const P = vm["p"].as<double>();
        std::size_t const mintries = vm["mintries"].as<std::size_t>();

        std::cout << "Loading\n";

        CNF cnf_ign(cnf_path + ".ign");
        CNF cnf_smp(cnf_path + ".smp");
        //NNF nnf(cnf_path + ".unnf");

        //MOMSVarBase heuristic;
        //MOMSLitBase heuristic;
        //MOMSVarPicker heuristic;
        //MOMSLitPicker heuristic;
        LitCount heuristic;
        //LitCountSMP heuristic;
        //LitWeight heuristic;
        //LitWeightSMP heuristic;
        //VarCount heuristic;
        //VarWeight heuristic;
        //VarWeightSMP heuristic;
        //VarCountSMP heuristic;
        //NNFLookahead heuristic(nnf);

        auto const start = std::chrono::high_resolution_clock::now();

        std::cout << "Compiling\n";
        auto cubes = compile_main(heuristic, cnf_ign, cnf_smp, N, timeout);

        auto const compile_end = std::chrono::high_resolution_clock::now();

        std::vector<BitSet<std::size_t> > known(cubes.size());

        std::cout << "Computing known clauses\n";

        #pragma omp parallel for
        for(std::size_t i = 0; i < cubes.size(); i++) {
            known[i] = compute_known(cnf_ign, cubes[i]);
        }

        auto const known_compute_end = std::chrono::high_resolution_clock::now();

        for(std::size_t i = 0; i < cubes.size(); i++) {
            std::cout << "csv " << i << ", " << known[i].size() << "\n";
        }

        // std::cout << "Initializing merger\n";
        // Merger merger(known, P, mintries);

        // auto const merger_init_end = std::chrono::high_resolution_clock::now();

        // std::cout << "Merging\n";
        // while(merger.known.size() > K) {
        //     if(merger.known.size() % 100 == 0) {
        //         std::cerr << "s> " << merger.known.size() << "\n";
        //     }
        //     merger.merge_best();
        // }

        // auto const merging_end = std::chrono::high_resolution_clock::now();

        // std::size_t split_id = 0;
        // for(auto const& e : merger.known) {
        //     for(std::size_t i = 0; i < cnf_smp.nb_clauses(); i++) {
        //         cnf_smp.set_active(i, true);
        //     }

        //     for(std::size_t const cid : e.second) {
        //         cnf_smp.set_active(cid, false);
        //     }

        //     std::ofstream out_file(cnf_path + ".smp_split_" + std::to_string(split_id));
        //     out_file << cnf_smp;
        //     out_file.close();
        //     split_id++;
        // }

        std::ofstream out_file(cnf_path + ".cubes");
        for(std::size_t cube_id = 0; cube_id < cubes.size(); cube_id++) {
            for(int i = 1; i <= cnf_ign.nb_vars(); i++) {
                Literal lp(i);
                Literal ln = ~lp;

                if(cubes[cube_id].contains(lp)) {
                    out_file << lp << " ";
                }
                if(cubes[cube_id].contains(ln)) {
                    out_file << ln << " ";
                }
            }
            //for(Literal const l : cubes[cube_id]) {
            //    out_file << l << " ";
            //}
            out_file << "0\n";
        }
        out_file.close();

        std::chrono::duration<double, std::milli> const compile_duration = compile_end - start;
        std::chrono::duration<double, std::milli> const known_compute_duration = known_compute_end - compile_end;
        //std::chrono::duration<double, std::milli> const merger_init_duration = merger_init_end - known_compute_end;
        //std::chrono::duration<double, std::milli> const merging_duration = merging_end - merger_init_end;

        std::cout << "t Compile: " << compile_duration.count() << " ms\n";
        std::cout << "t known compute: " << known_compute_duration.count() << " ms\n";
        //std::cout << "t merger init: " << merger_init_duration.count() << " ms\n";
        //std::cout << "t merging: " << merging_duration.count() << " ms\n";
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

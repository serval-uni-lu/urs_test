#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>
#include <random>
#include <atomic>
#include <algorithm>

#include <boost/program_options.hpp>

#include "cnf.hpp"
#include "nnf.hpp"
#include "xoshiro.hpp"

#include "assignment_generator.hpp"

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("n", po::value<std::size_t>()->default_value(1000), "number of samples to generate")
        ("cubes", "Use cubes")
        ("dont-print-samples", "Don't print the samples")
        ("cnf", po::value<std::string>(), "path to CNF file");

    return desc;
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
        bool const CUBES = vm.count("cubes") != 0;
        bool const PRINT = vm.count("dont-print-samples") == 0;

        std::cerr << "Initializing\n";

        CNF cnf(cnf_path);
        NNF const unnf(cnf_path + ".unnf");

        std::random_device dev;
        xoshiro512plusplus prng(dev);

        if(CUBES) {
            auto cubes = dnf::read_cubes_from_file(cnf_path + ".cubes");
            sampler::RandomCubeAssignmentGenerator rag(cnf, unnf, cubes);

            std::cerr << "cubes\n";
            std::cout << "MC " << rag.get_mc() << "\n";

            std::cerr << "Sampling\n";
            for(std::size_t i = 0; i < N; i++) {
                sampler::Cube const c = rag(prng);

                if(PRINT) {
                    std::cout << "s " << c.m << "\n";
                }
            }
        }
        else {
            ANNF aunnf(unnf);
            aunnf.annotate_mc();
            sampler::RandomNNFAssignmentGenerator rag(cnf, aunnf);

            std::cerr << "no cubes\n";
            std::cout << "MC " << rag.get_mc() << "\n";

            std::cerr << "Sampling\n";
            for(std::size_t i = 0; i < N; i++) {
                sampler::Cube const c = rag(prng);

                if(PRINT) {
                    std::cout << "s " << c.m << "\n";
                }
            }
        }
    }
    catch(std::exception & e) {
        std::cerr << "EXCEPTION ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

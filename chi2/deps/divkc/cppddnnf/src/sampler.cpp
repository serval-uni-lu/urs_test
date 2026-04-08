#include<iostream>

#include "nnf.hpp"
#include "pdac.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("k", po::value<std::size_t>()->default_value(50), "buffer size to use")
        ("nb", po::value<std::size_t>()->default_value(10), "the number of solutions to generate");

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
        std::size_t const N = vm["nb"].as<std::size_t>();
        std::size_t const k = vm["k"].as<std::size_t>();


        auto pdac = pdac_from_file(cnf_path);

        ksampler(pdac, N, k);

        return 0;
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

//
// Created by oz on 2/3/22.
//

#include<iostream>

#include "nnf.hpp"
#include "pdac.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("verbose", "Display all of the intermediate estimates as well")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("alpha", po::value<double>()->default_value(0.01), "alpha value for the CLT")
        ("nb", po::value<std::size_t>()->default_value(10'000), "the maximum number of samples to use")
        ("lnb", po::value<std::size_t>()->default_value(0), "the minimum number of samples to use, set to 0 to disable early stopping (see --epsilon)")
        ("epsilon", po::value<double>()->default_value(1.1), "the algorithm stops early (before --nb samples have been reached) if (Y / epsilon) <= Yl and (Y * epsilon) >= Yh");

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
        double const alpha = vm["alpha"].as<double>();
        std::size_t const N = vm["nb"].as<std::size_t>();

        std::size_t const lN = vm["lnb"].as<std::size_t>();
        double const epsilon = vm["epsilon"].as<double>();

        bool const verbose = vm.count("verbose") > 0;

        auto pdac = pdac_from_file(cnf_path);

        appmc(pdac, N, alpha, lN, epsilon, verbose);

        return 0;
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

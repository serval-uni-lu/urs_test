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

using boost::multiprecision::mpf_float;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("n", po::value<std::size_t>()->default_value(1000), "number of samples to generate")
        ("cubes", "Use cubes")
        ("print-samples", "Print the samples")

        ("no-nnf", "Doesn't use the .unnf file, instead generates random assignments without considering any constraints")

        ("splitting", "Use the splitting method")
        ("splitting-population", po::value<std::size_t>()->default_value(1000), "population size for the splitting method")
        ("splitting-retention", po::value<double>()->default_value(0.3), "the proportion of the population to keep at each step for the splitting method")
        ("splitting-sweeps", po::value<std::size_t>()->default_value(5), "the number of sweeps to do at each step for the splitting method")
        ("splitting-block-size", po::value<std::size_t>()->default_value(1), "the number of literals to flip at a time during the gibbs move for the splitting method")
        ("splitting-maximum-unsat-clauses", po::value<std::size_t>()->default_value(0), "The maximum number of unsat clauses for the algorithm to add the cube to the initial population. Small values will make initialization slower and 0 deactivates this option as it would otherwise collapse to regular monte carlo")

        ("rwalk", "Use the random walk method")

        ("rwalk-descent-pure-random-walk-p", po::value<double>()->default_value(0.75), "Descent phase: Set the probability of doing a random walk move with no MH correction. (Either this step gets executed or an MH step).")
        ("rwalk-descent-random-walk-p", po::value<double>()->default_value(0.75), "Descent phase: Set the probability of doing a random walk subject to the MH correction.")
        ("rwalk-descent-beta", po::value<double>()->default_value(1.0), "Descent phase: Set the value for beta in the MH algorithm.")
        ("rwalk-descent-max-nb-steps", po::value<double>()->default_value(2.0), "Descent phase: Set the maximum number of steps, if exceeded, a restart is triggered. #steps = #variables * parameter")

        ("rwalk-burnin-pure-random-walk-p", po::value<double>()->default_value(0.85), "Burn-in phase: Set the probability of doing a random walk move with no MH correction. (Either this step gets executed or an MH step).")
        ("rwalk-burnin-random-walk-p", po::value<double>()->default_value(0.75), "Burn-in phase: Set the probability of doing a random walk subject to the MH correction.")
        ("rwalk-burnin-beta", po::value<double>()->default_value(2), "Burn-in phase: Set the value for beta in the MH algorithm.")
        ("rwalk-burnin-nb-steps", po::value<double>()->default_value(2.0), "Burn-in phase: Set the maximum number of steps, if exceeded, a restart is triggered. #steps = #variables * parameter")

        ("rwalk-pure-random-walk-p", po::value<double>()->default_value(0.95), "Sampling phase: Set the probability of doing a random walk move with no MH correction. (Either this step gets executed or an MH step).")
        ("rwalk-random-walk-p", po::value<double>()->default_value(0.75), "Sampling phase: Set the probability of doing a random walk subject to the MH correction.")
        ("rwalk-beta", po::value<double>()->default_value(2), "Sampling phase: Set the value for beta in the MH algorithm.")
        ("rwalk-restart-p", po::value<double>()->default_value(0.01), "Sampling phase: Set the probability of restarting from scratch.")

        ("cnf", po::value<std::string>(), "path to CNF file");

    return desc;
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

template<typename RAG>
int setup_runner(po::variables_map const& vm, RAG & rag) {
    try {
        std::size_t const N = vm["n"].as<std::size_t>();
        bool const PRINT = vm.count("print-samples") != 0;
        bool const SPLITTING = vm.count("splitting") != 0;
        bool const RANDOM_WALK = vm.count("rwalk") != 0;

        std::size_t const splitting_population = vm["splitting-population"].as<std::size_t>();
        double const splitting_retention = vm["splitting-retention"].as<double>();
        std::size_t const splitting_sweeps = vm["splitting-sweeps"].as<std::size_t>();
        std::size_t const splitting_block_size = vm["splitting-block-size"].as<std::size_t>();
        std::size_t const splitting_maximum_unsat_clauses = vm["splitting-maximum-unsat-clauses"].as<std::size_t>();

        double const rwalk_pure_random_walk_p = vm["rwalk-pure-random-walk-p"].as<double>();
        double const rwalk_random_walk_p = vm["rwalk-random-walk-p"].as<double>();
        double const rwalk_beta = vm["rwalk-beta"].as<double>();
        double const rwalk_restart_p = vm["rwalk-restart-p"].as<double>();

        double const rwalk_descent_pure_random_walk_p = vm["rwalk-descent-pure-random-walk-p"].as<double>();
        double const rwalk_descent_random_walk_p = vm["rwalk-descent-random-walk-p"].as<double>();
        double const rwalk_descent_beta = vm["rwalk-descent-beta"].as<double>();
        double const rwalk_descent_max_nb_steps = vm["rwalk-descent-max-nb-steps"].as<double>();

        double const rwalk_burnin_pure_random_walk_p = vm["rwalk-burnin-pure-random-walk-p"].as<double>();
        double const rwalk_burnin_random_walk_p = vm["rwalk-burnin-random-walk-p"].as<double>();
        double const rwalk_burnin_beta = vm["rwalk-burnin-beta"].as<double>();
        double const rwalk_burnin_nb_steps = vm["rwalk-burnin-nb-steps"].as<double>();

        std::random_device dev;
        xoshiro512plusplus prng(dev);

        CNF const& cnf = rag.get_cnf();

        if(SPLITTING) {
            sampler::SplittingSampler runner(rag, PRINT);
            runner.set_population_size(splitting_population);
            runner.set_retention(splitting_retention);
            runner.set_nb_sweeps(splitting_sweeps);
            runner.set_nb_lits(splitting_block_size);
            runner.set_maximum_unsat_clauses(splitting_maximum_unsat_clauses);
            runner.run(N, prng);
        }
        else if(RANDOM_WALK) {
            sampler::MetropolisHastings runner(rag, PRINT);

            runner.set_descent_pure_random_walk_probability(rwalk_descent_pure_random_walk_p);
            runner.set_descent_random_walk_probability(rwalk_descent_random_walk_p);
            runner.set_descent_beta(rwalk_descent_beta);

            std::size_t const max_nb_descent_steps = (double)cnf.nb_vars() * rwalk_descent_max_nb_steps;
            runner.set_descent_max_nb_steps(max_nb_descent_steps);
            std::cout << "c rwalk-descent-max-nb-steps " << max_nb_descent_steps << "\n";

            runner.set_burnin_pure_random_walk_probability(rwalk_burnin_pure_random_walk_p);
            runner.set_burnin_random_walk_probability(rwalk_burnin_random_walk_p);
            runner.set_burnin_beta(rwalk_burnin_beta);

            std::size_t const nb_burnin_steps = cnf.nb_vars() * rwalk_burnin_nb_steps;
            runner.set_burnin_nb_steps(nb_burnin_steps);
            std::cout << "c rwalk-burnin-nb-steps " << nb_burnin_steps << "\n";

            runner.set_pure_random_walk_probability(rwalk_pure_random_walk_p);
            runner.set_random_walk_probability(rwalk_random_walk_p);
            runner.set_beta(rwalk_beta);
            runner.set_restart_probability(rwalk_restart_p);

            runner.run(N, prng);
        }
        else {
            sampler::MonteCarlo runner(rag, PRINT);
            runner.run(N, prng);
        }
    }
    catch(std::exception & e) {
        std::cerr << "EXCEPTION ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
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
        bool const CUBES = vm.count("cubes") != 0;
        bool const NO_NNF = vm.count("no-nnf") != 0;

        std::cout << "c Initializing\n";

        CNF cnf(cnf_path + ".smp");
        //CNF cnf_ign(cnf_path + ".ign");
        NNF const unnf(cnf_path + ".unnf");


        if(CUBES) {
            auto cubes = dnf::read_cubes_from_file(cnf_path + ".cubes");
            sampler::RandomCubeAssignmentGenerator rag(cnf, unnf, cubes);
            std::cout << "c Using cubes\n";

            return setup_runner(vm, rag);
        }
        else if(NO_NNF) {
            sampler::RandomAssignmentGenerator rag(cnf);
            std::cout << "c Not using a d-DNNF\n";
            return setup_runner(vm, rag);
        }
        else {
            ANNF aunnf(unnf);
            aunnf.annotate_mc();
            sampler::RandomNNFAssignmentGenerator rag(cnf, aunnf);
            std::cout << "c Using unnf\n";
            return setup_runner(vm, rag);
        }
    }
    catch(std::exception & e) {
        std::cerr << "EXCEPTION ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

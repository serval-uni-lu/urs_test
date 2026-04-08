#include<iostream>
#include<fstream>
#include<string>

#include <boost/program_options.hpp>

#include "CNF.hpp"

namespace po = boost::program_options;

po::options_description get_program_options() {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display help message")
        ("cnf", po::value<std::string>(), "path to CNF file")
        ("timeout", po::value<std::size_t>()->default_value(3600), "timeout in seconds");

    return desc;
}

void print_stats(std::string const& path, CNF & cnf) {
    cnf.simplify();
    cnf.compute_free_vars();
    
    std::cout << "\nc file, #v, #vu, #vf, #c-u, #c2, #v2, #c3, #v3, #c4, #v4, #c5, #v5\nc ";

    std::cout << path << ", ";

    std::cout << cnf.nb_vars() << ", ";
    std::cout << cnf.nb_units() << ", ";
    std::cout << cnf.nb_free_vars() << ", ";
    std::cout << cnf.nb_active_clauses() << ", ";

    auto nb_by_k = cnf.get_nb_by_clause_len();
    auto var_by_k = cnf.get_vars_by_clause_len();

    for(std::size_t i = 2; i <= 5; i++) {
        if(i < nb_by_k.size()) {
            std::cout << nb_by_k[i] << ", " << var_by_k[i].size();
        }
        else {
            std::cout << "0, 0";
        }
        if(i < 5) {
            std::cout << ", ";
        }
    }
    std::cout << "\n";
}

int main(int argc, char const** argv) {
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

        std::string const path = vm["cnf"].as<std::string>();
        std::size_t const timeout = vm["timeout"].as<std::size_t>();

        CNF cnf(path.c_str());
        cnf.simplify();
        // cnf.subsumption();
        CNF up(cnf);
        CNF upp(cnf);

        cnf.project(timeout);


        std::ofstream out(path + ".p");
#ifdef RENAME
        //cnf.compute_free_vars();
        out << cnf.rename_vars();
#else
        out << cnf;
#endif
        out.close();

        auto tprj = cnf.compute_true_projection();
        auto puvar = up.inplace_upper_bound(tprj);
        up.reset_prj();
        std::ofstream upout(path + ".pup");
        upout << up;
        upout.close();

        std::cout << "c v " << puvar.size() << " / " << cnf.nb_vars() << "\n";

        // upp.set_prj(puvar);
        // upp.project();
        // std::ofstream uppout(path + ".pupp");
        // uppout << upp;
        // uppout.close();

        std::ofstream logout(path + ".log");
        logout << "c p show ";
        for(auto const & v : tprj) {
            logout << v << " ";
        }
        logout << "0\n";

#ifdef STATS
        print_stats(path, cnf);
#endif

        return 0;
    }
    catch(std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

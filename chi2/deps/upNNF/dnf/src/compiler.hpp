#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <queue>
#include <chrono>
#include <iostream>

#include "cnf.hpp"
#include "cubes.hpp"
#include "heuristics.hpp"

//bool operator<(Cube const& a, Cube const& b);

struct Cube {
    std::size_t id;
    std::size_t weight;
};

bool operator<(Cube const& a, Cube const& b);

template<typename VSH>
class Compiler {
public:
    Compiler(CNF const& ign, CNF const& smp, VSH h)
        : mcnf_ign(ign), mcnf_smp(smp), mcube_manager(ign, smp), variable_selection_heuristic(h)
    {
    }

    void init() {
        mcube_manager.clear();
        mcubes = std::priority_queue<Cube>();

        std::size_t const id = mcube_manager.new_cube();
        insert_cube_into_queue(id);
    }

    void compile(std::size_t const N, std::size_t const timeout) {
        auto const start = std::chrono::system_clock::now();

        while(mcubes.size() > 0 && mcubes.size() < N) {
            Cube top_cube = mcubes.top();
            if(mcube_manager.get(top_cube.id).munknown_ign.size() == 0) {
                break;
            }
            mcubes.pop();

            // std::cout << top_cube.id << ":\n";
            // std::cout << "   " << mcube_manager.vars(top_cube.id).size() << "\n";
            // std::cout << "   " << mcube_manager.unknown_ign(top_cube.id).size() << "\n";
            // std::cout << "   " << mcube_manager.unknown_smp(top_cube.id).size() << "\n";

            auto const res = variable_selection_heuristic(mcube_manager, top_cube.id);
            if(res.has_value()) {
                Variable const decision = res.value();
                Literal positive_decision(decision, 1);
                Literal negative_decision = ~positive_decision;

                std::vector<int> positive_implicants;
                int const positive_solver_res = mcube_manager.propagate(top_cube.id, \
                        positive_decision, positive_implicants);

                std::vector<int> negative_implicants;
                int const negative_solver_res = mcube_manager.propagate(top_cube.id, \
                        negative_decision, negative_implicants);

                if(positive_solver_res == CaDiCaL::SATISFIABLE) {
                    if(negative_solver_res == CaDiCaL::SATISFIABLE) {
                        std::size_t const nid = mcube_manager.clone_cube(top_cube.id);

                        mcube_manager.insert(top_cube.id, positive_implicants);
                        mcube_manager.insert(nid, negative_implicants);

                        insert_cube_into_queue(top_cube.id);
                        insert_cube_into_queue(nid);
                    }
                    else  {
                        assert(negative_solver_res == CaDiCaL::UNSATISFIABLE);

                        mcube_manager.insert(top_cube.id, positive_implicants);
                        insert_cube_into_queue(top_cube.id);
                    }
                }
                else {
                    assert(negative_solver_res == CaDiCaL::SATISFIABLE);
                    assert(positive_solver_res == CaDiCaL::UNSATISFIABLE);

                    mcube_manager.insert(top_cube.id, negative_implicants);
                    insert_cube_into_queue(top_cube.id);
                }
            }
            else {
                assert(false && "No decision variable returned from the heuristic");
            }

            if(mcubes.size() % 100 == 0) {
                std::cerr << "s " << mcubes.size() << "\n";
            }

            auto const now = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::duration<std::size_t> >(now - start).count();

            if(diff >= timeout) {
                break;
            }
        }
    }

    inline Cubes const& cube_manager() const { return mcube_manager; }

private:
    void insert_cube_into_queue(std::size_t const id) {
        mcubes.emplace(id, mcube_manager.get(id).munknown_ign.size());
    }

    CNF const& mcnf_ign;
    CNF const& mcnf_smp;

    Cubes mcube_manager;
    std::priority_queue<Cube> mcubes;

    VSH variable_selection_heuristic;
};


#endif

#include "compiler.hpp"

bool operator<(Cube const& a, Cube const& b) {
    return a.weight < b.weight;
}

//void Compiler::init() {
//    mcube_manager.clear();
//    mcubes = std::priority_queue<Cube>();
//
//    std::size_t id = mcube_manager.new_cube();
//    mcubes.emplace(id, mcube_manager.unknown_ign(id).size());
//}
//
//void Compiler::compile(std::size_t const N) {
//    while(mcubes.size() > 0 && mcubes.size() < N) {
//    }
//}

#ifndef CUBES_HPP
#define CUBES_HPP

#include <unordered_set>
#include <vector>

#include "../dep/cadical-rel-2.1.3/src/cadical.hpp"

#include "cnf.hpp"
#include "util.hpp"
#include "bitset.hpp"

class Cubes {
public:
    struct Cube {
        LitSet mlits;
        VarSet mvars;

        BitSet<std::size_t> munknown_ign;
        BitSet<std::size_t> munknown_smp;
    };

    Cubes(CNF const& ign, CNF const& smp);
    ~Cubes();

    std::size_t nb_cubes() const { return mcubes.size(); }

    int nb_vars() const { return mcnf_smp.nb_vars(); }

    std::size_t new_cube();
    std::size_t clone_cube(std::size_t const& cid);
    // void pop_cube();
    void clear();

    bool is_sat(std::size_t const cid) const;
    int propagate(std::size_t const cid, Literal const l, std::vector<int> & imp) const;

    //std::size_t size(std::size_t const cid) const { return mcubes.size(); }
    bool contains(std::size_t const cid, Variable const v) const;
    void insert(std::size_t const cid, Literal const& l);
    void insert(std::size_t const cid, std::vector<int> const& imp);

    Cube const& get(std::size_t const cid) const {
        return mcubes[cid];
    }

    // std::unordered_set<std::size_t> const& unknown_ign(std::size_t const cid) const {
    //     return munknown_ign[cid];
    // }

    // std::unordered_set<std::size_t> const& unknown_smp(std::size_t const cid) const {
    //     return munknown_smp[cid];
    // }

    // std::unordered_set<std::size_t> const& known_ign(std::size_t const cid) const {
    //     return mknown_ign[cid];
    // }

    // std::unordered_set<std::size_t> const& known_smp(std::size_t const cid) const {
    //     return mknown_smp[cid];
    // }

    // std::unordered_set<Variable> const& vars(std::size_t const cid) const {
    //     return mvars[cid];
    // }

    // std::unordered_set<Literal> const& lits(std::size_t const cid) const {
    //     return mlits[cid];
    // }

    inline CNF const& cnf_smp() const { return mcnf_smp; }
    inline CNF const& cnf_ign() const { return mcnf_ign; }

    inline auto begin() const { return mcubes.begin(); }
    inline auto end() const { return mcubes.end(); }

private:
    CNF const& mcnf_ign;
    CNF const& mcnf_smp;

    mutable CaDiCaL::Solver* msolver;

    // std::vector<std::unordered_set<Literal> > mlits;
    // std::vector<std::unordered_set<Variable> > mvars;

    // std::vector<std::unordered_set<std::size_t> > munknown_ign;
    // std::vector<std::unordered_set<std::size_t> > munknown_smp;

    // std::vector<std::unordered_set<std::size_t> > mknown_ign;
    // std::vector<std::unordered_set<std::size_t> > mknown_smp;
    std::vector<Cube> mcubes;
};

#endif

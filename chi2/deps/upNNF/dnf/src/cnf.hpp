//
// Created by oz on 1/12/22.
//
//
//

#ifndef BN_CNF_HPP
#define BN_CNF_HPP

#include<unordered_set>
#include<vector>
#include<map>
#include<ostream>

#include "var.hpp"

std::string mtrim(std::string const& s);

struct Clause {
    private:
    //std::set<Literal> c;
    std::vector<Literal> c;
    mutable LitSet ls_c;

    public:
    Clause();
    Clause(Clause const& c) = default;
    Clause(Clause && c) = default;

    Clause& operator=(Clause const& c) = default;
    Clause& operator=(Clause && c) = default;

    void push(Literal const& l);
    void remove(Literal const& l);
    void remove(Variable const& v);
    bool contains(Literal const& l) const;
    bool contains(Clause const& cls) const;

    inline auto begin() const {
        return c.begin();
    }

    inline auto end() const {
        return c.end();
    }

    inline std::size_t size() const {
        return c.size();
    }

    inline auto const& operator[](std::size_t i) const {
        return c[i];
        //for(auto const& l : c){
            //if(i == 0) {
                //return l;
            //}
            //i--;
        //}
        //throw std::out_of_range ("Clause[i] i is out of range");
    }

    friend bool operator==(Clause const& a, Clause const& b);
};

inline std::ostream & operator<<(std::ostream & out, Clause const& c) {
    for(auto const& l : c) {
        out << l << " ";
    }
    out << "0";
    return out;
}

class CNF {
    private:
        std::vector<Clause> clauses;
        std::vector<bool> active;
        std::vector<std::unordered_set<std::size_t> > idx;

        std::unordered_set<Literal> units;
        std::unordered_set<Variable> free;

        std::unordered_set<Variable> vars;

        std::vector<std::size_t> available_ids;

        std::size_t nb_active = 0;

        void compute_idx();

    public:
        CNF() = default;
        CNF(std::size_t nbv);
        CNF(std::string const& path);
        CNF(CNF const& c) = default;
        CNF(CNF && c) = default;

        CNF& operator=(CNF const& c) = default;
        CNF& operator=(CNF && c) = default;

        void compute_free_vars();
        void simplify();
        void subsumption();

        void add_clause(Clause c);
        void add_clause_nonredundant(Clause c);
        void rm_clause(std::size_t id);

        std::size_t occurrence_count(Variable v);
        std::size_t occurrence_product(Variable v);

        inline int nb_vars() const { return vars.size(); }
        inline int nb_free_vars() const { return free.size(); }
        inline int nb_units() const { return units.size(); }
        inline int nb_c_vars() const { return nb_vars() - nb_free_vars(); }
        inline std::size_t nb_clauses() const { return clauses.size(); }
        inline std::size_t nb_active_clauses() const { return nb_active; }

        inline std::unordered_set<Literal> const& get_units() const {
            return units;
        }

        inline bool is_unit(Literal & l) const {
            if(units.find(l) != units.end()) {
                return true;
            }
            if(units.find(~l) != units.end()) {
                l = ~l;
                return true;
            }
            return false;
        }

        inline void set_active(std::size_t const id, bool v) {
            if(active[id] && !v) {
                nb_active--;
            }
            else if(!active[id] && v) {
                nb_active++;
            }
            active[id] = v;
        }

        inline std::unordered_set<std::size_t> const& get_idx(Literal const l) const { return idx[l.get()]; }
        inline bool is_active(std::size_t const id) const { return active[id]; }
        inline Clause const& clause(std::size_t const id) const { return clauses[id]; }

    friend std::ostream & operator<<(std::ostream & out, CNF const& cnf);
};

std::ostream & operator<<(std::ostream & out, CNF const& cnf);

bool clause_is_sat(Clause const& c, LitSet const& cube);
std::size_t nb_unsat(CNF const& cnf, LitSet const& cube);
std::size_t nb_unsat_flip(CNF const& cnf, LitSet & cube, std::size_t const n_unsat, std::vector<Literal> const& lits);

std::size_t nb_flip_unsatisfied(CNF const& cnf, LitSet & m, std::size_t const n_unsat, Literal & lp);

BitSet<std::size_t> unsat_clause_ids(CNF const& cnf, LitSet const& cube);
BitSet<std::size_t> unsat_clause_ids_flip(CNF const& cnf, LitSet & m, BitSet<std::size_t> const& unsat_ids, Literal & lp);

#endif //BN_CNF_HPP

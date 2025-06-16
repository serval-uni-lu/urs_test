//
// Created by oz on 1/12/22.
//

#ifndef BN_CNF_HPP
#define BN_CNF_HPP

#include<set>
#include<vector>
#include<map>
#include<ostream>

//using Literal = int;
struct Literal;
struct Variable;

struct Literal {
    private:
    int l;

    public:
    Literal (Variable v);
    Literal (Variable v, int sign);
    Literal (int i);

    Literal(Literal const& v) = default;
    Literal(Literal && v) = default;

    Literal& operator= (Literal const& p) = default;
    Literal& operator= (Literal && p) = default;

    inline int sign() const { return l & 1 ? -1 : 1; }
    inline int to_int() const { return sign() * ((l >> 1) + 1); }
    inline int get() const { return l; }

    friend inline Literal operator ~ (Literal p);
};

inline Literal operator ~ (Literal p) {
    p.l ^= 1;
    return p;
}

inline bool operator<(Literal const& a, Literal const& b) {
    return a.get() < b.get();
}

inline bool operator>(Literal const& a, Literal const& b) {
    return a.get() > b.get();
}

inline bool operator==(Literal const& a, Literal const& b) {
    return a.get() == b.get();
}

inline bool operator>=(Literal const& a, Literal const& b) {
    return a > b || a == b;
}

inline bool operator<=(Literal const& a, Literal const& b) {
    return a < b || a == b;
}

inline bool operator!=(Literal const& a, Literal const& b) {
    return a.get() != b.get();
}

inline std::ostream & operator<<(std::ostream & out, Literal const& l) {
    out << l.to_int();
    //out << l.get();
    return out;
}

struct Variable {
    private:
    int v;

    public:
    Variable(int i);
    Variable(Literal l);

    Variable(Variable const& v) = default;
    Variable(Variable && v) = default;

    Variable& operator= (Variable const& p) = default;
    Variable& operator= (Variable && p) = default;

    inline int to_int() const { return v + 1; }
    inline int get() const { return v; }

    bool operator == (Variable p) const;
    bool operator != (Variable p) const;
    bool operator <  (Variable p) const;
};

inline std::ostream & operator<<(std::ostream & out, Variable const& v) {
    out << v.to_int();
    return out;
}

struct Clause {
    private:
    //std::set<Literal> c;
    std::vector<Literal> c;

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
        std::vector<std::set<std::size_t> > idx;

        std::set<Literal> units;
        std::set<Variable> free;

        std::set<Variable> vars;
        std::set<Variable> ind;
        std::set<Variable> prj;

        std::set<Variable> ign;
        std::set<Variable> dign;

        std::vector<std::size_t> available_ids;

        std::size_t nb_active = 0;

        void compute_idx();

    public:
        CNF() = default;
        CNF(std::size_t nbv);
        CNF(char const* path);
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

        void forget(Variable v);
        void set_prj(std::set<Variable> const& v) { prj = v; }
        void project(int const timeout = -1);

        std::size_t occurrence_count(Variable v);
        std::size_t occurrence_product(Variable v);

        CNF rename_vars();
        std::set<Variable> compute_true_projection() const;
        std::set<Variable> inplace_upper_bound(std::set<Variable> const& v);
        void reset_prj() {
            prj = vars;
            ign.clear();
            dign.clear();
        }

        std::vector<std::size_t> get_nb_by_clause_len() const;
        std::vector<std::set<Variable> > get_vars_by_clause_len() const;

        inline std::size_t nb_vars() const { return vars.size(); }
        inline std::size_t nb_free_vars() const { return free.size(); }
        inline std::size_t nb_units() const { return units.size(); }
        inline std::size_t nb_c_vars() const { return nb_vars() - nb_free_vars(); }
        inline std::size_t nb_clauses() const { return clauses.size(); }
        inline std::size_t nb_active_clauses() const { return nb_active; }

        inline Clause const& operator[](std::size_t const i) const {
            return clauses[i];
        }

    friend std::ostream & operator<<(std::ostream & out, CNF const& cnf);
};

std::ostream & operator<<(std::ostream & out, CNF const& cnf);

#endif //BN_CNF_HPP

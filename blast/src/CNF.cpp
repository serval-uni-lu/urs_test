//
// Created by oz on 1/12/22.
//

#include "CNF.hpp"

#include<fstream>
#include<iostream>
#include<sstream>
#include<algorithm>

#include "xoshiro512starstar.h"

Literal::Literal (Variable v) : l((v.get() << 1)) {
}

Literal::Literal (Variable v, int sign) : l((v.get() << 1) ^ (sign < 0 ? 1 : 0)) {
}

Literal::Literal (int i) : l(((std::abs(i) - 1) << 1) ^ (i < 0 ? 1 : 0)) {
}

bool Literal::operator == (Literal p) const {
    return l == p.get();
}

bool Literal::operator != (Literal p) const {
    return l != p.get();
}

// '<' makes p, ~p adjacent in the ordering.
bool Literal::operator <  (Literal p) const {
    return l < p.get();
}  

Variable::Variable(int i) : v(std::abs(i) - 1) {
}

Variable::Variable(Literal l) : v(l.get() >> 1) {
}

bool Variable::operator == (Variable p) const {
    return v == p.v;
}

bool Variable::operator != (Variable p) const {
    return v != p.v;
}

bool Variable::operator <  (Variable p) const {
    return v < p.v;
}

Clause::Clause() {
}

void Clause::push(Literal const& l) {
    for(auto const& i : c) {
        if(i == l)
            return;
    }
    c.push_back(l);
}

void Clause::remove(Literal const& l) {
    auto it = std::remove(c.begin(), c.end(), l);
    c.erase(it, c.end());
}

void Clause::remove(Variable const& v) {
    auto it = std::remove_if(c.begin(), c.end()
            , [v] (Literal const& li) {
                return Variable(li) == v;
            });
    c.erase(it, c.end());
}

bool Clause::contains(Literal const& l) const {
    return std::find(c.begin(), c.end(), l) != c.end();
}

bool Clause::contains(Clause const& cls) const {
    return std::all_of(cls.c.begin(), cls.c.end(), [this](auto const& l) {
        return contains(l);
    });
}

CNF::CNF(char const* path) {
    std::ifstream f(path);

    if(!f) {
        std::cerr << "error opening file " << path << "\n";
        return;
    }

    int nb_vars;
    std::string line;
    while(getline(f, line)) {
        if(line.rfind("p cnf ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;
            iss >> tmp >> tmp;
            iss >> nb_vars;

            for(int i = 1; i <= nb_vars; i++) {
                auto tmp = Variable(i);
                vars.insert(tmp);
            }
        }
        else if(line.rfind("c ind ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;

            iss >> tmp >> tmp;
            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    ind.insert(Variable(v));
                }
            }
        }
        else if(line[0] != 'c' && line[0] != 'p') {
            Clause clause;
            std::stringstream iss(line);

            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    auto tmp = Literal(v);
                    clause.push(tmp);
                }
            }

            clauses.push_back(clause);
            active.push_back(true);
        }
    }

    nb_active = active.size();
    //simplify();
    //compute_free_vars();
}

std::ostream & operator<<(std::ostream & out, CNF const& cnf) {
    out << "p cnf " << cnf.vars.size() << " " << (cnf.nb_active + cnf.units.size());
    for(std::size_t i = 0; i < cnf.clauses.size(); i++) {
        if(cnf.active[i]) {
            out << "\n" << cnf.clauses[i];
        }
    }

    for(Literal const& l : cnf.units) {
        out << "\n" << l << " 0";
    }

    for(Variable const& v : cnf.free) {
        out << "\nc " << v;
    }

    return out;
}

void CNF::compute_free_vars() {
    free = vars;

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            for(Literal const& l : clauses[i]) {
                free.erase(Variable(l));
            }
        }
    }

    for(Literal const& l : units) {
        free.erase(Variable(l));
    }
}

void CNF::insert_model(std::set<Literal> const& m) {
    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            Clause & c = clauses[i];

            bool found = false;
            for(std::size_t j = 0; j < c.size() && !found; j++) {
                auto it = m.find(c[j]);
                found = it != m.end();
            }

            if(!found) {
                uint64_t j = next_boundedrand(c.size());
                c[j] = ~c[j];
            }
        }
    }
}

void CNF::single_blast() {
    std::set<Literal> assignment;

    for(auto const& v : vars) {
        assignment.insert(Literal(v, (next() & 1) * 2 - 1));
    }

    for(auto const& l : assignment) {
        std::set<Literal> tmp = assignment;
        tmp.erase(l);
        tmp.insert(~l);

        insert_model(tmp);
    }
}

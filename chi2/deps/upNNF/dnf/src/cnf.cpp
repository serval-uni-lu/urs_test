//
// Created by oz on 1/12/22.
//

#include "cnf.hpp"

#include<fstream>
#include<iostream>
#include<sstream>
#include<algorithm>
#include<chrono>

Clause::Clause() {
}

void Clause::push(Literal const& l) {
    if(ls_c.contains(l)) {
        return;
    }

    ls_c.insert(l);

    for(auto it = c.begin(); it != c.end(); it++) {
        if(*it == l) {
            return;
        }
        else if(*it > l) {
            c.insert(it, l);
            return;
        }
    }
    c.push_back(l);
}

void Clause::remove(Literal const& l) {
    //c.erase(l);
    auto it = std::remove(c.begin(), c.end(), l);
    c.erase(it, c.end());

    ls_c.erase(l);
}

void Clause::remove(Variable const& v) {
    //Literal l(v);
    //c.erase(l);
    //c.erase(~l);
    auto it = std::remove_if(c.begin(), c.end(), [&](auto const& l) {
          return Variable(l) == v;
          });
    c.erase(it, c.end());

    Literal const l(v);
    ls_c.erase(l);
    ls_c.erase(~l);
}

bool Clause::contains(Literal const& l) const {
    return ls_c.contains(l);

    //for(std::size_t i = 0; i < c.size(); i++) {
    //    if(l == c[i]) {
    //        return true;
    //    }
    //    else if(l < c[i]) {
    //        return false;
    //    }
    //}
    //return false;
}

bool Clause::contains(Clause const& cls) const {
    //return std::all_of(cls.c.begin(), cls.c.end(), [this](auto const& l) {
    //    return contains(l);
    //});
    auto itl = begin();
    auto itcls = cls.begin();

    while(itl != end() && itcls != cls.end()) {
        if(*itl == *itcls) {
            itl++;
            itcls++;
        }
        else if(*itcls < *itl) {
            return false;
        }
        else {
            itl++;
        }
    }
    return itcls == cls.end();
}

bool operator==(Clause const& a, Clause const& b) {
    return equals(a.ls_c, b.ls_c);
}

std::string mtrim(std::string const& s) {
    auto b = s.begin();
    auto e = s.end();

    while(std::isspace(*b)) {
        b++;
    }

    if(b != e) {
        while(std::isspace(*(e - 1))) {
            e--;
        }
    }

    return std::string(b, e);
}

CNF::CNF(std::string const& path) {
    std::ifstream f(path);

    if(!f) {
        std::cerr << "error opening file " << path << "\n";
        return;
    }

    int nb_vars;
    bool missing_trailing_zero = false;
    std::string oline;
    while(getline(f, oline)) {
        std::string line = mtrim(oline);

        if(line.rfind("p cnf ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;
            iss >> tmp >> tmp;
            iss >> nb_vars;

            idx.reserve(2 * nb_vars);

            for(int i = 1; i <= nb_vars; i++) {
                auto tmp = Variable(i);
                vars.insert(tmp);

                idx.push_back({});
                idx.push_back({});
            }
        }
        else if(line.rfind("c ind ", 0) == 0) {
        }
        else if(line.rfind("c p show ", 0) == 0) {
        }
        else if(line[0] != 'c' && line[0] != 'p' && line.size() != 0) {
            Clause clause;
            std::stringstream iss(line);
            bool n_missing_trailing_zero = true;

            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    auto tmp = Literal(v);
                    clause.push(tmp);

                    idx[tmp.get()].insert(clauses.size());
                }
                else {
                    n_missing_trailing_zero = false;
                }
            }

            if(clause.size() != 0 || !missing_trailing_zero) {
                if(clause.size() == 0) {
                    std::cerr << "empty clause in input: \"" << line << "\"\n";
                }

                clauses.push_back(clause);
                active.push_back(true);
            }
            missing_trailing_zero = n_missing_trailing_zero;
        }
    }

    nb_active = active.size();
    //simplify();
    //compute_free_vars();
}

CNF::CNF(std::size_t nbv) : idx(nbv * 2, std::unordered_set<std::size_t>()) {
    for(std::size_t i = 1; i <= nbv; i++) {
        vars.insert(Variable(i));
    }
}

std::ostream & operator<<(std::ostream & out, CNF const& cnf) {
    out << "p cnf " << cnf.vars.size() << " " << (cnf.nb_active);
    for(std::size_t i = 0; i < cnf.clauses.size(); i++) {
        if(cnf.active[i]) {
            out << "\n" << cnf.clauses[i];
        }
    }

    // for(Literal const& l : cnf.units) {
    //     out << "\n" << l << " 0";
    // }

    for(Variable const& v : cnf.free) {
        out << "\nc " << v;
    }

    return out;
}

void CNF::simplify() {
    for(std::size_t id = 0; id < clauses.size(); id++) {
        if(active[id]) {
            bool trivial = false;
            for(auto const& l : clauses[id]) {
                if(clauses[id].contains(~l)) {
                    trivial = true;
                    break;
                }
            }

            if(trivial) {
                rm_clause(id);
            }
        }
    }

    bool change = true;
    while(change) {
        change = false;
        std::unordered_set<Literal> u;

        for(std::size_t i = 0; i < clauses.size(); i++) {
            if(active[i] && clauses[i].size() == 1) {
                u.insert(clauses[i][0]);
                units.insert(clauses[i][0]);
                rm_clause(i);
            }
        }

        change = u.size() != 0;

        for(Literal const& l : u) {
            Literal nl = ~l;

            auto tmp = idx[l.get()];
            for(auto const& i : tmp) {
                rm_clause(i);
            }

            for(std::size_t id : idx[(nl).get()]) {
                clauses[id].remove(nl);
            }
            idx[(nl).get()].clear();

        }
    }
    
    for(Literal const& l : units) {
        Clause c;
        c.push(l);
        add_clause(c);
    }
}

std::unordered_set<std::size_t> intersection(std::unordered_set<std::size_t> const& a, std::unordered_set<std::size_t> const& b) {
    std::unordered_set<std::size_t> res;
    for(auto const& i : a) {
        if(b.find(i) != b.end()) {
            res.insert(i);
        }
    }
    return res;
}

void CNF::subsumption() {
    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            auto j = *(clauses[i].begin());
            for(auto const& ji : clauses[i]) {
                if(idx[j.get()].size() > idx[ji.get()].size()) {
                    j = ji;
                }
            }
            auto ids = idx[j.get()];

            //auto const& j = *(clauses[i].begin());
            //auto ids = idx[j.get()];

            // auto const& j = *(clauses[i].begin());
            // std::unordered_set<std::size_t> ids = idx[j.get()];

            // for(auto const& ji : clauses[i]) {
            //     ids = intersection(ids, idx[ji.get()]);
            // }

            for(auto const& id : ids) {
                if( active[id] && id != i && clauses[id].contains(clauses[i])) {
                    rm_clause(id);
                }
            }
        }
    }
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

void CNF::add_clause(Clause c) {
    std::size_t id = clauses.size();

    if(available_ids.size() > 0) {
        id = available_ids[available_ids.size() - 1];
        available_ids.pop_back();
        active[id] = true;
        clauses[id] = c;
        nb_active += 1;
    }
    else {
        id = clauses.size();
        active.push_back(true);
        clauses.push_back(c);
        nb_active += 1;
    }


    for(auto const& l : c) {
        idx[l.get()].insert(id);
        //std::cout << idx[l.get()].size() << "\n";
    }
}

void CNF::add_clause_nonredundant(Clause c) {
    auto j = *(c.begin());
    for(auto const& ji : c) {
        if(idx[j.get()].size() > idx[ji.get()].size()) {
            j = ji;
        }
    }
    auto ids = idx[j.get()];

    //auto const& j = *(c.begin());
    //auto ids = idx[j.get()];

    bool add = true;
    for(auto const& id : ids) {
        if(active[id] && c.contains(clauses[id])) {
            // clause c does not need to be added
            add = false;
            break;
        }
    }

    if(add) {
        add_clause(c);
    }
}

void CNF::rm_clause(std::size_t id) {
    for(auto const& l : clauses[id]) {
        idx[l.get()].erase(id);
    }
    active[id] = false;
    available_ids.push_back(id);
    nb_active -= 1;
}

std::size_t CNF::occurrence_count(Variable v) {
    auto const& lp = Literal(v, 1);
    auto const& ln = Literal(v, -1);

    return idx[lp.get()].size() + idx[ln.get()].size();
}

std::size_t CNF::occurrence_product(Variable v) {
    auto const& lp = Literal(v, 1);
    auto const& ln = Literal(v, -1);

    return idx[lp.get()].size() * idx[ln.get()].size();
}

void CNF::compute_idx() {
    for(auto & i : idx) {
        i.clear();
    }
    for(std::size_t id = 0; id < clauses.size(); id++) {
        if(active[id]) {
            for(auto const& l : clauses[id]) {
                idx[l.get()].insert(id);
            }
        }
    }
}

bool clause_is_sat(Clause const& c, LitSet const& cube) {
    for(Literal const& l : c) {
        if(cube.contains(l)) {
            return true;
        }
    }
    return false;
}

std::size_t nb_unsat(CNF const& cnf, LitSet const& cube) {
    std::size_t result = 0;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            if(!clause_is_sat(cnf.clause(i), cube)) {
                result += 1;
            }
        }
    }

    return result;
}

std::size_t nb_unsat_flip(CNF const& cnf, LitSet & cube, std::size_t const n_unsat, std::vector<Literal> const& lits) {
    std::vector<Literal> mlits;
    for(Literal l : lits) {
        assert(cube.contains(l) || cube.contains(~l));
        if(cube.contains(~l)) {
            mlits.push_back(~l);
        }
    }

    if(mlits.size() == 0) {
        return n_unsat;
    }

    std::size_t result = n_unsat;
    std::unordered_set<std::size_t> clause_ids;

    for(Literal const& l : mlits) {
        for(std::size_t id : cnf.get_idx(l)) {
            if(cnf.is_active(id)) {
                clause_ids.insert(id);
            }
        }

        for(std::size_t id : cnf.get_idx(~l)) {
            if(cnf.is_active(id)) {
                clause_ids.insert(id);
            }
        }
    }

    for(std::size_t const cid : clause_ids) {
        if(!clause_is_sat(cnf.clause(cid), cube)) {
            result -= 1;
        }
    }

    for(Literal const& l : mlits) {
        cube.erase(l);
        cube.insert(~l);
    }

    for(std::size_t const cid : clause_ids) {
        if(!clause_is_sat(cnf.clause(cid), cube)) {
            result += 1;
        }
    }

    assert(result == nb_unsat(cnf, cube));

    for(Literal const& l : mlits) {
        cube.erase(~l);
        cube.insert(l);
    }

    return result;
}

std::size_t nb_flip_unsatisfied(CNF const& cnf, LitSet & m, std::size_t const n_unsat, Literal & lp) {
    if(!m.contains(lp)) {
        lp = ~lp;
    }
    Literal ln = ~lp;

    std::size_t res = n_unsat;
    for(std::size_t id : cnf.get_idx(lp)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            res--;
        }
    }
    for(std::size_t id : cnf.get_idx(ln)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            res--;
        }
    }

    m.erase(lp);
    m.insert(ln);
    for(std::size_t id : cnf.get_idx(lp)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            res++;
        }
    }
    for(std::size_t id : cnf.get_idx(ln)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            res++;
        }
    }
    m.erase(ln);
    m.insert(lp);

    return res;
}

BitSet<std::size_t> unsat_clause_ids(CNF const& cnf, LitSet const& cube) {
    BitSet<std::size_t> result;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            if(!clause_is_sat(cnf.clause(i), cube)) {
                result.insert(i);
            }
        }
    }

    return result;
}

BitSet<std::size_t> unsat_clause_ids_flip(CNF const& cnf, LitSet & m, BitSet<std::size_t> const& unsat_ids, Literal & lp) {
    if(!m.contains(lp)) {
        lp = ~lp;
    }
    Literal ln = ~lp;

    BitSet<std::size_t> result = unsat_ids;

    m.erase(lp);
    m.insert(ln);

    for(std::size_t id : cnf.get_idx(lp)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            result.insert(id);
        }
        else {
            result.erase(id);
        }
    }
    for(std::size_t id : cnf.get_idx(ln)) {
        if(cnf.is_active(id) && !clause_is_sat(cnf.clause(id), m)) {
            result.insert(id);
        }
        else {
            result.erase(id);
        }
    }

    m.erase(ln);
    m.insert(lp);

    return result;
}

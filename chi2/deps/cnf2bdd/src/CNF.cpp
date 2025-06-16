//
// Created by oz on 1/12/22.
//

#include "CNF.hpp"

#include<fstream>
#include<iostream>
#include<sstream>
#include<algorithm>
#include<chrono>

Literal::Literal (Variable v) : l((v.get() << 1)) {
}

Literal::Literal (Variable v, int sign) : l((v.get() << 1) | (sign < 0 ? 1 : 0)) {
}

Literal::Literal (int i) : l(((std::abs(i) - 1) << 1) ^ (i < 0 ? 1 : 0)) {
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
    //c.insert(l);
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
}

void Clause::remove(Variable const& v) {
    //Literal l(v);
    //c.erase(l);
    //c.erase(~l);
    auto it = std::remove_if(c.begin(), c.end(), [&](auto const& l) {
          return Variable(l) == v;
          });
    c.erase(it, c.end());
}

bool Clause::contains(Literal const& l) const {
    for(std::size_t i = 0; i < c.size(); i++) {
        if(l == c[i]) {
            return true;
        }
        else if(l < c[i]) {
            return false;
        }
    }
    return false;
    //return c.find(l) != c.end();
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

CNF::CNF(char const* path) {
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
        else if(line.rfind("c p show ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;

            iss >> tmp >> tmp >> tmp;
            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    prj.insert(Variable(v));
                }
            }
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

CNF::CNF(std::size_t nbv) : idx(nbv * 2, std::set<std::size_t>()) {
    for(std::size_t i = 1; i <= nbv; i++) {
        vars.insert(Variable(i));
    }
}

CNF CNF::rename_vars() {
    std::size_t next_v = 1;
    std::set<Variable> done;

    std::map<Variable, Variable> m;

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i] && clauses[i].size() > 1) {
            for(auto const& l : clauses[i]) {
                Variable v(l);

                if(done.find(v) == done.end()) {
                    //m[v] = Variable(next_v);
                    m.emplace(v, next_v);
                    next_v += 1;
                    done.insert(v);
                }
            }
        }
    }

#ifdef PRINT_MAP
    for(auto const& e : m) {
        std::cout << "c " << e.first << " -> " << e.second << "\n";
    }
#endif

    CNF res(m.size());

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i] && clauses[i].size() > 1) {
            Clause tmp;
            for(auto const& l : clauses[i]) {
                Variable v(l);
                Literal nl(m.find(v)->second, l.sign());

                tmp.push(nl);
            }

            //std::cout << clauses[i] << " -> " << tmp << "\n";
            res.add_clause(tmp);
        }
    }

    return res;
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

    out << "\nc forgotten";
    for(Variable const& v : cnf.dign) {
        Literal l(v, 1);
        out << "\n" << l << " 0";
    }

    out << "\nc p show";
    for(Variable const& v : cnf.prj) {
        out << " " << v;
    }
    out << " 0";

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
        std::set<Literal> u;

        for(std::size_t i = 0; i < clauses.size(); i++) {
            if(active[i] && clauses[i].size() == 1) {
                u.insert(clauses[i][0]);
                units.insert(clauses[i][0]);
                active[i] = false;
                nb_active -= 1;
            }
        }

        change = u.size() != 0;

        for(Literal const& l : u) {
            Literal nl = ~l;

            for(std::size_t i : idx[l.get()]) {
                if(active[i]) {
                    nb_active -= 1;
                }
                active[i] = false;

                for(Literal const& li : clauses[i]) {
                    if(li != l) {
                        idx[li.get()].erase(i);
                    }
                }
            }
            idx[l.get()].clear();

            for(std::size_t id : idx[(nl).get()]) {
                clauses[id].remove(nl);
            }
            idx[(nl).get()].clear();
        }
    }
}

std::set<std::size_t> intersection(std::set<std::size_t> const& a, std::set<std::size_t> const& b) {
    std::set<std::size_t> res;
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
            // std::set<std::size_t> ids = idx[j.get()];

            // for(auto const& ji : clauses[i]) {
            //     ids = intersection(ids, idx[ji.get()]);
            // }

            for(auto const& id : ids) {
                if( active[id] && id != i && clauses[id].contains(clauses[i])) {
                    active[id] = false;
                    nb_active--;
                    for(auto const& l : clauses[id]) {
                        idx[l.get()].erase(id);
                    }
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

std::vector<std::size_t> CNF::get_nb_by_clause_len() const {
    std::vector<std::size_t> res;
    res.push_back(0);
    res.push_back(units.size());

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            Clause const& c = clauses[i];

            while(c.size() >= res.size()) {
                res.push_back(0);
            }
            res[c.size()]++;
        }
    }

    return res;
}

std::vector<std::set<Variable> > CNF::get_vars_by_clause_len() const {
    std::vector<std::set<Variable> > res;
    res.reserve(vars.size());
    res.push_back({});
    res.push_back({});

    for(Literal const& l : units) {
        res[1].insert(Variable(l));
    }

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            Clause const& c = clauses[i];

            while(c.size() >= res.size()) {
                res.push_back({});
            }
            for(auto const& l : c) {
                res[c.size()].insert(Variable(l));
                res[0].insert(Variable(l));
            }
        }
    }

    return res;
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

/*
 * fix:
 * properly handle unit clauses
 * verify correctness
 */
void CNF::forget(Variable v) {
    auto const& lp = Literal(v, 1);
    auto const& ln = ~lp;

    if(units.find(lp) != units.end()) {
        units.erase(lp);

        for(std::size_t id : idx[ln.get()]) {
            clauses[id].remove(ln);
        }
        idx[(ln).get()].clear();
    }
    else if(units.find(ln) != units.end()) {
        units.erase(ln);

        for(std::size_t id : idx[lp.get()]) {
            clauses[id].remove(lp);
        }
        idx[(lp).get()].clear();
    }
    else {
        for(auto const& ip : idx[lp.get()]) {
            for(auto const& in : idx[ln.get()]) {
                Clause p = clauses[ip];
                p.remove(lp);

                bool trivial = false;
                for(auto const& l : clauses[in]) {
                    if(ln != l && lp != l) {
                        if(p.contains(~l)) {
                            trivial = true;
                            break;
                        }

                        p.push(l);
                    }
                }

                if(!trivial) {
                    // add_clause(p);
                    add_clause_nonredundant(p);
                }
            }
        }
    }

    auto ids = idx[lp.get()];
    for(auto const& ip : ids) {
        rm_clause(ip);
    }

    ids = idx[ln.get()];
    for(auto const& in : ids) {
        rm_clause(in);
    }

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

void CNF::project(int const timeout) {
    auto const st = std::chrono::steady_clock::now();
    auto const ina = nb_active;

    std::set<Variable> frgt = vars;
    for(auto const& v : prj) {
        frgt.erase(v);
    }
    ign = frgt;

    int nbf = 0;
    while(frgt.size() > 0) {
        Variable v = *(frgt.begin());
        for(auto const& vi : frgt) {
            if(occurrence_product(v) > occurrence_product(vi)) {
                v = vi;
            }
        }

        forget(v);
        frgt.erase(v);
        nbf += 1;

        dign.insert(v);

        auto const ed = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(ed - st);
        if(nb_active >= 3 * ina || (timeout != -1 && elapsed.count() >= timeout)) {
        // if(elapsed.count() > 60) {
            std::cerr << "c break " << nbf << " / " << ign.size() << "\n";
            break;
        }
    }
}

std::set<Variable> CNF::compute_true_projection() const {
    std::set<Variable> res = vars;
    for(auto const& v : dign) {
        res.erase(v);
    }
    return res;
}


std::set<Variable> CNF::inplace_upper_bound(std::set<Variable> const& v) {
    std::set<Variable> res;

    for(std::size_t id = 0; id < clauses.size(); id++) {
        bool br = false;
        for(auto const& l : clauses[id]) {
            if(v.find(Variable(l)) == v.end()) {
                br = true;
                break;
            }
        }

        if(! br) {
            rm_clause(id);
        }
        else {
            for(auto const& l : clauses[id]) {
                res.insert(Variable(l));
            }
        }
    }

    return res;
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

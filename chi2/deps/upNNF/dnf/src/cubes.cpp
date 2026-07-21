#include "cubes.hpp"

Cubes::Cubes(CNF const& ign, CNF const& smp) : mcnf_ign(ign), mcnf_smp(smp) {
    msolver = cnf_to_cadical(smp);
}

Cubes::~Cubes() {
    delete msolver;
}

std::size_t Cubes::new_cube() {
    std::size_t const cid = mcubes.size();

    // mlits.emplace_back();
    // mvars.emplace_back();
    // munknown_ign.emplace_back();
    // munknown_smp.emplace_back();
    // mknown_ign.emplace_back();
    // mknown_smp.emplace_back();

    mcubes.emplace_back();

    //for(int i = 1; i <= mcnf_smp.nb_vars(); i++) {
    //    mvars[cid].emplace(i);
    //}

    for(std::size_t cl_id = 0; cl_id < mcnf_smp.nb_clauses(); cl_id++) {
        if(mcnf_smp.is_active(cl_id)) {
            mcubes[cid].munknown_smp.insert(cl_id);
        }
    }

    for(std::size_t cl_id = 0; cl_id < mcnf_ign.nb_clauses(); cl_id++) {
        if(mcnf_ign.is_active(cl_id)) {
            mcubes[cid].munknown_ign.insert(cl_id);

            for(Literal const l : mcnf_ign.clause(cl_id)) {
                //mcubes[cid].mvars.emplace(l);
                mcubes[cid].mvars.insert(Variable(l));
            }
        }
    }

    int const res = msolver->propagate();
    if(CaDiCaL::UNKNOWN == res) {
        std::vector<int> imp;
        msolver->get_entrailed_literals(imp);
        insert(cid, imp);
    }
    msolver->reset_assumptions();

    return cid;
}

std::size_t Cubes::clone_cube(std::size_t const& cid) {
    std::size_t ncid = mcubes.size();

    mcubes.emplace_back(mcubes[cid]);

    // mlits.emplace_back(mlits[cid]);
    // mvars.emplace_back(mvars[cid]);
    // munknown_ign.emplace_back(munknown_ign[cid]);
    // munknown_smp.emplace_back(munknown_smp[cid]);
    // mknown_ign.emplace_back(mknown_ign[cid]);
    // mknown_smp.emplace_back(mknown_smp[cid]);

    return ncid;
}

// void Cubes::pop_cube() {
//     mlits.pop_back();
//     mvars.pop_back();
//     munknown_ign.pop_back();
//     munknown_smp.pop_back();
// }

void Cubes::clear() {
    mcubes.clear();
    // mlits.clear();
    // mvars.clear();
    // munknown_ign.clear();
    // munknown_smp.clear();
    // mknown_ign.clear();
    // mknown_smp.clear();
}

bool Cubes::is_sat(std::size_t const cid) const {
    for(Literal const l : mcubes[cid].mlits) {
        msolver->assume(l.to_int());
    }
    auto const res = msolver->solve();
    assert(CaDiCaL::UNKNOWN != res);
    return CaDiCaL::SATISFIABLE == res;
}

bool vector_contains(std::vector<int> const& imp, Literal const l) {
    for(int const i : imp) {
        if(l.to_int() == i) {
            return true;
        }
    }
    return false;
}

int Cubes::propagate(std::size_t const cid, Literal const l, std::vector<int> & imp) const {
    for(Literal const li : mcubes[cid].mlits) {
        msolver->assume(li.to_int());
    }
    msolver->assume(l.to_int());
    int sr = msolver->propagate();

    switch(sr) {
        case CaDiCaL::UNSATISFIABLE:
            msolver->reset_assumptions();
            return CaDiCaL::UNSATISFIABLE;

        case CaDiCaL::UNKNOWN:
            msolver->get_entrailed_literals(imp);
            //assert(vector_contains(imp, l));
            imp.push_back(l.to_int());
            return msolver->solve();

        case CaDiCaL::SATISFIABLE:
            msolver->reset_assumptions();
            imp.push_back(l.to_int());
            return CaDiCaL::SATISFIABLE;

        default:
            assert(false && "Impossible state in Cubes::propagate");
            break;
    };

    assert(false && "Impossible state in Cubes::propagate");
}

bool Cubes::contains(std::size_t const cid, Variable const v) const {
    Literal const lp(v);
    Literal const ln = ~lp;

    auto const& lits = mcubes[cid].mlits;
    //return (lits.find(lp) != lits.end()) || (lits.find(ln) != lits.end());
    return (lits.contains(lp)) || (lits.contains(ln));
}

void Cubes::insert(std::size_t const cid, Literal const& l) {
    //if(mcubes[cid].mlits.find(l) != mcubes[cid].mlits.end()) {
    if(mcubes[cid].mlits.contains(l)) {
        return;
    }
    //assert(mcubes[cid].mlits.find(~l) == mcubes[cid].mlits.end());
    assert(! mcubes[cid].mlits.contains(~l));

    for(std::size_t const cl_id : mcnf_ign.get_idx(l)) {
        mcubes[cid].munknown_ign.erase(cl_id);
        //mcubes[cid].mknown_ign.insert(cl_id);
    }

    for(std::size_t const cl_id : mcnf_smp.get_idx(l)) {
        mcubes[cid].munknown_smp.erase(cl_id);
        //mcubes[cid].mknown_smp.insert(cl_id);
    }

    mcubes[cid].mlits.insert(l);
    mcubes[cid].mvars.erase(Variable(l));
}

void Cubes::insert(std::size_t const cid, std::vector<int> const& imp) {
    for(int const li : imp) {
        insert(cid, Literal(li));
    }
}

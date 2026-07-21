#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include <optional>

#include "cnf.hpp"
#include "nnf.hpp"
#include "cubes.hpp"

class MOMSVarPicker {
    std::size_t const factor;

    std::optional<Variable> fallback(Cubes const& cubes, std::size_t const cid) {
        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();
        
        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_ign) {
            std::size_t s = cnf_ign.clause(cl_id).size();
            for(Literal const l : cnf_ign.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_ign.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_ign.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            std::size_t vscore = lp_score + ln_score + factor * lp_score * ln_score;

            if(vscore > score) {
                mv = v;
                score = vscore;
            }
        }

        return mv;
    }

    std::optional<Variable> main(Cubes const& cubes, std::size_t const cid) {
        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t s = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_smp.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_smp.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            std::size_t vscore = lp_score + ln_score + factor * lp_score * ln_score;

            if(vscore > score) {
                mv = v;
                score = vscore;
            }
        }

        return mv;
    }

public:
    MOMSVarPicker(std::size_t const f = 0) : factor(f) {
    }

    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> const res_main = main(cubes, cid);

        if(res_main.has_value()) {
            return res_main;
        }

        std::optional<Variable> const res_fallback = fallback(cubes, cid);
        assert(res_fallback.has_value());
        return res_fallback;
    }
};

class MOMSLitPicker {
    std::optional<Variable> fallback(Cubes const& cubes, std::size_t const cid) {
        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();
        
        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_ign) {
            std::size_t s = cnf_ign.clause(cl_id).size();
            for(Literal const l : cnf_ign.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_ign.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_ign.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            if(lp_score > score) {
                mv = Variable(lp);
                score = lp_score;
            }

            if(ln_score > score) {
                mv = Variable(ln);
                score = ln_score;
            }
        }

        return mv;
    }

    std::optional<Variable> main(Cubes const& cubes, std::size_t const cid) {
        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t s = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_smp.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_smp.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            if(lp_score > score) {
                mv = Variable(lp);
                score = lp_score;
            }

            if(ln_score > score) {
                mv = Variable(ln);
                score = ln_score;
            }
        }

        return mv;
    }

public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> const res_main = main(cubes, cid);

        if(res_main.has_value()) {
            return res_main;
        }

        std::optional<Variable> const res_fallback = fallback(cubes, cid);
        assert(res_fallback.has_value());
        return res_fallback;
    }
};

class MOMSVarBase {
    std::size_t const factor;

    std::optional<Variable> main(Cubes const& cubes, std::size_t const cid) {
        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t s = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_smp.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_smp.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            std::size_t vscore = lp_score + ln_score + factor * lp_score * ln_score;

            if(vscore > score) {
                mv = v;
                score = vscore;
            }
        }

        if(!mv.has_value()) {
            std::vector<std::size_t> lit_occ(cnf_smp.nb_vars() * 2, 0);
            score = 0;

            for(std::size_t const cid : smap) {
                for(Literal const l : cnf_smp.clause(cid)) {
                    assert(!cube.mlits.contains(l));

                    if(!cube.mlits.contains(~l)) {
                        lit_occ[l.get()] += 1;
                    }
                    std::size_t const lp_score = lit_occ[l.get()];
                    std::size_t const ln_score = lit_occ[(~l).get()];
                    std::size_t const vscore = lp_score + ln_score + factor * lp_score * ln_score;

                    if(vscore > score) {
                        mv = Variable(l);
                        score = vscore;
                    }
                }
            }
        }

        assert(mv.has_value());
        return mv;
    }

public:
    MOMSVarBase(std::size_t const f = 0) : factor(f) {
    }

    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> const res_main = main(cubes, cid);
        assert(res_main.has_value());
        return res_main;
    }
};

class MOMSLitBase {
    std::optional<Variable> main(Cubes const& cubes, std::size_t const cid) {
        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> smap;
        std::size_t low = cubes.nb_vars() * 2;

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t s = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                Variable v(l);

                assert(!cube.mlits.contains(l));

                if(cubes.contains(cid, v)) {
                    assert(cube.mlits.contains(~l));
                    s -= 1;
                }
            }
            if(s == low) {
                smap.push_back(cl_id);
            }
            else if(s < low) {
                smap.clear();
                smap.push_back(cl_id);
                low = s;
            }
        }

        std::optional<Variable> mv;
        std::size_t score = 0;
        for(Variable const v : cube.mvars) {
            Literal const lp(v, 0);
            Literal const ln = ~lp;

            std::size_t lp_score = 0;
            std::size_t ln_score = 0;

            for(std::size_t const cid : smap) {
                if(cnf_smp.clause(cid).contains(lp)) {
                    lp_score += 1;
                }

                if(cnf_smp.clause(cid).contains(ln)) {
                    ln_score += 1;
                }
            }

            if(lp_score > score) {
                mv = Variable(lp);
                score = lp_score;
            }

            if(ln_score > score) {
                mv = Variable(ln);
                score = ln_score;
            }
        }

        if(!mv.has_value()) {
            std::vector<std::size_t> lit_occ(cnf_smp.nb_vars() * 2, 0);
            score = 0;

            for(std::size_t const cid : smap) {
                for(Literal const l : cnf_smp.clause(cid)) {
                    assert(!cube.mlits.contains(l));

                    if(!cube.mlits.contains(~l)) {
                        lit_occ[l.get()] += 1;
                    }
                    std::size_t const lp_score = lit_occ[l.get()];
                    std::size_t const ln_score = lit_occ[(~l).get()];

                    if(lp_score > score) {
                        mv = Variable(l);
                        score = lp_score;
                    }

                    if(ln_score > score) {
                        mv = Variable(~l);
                        score = ln_score;
                    }
                }
            }
        }

        assert(mv.has_value());
        return mv;
    }

public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> const res_main = main(cubes, cid);
        assert(res_main.has_value());
        return res_main;
    }
};

class NNFLookahead {
    ANNF annf;
public:

    NNFLookahead(NNF const& nnf) : annf(nnf) {
    }

    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::cout << "  h init\n";
        Cubes::Cube const& cube = cubes.get(cid);
        annf.set_assumps(cube.mlits);
        std::cout << "  h annotation\n";
        annf.annotate_mc();

        mpz_int const base_mc = annf.mc(NNF::ROOT);
        LitSet lits = cube.mlits;

        mpz_int reduction = 0;

        std::optional<Variable> result;

        //std::cout << "  h var comp\n";
        for(Variable const v : cube.mvars) {
            // std::cout << "    h " << v << "\n";
            Literal const lp(v);
            Literal const ln = ~lp;

            assert(!lits.contains(lp));
            assert(!lits.contains(ln));

            lits.insert(lp);
            annf.set_assumps(lits);
            annf.annotate_mc();
            lits.erase(lp);
            mpz_int const positive_reduction = base_mc - annf.mc(NNF::ROOT);

            assert(!lits.contains(lp));
            assert(!lits.contains(ln));

            lits.insert(ln);
            annf.set_assumps(lits);
            annf.annotate_mc();
            lits.erase(ln);
            mpz_int const negative_reduction = base_mc - annf.mc(NNF::ROOT);

            assert(!lits.contains(lp));
            assert(!lits.contains(ln));

            if(positive_reduction > reduction) {
                reduction = positive_reduction;
                result = Variable(lp);
            }

            if(negative_reduction > reduction) {
                reduction = negative_reduction;
                result = Variable(ln);
            }
        }
        // std::cout << "\n";

        std::cout << "  h done\n";
        assert(result.has_value());
        return result;
    }
};

class LitCount {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> lit_count(cnf_ign.nb_vars() * 2, 0);

        for(std::size_t const cl_id : cube.munknown_ign) {
            for(Literal const l : cnf_ign.clause(cl_id)) {
                lit_count[l.get()] += 1;
            }
        }

        std::size_t max_count = 0;
        Literal max_lit(0);

        for(Variable const v : cube.mvars) {
            Literal const lp(v);
            Literal const ln = ~lp;

            if(lit_count[lp.get()] > max_count) {
                max_lit = lp;
                max_count = lit_count[lp.get()];
            }

            if(lit_count[ln.get()] > max_count) {
                max_lit = ln;
                max_count = lit_count[ln.get()];
            }
        }

        assert(max_lit != Literal(0));

        result = Variable(max_lit);

        assert(result.has_value());
        return result;
    }
};

class LitCountSMP {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> lit_count(cnf_smp.nb_vars() * 2, 0);

        for(std::size_t const cl_id : cube.munknown_smp) {
            for(Literal const l : cnf_smp.clause(cl_id)) {
                lit_count[l.get()] += 1;
            }
        }

        std::size_t max_count = 0;
        Literal max_lit(0);

        for(Variable const v : cube.mvars) {
            Literal const lp(v);
            Literal const ln = ~lp;

            if(lit_count[lp.get()] > max_count) {
                max_lit = lp;
                max_count = lit_count[lp.get()];
            }

            if(lit_count[ln.get()] > max_count) {
                max_lit = ln;
                max_count = lit_count[ln.get()];
            }
        }

        assert(max_lit != Literal(0));

        result = Variable(max_lit);

        assert(result.has_value());
        return result;
    }
};

class LitWeight {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<double> lit_weight(cnf_ign.nb_vars() * 2, 0);

        for(std::size_t const cl_id : cube.munknown_ign) {
            std::size_t cl_sz = cnf_ign.clause(cl_id).size();
            for(Literal const l : cnf_ign.clause(cl_id)) {
                assert(!cube.mlits.contains(l));
                if(cube.mlits.contains(~l)) {
                    cl_sz -= 1;
                }
            }

            for(Literal const l : cnf_ign.clause(cl_id)) {
                lit_weight[l.get()] += pow(2, -1 * cl_sz);
            }
        }

        double max_weight = 0;
        Literal max_lit(0);

        for(Variable const v : cube.mvars) {
            Literal const lp(v);
            Literal const ln = ~lp;

            if(lit_weight[lp.get()] > max_weight) {
                max_weight = lit_weight[lp.get()];
                max_lit = lp;
            }

            if(lit_weight[ln.get()] > max_weight) {
                max_weight = lit_weight[ln.get()];
                max_lit = ln;
            }
        }

        assert(Literal(0) != max_lit);
        result = Variable(max_lit);

        assert(result.has_value());
        return result;
    }
};

class LitWeightSMP {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<double> lit_weight(cnf_smp.nb_vars() * 2, 0);

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t cl_sz = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                assert(!cube.mlits.contains(l));
                if(cube.mlits.contains(~l)) {
                    cl_sz -= 1;
                }
            }

            for(Literal const l : cnf_smp.clause(cl_id)) {
                lit_weight[l.get()] += pow(2, -1 * cl_sz);
            }
        }

        double max_weight = 0;
        Literal max_lit(0);

        for(Variable const v : cube.mvars) {
            Literal const lp(v);
            Literal const ln = ~lp;

            if(lit_weight[lp.get()] > max_weight) {
                max_weight = lit_weight[lp.get()];
                max_lit = lp;
            }

            if(lit_weight[ln.get()] > max_weight) {
                max_weight = lit_weight[ln.get()];
                max_lit = ln;
            }
        }

        assert(Literal(0) != max_lit);
        result = Variable(max_lit);

        assert(result.has_value());
        return result;
    }
};

class VarCount {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> var_count(cnf_ign.nb_vars(), 0);

        for(std::size_t const cl_id : cube.munknown_ign) {
            for(Literal const l : cnf_ign.clause(cl_id)) {
                var_count[Variable(l).get()] += 1;
            }
        }

        std::size_t max_count = 0;
        Variable max_var(0);

        for(Variable const v : cube.mvars) {
            if(var_count[v.get()] > max_count) {
                max_var = v;
                max_count = var_count[v.get()];
            }
        }

        assert(max_var != Variable(0));

        result = max_var;

        assert(result.has_value());
        return result;
    }
};

class VarCountSMP {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<std::size_t> var_count(cnf_smp.nb_vars(), 0);

        for(std::size_t const cl_id : cube.munknown_smp) {
            for(Literal const l : cnf_smp.clause(cl_id)) {
                var_count[Variable(l).get()] += 1;
            }
        }

        std::size_t max_count = 0;
        Variable max_var(0);

        for(Variable const v : cube.mvars) {
            if(var_count[v.get()] > max_count) {
                max_var = v;
                max_count = var_count[v.get()];
            }
        }

        assert(max_var != Variable(0));

        result = max_var;

        assert(result.has_value());
        return result;
    }
};

class VarWeight {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        //CNF const& cnf_smp = cubes.cnf_smp();
        CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<double> var_weight(cnf_ign.nb_vars(), 0);

        for(std::size_t const cl_id : cube.munknown_ign) {
            std::size_t cl_sz = cnf_ign.clause(cl_id).size();
            for(Literal const l : cnf_ign.clause(cl_id)) {
                assert(!cube.mlits.contains(l));
                if(cube.mlits.contains(~l)) {
                    cl_sz -= 1;
                }
            }

            for(Literal const l : cnf_ign.clause(cl_id)) {
                var_weight[Variable(l).get()] += pow(2, -1 * cl_sz);
            }
        }

        double max_weight = 0;
        Variable max_var(0);

        for(Variable const v : cube.mvars) {
            if(var_weight[v.get()] > max_weight) {
                max_weight = var_weight[v.get()];
                max_var = v;
            }
        }

        assert(Variable(0) != max_var);
        result = max_var;

        assert(result.has_value());
        return result;
    }
};

class VarWeightSMP {
public:
    std::optional<Variable> operator()(Cubes const& cubes, std::size_t const cid) {
        std::optional<Variable> result;

        CNF const& cnf_smp = cubes.cnf_smp();
        //CNF const& cnf_ign = cubes.cnf_ign();

        Cubes::Cube const& cube = cubes.get(cid);

        std::vector<double> var_weight(cnf_smp.nb_vars(), 0);

        for(std::size_t const cl_id : cube.munknown_smp) {
            std::size_t cl_sz = cnf_smp.clause(cl_id).size();
            for(Literal const l : cnf_smp.clause(cl_id)) {
                assert(!cube.mlits.contains(l));
                if(cube.mlits.contains(~l)) {
                    cl_sz -= 1;
                }
            }

            for(Literal const l : cnf_smp.clause(cl_id)) {
                var_weight[Variable(l).get()] += pow(2, -1 * cl_sz);
            }
        }

        double max_weight = 0;
        Variable max_var(0);

        for(Variable const v : cube.mvars) {
            if(var_weight[v.get()] > max_weight) {
                max_weight = var_weight[v.get()];
                max_var = v;
            }
        }

        assert(Variable(0) != max_var);
        result = max_var;

        assert(result.has_value());
        return result;
    }
};

#endif

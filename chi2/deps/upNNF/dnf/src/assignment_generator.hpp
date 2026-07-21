#ifndef ASSIGNMENT_GENERATOR_HPP
#define ASSIGNMENT_GENERATOR_HPP

#include <vector>
#include <iostream>
#include <algorithm>
#include <atomic>

#include <boost/multiprecision/gmp.hpp>
#include <boost/random/bernoulli_distribution.hpp>

#include "nnf.hpp"
#include "cnf.hpp"

std::size_t nb_unknown(CNF const& cnf, LitSet const& cube) {
    std::size_t nb = 0;

    for(std::size_t i = 0; i < cnf.nb_clauses(); i++) {
        if(cnf.is_active(i)) {
            bool is_sat = false;
            for(Literal const& l : cnf.clause(i)) {
                is_sat = cube.contains(l);

                if(is_sat) {
                    break;
                }
            }

            if(!is_sat) {
                nb += 1;
            }
        }
    }

    return nb;
}

namespace dnf {

struct Cube {
    LitSet lits;
    mpz_int mc;
};

using Cubes = std::vector<Cube>;

Cubes read_cubes_from_file(std::string const& path) {
    Cubes cubes;

    std::ifstream f(path);

    std::string oline;
    while(getline(f, oline)) {
        std::string const line = mtrim(oline);

        Cube cube;
        std::stringstream iss(line);

        while(iss) {
            int v;
            iss >> v;
            if(v != 0) {
                cube.lits.insert(Literal(v));
            }
        }

        cubes.push_back(cube);
    }
    return cubes;
}

};


namespace sampler {

using boost::multiprecision::mpz_int;
using boost::multiprecision::mpf_float;
//using namespace boost::random;
using boost::random::bernoulli_distribution;


/*
 * generate a random int in [0, R - 1]
 */
template<typename PRNG>
mpz_int gen_random_int(PRNG & g, mpz_int const& R) {
    std::size_t const needed = msb(R) + 1;
    std::size_t const n_limbs = ((needed + 63) / 64);

    mpz_t result;
    mpz_init2(result, n_limbs * 64);
    std::vector<uint64_t> limbs(n_limbs);
    while(true) {

        for(std::size_t i = 0; i < n_limbs; i++) {
            limbs[i] = g();
        }

        std::size_t const extra_bits = n_limbs * 64 - needed;
        if(extra_bits > 0) {
            limbs[n_limbs - 1] &= (uint64_t(1) << (64 - extra_bits)) - 1;
        }

        mpz_import(result, n_limbs, -1, sizeof(uint64_t), 0, 0, limbs.data());

        if(mpz_cmp(result, R.backend().data()) < 0) {
            mpz_int res(result);
            mpz_clear(result);
            return res;
        }
    }
}

struct Cube {
    LitSet m;
    std::size_t nb_unsat;
};

using Cubes = std::vector<Cube>;

class RandomAssignmentGenerator {
private:
    CNF const& cnf;
    std::bernoulli_distribution bdist;

public:
    RandomAssignmentGenerator(CNF const& cnf) : cnf(cnf), bdist(0.5) {
    }

    RandomAssignmentGenerator(RandomAssignmentGenerator const& rag) = default;

    RandomAssignmentGenerator& operator=(RandomAssignmentGenerator const& rag) = default;

    inline CNF const& get_cnf() const { return cnf; }

    inline mpz_int get_mc() const {
        mpz_int mc = 1;
        mc <<= cnf.nb_vars();
        return mc;
    }

    template<typename PRNG>
    Cube operator()(PRNG & prng) {
        Cube c;
        for(int i = 1; i <= cnf.nb_vars(); i++) {
            Literal l(i);
            if(bdist(prng)) {
                l = ~l;
            }
            c.m.insert(l);
        }
        c.nb_unsat = nb_unknown(cnf, c.m);

        return c;
    }
};

class RandomNNFAssignmentGenerator {
private:
    CNF const& cnf;
    ANNF const& annf;

public:
    RandomNNFAssignmentGenerator(CNF const& cnf, ANNF const& annf) : cnf(cnf), annf(annf) {
    }

    RandomNNFAssignmentGenerator(RandomNNFAssignmentGenerator const& rag) = default;

    RandomNNFAssignmentGenerator& operator=(RandomNNFAssignmentGenerator const& rag) = default;

    inline CNF const& get_cnf() const { return cnf; }

    inline mpz_int get_mc() const { return annf.mc(NNF::ROOT); }

    template<typename PRNG>
    Cube operator()(PRNG & gen) {
        LitSet res;
        mpz_int const id = gen_random_int(gen, annf.mc(NNF::ROOT)) + 1;
        annf.get_solution(id, res);

        Cube c;
        c.m = res;
        c.nb_unsat = nb_unknown(cnf, res);

        return c;
    }
};

class RandomCubeAssignmentGenerator {
private:
    CNF const& cnf;
    //ANNF annf;
    std::vector<ANNF> annfs;
    dnf::Cubes cubes;
    mpz_int mc;

public:
    RandomCubeAssignmentGenerator(CNF const& cnf, NNF const& nnf, dnf::Cubes const& cubes) : cnf(cnf), annfs(cubes.size(), ANNF(nnf)), cubes(cubes) {
        mc = 0;

        #pragma omp parallel for
        for(std::size_t cid = 0; cid < this->cubes.size(); cid++) {
            annfs[cid].set_assumps(this->cubes[cid].lits);
            annfs[cid].annotate_mc();
            this->cubes[cid].mc = annfs[cid].mc(NNF::ROOT);

            #pragma omp critical
            {
                mc += this->cubes[cid].mc;
            }
        }

        //for(std::size_t cid = 0; cid < this->cubes.size(); cid++) {
        //    auto& launnf = annfs.emplace_back(nnf);
        //    launnf.set_assumps(this->cubes[cid].lits);
        //    launnf.annotate_mc();
        //    this->cubes[cid].mc = launnf.mc(NNF::ROOT);

        //    mc += this->cubes[cid].mc;
        //}
    }

    RandomCubeAssignmentGenerator(RandomCubeAssignmentGenerator const& rag) = default;

    RandomCubeAssignmentGenerator& operator=(RandomCubeAssignmentGenerator const& rag) = default;

    inline CNF const& get_cnf() const { return cnf; }

    inline mpz_int get_mc() const { return mc; }

    template<typename PRNG>
    Cube operator()(PRNG & gen) {
        LitSet res;
        mpz_int id = gen_random_int(gen, mc) + 1;

        //for(auto const& cube : cubes) {
        //    assert(id > 0);

        //    if(id <= cube.mc) {
        //        annf.set_assumps(cube.lits);
        //        annf.annotate_mc();
        //        annf.get_solution(id, res);

        //        for(Literal const l : cube.lits) {
        //            assert(res.contains(l));
        //        }

        //        Cube c;
        //        c.m = res;
        //        c.nb_unsat = nb_unknown(cnf, res);
        //        return c;
        //    }

        //    id -= cube.mc;
        //}

        for(std::size_t cid = 0; cid < cubes.size(); cid++) {
            assert(id > 0);

            if(id <= cubes[cid].mc) {
                annfs[cid].get_solution(id, res);

                for(Literal const l : cubes[cid].lits) {
                    assert(res.contains(l));
                }

                Cube c;
                c.m = res;
                c.nb_unsat = nb_unknown(cnf, res);
                return c;
            }

            id -= cubes[cid].mc;
        }

        assert(false && "Impossible state reached");
    }
};

template<typename G>
class MonteCarlo {
private:
    G generator;
    bool print;

public:
    MonteCarlo(G gen, bool print = false)
        : generator(gen), print(print) {
    }

    template<typename PRNG>
    std::vector<LitSet> run(std::size_t const N, PRNG & prng) {
        std::cout << "Sampling\n";
        std::cout << "UMC " << generator.get_mc() << "\n";

        std::size_t nb_tries = 0;
        std::size_t nb_success = 0;

        std::vector<LitSet> result;

        std::atomic<size_t> result_count(result.size());
        #pragma omp parallel
        {
            PRNG lprng;
            #pragma omp critical(init)
            {
                lprng = prng;
                prng.jump();
            }

            while(result_count.load() < N) {
                #pragma omp atomic
                nb_tries++;

                auto tmp = generator(lprng);

                if(tmp.nb_unsat == 0 && result_count.load() < N) {
                    #pragma omp critical(push)
                    {
                        if(result.size() < N) {
                            result.push_back(tmp.m);
                            result_count++;

                            if(print) {
                                std::cout << "s " << tmp.m << "\n";
                            }

                            nb_success++;
                        }
                    }
                }
            }
        }

        std::cout << "nb_tries " << nb_tries << "\n";
        std::cout << "nb_success " << nb_success << "\n";

        mpf_float amc = generator.get_mc();
        amc *= nb_success;
        amc /= nb_tries;

        std::cout << "AMC " << amc << "\n";

        return result;
    }
};

template<typename G>
class SplittingSampler {
private:
    G generator;

    std::size_t population_size = 100;
    double retention = 0.5;
    std::size_t nb_sweeps = 2;
    std::size_t nb_lits = 2;
    std::size_t maximum_unsat_clauses = 0;

    bool print;

    std::vector<Variable> vars;


public:
    SplittingSampler(G gen, bool print = true)
        : generator(gen), print(print) {

        for(int i = 1; i <= gen.get_cnf().nb_vars(); i++) {
            vars.emplace_back(i);
        }
    }

    inline void set_population_size(std::size_t const v) { population_size = v; }
    inline void set_retention(double const v) { retention = v; }
    inline void set_nb_sweeps(std::size_t const v) { nb_sweeps = v; }
    inline void set_nb_lits(std::size_t const v) { nb_lits = v; }
    inline void set_maximum_unsat_clauses(std::size_t const v) { maximum_unsat_clauses = v; }

    std::size_t filter(Cubes & cubes) {
        std::cout << "Splitting filtering\n";
        std::sort(cubes.begin(), cubes.end(), [](auto const& a, auto const& b) {
                    return a.nb_unsat < b.nb_unsat;
                });

        std::cout << "  " << cubes[0].nb_unsat << " <= " << cubes[cubes.size() - 1].nb_unsat << "\n";

        std::size_t const thresh = cubes[ceil(cubes.size() * retention)].nb_unsat;
        while(cubes[cubes.size() - 1].nb_unsat > thresh) {
            cubes.pop_back();
        }

        return thresh;
    }

    template<typename PRNG>
    void splitting_clone(Cubes & cubes, std::size_t const new_size, PRNG & prng) {
        std::cout << "Splitting cloning\n";
        std::shuffle(cubes.begin(), cubes.end(), prng);

        std::size_t ms = cubes.size();
        for(std::size_t i = 0; i < ms; i++) {
            std::size_t const Ri = (new_size / ms) + (i < (new_size % ms) ? 1 : 0);
            for(std::size_t j = 1; j < Ri; j++) {
                cubes.push_back(cubes[i]);
            }
        }

        assert(cubes.size() == new_size);
    }

    template<typename PRNG>
    void mutate_cube(std::size_t const thresh, Cube & cube, PRNG & prng) {
        std::size_t const nb_assignments = 1 << nb_lits;
        std::shuffle(vars.begin(), vars.end(), prng);

        assert(cube.nb_unsat == nb_unsat(generator.get_cnf(), cube.m));
        assert(cube.nb_unsat <= thresh);

        for(std::size_t i = 0; i + nb_lits <= vars.size(); i += nb_lits) {
            std::size_t best = cube.nb_unsat * 2;
            std::vector<std::vector<Literal> > valid;

            for(std::size_t assignment_id = 0; assignment_id < nb_assignments; assignment_id++) {
                std::size_t lassignment = assignment_id;

                std::vector<Literal> lits;
                for(std::size_t j = 0; j < nb_lits; j++) {
                    lits.emplace_back(vars[i + j], 2 * (lassignment & 1) - 1);
                    lassignment >>= 1;
                }

                Cube tmp = cube;
                std::size_t const nb = nb_unsat_flip(generator.get_cnf(), cube.m, cube.nb_unsat, lits);
                bool contains_all = true;
                for(Literal const l : lits) {
                    contains_all = contains_all && cube.m.contains(l);
                }
                assert(!contains_all || nb == cube.nb_unsat);

                assert(equals(tmp.m, cube.m));

                for(Literal const l : lits) {
                    tmp.m.erase(~l);
                    tmp.m.insert(l);
                }
                tmp.nb_unsat = nb_unsat(generator.get_cnf(), tmp.m);
                assert(tmp.nb_unsat == nb);

                best = std::min(best, nb);

                if(nb <= thresh) {
                    valid.push_back(lits);
                }
            }

            if(valid.size() == 0) {
                std::cout << cube.nb_unsat << " | " << thresh << "|" << best << "\n";
            }
            assert(valid.size() > 0);
        }
    }

    template<typename PRNG>
    void mutate_cube_single(std::size_t const thresh, Cube & cube, PRNG & prng) {
        assert(nb_lits == 1);

        bernoulli_distribution dist(0.5);

        std::shuffle(vars.begin(), vars.end(), prng);

        for(Variable const& v : vars) {
            Literal lp(v);
            std::size_t tmp = nb_flip_unsatisfied(generator.get_cnf(), cube.m, cube.nb_unsat, lp);

            if(tmp <= thresh && dist(prng)) {
                cube.m.erase(lp);
                cube.m.insert(~lp);
                cube.nb_unsat = tmp;
            }
        }
    }

    template<typename PRNG>
    void mutate(std::size_t const thresh, Cubes & cubes, PRNG & prng) {
        std::cout << "Splitting mutation\n";

        #pragma omp parallel for
        for(std::size_t cid = 0; cid < cubes.size(); cid++) {
            auto& cube = cubes[cid];

            PRNG lprng;
            #pragma omp critical
            {
                lprng = prng;
                prng.jump();
            }

            for(std::size_t sweep = 0; sweep < nb_sweeps; sweep++) {
                if(nb_lits == 1) {
                    mutate_cube_single(thresh, cube, lprng);
                }
                else {
                    mutate_cube(thresh, cube, lprng);
                }
            }
        }
        std::cout << "End splitting mutation\n";
    }

    template<typename PRNG>
    std::vector<LitSet> run(std::size_t const N, PRNG & prng) {
        std::cout << "Sampling\n";

        std::vector<LitSet> result;

        while(result.size() < N) {
            Cubes population;
            std::size_t nb_tries = 0;
            std::atomic<size_t> population_count(population.size());
            #pragma omp parallel
            {
                PRNG lprng;
                #pragma omp critical(init)
                {
                    lprng = prng;
                    prng.jump();
                }

                while(population_count.load() < population_size) {
                    #pragma omp atomic
                    nb_tries++;

                    auto tmp = generator(lprng);

                    if((maximum_unsat_clauses == 0 || maximum_unsat_clauses >= tmp.nb_unsat) && population_count.load() < population_size) {
                        #pragma omp critical(push)
                        {
                            if(population.size() < population_size) {
                                population.push_back(tmp);
                                population_count++;
                            }
                        }
                    }
                }
            }
            std::cout << "Population init nb tries: " << nb_tries << "\n";
            assert(population.size() == population_size);

            while(true) {
                std::size_t const thresh = filter(population);
                std::cout << thresh << " >> " << population.size() << "\n";
                assert(population.size() > 0);

                if(population[0].nb_unsat == 0) {
                    for(std::size_t j = 0; j < population.size() && result.size() < N && population[j].nb_unsat == 0; j++) {
                        if(print) {
                            std::cout << "s " << population[j].m << "\n";
                        }
                        result.push_back(population[j].m);
                    }
                    break;
                }

                splitting_clone(population, population_size, prng);
                mutate(thresh, population, prng);
            }
        }

        return result;
    }
};

template<typename G>
class MetropolisHastings {
public:
    struct Cube {
        LitSet lits;
        BitSet<std::size_t> unsat_ids;
    };

    using Cubes = std::vector<Cube>;

private:
    G generator;
    bool print;

    double burnin_pure_random_walk_probability = 0.5;
    double burnin_random_walk_probability = 0.8;
    double burnin_beta = 2;
    std::size_t burnin_nb_steps = 1000;

    double descent_pure_random_walk_probability = 0.5;
    double descent_random_walk_probability = 0.8;
    double descent_beta = 2;
    std::size_t descent_max_nb_steps = 1000;

    double pure_random_walk_probability = 0.5;
    double random_walk_probability = 0.8;
    double beta = 2;
    double restart_probability = 0.01;

    template<typename PRNG>
    Cube generate_cube(PRNG & prng){
        Cube res;
        res.lits = generator(prng).m;
        res.unsat_ids = unsat_clause_ids(generator.get_cnf(), res.lits);

        return res;
    }

    template<typename PRNG>
    std::size_t random_element(BitSet<std::size_t> const& b, PRNG & prng) {
        assert(b.size() > 0);
        std::uniform_int_distribution<> distrib(0, b.size() - 1);

        std::size_t element_id = distrib(prng);
        auto it = b.begin();
        while(it != b.end()) {
            if(element_id == 0) {
                return *it;
            }
            element_id--;
            ++it;
        }

        std::cout << "Should have found an element but didn't?: " << b.size() << "\n";
        assert(false && "impossible state reached in MetropolisHastings::random_element");
    }

    template<typename PRNG>
    Literal random_element(Clause const& c, PRNG & prng) {
        std::uniform_int_distribution<> distrib(0, c.size() - 1);

        std::size_t element_id = distrib(prng);
        return c[element_id];
    }

    double compute_flip_probability(Cube const& src, Literal & lp) {
        if(!src.lits.contains(lp)) {
            lp = ~lp;
        }
        if(src.unsat_ids.size() == 0) {
            return 0;
        }
        Literal const ln = ~lp;

        CNF const& cnf = generator.get_cnf();

        double pres = 0;

        for(std::size_t const cid : src.unsat_ids) {
            if(cnf.is_active(cid)) {
                if(cnf.clause(cid).contains(ln)) {
                    pres += (1.0 / cnf.clause(cid).size());
                }
            }
        }

        pres /= src.unsat_ids.size();

        return pres;
    }

    enum class Phase {
        Descent = 1,
        Burnin,
        Sampling
    };

public:
    MetropolisHastings(G gen, bool print = false)
        : generator(gen), print(print) {
    }

    inline void set_descent_pure_random_walk_probability(double const v) { descent_pure_random_walk_probability = v; }
    inline void set_descent_random_walk_probability(double const v) { descent_random_walk_probability = v; }
    inline void set_descent_beta(double const v) { descent_beta = v; }
    inline void set_descent_max_nb_steps(std::size_t const v) { descent_max_nb_steps = v; }

    inline void set_burnin_pure_random_walk_probability(double const v) { burnin_pure_random_walk_probability = v; }
    inline void set_burnin_random_walk_probability(double const v) { burnin_random_walk_probability = v; }
    inline void set_burnin_beta(double const v) { burnin_beta = v; }
    inline void set_burnin_nb_steps(std::size_t const v) { burnin_nb_steps = v; }

    inline void set_pure_random_walk_probability(double const v) { pure_random_walk_probability = v; }
    inline void set_random_walk_probability(double const v) { random_walk_probability = v; }
    inline void set_beta(double const v) { beta = v; }
    inline void set_restart_probability(double const v) { restart_probability = v; }

    template<typename PRNG>
    std::vector<LitSet> run(std::size_t const N, PRNG & prng) {
        std::cout << "Sampling\n";
        std::cout << "UMC " << generator.get_mc() << "\n";

        std::vector<LitSet> result;

        std::uniform_int_distribution random_variable_d(1, generator.get_cnf().nb_vars());
        std::bernoulli_distribution restart_d(restart_probability);

        while(result.size() < N) {
            Cube cube = generate_cube(prng);

            Phase phase = Phase::Descent;
            double l_pure_random_walk_probability = descent_pure_random_walk_probability;
            double l_random_walk_probability = descent_random_walk_probability;
            double l_beta = descent_beta;

            std::bernoulli_distribution random_walk_d(l_random_walk_probability);
            std::bernoulli_distribution pure_random_walk_d(l_pure_random_walk_probability);

            std::size_t burnin_steps_left = burnin_nb_steps;
            std::size_t descent_steps_left = descent_max_nb_steps;

            if(cube.unsat_ids.size() == 0) {
                result.push_back(cube.lits);
                continue;
            }
            //std::cout << "init: " << cube.unsat_ids.size() << "\n";

            while(result.size() < N) {

                if(cube.unsat_ids.size() > 0 && pure_random_walk_d(prng)) {
                    assert(cube.unsat_ids.size() > 0);

                    std::size_t const cid = random_element(cube.unsat_ids, prng);
                    Literal lit = random_element(generator.get_cnf().clause(cid), prng);

                    cube.unsat_ids = unsat_clause_ids_flip(generator.get_cnf(), cube.lits, cube.unsat_ids, lit);
                    cube.lits.erase(lit);
                    cube.lits.insert(~lit);
                }
                else {
                    Literal lit(0);

                    if(cube.unsat_ids.size() > 0 && random_walk_d(prng)) {
                        std::size_t const cid = random_element(cube.unsat_ids, prng);
                        lit = random_element(generator.get_cnf().clause(cid), prng);
                    }
                    else {
                        lit = Literal(random_variable_d(prng), 0);
                    }

                    double forward_prob = 0;
                    forward_prob += compute_flip_probability(cube, lit) * random_walk_probability;
                    forward_prob += (1 - random_walk_probability) * (1.0 / generator.get_cnf().nb_vars());

                    Cube flipped = cube;
                    flipped.unsat_ids = unsat_clause_ids_flip(generator.get_cnf(), cube.lits, cube.unsat_ids, lit);
                    flipped.lits.erase(lit);
                    flipped.lits.insert(~lit);

                    double backward_prob = 0;
                    backward_prob += compute_flip_probability(flipped, lit) * random_walk_probability;
                    backward_prob += (1 - random_walk_probability) * (1.0 / generator.get_cnf().nb_vars());

                    double const delta_unsat = static_cast<double>(flipped.unsat_ids.size()) - static_cast<double>(cube.unsat_ids.size());

                    double acceptance_p = exp(-l_beta * delta_unsat) * backward_prob / forward_prob;
                    std::bernoulli_distribution bd(std::min(1.0, acceptance_p));

                    if(bd(prng)) {
                        cube = flipped;
                    }
                }

                if(phase == Phase::Descent) {
                    //std::cout << "descent " << descent_steps_left << " :: " << cube.unsat_ids.size() << "\n";
                }
                else if(phase == Phase::Burnin) {
                    //std::cout << "burnin :: " << cube.unsat_ids.size() << "\n";
                }
                else if(phase == Phase::Sampling) {
                    //std::cout << "sampling :: " << cube.unsat_ids.size() << "\n";
                }
                else {
                    //std::cout << "Impossible phase: " << static_cast<int>(phase) << "\n";
                    assert(false && "Impossible state reached in MetropolisHastings::run");
                }

                if(phase == Phase::Descent && descent_steps_left > 0 && cube.unsat_ids.size() > 0) {
                    descent_steps_left--;
                }
                else if(phase == Phase::Descent && cube.unsat_ids.size() == 0) {
                    l_pure_random_walk_probability = burnin_pure_random_walk_probability;
                    l_random_walk_probability = burnin_random_walk_probability;
                    l_beta = burnin_beta;

                    random_walk_d = std::bernoulli_distribution(l_random_walk_probability);
                    pure_random_walk_d = std::bernoulli_distribution(l_pure_random_walk_probability);

                    burnin_steps_left = burnin_nb_steps;
                    phase = Phase::Burnin;

                    if(burnin_steps_left == 0) {
                        l_pure_random_walk_probability = pure_random_walk_probability;
                        l_random_walk_probability = random_walk_probability;
                        l_beta = beta;

                        random_walk_d = std::bernoulli_distribution(l_random_walk_probability);
                        pure_random_walk_d = std::bernoulli_distribution(l_pure_random_walk_probability);
                        phase = Phase::Sampling;
                    }
                }
                else if(phase == Phase::Descent && descent_steps_left <= 0 && cube.unsat_ids.size() > 0) {
                    //std::cout << "Descent failed, restarting\n";
                    break;
                }
                else if(phase == Phase::Burnin) {
                    if(burnin_steps_left > 0) {
                        burnin_steps_left--;
                    }

                    if(burnin_steps_left == 0) {
                        l_pure_random_walk_probability = pure_random_walk_probability;
                        l_random_walk_probability = random_walk_probability;
                        l_beta = beta;

                        random_walk_d = std::bernoulli_distribution(l_random_walk_probability);
                        pure_random_walk_d = std::bernoulli_distribution(l_pure_random_walk_probability);
                        phase = Phase::Sampling;
                    }
                }
                else if(phase == Phase::Sampling && cube.unsat_ids.size() == 0) {
                    //std::cout << "found sample: " << cube.unsat_ids.size() << "\n";
                    result.push_back(cube.lits);

                    if(print) {
                        std::cout << "s " << cube.lits << "\n";
                    }

                }

                if(phase == Phase::Sampling && restart_d(prng)) {
                    //std::cout << "Triggered a restart\n";
                    break;
                }

            }
        }

        //std::cout << "nb samples generated: " << result.size() << "\n";

        return result;
    }
};

};

#endif

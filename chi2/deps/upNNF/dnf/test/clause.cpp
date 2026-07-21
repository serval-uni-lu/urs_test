#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CLAUSE_TEST
#include <boost/test/unit_test.hpp>

#include <set>
#include <random>
#include <algorithm>

#include "var.hpp"
#include "cnf.hpp"

BOOST_AUTO_TEST_CASE(creation_test)
{
    int const nb_var = 50;
    std::size_t const N = 1000;
    std::size_t const k = 100;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> kd(1, k);
    std::uniform_int_distribution<> sd(0, 1);

    for(std::size_t nb = 0; nb < N; nb++) {
        std::set<Literal> data;
        Clause cl;

        std::size_t const lk = kd(gen);
        for(std::size_t ki = 0; ki < lk; ki++) {
            Literal l(vard(gen) * (sd(gen) * 2 - 1));

            data.insert(l);
            cl.push(l);
        }

        for(Literal const& l : data) {
            BOOST_CHECK(cl.contains(l));
        }

        for(Literal const& l : cl) {
            BOOST_CHECK(data.find(l) != data.end());
        }

        for(std::size_t i = 1; i < cl.size(); i++) {
            BOOST_CHECK(cl[i - 1] < cl[i]);
        }

        std::size_t const lk2 = kd(gen);
        Literal l(vard(gen) * (sd(gen) * 2 - 1));
        cl.push(l);
        std::size_t const cl_size = cl.size();
        for(std::size_t ki = 0; ki < lk2; ki++) {
            cl.push(l);
            BOOST_CHECK(cl.size() == cl_size);
            BOOST_CHECK(cl.contains(l));
        }

        std::size_t const lk3 = kd(gen);
        for(std::size_t ki = 0; ki < lk3; ki++) {
            cl.remove(l);
            BOOST_CHECK(cl.size() == cl_size - 1);
            BOOST_CHECK(! cl.contains(l));
        }
    }
}

BOOST_AUTO_TEST_CASE(creation2_test)
{
    int const nb_var = 50;
    std::size_t const N = 1000;
    std::size_t const k = 100;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> kd(1, k);
    std::uniform_int_distribution<> sd(0, 1);

    for(std::size_t nb = 0; nb < N; nb++) {
        Clause cl;

        std::size_t const lk = kd(gen);
        Literal l(vard(gen) * (sd(gen) * 2 - 1));
        for(std::size_t ki = 0; ki < lk; ki++) {
            cl.push(l);
        }

        BOOST_CHECK(cl.size() == 1);
    }
}

Clause build_clause(std::vector<int> const& v) {
    Clause c;
    for(int i : v) {
        c.push(Literal(i));
    }
    return c;
}

LitSet build_litset(std::vector<int> const& v) {
    LitSet res;
    for(int i : v) {
        res.insert(Literal(i));
    }
    return res;
}

BOOST_AUTO_TEST_CASE(nb_unsat_test)
{
    CNF cnf(10);

    cnf.add_clause(build_clause({1, 2, 3}));
    cnf.add_clause(build_clause({2, 3, 4}));
    cnf.add_clause(build_clause({4, 5, 6}));
    cnf.add_clause(build_clause({7, 8, 9}));
    cnf.add_clause(build_clause({8, 9, 10}));

    {
        LitSet cube = build_litset({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        BOOST_CHECK(0 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, -2, -3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(5 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({1, -2, -3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(4 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, 2, -3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(3 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(3 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, -2, -3, -4, -5, -6, -7, -8, -9, 10});
        BOOST_CHECK(4 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({1, 2, -3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(3 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, 2, 3, -4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(3 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, 4, -5, -6, -7, -8, -9, -10});
        BOOST_CHECK(2 == nb_unsat(cnf, cube));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, 4, -5, -6, -7, 8, -9, -10});
        BOOST_CHECK(0 == nb_unsat(cnf, cube));
    }

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> sd(0, 1);

    for(std::size_t i = 0; i < 1000; i++) {
        LitSet cube;
        for(int j = 1; j <= cnf.nb_vars(); j++) {
            Literal l(j * (sd(gen) * 2 - 1));

            cube.insert(l);
        }

        std::size_t const res = nb_unsat(cnf, cube);
        //BOOST_CHECK(0 <= res);
        BOOST_CHECK(res <= cnf.nb_active_clauses());
    }
}

BOOST_AUTO_TEST_CASE(nb_unsat_flip_test)
{
    int constexpr nb_vars = 100;
    int constexpr nb_clauses = 1000;
    int constexpr k = 10;
    int constexpr N = 100;
    int constexpr Nl = 100;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> sd(0, 1);
    std::uniform_int_distribution<> vard(1, nb_vars);
    std::uniform_int_distribution<> kd(1, k);
    std::uniform_int_distribution<> n_flip_d(1, 10);

    std::vector<Literal> lits;
    for(int i = 1; i <= nb_vars; i++) {
        lits.emplace_back(i);
    }

    for(int i = 0; i < N; i++) {
        CNF cnf(nb_vars);
        for(int j = 0; j < nb_clauses; j++) {
            int const clause_size = kd(gen);
            Clause tmp;
            for(int ki = 0; ki < clause_size; ki++) {
                Literal l(vard(gen) * (sd(gen) * 2 - 1));
                tmp.push(l);
            }
            cnf.add_clause(tmp);
        }

        for(int ii = 0; ii < Nl; ii++) {
            LitSet cube;

            for(int j = 1; j <= nb_vars; j++) {
                Literal l(j * (sd(gen) * 2 - 1));
                cube.insert(l);
            }

            std::size_t const n_unsat = nb_unsat(cnf, cube);
            BOOST_CHECK(n_unsat <= cnf.nb_active_clauses());

            std::shuffle(lits.begin(), lits.end(), gen);

            int const nb_flips = n_flip_d(gen);
            std::vector<Literal> flip_lit;
            for(int i = 0; i < nb_flips; i++) {
                if(sd(gen) == 0) {
                    flip_lit.push_back(lits[i]);
                }
                else {
                    flip_lit.push_back(~lits[i]);
                }
            }

            LitSet old_cube = cube;

            std::size_t const n_flip_unsat = nb_unsat_flip(cnf, cube, n_unsat, flip_lit);

            BOOST_CHECK(equals(old_cube, cube));

            for(Literal const l : flip_lit) {
                //BOOST_CHECK(cube.contains(l));
                cube.erase(~l);
                cube.insert(l);
            }

            BOOST_CHECK(n_flip_unsat == nb_unsat(cnf, cube));
        }
    }
}

BOOST_AUTO_TEST_CASE(nb_flip_unsatisfied_test)
{
    int constexpr nb_vars = 100;
    int constexpr nb_clauses = 1000;
    int constexpr k = 10;
    int constexpr N = 100;
    int constexpr Nl = 100;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> sd(0, 1);
    std::uniform_int_distribution<> vard(1, nb_vars);
    std::uniform_int_distribution<> kd(1, k);
    std::uniform_int_distribution<> n_flip_d(1, 10);

    std::vector<Literal> lits;
    for(int i = 1; i <= nb_vars; i++) {
        lits.emplace_back(i);
    }

    for(int i = 0; i < N; i++) {
        CNF cnf(nb_vars);
        for(int j = 0; j < nb_clauses; j++) {
            int const clause_size = kd(gen);
            Clause tmp;
            for(int ki = 0; ki < clause_size; ki++) {
                Literal l(vard(gen) * (sd(gen) * 2 - 1));
                tmp.push(l);
            }
            cnf.add_clause(tmp);
        }

        for(int ii = 0; ii < Nl; ii++) {
            LitSet cube;

            for(int j = 1; j <= nb_vars; j++) {
                Literal l(j * (sd(gen) * 2 - 1));
                cube.insert(l);
            }

            std::size_t const n_unsat = nb_unsat(cnf, cube);
            BOOST_CHECK(n_unsat <= cnf.nb_active_clauses());

            std::shuffle(lits.begin(), lits.end(), gen);

            LitSet old_cube = cube;

            for(std::size_t i = 0; i < lits.size(); i++) {
                Literal lp = lits[i];
                if(!cube.contains(lp)) {
                    lp = ~lp;
                }

                BOOST_CHECK(cube.contains(lp));

                auto const nn_unsat = nb_flip_unsatisfied(cnf, cube, n_unsat, lp);

                BOOST_CHECK(cube.contains(lp));

                LitSet new_cube = cube;
                new_cube.erase(lp);
                new_cube.insert(~lp);

                BOOST_CHECK(equals(cube, old_cube));

                BOOST_CHECK(cube.contains(lp));
                BOOST_CHECK(!cube.contains(~lp));
                BOOST_CHECK(new_cube.contains(~lp));
                BOOST_CHECK(!new_cube.contains(lp));

                BOOST_CHECK(nn_unsat == nb_unsat(cnf, new_cube));
            }

        }
    }
}

BOOST_AUTO_TEST_CASE(unsat_clause_ids_test)
{
    CNF cnf(10);

    cnf.add_clause(build_clause({1, 2, 3}));
    cnf.add_clause(build_clause({2, 3, 4}));
    cnf.add_clause(build_clause({4, 5, 6}));
    cnf.add_clause(build_clause({7, 8, 9}));
    cnf.add_clause(build_clause({8, 9, 10}));

    {
        LitSet cube = build_litset({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(!vec.contains(2));
        BOOST_CHECK(!vec.contains(3));
        BOOST_CHECK(!vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, -2, -3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(vec.contains(0));
        BOOST_CHECK(vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({1, -2, -3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, 2, -3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, -2, -3, -4, -5, -6, -7, -8, -9, 10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(vec.contains(0));
        BOOST_CHECK(vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(!vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({1, 2, -3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, 2, 3, -4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, 4, -5, -6, -7, -8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(!vec.contains(2));
        BOOST_CHECK(vec.contains(3));
        BOOST_CHECK(vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    {
        LitSet cube = build_litset({-1, -2, 3, 4, -5, -6, -7, 8, -9, -10});
        auto vec = unsat_clause_ids(cnf, cube);
        BOOST_CHECK(!vec.contains(0));
        BOOST_CHECK(!vec.contains(1));
        BOOST_CHECK(!vec.contains(2));
        BOOST_CHECK(!vec.contains(3));
        BOOST_CHECK(!vec.contains(4));
        BOOST_CHECK(!vec.contains(5));
    }

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> sd(0, 1);

    for(std::size_t i = 0; i < 1000; i++) {
        LitSet cube;
        for(int j = 1; j <= cnf.nb_vars(); j++) {
            Literal l(j * (sd(gen) * 2 - 1));

            cube.insert(l);
        }

        auto const vec = unsat_clause_ids(cnf, cube);

        for(std::size_t id = 0; id < cnf.nb_clauses(); id++) {
            if(cnf.is_active(id)) {
                if(clause_is_sat(cnf.clause(id), cube)) {
                    BOOST_CHECK(!vec.contains(id));
                }
                else {
                    BOOST_CHECK(vec.contains(id));
                }
            }
            else {
                BOOST_CHECK(!vec.contains(id));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(unsat_clause_ids_flip_test)
{
    int constexpr nb_vars = 100;
    int constexpr nb_clauses = 1000;
    int constexpr k = 10;
    int constexpr N = 100;
    int constexpr Nl = 10;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> sd(0, 1);
    std::uniform_int_distribution<> vard(1, nb_vars);
    std::uniform_int_distribution<> kd(1, k);
    std::uniform_int_distribution<> n_flip_d(1, 10);

    std::vector<Literal> lits;
    for(int i = 1; i <= nb_vars; i++) {
        lits.emplace_back(i);
    }

    for(int i = 0; i < N; i++) {
        CNF cnf(nb_vars);
        for(int j = 0; j < nb_clauses; j++) {
            int const clause_size = kd(gen);
            Clause tmp;
            for(int ki = 0; ki < clause_size; ki++) {
                Literal l(vard(gen) * (sd(gen) * 2 - 1));
                tmp.push(l);
            }
            cnf.add_clause(tmp);
        }

        for(int ii = 0; ii < Nl; ii++) {
            LitSet cube;

            for(int j = 1; j <= nb_vars; j++) {
                Literal l(j * (sd(gen) * 2 - 1));
                cube.insert(l);
            }

            auto const unsat_ids = unsat_clause_ids(cnf, cube);

            std::shuffle(lits.begin(), lits.end(), gen);

            LitSet old_cube = cube;

            for(std::size_t i = 0; i < lits.size(); i++) {
                Literal lp = lits[i];
                if(!cube.contains(lp)) {
                    lp = ~lp;
                }

                BOOST_CHECK(cube.contains(lp));
                auto unsat_ids_flip = unsat_clause_ids_flip(cnf, cube, unsat_ids, lp);

                BOOST_CHECK(cube.contains(lp));

                LitSet new_cube = cube;
                new_cube.erase(lp);
                new_cube.insert(~lp);

                BOOST_CHECK(equals(cube, old_cube));

                BOOST_CHECK(cube.contains(lp));
                BOOST_CHECK(!cube.contains(~lp));
                BOOST_CHECK(new_cube.contains(~lp));
                BOOST_CHECK(!new_cube.contains(lp));

                auto flipped = unsat_clause_ids(cnf, new_cube);
                BOOST_CHECK(equals(unsat_ids_flip, flipped));

                //for(std::size_t cid = 0; cid < cnf.nb_clauses(); cid++) {
                //    if(cnf.is_active(cid)) {
                //        if(clause_is_sat(cnf.clause(cid), new_cube)) {
                //            BOOST_CHECK(unsat_ids_flip.contains(cid));
                //            BOOST_CHECK(flipped.contains(cid));
                //        }
                //        else {
                //            BOOST_CHECK(! unsat_ids_flip.contains(cid));
                //            BOOST_CHECK(! flipped.contains(cid));
                //        }
                //    }
                //}
            }
        }
    }
}

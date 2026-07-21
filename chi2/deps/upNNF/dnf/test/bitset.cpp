#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BITSET_TEST

#include <set>
#include <random>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "var.hpp"
#include "bitset.hpp"

BOOST_AUTO_TEST_CASE(bitset_literal_test)
{
    std::size_t const N = 1000;
    std::size_t const k = 100;
    int const nb_var = 50;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> sd(0, 1);
    std::uniform_int_distribution<> kd(1, k);

    for(std::size_t i = 0; i < N; i++) {
        BitSet<Literal> bitset;
        std::set<Literal> base_set;

        BOOST_CHECK(bitset.size() == 0);
        BOOST_CHECK(base_set.size() == 0);

        std::size_t const lk = kd(gen);
        for(std::size_t ki = 0; ki < lk; ki++) {
            Literal l(vard(gen) * (sd(gen) * 2 - 1));

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.insert(l);
            base_set.insert(l);

            BOOST_CHECK(bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(!contains) {
                BOOST_CHECK((old_size + 1) == bitset.size());

                if(bitset.size() == 1) {
                    BOOST_CHECK(l == *(bitset.begin()));
                }
            }
        }

        for(Literal const l : bitset) {
            BOOST_CHECK(base_set.find(l) != base_set.end());
        }

        for(Literal const& l : base_set) {
            BOOST_CHECK(bitset.contains(l));
        }

        for(std::size_t ki = 0; ki < lk; ki++) {
            Literal l(vard(gen) * (sd(gen) * 2 - 1));

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.erase(l);
            base_set.erase(l);

            BOOST_CHECK(! bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(contains) {
                BOOST_CHECK((old_size - 1) == bitset.size());
            }
        }

        std::size_t old_size = bitset.size();
        for(Literal const& l : base_set) {
            bool const contains = bitset.contains(l);

            bitset.erase(l);

            if(contains) {
                BOOST_CHECK(! bitset.contains(l));
                old_size -= 1;
                BOOST_CHECK(old_size == bitset.size());
            }
            else {
                BOOST_CHECK(bitset.size() == old_size);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(bitset_variable_test)
{
    std::size_t const N = 1000;
    std::size_t const k = 100;
    int const nb_var = 50;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> kd(1, k);

    for(std::size_t i = 0; i < N; i++) {
        BitSet<Variable> bitset;
        std::set<Variable> base_set;

        BOOST_CHECK(bitset.size() == 0);
        BOOST_CHECK(base_set.size() == 0);

        std::size_t const lk = kd(gen);
        for(std::size_t ki = 0; ki < lk; ki++) {
            Variable l(vard(gen));

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.insert(l);
            base_set.insert(l);

            BOOST_CHECK(bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(!contains) {
                BOOST_CHECK((old_size + 1) == bitset.size());

                if(bitset.size() == 1) {
                    BOOST_CHECK(l == *(bitset.begin()));
                }
            }
        }

        for(Variable const l : bitset) {
            BOOST_CHECK(base_set.find(l) != base_set.end());
        }

        for(Variable const& l : base_set) {
            BOOST_CHECK(bitset.contains(l));
        }

        for(std::size_t ki = 0; ki < lk; ki++) {
            Variable l(vard(gen));

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.erase(l);
            base_set.erase(l);

            BOOST_CHECK(! bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(contains) {
                BOOST_CHECK((old_size - 1) == bitset.size());
            }
        }

        std::size_t old_size = bitset.size();
        for(Variable const& l : base_set) {
            bool const contains = bitset.contains(l);

            bitset.erase(l);

            if(contains) {
                BOOST_CHECK(! bitset.contains(l));
                old_size -= 1;
                BOOST_CHECK(old_size == bitset.size());
            }
            else {
                BOOST_CHECK(bitset.size() == old_size);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(bitset_std_size_t_test)
{
    std::size_t const N = 1000;
    std::size_t const k = 100;
    int const nb_var = 50;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> kd(1, k);

    for(std::size_t i = 0; i < N; i++) {
        BitSet<std::size_t> bitset;
        std::set<std::size_t> base_set;

        BOOST_CHECK(bitset.size() == 0);
        BOOST_CHECK(base_set.size() == 0);

        std::size_t const lk = kd(gen);
        for(std::size_t ki = 0; ki < lk; ki++) {
            std::size_t l = vard(gen);

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.insert(l);
            base_set.insert(l);

            BOOST_CHECK(bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(!contains) {
                BOOST_CHECK((old_size + 1) == bitset.size());

                if(bitset.size() == 1) {
                    BOOST_CHECK(l == *(bitset.begin()));
                }
            }
        }

        for(std::size_t const l : bitset) {
            BOOST_CHECK(base_set.find(l) != base_set.end());
        }

        for(std::size_t const& l : base_set) {
            BOOST_CHECK(bitset.contains(l));
        }

        for(std::size_t ki = 0; ki < lk; ki++) {
            std::size_t l = vard(gen);

            BOOST_CHECK(bitset.size() == base_set.size());
            std::size_t const old_size = base_set.size();

            bool const contains = bitset.contains(l);
            BOOST_CHECK(contains == (base_set.find(l) != base_set.end()));

            bitset.erase(l);
            base_set.erase(l);

            BOOST_CHECK(! bitset.contains(l));
            BOOST_CHECK(bitset.size() == base_set.size());

            if(contains) {
                BOOST_CHECK((old_size - 1) == bitset.size());
            }
        }

        std::size_t old_size = bitset.size();
        for(std::size_t const& l : base_set) {
            bool const contains = bitset.contains(l);

            bitset.erase(l);

            if(contains) {
                BOOST_CHECK(! bitset.contains(l));
                old_size -= 1;
                BOOST_CHECK(old_size == bitset.size());
            }
            else {
                BOOST_CHECK(bitset.size() == old_size);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(bitset_std_size_t_intersection_test)
{
    std::size_t const N = 1000;
    std::size_t const k = 100;
    int const nb_var = 50;

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> vard(1, nb_var);
    std::uniform_int_distribution<> kd(1, k);

    for(std::size_t i = 0; i < N; i++) {
        BitSet<std::size_t> bitset1;
        BitSet<std::size_t> bitset2;

        std::size_t const lk1 = kd(gen);
        for(std::size_t ki = 0; ki < lk1; ki++) {
            std::size_t l = vard(gen);

            bitset1.insert(l);
        }

        std::size_t const lk2 = kd(gen);
        for(std::size_t ki = 0; ki < lk2; ki++) {
            std::size_t l = vard(gen);

            bitset2.insert(l);
        }

        auto intersection = bitset1;
        intersection.intersect(bitset2);

        BitSet<std::size_t> inter_b;

        for(std::size_t const i : bitset1) {
            BOOST_CHECK((! bitset2.contains(i)) || intersection.contains(i));

            if(bitset2.contains(i)) {
                inter_b.insert(i);
            }
        }

        for(std::size_t const i : bitset2) {
            BOOST_CHECK((! bitset1.contains(i)) || (intersection.contains(i) && inter_b.contains(i)));
        }

        for(auto const i : inter_b) {
            BOOST_CHECK(intersection.contains(i));
            BOOST_CHECK(bitset1.contains(i));
            BOOST_CHECK(bitset2.contains(i));
        }

        for(auto const i : intersection) {
            BOOST_CHECK(inter_b.contains(i));
            BOOST_CHECK(bitset1.contains(i));
            BOOST_CHECK(bitset2.contains(i));
        }
    }
}

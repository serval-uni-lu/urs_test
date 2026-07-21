#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LITSET_TEST
#include <boost/test/unit_test.hpp>

#include "var.hpp"

BOOST_AUTO_TEST_CASE(creation_test)
{
    int const n_vars = 50;
    LitSet ls;

    for(int i = 1; i <= n_vars; i++) {
        Literal ln(-1 * i);
        Literal lp(i);

        BOOST_CHECK(! ls.contains(ln));
        BOOST_CHECK(! ls.contains(lp));

        ls.insert(ln);

        for(int j = 1; j <= 2 * n_vars; j++) {
            Literal lnj(-1 * j);
            Literal lpj(j);

            if(j == i) {
                BOOST_CHECK(ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
            else {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
        }

        ls.erase(ln);
        ls.insert(lp);

        for(int j = 1; j <= 2 * n_vars; j++) {
            Literal lnj(-1 * j);
            Literal lpj(j);

            if(j == i) {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(ls.contains(lpj));
            }
            else {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
        }

        ls.erase(lp);
    }
}

BOOST_AUTO_TEST_CASE(creation2_test)
{
    int const n_vars = 50;
    LitSet ls;

    for(int i = 1; i <= n_vars; i++) {
        Literal ln(-1 * i);
        Literal lp(i);

        BOOST_CHECK(! ls.contains(ln));
        BOOST_CHECK(! ls.contains(lp));

        ls.insert(ln);

        for(int j = 1; j <= 2 * n_vars; j++) {
            Literal lnj(-1 * j);
            Literal lpj(j);

            if(j == i) {
                BOOST_CHECK(ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
            else {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
        }

        ls.erase(ln);
        ls.insert(lp);

        for(int j = 1; j <= 2 * n_vars; j++) {
            Literal lnj(-1 * j);
            Literal lpj(j);

            if(j == i) {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(ls.contains(lpj));
            }
            else {
                BOOST_CHECK(! ls.contains(lnj));
                BOOST_CHECK(! ls.contains(lpj));
            }
        }

        ls.erase(lp);
    }
}

// BOOST_AUTO_TEST_CASE(resize_test)
// {
//     int const n_vars = 50;
//     LitSet ls;
// 
//     Literal l0(1);
//     Literal l1(-1);
//     Literal l2(50);
//     Literal l3(-50);
// 
//     Literal l4(30);
//     Literal l5(-25);
// 
//     Literal l6(51);
//     Literal l7(-51);
// 
//     BOOST_CHECK(! ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(! ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(! ls.contains(l4));
//     BOOST_CHECK(! ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.insert(l0);
//     ls.insert(l1);
//     ls.insert(l2);
//     ls.insert(l3);
//     ls.insert(l4);
//     ls.insert(l5);
// 
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.resize(n_vars * 4);
// 
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     for(int i = n_vars + 1; i <= static_cast<int>(n_vars * 2); i++) {
//         Literal ln(-1 * i);
//         Literal lp(i);
//         BOOST_CHECK(! ls.contains(ln));
//         BOOST_CHECK(! ls.contains(lp));
//     }
// 
//     ls.erase(l1);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.erase(l3);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.erase(l2);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(! ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// }
// 
// BOOST_AUTO_TEST_CASE(resize2_test)
// {
//     int const n_vars = 50;
//     LitSet ls;
// 
//     Literal l0(1);
//     Literal l1(-1);
//     Literal l2(50);
//     Literal l3(-50);
// 
//     Literal l4(30);
//     Literal l5(-25);
// 
//     Literal l6(51);
//     Literal l7(-51);
// 
//     BOOST_CHECK(! ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(! ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(! ls.contains(l4));
//     BOOST_CHECK(! ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.insert(l0);
//     ls.insert(l1);
//     ls.insert(l2);
//     ls.insert(l3);
//     ls.insert(l4);
//     ls.insert(l5);
// 
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.resize(n_vars * 4);
// 
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     for(int i = n_vars + 1; i <= static_cast<int>(n_vars * 2); i++) {
//         Literal ln(-1 * i);
//         Literal lp(i);
//         BOOST_CHECK(! ls.contains(ln));
//         BOOST_CHECK(! ls.contains(lp));
//     }
// 
//     ls.erase(l1);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.erase(l3);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// 
//     ls.erase(l2);
//     BOOST_CHECK(ls.contains(l0));
//     BOOST_CHECK(! ls.contains(l1));
//     BOOST_CHECK(! ls.contains(l2));
//     BOOST_CHECK(! ls.contains(l3));
//     BOOST_CHECK(ls.contains(l4));
//     BOOST_CHECK(ls.contains(l5));
//     BOOST_CHECK(! ls.contains(~l4));
//     BOOST_CHECK(! ls.contains(~l5));
//     BOOST_CHECK(! ls.contains(l6));
//     BOOST_CHECK(! ls.contains(l7));
// }

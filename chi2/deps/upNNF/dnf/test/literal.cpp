#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LITERAL_TEST
#include <boost/test/unit_test.hpp>

#include "var.hpp"

BOOST_AUTO_TEST_CASE(creation_test)
{
    Literal l0(10);
    Literal l1(0);
    Literal l2(1);

    Literal l3(-10);
    Literal l4(-1);

    BOOST_CHECK(l0.get() == 18);
    BOOST_CHECK(l1.get() == (0 - 1) << 1);
    BOOST_CHECK(l2.get() == 0);
    BOOST_CHECK(l3.get() == 19);
    BOOST_CHECK(l4.get() == 1);
}

BOOST_AUTO_TEST_CASE(equality_test)
{
    Literal l0(10);
    Literal l1(10);
    Literal l2(1);
    Literal l3(-10);
    Literal l4(-1);

    Literal l5(Variable(10), 1);
    Literal l6(Variable(10), 1);
    Literal l7(Variable(1), 1);
    Literal l8(Variable(10), -1);
    Literal l9(Variable(1), -1);

    BOOST_CHECK(l0 == l0);
    BOOST_CHECK(l1 == l1);
    BOOST_CHECK(l2 == l2);
    BOOST_CHECK(l3 == l3);
    BOOST_CHECK(l4 == l4);
    BOOST_CHECK(l5 == l5);
    BOOST_CHECK(l6 == l6);
    BOOST_CHECK(l7 == l7);
    BOOST_CHECK(l8 == l8);
    BOOST_CHECK(l9 == l9);

    BOOST_CHECK(l0 == l1);
    BOOST_CHECK(l0 == l5);
    BOOST_CHECK(l0 == l6);
    BOOST_CHECK(l1 == l5);
    BOOST_CHECK(l1 == l6);
    BOOST_CHECK(l2 == l7);
    BOOST_CHECK(l3 == l8);
    BOOST_CHECK(l4 == l9);

    BOOST_CHECK(l0 != l2);
    BOOST_CHECK(l0 != l3);
    BOOST_CHECK(l0 != l4);
    BOOST_CHECK(l0 != l7);
    BOOST_CHECK(l0 != l8);
    BOOST_CHECK(l0 != l9);
    BOOST_CHECK(l1 != l2);
    BOOST_CHECK(l1 != l3);
    BOOST_CHECK(l1 != l4);
    BOOST_CHECK(l1 != l7);
    BOOST_CHECK(l1 != l8);
    BOOST_CHECK(l1 != l9);
    BOOST_CHECK(l2 != l3);
    BOOST_CHECK(l2 != l5);
    BOOST_CHECK(l2 != l6);
    BOOST_CHECK(l2 != l8);
    BOOST_CHECK(l2 != l9);
    BOOST_CHECK(l2 != l4);
    BOOST_CHECK(l2 != l5);
    BOOST_CHECK(l2 != l6);
    BOOST_CHECK(l2 != l8);
    BOOST_CHECK(l2 != l9);
    BOOST_CHECK(l3 != l4);
    BOOST_CHECK(l3 != l5);
    BOOST_CHECK(l3 != l6);
    BOOST_CHECK(l3 != l7);
    BOOST_CHECK(l3 != l9);
    BOOST_CHECK(l4 != l5);
    BOOST_CHECK(l4 != l6);
    BOOST_CHECK(l4 != l7);
    BOOST_CHECK(l4 != l8);

    l2 = l0;

    BOOST_CHECK(l0 == l2);
    BOOST_CHECK(l1 == l2);
}

BOOST_AUTO_TEST_CASE(inequlity_test)
{
    Literal l0(10);
    Literal l1(1);
    Literal l2(-10);
    Literal l3(-1);

    BOOST_CHECK(l0 > l1);
    BOOST_CHECK(l0 < l2);
    BOOST_CHECK(l0 > l3);

    BOOST_CHECK(l1 < l2);
    BOOST_CHECK(l1 < l3);

    BOOST_CHECK(l2 > l3);
}

BOOST_AUTO_TEST_CASE(sign_test)
{
    Literal l0(10);
    Literal l1(1);
    Literal l2(-10);
    Literal l3(-1);

    BOOST_CHECK(l0.sign() == 1);
    BOOST_CHECK(l1.sign() == 1);
    BOOST_CHECK(l2.sign() == -1);
    BOOST_CHECK(l3.sign() == -1);
}

BOOST_AUTO_TEST_CASE(to_int_test)
{
    Literal l0(10);
    Literal l1(1);
    Literal l2(-10);
    Literal l3(-1);

    BOOST_CHECK(l0.to_int() == 10);
    BOOST_CHECK(l1.to_int() == 1);
    BOOST_CHECK(l2.to_int() == -10);
    BOOST_CHECK(l3.to_int() == -1);
}

BOOST_AUTO_TEST_CASE(negation_test)
{
    Literal l0(10);
    Literal l1(1);
    Literal l2(-10);
    Literal l3(-1);

    BOOST_CHECK((~l0).to_int() == -10);
    BOOST_CHECK((~l1).to_int() == -1);
    BOOST_CHECK((~l2).to_int() == 10);
    BOOST_CHECK((~l3).to_int() == 1);
}

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE VARIABLE_TEST
#include <boost/test/unit_test.hpp>

#include "var.hpp"

BOOST_AUTO_TEST_CASE(creation_test)
{
    Variable v0(10);
    Variable v1(0);
    Variable v2(1);
    BOOST_CHECK(v0.get() == 9);
    BOOST_CHECK(v1.get() == -1);
    BOOST_CHECK(v2.get() == 0);
}

BOOST_AUTO_TEST_CASE(equality_test)
{
    Variable v0(10);
    Variable v1(10);
    Variable v2(1);

    BOOST_CHECK(v0 == v0);
    BOOST_CHECK(v1 == v1);
    BOOST_CHECK(v2 == v2);

    BOOST_CHECK(v0 == v1);

    BOOST_CHECK(v0 != v2);
    BOOST_CHECK(v1 != v2);

    v2 = v0;

    BOOST_CHECK(v0 == v2);
    BOOST_CHECK(v1 == v2);
}

BOOST_AUTO_TEST_CASE(inequlity_test)
{
    Variable v0(10);
    Variable v1(10);
    Variable v2(1);

    BOOST_CHECK(v0 > v2);
    BOOST_CHECK(v1 > v2);
    BOOST_CHECK(!(v1 < v2));
    BOOST_CHECK(!(v0 < v2));
}

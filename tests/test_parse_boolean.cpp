#define BOOST_TEST_MODULE "test_parse_boolean"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/parser.hpp>
#include "test_parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_boolean)
{
    TOML11_TEST_PARSE_EQUAL(parse_boolean,  "true",  true);
    TOML11_TEST_PARSE_EQUAL(parse_boolean, "false", false);
}

BOOST_AUTO_TEST_CASE(test_boolean_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,  "true", toml::value( true));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "false", toml::value(false));
}

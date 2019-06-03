#define BOOST_TEST_MODULE "parse_integer_test"
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

BOOST_AUTO_TEST_CASE(test_decimal)
{
    TOML11_TEST_PARSE_EQUAL(parse_integer,        "1234",       1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer,       "+1234",       1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer,       "-1234",      -1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer,           "0",          0);
    TOML11_TEST_PARSE_EQUAL(parse_integer,     "1_2_3_4",       1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer,    "+1_2_3_4",      +1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer,    "-1_2_3_4",      -1234);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "123_456_789",  123456789);
}

BOOST_AUTO_TEST_CASE(test_decimal_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,        "1234", toml::value(     1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,       "+1234", toml::value(     1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,       "-1234", toml::value(    -1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,           "0", toml::value(        0));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,     "1_2_3_4", toml::value(     1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,    "+1_2_3_4", toml::value(    +1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>,    "-1_2_3_4", toml::value(    -1234));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "123_456_789", toml::value(123456789));
}

BOOST_AUTO_TEST_CASE(test_hex)
{
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xDEADBEEF",  0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xdeadbeef",  0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xDEADbeef",  0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xDEAD_BEEF", 0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xdead_beef", 0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xdead_BEEF", 0xDEADBEEF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0xFF",        0xFF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0x00FF",      0xFF);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0x0000FF",    0xFF);
}

BOOST_AUTO_TEST_CASE(test_hex_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xDEADBEEF",  value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xdeadbeef",  value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xDEADbeef",  value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xDEAD_BEEF", value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xdead_beef", value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xdead_BEEF", value(0xDEADBEEF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0xFF",        value(0xFF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0x00FF",      value(0xFF));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0x0000FF",    value(0xFF));
}

BOOST_AUTO_TEST_CASE(test_oct)
{
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0o777",   64*7+8*7+7);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0o7_7_7", 64*7+8*7+7);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0o007",   7);
}

BOOST_AUTO_TEST_CASE(test_oct_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0o777",   value(64*7+8*7+7));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0o7_7_7", value(64*7+8*7+7));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0o007",   value(7));
}

BOOST_AUTO_TEST_CASE(test_bin)
{
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0b10000",    16);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0b010000",   16);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0b01_00_00", 16);
    TOML11_TEST_PARSE_EQUAL(parse_integer, "0b111111",   63);
}

BOOST_AUTO_TEST_CASE(test_bin_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0b10000",    value(16));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0b010000",   value(16));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0b01_00_00", value(16));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "0b111111",   value(63));
}

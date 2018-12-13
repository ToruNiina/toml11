#define BOOST_TEST_MODULE "parse_array_test"
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

BOOST_AUTO_TEST_CASE(test_oneline_array)
{
    TOML11_TEST_PARSE_EQUAL(parse_array, "[]", array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array, "[3,1,4,1,5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array, "[\"foo\", \"bar\",  \"baz\"]", a);
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array, "[3,1,4,1,5,]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array, "[\"foo\", \"bar\",  \"baz\",]", a);
    }
}

BOOST_AUTO_TEST_CASE(test_oneline_array_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[]", toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[3,1,4,1,5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[\"foo\", \"bar\",  \"baz\"]", toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[3,1,4,1,5,]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[\"foo\", \"bar\",  \"baz\",]", toml::value(a));
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array)
{
    TOML11_TEST_PARSE_EQUAL(parse_array, "[\n#comment\n]", array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array, "[3,\n1,\n4,\n1,\n5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array, "[\"foo\",\n\"bar\",\n\"baz\"]", a);
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML11_TEST_PARSE_EQUAL(parse_array, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", a);
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[\n#comment\n]", toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[3,\n1,\n4,\n1,\n5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[\"foo\",\n\"bar\",\n\"baz\"]", toml::value(a));
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", toml::value(a));
    }
}

#define BOOST_TEST_MODULE "parse_array<toml::value>_test"
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
    TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[]", array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[3,1,4,1,5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[\"foo\", \"bar\",  \"baz\"]", a);
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[3,1,4,1,5,]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[\"foo\", \"bar\",  \"baz\",]", a);
    }
}

BOOST_AUTO_TEST_CASE(test_oneline_array_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[]", toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[3,1,4,1,5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\", \"bar\",  \"baz\"]", toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[3,1,4,1,5,]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\", \"bar\",  \"baz\",]", toml::value(a));
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array)
{
    TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[\n#comment\n]", array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[3,\n1,\n4,\n1,\n5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[\"foo\",\n\"bar\",\n\"baz\"]", a);
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML11_TEST_PARSE_EQUAL(parse_array<toml::value>, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", a);
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\n#comment\n]", toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[3,\n1,\n4,\n1,\n5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\",\n\"bar\",\n\"baz\"]", toml::value(a));
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", toml::value(a));
    }
}

BOOST_AUTO_TEST_CASE(test_heterogeneous_array)
{
#ifndef TOML11_USE_UNRELEASED_TOML_FEATURES
    BOOST_TEST_MESSAGE("In strict TOML v0.5.0, heterogeneous arrays are not allowed.");
#else
    {
        array a(5);
        a[0] = toml::value("foo");
        a[1] = toml::value(3.14);
        a[2] = toml::value(42);
        a[3] = toml::value{toml::value("array"), toml::value("of"), toml::value("hetero-array"), toml::value(1)};
        a[4] = toml::value{{"key", "value"}};
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\", 3.14, 42, [\"array\", \"of\", \"hetero-array\", 1], {key = \"value\"}]", toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value("foo");
        a[1] = toml::value(3.14);
        a[2] = toml::value(42);
        a[3] = toml::value{toml::value("array"), toml::value("of"), toml::value("hetero-array"), toml::value(1)};
        a[4] = toml::value{{"key", "value"}};
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\",\n 3.14,\n 42,\n [\"array\", \"of\", \"hetero-array\", 1],\n {key = \"value\"},\n]", toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value("foo");
        a[1] = toml::value(3.14);
        a[2] = toml::value(42);
        a[3] = toml::value{toml::value("array"), toml::value("of"), toml::value("hetero-array"), toml::value(1)};
        a[4] = toml::value{{"key", "value"}};
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\",#comment\n 3.14,#comment\n 42,#comment\n [\"array\", \"of\", \"hetero-array\", 1],#comment\n {key = \"value\"},#comment\n]#comment", toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value("foo");
        a[1] = toml::value(3.14);
        a[2] = toml::value(42);
        a[3] = toml::value{toml::value("array"), toml::value("of"), toml::value("hetero-array"), toml::value(1)};
        a[4] = toml::value{{"key", "value"}};
        TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "[\"foo\",\n 3.14,\n 42,\n [\"array\",\n \"of\",\n \"hetero-array\",\n 1],\n {key = \"value\"},\n]", toml::value(a));
    }
#endif
}

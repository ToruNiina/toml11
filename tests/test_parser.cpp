#define BOOST_TEST_MODULE "test_parser"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/acceptor.hpp>
#include <toml/parser.hpp>
#include <toml/from_toml.hpp>
#include <iostream>


BOOST_AUTO_TEST_CASE(test_parse_basic_inline_string)
{
    typedef toml::parse_basic_inline_string parser;
    typedef toml::is_basic_inline_string<toml::character> acceptor;
    {
        const std::string source("\"simple\"");
        const std::string expected("simple");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("\"I'm a string. \\\"You can quote me\\\". Name\\tJos\\u00E9\\nLocation\\tSF.\"");
        const std::string expected("I'm a string. \"You can quote me\". Name\tJosé\nLocation\tSF.");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_basic_multiline_string)
{
    typedef toml::parse_basic_multiline_string parser;
    typedef toml::is_basic_multiline_string<toml::character> acceptor;
    {
        //XXX ifdef windows platform
        const std::string source("\"\"\"\nRoses are red\nViolets are blue\"\"\"");
        const std::string expected("Roses are red\nViolets are blue");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n    the lazy dog.\"\"\"");
        const std::string expected("The quick brown fox jumps over the lazy dog.");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("\"\"\"\nThe quick brown \\\n    fox jumps over \\\n    the lazy dog.\\\n    \"\"\"");
        const std::string expected("The quick brown fox jumps over the lazy dog.");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_literal_inline_string)
{
    typedef toml::parse_literal_inline_string parser;
    typedef toml::is_literal_inline_string<toml::character> acceptor;
    {
        const std::string source("'C:\\Users\\nodejs\\templates'");
        const std::string expected("C:\\Users\\nodejs\\templates");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'\\\\ServerX\\admin$\\system32\\'");
        const std::string expected("\\\\ServerX\\admin$\\system32\\");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'Tom \"Dubs\" Preston-Werner'");
        const std::string expected("Tom \"Dubs\" Preston-Werner");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'<\\i\\c*\\s*>'");
        const std::string expected("<\\i\\c*\\s*>");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_literal_multiline_string)
{
    typedef toml::parse_literal_multiline_string parser;
    typedef toml::is_literal_multiline_string<toml::character> acceptor;
    {
        const std::string source("'''I [dw]on't need \\d{2} apples'''");
        const std::string expected("I [dw]on't need \\d{2} apples");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'''\nThe first newline is \ntrimmed in raw strings.\n    All other whitespace\n    is preserved.'''");
        const std::string expected("The first newline is \ntrimmed in raw strings.\n    All other whitespace\n    is preserved.");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_string)
{
    typedef toml::parse_string parser;
    typedef toml::is_string<toml::character> acceptor;
    {
        const std::string source("\"string\"");
        const std::string expected("string");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("\"\"\"string\"\"\"");
        const std::string expected("string");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'string'");
        const std::string expected("string");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("'''string'''");
        const std::string expected("string");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_integer)
{
    typedef toml::parse_integer parser;
    typedef toml::is_integer<toml::character> acceptor;
    {
        const std::string   source("42");
        const toml::Integer expected(42);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string   source("+42");
        const toml::Integer expected(42);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string   source("-42");
        const toml::Integer expected(-42);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string   source("-4_2");
        const toml::Integer expected(-42);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_float)
{
    typedef toml::parse_float parser;
    typedef toml::is_float<toml::character> acceptor;
    {
        const std::string source("42.0");
        const toml::Float expected(42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("+42.0");
        const toml::Float expected(42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("-42.0");
        const toml::Float expected(-42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("-4_2.0");
        const toml::Float expected(-42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("-42e0");
        const toml::Float expected(-42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("-42.0e0");
        const toml::Float expected(-42.0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
    {
        const std::string source("42");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_boolean)
{
    typedef toml::parse_boolean parser;
    typedef toml::is_boolean<toml::character> acceptor;
    {
        const std::string   source("true");
        const toml::Boolean expected(true);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string   source("false");
        const toml::Boolean expected(false);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string   source("dummy");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}



BOOST_AUTO_TEST_CASE(test_parse_local_time)
{
    typedef toml::parse_local_time parser;
    typedef toml::is_local_time<toml::character> acceptor;
    {
        const std::string source("12:34:56");
        const toml::Datetime expected(12, 34, 56, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("12:34:56.7");
        const toml::Datetime expected(12, 34, 56, 700, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("12:34:56.7891");
        const toml::Datetime expected(12, 34, 56, 789, 100);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("10");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_local_date)
{
    typedef toml::parse_local_date parser;
    typedef toml::is_local_date<toml::character>  acceptor;
    {
        const std::string source("1979-09-27");
        const toml::Datetime expected(1979, 9, 27);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("10");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_local_date_time)
{
    typedef toml::parse_local_date_time parser;
    typedef toml::is_local_date_time<toml::character> acceptor;
    {
        const std::string source("1979-09-27T12:34:56");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27T12:34:56.789000");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1000-11-11");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_offset_date_time)
{
    typedef toml::parse_offset_date_time parser;
    typedef toml::is_offset_date_time<toml::character> acceptor;
    {
        const std::string source("1979-09-27T12:34:56Z");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27T12:34:56.789000Z");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("1979-09-27T12:34:56+07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, 7, 30);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27T12:34:56.789000+07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, 7, 30);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("1979-09-27T12:34:56-07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, -7, -30);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27T12:34:56.789000-07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, -7, -30);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1000-11-11");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_datetime)
{
    typedef toml::parse_datetime parser;
    typedef toml::is_datetime<toml::character> acceptor;
    {
        const std::string source("1979-09-27T12:34:56Z");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27T12:34:56");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("1979-09-27");
        const toml::Datetime expected(1979, 9, 27);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("12:34:56");
        const toml::Datetime expected(12, 34, 56, 0, 0);
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK_EQUAL(result.first.get(), expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("12");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}


BOOST_AUTO_TEST_CASE(test_parse_array)
{
    typedef toml::parse_array<toml::character> parser;
    typedef toml::is_array<toml::character>  acceptor;
    {
        const std::string source("[1,2,3]");
        const toml::Array expected{1, 2, 3};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[1, 2, 3]");
        const toml::Array expected{1, 2, 3};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[ 1,2,3 ]");
        const toml::Array expected{1, 2, 3};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[ 1 , 2 , 3 ]");
        const toml::Array expected{1, 2, 3};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[ 1 \n,#comment\n 2 ,\n 3\n ]");
        const toml::Array expected{1, 2, 3};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[ # empty array\n ]");
        const toml::Array expected{};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[ \"] \", ' # ', \n']', # ] \n]");
        const toml::Array expected{"] ", " # ", "]"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("[ \"Test #11 ]proved that\", 'Experiment #9 was a success' ]");
        const toml::Array expected{"Test #11 ]proved that", "Experiment #9 was a success"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("[ \"Test #11 ]proved that\", 'Experiment #9 was a success' ]");
        const toml::Array expected{"Test #11 ]proved that", "Experiment #9 was a success"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("[ [1,2,3] , ['a', 'b', 'c'] ]");
        const toml::Array expected{{1,2,3}, {"a", "b", "c"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("[ {foo=1}, {foo=1, bar=2.0}, {foo=1, bar=2.0, baz='str'} ]");
        const toml::Array expected{{{"foo", 1}}, {{"foo", 1}, {"bar", 2.0}}, {{"foo", 1}, {"bar", 2.0}, {"baz", "str"}}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[dummy]");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}


BOOST_AUTO_TEST_CASE(test_parse_inline_table)
{
    typedef toml::parse_inline_table<toml::character> parser;
    typedef toml::is_inline_table<toml::character>  acceptor;
    {
        const std::string source("{foo=1,bar=2.0,baz='str'}");
        const toml::Table expected{{"foo", 1}, {"bar", 2.0}, {"baz", "str"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("{ foo=1, bar=2.0, baz='str' }");
        const toml::Table expected{{"foo", 1}, {"bar", 2.0}, {"baz", "str"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("{ foo = 1, bar = 2.0, baz = 'str' }");
        const toml::Table expected{{"foo", 1}, {"bar", 2.0}, {"baz", "str"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }

    {
        const std::string source("{b=true, i=1, f=2.0, d=1907-03-02T07:32:00, s='str', a=[1,2,3], t={foo=1}}");
        const toml::Table expected{{"b", true}, {"i", 1}, {"f", 2.0},
                                   {"d", toml::Datetime(1907,3,2,7,32,0,0,0)},
                                   {"s", "str"}, {"a", {1, 2, 3}},
                                   {"t", {{"foo", 1}}}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("{dummy}");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(!result.first.ok());
        BOOST_CHECK(result.second == source.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_barekey)
{
    typedef toml::parse_barekey parser;
    typedef toml::is_barekey<toml::character> acceptor;
    {
        const std::string source("hoge");
        const toml::key expected("hoge");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("bare-key");
        const toml::key expected("bare-key");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("bare_key");
        const toml::key expected("bare_key");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("42");
        const toml::key expected("42");
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
}



BOOST_AUTO_TEST_CASE(test_key_value_pair)
{
    typedef toml::parse_key_value_pair<char> parser;
    typedef toml::is_key_value_pair<char>  acceptor;
    {
        const std::string source("key=1");
        const std::pair<toml::key, toml::value> expected{"key", 1};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key =\t1");
        const std::pair<toml::key, toml::value> expected{"key", 1};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = true");
        const std::pair<toml::key, toml::value> expected{"key", true};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = -42");
        const std::pair<toml::key, toml::value> expected{"key", -42};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = -42.0");
        const std::pair<toml::key, toml::value> expected{"key", -42.};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = \"string\"");
        const std::pair<toml::key, toml::value> expected{"key", "string"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = 1901-01-01T00:00:00");
        const std::pair<toml::key, toml::value> expected{"key", toml::Datetime(1901, 1,1,0,0,0,0,0)};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = [1,2,3]");
        const std::pair<toml::key, toml::value> expected{"key", {1,2,3}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("key = {foo=1,bar=2.0,baz='3'}");
        const std::pair<toml::key, toml::value> expected{"key",
            {{"foo", 1}, {"bar", 2.0}, {"baz", "3"}}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
}

BOOST_AUTO_TEST_CASE(test_table_definition)
{
    typedef toml::parse_table_definition parser;
    typedef toml::is_table_definition<char>  acceptor;
    {
        const std::string source("[foo]");
        const std::vector<toml::key> expected{"foo"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[foo.bar.baz]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[foo . bar. baz]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[foo . \"bar\" . baz]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[foo . \"b\\tar\" . baz]");
        const std::vector<toml::key> expected{"foo", "b\tar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
}

BOOST_AUTO_TEST_CASE(test_array_of_table_definition)
{
    typedef toml::parse_array_of_table_definition parser;
    typedef toml::is_array_of_table_definition<char>  acceptor;
    {
        const std::string source("[[foo]]");
        const std::vector<toml::key> expected{"foo"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[[foo.bar.baz]]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[[foo . bar. baz]]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[[foo . \"bar\" . baz]]");
        const std::vector<toml::key> expected{"foo", "bar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
    {
        const std::string source("[[foo . \"b\\tar\" . baz]]");
        const std::vector<toml::key> expected{"foo", "b\tar", "baz"};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result.first.ok());
        BOOST_CHECK(result.first.get() == expected);
        BOOST_CHECK(result.second == acceptor::invoke(source.begin(), source.end()));
    }
}

BOOST_AUTO_TEST_CASE(test_parse_data)
{
    typedef toml::parse_data parser;
    {
        const std::string source("#hogehoge");
        const toml::Table expected{};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("key = 'value'");
        const toml::Table expected{{"key", "value"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("key = 'value' #hoge");
        const toml::Table expected{{"key", "value"}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("[table]\nkey = 'value' #hoge");
        const toml::Table expected{{"table", {{"key", "value"}} }};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("[table]\n\tkey = 'value'\n\t#hoge");
        const toml::Table expected{{"table", {{"key", "value"}} }};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("[table]\n\tkey = 'value'\n\t#hoge");
        const toml::Table expected{{"table", {{"key", "value"}} }};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
    {
        const std::string source("[nested.table]\n\tkey = 'value'\n\t#hoge");
        const toml::Table expected{{"nested", {{"table", {{"key", "value"}}}}}};
        const auto result = parser::invoke(source.cbegin(), source.cend());
        BOOST_CHECK(result == expected);
    }
}

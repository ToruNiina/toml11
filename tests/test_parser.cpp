#define BOOST_TEST_MODULE "test_barekey"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/acceptor.hpp>
#include <toml/parser.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE(test_parse_barekey)
{
    {
        const std::string source("hoge");
        const std::string result = toml::parse_barekey<char>::invoke(
                source.cbegin(), toml::is_barekey<char>::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(source, result);
    }
    {
        const std::string source("bare-key");
        const std::string result = toml::parse_barekey<char>::invoke(
                source.cbegin(), toml::is_barekey<char>::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(source, result);
    }
    {
        const std::string source("bare_key");
        const std::string result = toml::parse_barekey<char>::invoke(
                source.cbegin(), toml::is_barekey<char>::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(source, result);
    }
    {
        const std::string source("42");
        const std::string result = toml::parse_barekey<char>::invoke(
                source.cbegin(), toml::is_barekey<char>::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(source, result);
    }
}


BOOST_AUTO_TEST_CASE(test_parse_basic_inline_string)
{
    typedef toml::parse_basic_inline_string<char> parser;
    typedef toml::is_basic_inline_string<char>  acceptor;
    {
        const std::string source("\"simple\"");
        const std::string expected("simple");
        const std::string result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("\"I'm a string. \\\"You can quote me\\\". Name\\tJos\\u00E9\\nLocation\\tSF.\"");
        const std::string expected("I'm a string. \"You can quote me\". Name\tJosé\nLocation\tSF.");
        const std::string result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_basic_multiline_string)
{
    typedef toml::parse_basic_multiline_string<char> parser;
    typedef toml::is_basic_multiline_string<char>  acceptor;
    {
        const std::string source("\"\"\"\nRoses are red\nViolets are blue\"\"\"");
        //XXX ifdef windows platform
        const std::string expected("Roses are red\nViolets are blue");
        const std::string result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n    the lazy dog.\"\"\"");
        const std::string expected("The quick brown fox jumps over the lazy dog.");
        const std::string result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("\"\"\"\nThe quick brown \\\n    fox jumps over \\\n    the lazy dog.\\\n    \"\"\"");
        const std::string expected("The quick brown fox jumps over the lazy dog.");
        const std::string result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_local_time)
{
    typedef toml::parse_local_time<char> parser;
    typedef toml::is_local_time<char>  acceptor;
    {
        const std::string source("12:34:56");
        const toml::Datetime expected(12, 34, 56, 0, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("12:34:56.7");
        const toml::Datetime expected(12, 34, 56, 700, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("12:34:56.7891");
        const toml::Datetime expected(12, 34, 56, 789, 100);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_local_date)
{
    typedef toml::parse_local_date<char> parser;
    typedef toml::is_local_date<char>  acceptor;
    {
        const std::string source("1979-09-27");
        const toml::Datetime expected(1979, 9, 27);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_local_date_time)
{
    typedef toml::parse_local_date_time<char> parser;
    typedef toml::is_local_date_time<char>  acceptor;
    {
        const std::string source("1979-09-27T12:34:56");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("1979-09-27T12:34:56.789000");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(test_parse_offset_date_time)
{
    typedef toml::parse_offset_date_time<char> parser;
    typedef toml::is_offset_date_time<char>  acceptor;
    {
        const std::string source("1979-09-27T12:34:56Z");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, 0, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("1979-09-27T12:34:56.789000Z");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, 0, 0);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }

    {
        const std::string source("1979-09-27T12:34:56+07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, 7, 30);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("1979-09-27T12:34:56.789000+07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, 7, 30);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }

    {
        const std::string source("1979-09-27T12:34:56-07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 0, 0, -7, -30);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        const std::string source("1979-09-27T12:34:56.789000-07:30");
        const toml::Datetime expected(1979, 9, 27, 12, 34, 56, 789, 0, -7, -30);
        const toml::Datetime result = parser::invoke(
                source.cbegin(), acceptor::invoke(source.cbegin()));
        BOOST_CHECK_EQUAL(result, expected);
    }
}


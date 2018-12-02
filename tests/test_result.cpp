#define BOOST_TEST_MODULE "test_result"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <toml/result.hpp>

BOOST_AUTO_TEST_CASE(test_construct)
{
    {
        auto s = toml::ok(42);
        toml::result<int, std::string> result(s);
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }
    {
        const auto s = toml::ok(42);
        toml::result<int, std::string> result(s);
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }
    {
        toml::result<int, std::string> result(toml::ok(42));
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }

    {
        auto f = toml::err<std::string>("foobar");
        toml::result<int, std::string> result(f);
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "foobar");
    }
    {
        const auto f = toml::err<std::string>("foobar");
        toml::result<int, std::string> result(f);
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "foobar");
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "foobar");
    }
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        result = toml::ok(42);
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        auto s = toml::ok(42);
        result = s;
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        const auto s = toml::ok(42);
        result = s;
        BOOST_CHECK(!!result);
        BOOST_CHECK(result.is_ok());
        BOOST_CHECK(!result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap(), 42);
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        result = toml::err<std::string>("hoge");
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "hoge");
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        auto f = toml::err<std::string>("hoge");
        result = f;
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "hoge");
    }
    {
        toml::result<int, std::string> result(toml::err<std::string>("foobar"));
        const auto f = toml::err<std::string>("hoge");
        result = f;
        BOOST_CHECK(!result);
        BOOST_CHECK(!result.is_ok());
        BOOST_CHECK(result.is_err());
        BOOST_CHECK_EQUAL(result.unwrap_err(), "hoge");
    }
}

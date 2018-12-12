#define BOOST_TEST_MODULE "test_get_or"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <map>
#include <unordered_map>
#include <list>
#include <deque>
#include <array>


BOOST_AUTO_TEST_CASE(test_get_or_exist)
{
    toml::Boolean  raw_v1(true);
    toml::Integer  raw_v2(42);
    toml::Float    raw_v3(3.14);
    toml::String   raw_v4("hoge");
    toml::Array    raw_v5{2,7,1,8,2};
    toml::Table    raw_v6{{"key", 42}};

    toml::value v1(raw_v1);
    toml::value v2(raw_v2);
    toml::value v3(raw_v3);
    toml::value v4(raw_v4);
    toml::value v5(raw_v5);
    toml::value v6(raw_v6);

    toml::Table table{
        {"value1", v1},
        {"value2", v2},
        {"value3", v3},
        {"value4", v4},
        {"value5", v5},
        {"value6", v6}
    };

    toml::Boolean u1 = toml::get_or(table, "value1", raw_v1);
    toml::Integer u2 = toml::get_or(table, "value2", raw_v2);
    toml::Float   u3 = toml::get_or(table, "value3", raw_v3);
    toml::String  u4 = toml::get_or(table, "value4", raw_v4);
    toml::Array   u5 = toml::get_or(table, "value5", raw_v5);
    toml::Table   u6 = toml::get_or(table, "value6", raw_v6);

    BOOST_CHECK_EQUAL(u1, raw_v1);
    BOOST_CHECK_EQUAL(u2, raw_v2);
    BOOST_CHECK_EQUAL(u3, raw_v3);
    BOOST_CHECK_EQUAL(u4, raw_v4);
    BOOST_CHECK_EQUAL(u5.at(0).cast<toml::value_t::Integer>(), raw_v5.at(0).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(1).cast<toml::value_t::Integer>(), raw_v5.at(1).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(2).cast<toml::value_t::Integer>(), raw_v5.at(2).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(3).cast<toml::value_t::Integer>(), raw_v5.at(3).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(4).cast<toml::value_t::Integer>(), raw_v5.at(4).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at("key").cast<toml::value_t::Integer>(), 42);
}

BOOST_AUTO_TEST_CASE(test_get_or_empty)
{
    toml::Boolean  raw_v1(true);
    toml::Integer  raw_v2(42);
    toml::Float    raw_v3(3.14);
    toml::String   raw_v4("hoge");
    toml::Array    raw_v5{2,7,1,8,2};
    toml::Table    raw_v6{{"key", 42}};

    toml::Table    table; // empty!

    toml::Boolean  u1 = toml::get_or(table, std::string("value1"), raw_v1);
    toml::Integer  u2 = toml::get_or(table, std::string("value2"), raw_v2);
    toml::Float    u3 = toml::get_or(table, std::string("value3"), raw_v3);
    toml::String   u4 = toml::get_or(table, std::string("value4"), raw_v4);
    toml::Array    u5 = toml::get_or(table, std::string("value5"), raw_v5);
    toml::Table    u6 = toml::get_or(table, std::string("value6"), raw_v6);

    BOOST_CHECK_EQUAL(u1, raw_v1);
    BOOST_CHECK_EQUAL(u2, raw_v2);
    BOOST_CHECK_EQUAL(u3, raw_v3);
    BOOST_CHECK_EQUAL(u4, raw_v4);
    BOOST_CHECK_EQUAL(u5.at(0).cast<toml::value_t::Integer>(), raw_v5.at(0).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(1).cast<toml::value_t::Integer>(), raw_v5.at(1).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(2).cast<toml::value_t::Integer>(), raw_v5.at(2).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(3).cast<toml::value_t::Integer>(), raw_v5.at(3).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u5.at(4).cast<toml::value_t::Integer>(), raw_v5.at(4).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at("key").cast<toml::value_t::Integer>(), 42);
}

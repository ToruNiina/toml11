#define BOOST_TEST_MODULE "test_get"
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


BOOST_AUTO_TEST_CASE(test_get_exact)
{
    toml::Boolean  b(true);
    toml::Integer  i(42);
    toml::Float    f(3.14);
    toml::String   s("hoge");
    toml::Array    a;
    a.emplace_back(2);
    a.emplace_back(7);
    a.emplace_back(1);
    a.emplace_back(8);
    a.emplace_back(2);
    toml::Table    t;
    t.emplace("val1", true);
    t.emplace("val2", 42);
    t.emplace("val3", 3.14);
    t.emplace("val4", "piyo");

    toml::value v1(b);
    toml::value v2(i);
    toml::value v3(f);
    toml::value v4(s);
    toml::value v6(a);
    toml::value v7(t);

    toml::Boolean  u1 = toml::get<toml::Boolean >(v1);
    toml::Integer  u2 = toml::get<toml::Integer >(v2);
    toml::Float    u3 = toml::get<toml::Float   >(v3);
    toml::String   u4 = toml::get<toml::String  >(v4);
    toml::Array    u6 = toml::get<toml::Array   >(v6);
    toml::Table    u7 = toml::get<toml::Table   >(v7);

    BOOST_CHECK_EQUAL(u1, b);
    BOOST_CHECK_EQUAL(u2, i);
    BOOST_CHECK_EQUAL(u3, f);
    BOOST_CHECK_EQUAL(u4, s);
    BOOST_CHECK_EQUAL(u6.at(0).cast<toml::value_t::Integer>(), a.at(0).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at(1).cast<toml::value_t::Integer>(), a.at(1).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at(2).cast<toml::value_t::Integer>(), a.at(2).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at(3).cast<toml::value_t::Integer>(), a.at(3).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u6.at(4).cast<toml::value_t::Integer>(), a.at(4).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(u7.at("val1").cast<toml::value_t::Boolean>(), true);
    BOOST_CHECK_EQUAL(u7.at("val2").cast<toml::value_t::Integer>(), 42);
    BOOST_CHECK_CLOSE_FRACTION(u7.at("val3").cast<toml::value_t::Float>(),3.14, 1e-3);
    BOOST_CHECK_EQUAL(u7.at("val4").cast<toml::value_t::String>(),  "piyo");
}


BOOST_AUTO_TEST_CASE(test_get_cast)
{
    toml::Integer  i(42);
    toml::Float    f(3.14);
    toml::String   s("hoge");
    toml::Datetime d(std::chrono::system_clock::now());
    toml::Array    a;
    a.emplace_back(2);
    a.emplace_back(7);
    a.emplace_back(1);
    a.emplace_back(8);
    a.emplace_back(2);
    toml::Table    t;
    t.emplace("val1", true);
    t.emplace("val2", 42);
    t.emplace("val3", 3.14);
    t.emplace("val4", "piyo");

    toml::value v2(i);
    toml::value v3(f);
    toml::value v4(s);
    toml::value v5(d);
    toml::value v6(a);
    toml::value v7(t);

    const auto u2 = toml::get<std::size_t>(v2);
    const auto u3 = toml::get<float>(v3);
    const auto u4 = toml::get<std::deque<int>>(v6);
    const auto u5 = toml::get<std::list<int> >(v6);
    const auto u6 = toml::get<std::array<int,5>>(v6);
    std::map<std::string, toml::value> u7 = toml::get<std::map<std::string, toml::value>>(v7);

    std::deque<int> r4{2,7,1,8,2};
    std::list<int>  r5{2,7,1,8,2};
    std::array<int, 5> r6{{2,7,1,8,2}};

    BOOST_CHECK_EQUAL(u2, 42ul);
    BOOST_CHECK_CLOSE_FRACTION(u3, 3.14, 1e-3);
    const bool dq = r4 == u4;
    const bool ls = r5 == u5;
    const bool ar = r6 == u6;
    BOOST_CHECK(dq);
    BOOST_CHECK(ls);
    BOOST_CHECK(ar);
    BOOST_CHECK_EQUAL(u7.at("val1").cast<toml::value_t::Boolean>(), true);
    BOOST_CHECK_EQUAL(u7.at("val2").cast<toml::value_t::Integer>(), 42);
    BOOST_CHECK_CLOSE_FRACTION(u7.at("val3").cast<toml::value_t::Float>(),3.14, 1e-3);
    BOOST_CHECK_EQUAL(u7.at("val4").cast<toml::value_t::String>(),  "piyo");
}

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

BOOST_AUTO_TEST_CASE(test_find_or)
{
    // requires conversion int -> uint
    {
        toml::value v = toml::table{{"num", 42}};
        BOOST_TEST(42u == toml::find_or(v, "num", 0u));
        BOOST_TEST(0u ==  toml::find_or(v, "foo", 0u));
    }
    // exact toml type
    {
        toml::value v1 = toml::table{{"key", 42  }};
        toml::value v2 = toml::table{{"key", 3.14}};
        toml::value v3 = toml::table{{"not", "key"}};

        BOOST_TEST(42 == toml::find_or(v1, "key", toml::integer(0)));
        BOOST_TEST( 0 == toml::find_or(v2, "key", toml::integer(0)));
        BOOST_TEST( 0 == toml::find_or(v3, "key", toml::integer(0)));

        toml::value v4 = toml::table{{"str", "foobar"}};
        toml::string s("bazqux");

        BOOST_TEST("foobar" == toml::find_or(v4, "str", s));
        BOOST_TEST("bazqux" == toml::find_or(v1, "str", s));
    }
    // std::string
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key", 42}};

        std::string       s1("bazqux");
        const std::string s2("bazqux");

        BOOST_TEST("foobar" == toml::find_or(v1, "key", s1));
        BOOST_TEST("bazqux" == toml::find_or(v2, "key", s1));

        std::string& v1r = toml::find_or(v1, "key", s1);
        std::string& s1r = toml::find_or(v2, "key", s1);

        BOOST_TEST("foobar" == v1r);
        BOOST_TEST("bazqux" == s1r);

        BOOST_TEST("foobar" == toml::find_or(v1, "key", s2));
        BOOST_TEST("bazqux" == toml::find_or(v2, "key", s2));

        BOOST_TEST("foobar" == toml::find_or(std::move(v1), "key", std::move(s1)));
        s1 = "bazqux"; // restoring moved value
        BOOST_TEST("bazqux" == toml::find_or(std::move(v2), "key", std::move(s1)));
    }
    // string literal
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key",42}};

        BOOST_TEST("foobar" == toml::find_or(v1, "key", "bazqux"));
        BOOST_TEST("bazqux" == toml::find_or(v2, "key", "bazqux"));

        const char* lit = "bazqux";
        BOOST_TEST("foobar" == toml::find_or(v1, "key", lit));
        BOOST_TEST("bazqux" == toml::find_or(v2, "key", lit));
    }
}

BOOST_AUTO_TEST_CASE(test_expect)
{
    {
        toml::value v1(42);
        toml::value v2(3.14);
        const auto v1_or_0 = toml::expect<int>(v1).unwrap_or(0);
        const auto v2_or_0 = toml::expect<int>(v2).unwrap_or(0);
        BOOST_TEST(42 == v1_or_0);
        BOOST_TEST( 0 == v2_or_0);

        const auto v1_or_none = toml::expect<int>(v1).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none"));
        const auto v2_or_none = toml::expect<int>(v2).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none"));
        BOOST_TEST("42" ==   v1_or_none);
        BOOST_TEST("none" == v2_or_none);
    }
}

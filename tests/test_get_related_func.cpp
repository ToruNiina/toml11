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

BOOST_AUTO_TEST_CASE(test_find)
{
    {
        toml::value v(true);
        bool thrown = false;
        try
        {
            toml::find<toml::boolean>(v, "key");
        }
        catch(toml::type_error const& te)
        {
            thrown = true;
        }
        BOOST_CHECK(thrown);
    }

    {
        toml::table v{{"num", 42}};
        BOOST_CHECK_EQUAL(42, toml::find<int>(v, "num"));
        toml::find<toml::integer>(v, "num") = 54;
        BOOST_CHECK_EQUAL(54, toml::find<int>(v, "num"));
    }

    {
        toml::value v = toml::table{{"num", 42}};
        BOOST_CHECK_EQUAL(42, toml::find<int>(v, "num"));
        toml::find<toml::integer>(v, "num") = 54;
        BOOST_CHECK_EQUAL(54, toml::find<int>(v, "num"));
    }
}

BOOST_AUTO_TEST_CASE(test_get_or)
{
    {
        toml::table v{{"num", 42}};
        BOOST_CHECK_EQUAL(42, toml::get_or<int>(v, "num", 0));
        BOOST_CHECK_EQUAL(0,  toml::get_or<int>(v, "foo", 0));
    }
    {
        toml::value v = toml::table{{"num", 42}};
        BOOST_CHECK_EQUAL(42, toml::get_or<int>(v, "num", 0));
        BOOST_CHECK_EQUAL(0,  toml::get_or<int>(v, "foo", 0));
    }


    // requires conversion int -> uint
    {
        toml::value v1(42);
        toml::value v2(3.14);
        BOOST_CHECK_EQUAL(42u, toml::get_or(v1, 0u));
        BOOST_CHECK_EQUAL(0u,  toml::get_or(v2, 0u));
    }

    // exact toml type
    {
        toml::value v1(42);
        toml::value v2(3.14);
        BOOST_CHECK_EQUAL(42, toml::get_or(v1, toml::integer(0)));
        BOOST_CHECK_EQUAL(0,  toml::get_or(v2, toml::integer(0)));

        toml::value v3("foobar");
        toml::string s("bazqux");

        BOOST_CHECK_EQUAL("foobar", toml::get_or(v3, s));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v1, s));

    }

    // std::string
    {
        toml::value v1("foobar");
        toml::value v2(42);

        std::string       s1("bazqux");
        const std::string s2("bazqux");

        BOOST_CHECK_EQUAL("foobar", toml::get_or(v1, s1));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v2, s1));

        std::string& v1r = toml::get_or(v1, s1);
        std::string& s1r = toml::get_or(v2, s1);

        BOOST_CHECK_EQUAL("foobar", v1r);
        BOOST_CHECK_EQUAL("bazqux", s1r);

        BOOST_CHECK_EQUAL("foobar", toml::get_or(v1, s2));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v2, s2));

        BOOST_CHECK_EQUAL("foobar", toml::get_or(v1, std::move(s1)));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v2, std::move(s1)));
    }

    // string literal
    {
        toml::value v1("foobar");
        toml::value v2(42);

        BOOST_CHECK_EQUAL("foobar", toml::get_or(v1, "bazqux"));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v2, "bazqux"));

        const char* lit = "bazqux";
        BOOST_CHECK_EQUAL("foobar", toml::get_or(v1, lit));
        BOOST_CHECK_EQUAL("bazqux", toml::get_or(v2, lit));
    }
}

BOOST_AUTO_TEST_CASE(test_expect)
{
    {
        toml::value v1(42);
        toml::value v2(3.14);
        const auto v1_or_0 = toml::expect<int>(v1).unwrap_or(0);
        const auto v2_or_0 = toml::expect<int>(v2).unwrap_or(0);
        BOOST_CHECK_EQUAL(42, v1_or_0);
        BOOST_CHECK_EQUAL( 0, v2_or_0);

        const auto v1_or_none = toml::expect<int>(v1).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none"));
        const auto v2_or_none = toml::expect<int>(v2).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none"));
        BOOST_CHECK_EQUAL("42",   v1_or_none);
        BOOST_CHECK_EQUAL("none", v2_or_none);
    }
}

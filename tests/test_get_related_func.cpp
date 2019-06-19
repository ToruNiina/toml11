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
    // the value corresponding to the key is not the expected type
    {
        toml::value v{{"key", 42}};
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

    // recursively search tables
    {
        toml::value v = toml::table{
            {"a", toml::table{
                {"b", toml::table{
                    {"c", toml::table{
                        {"d", 42}
                    }}
                }}
            }}
        };
        BOOST_CHECK_EQUAL(42, toml::find<int>(v, "a", "b", "c", "d"));

        // reference that can be used to modify the content
        auto& num = toml::find<toml::integer>(v, "a", "b", "c", "d");
        num = 54;
        BOOST_CHECK_EQUAL(54, toml::find<int>(v, "a", "b", "c", "d"));

        const std::string a("a"), b("b"), c("c"), d("d");
        auto& num2 = toml::find<toml::integer>(v, a, b, c, d);
        num2 = 42;
        BOOST_CHECK_EQUAL(42, toml::find<int>(v, a, b, c, d));
    }
}

BOOST_AUTO_TEST_CASE(test_get_or)
{
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

        toml::integer opt(0);
        BOOST_CHECK_EQUAL(42, toml::get_or(v1, opt));
        BOOST_CHECK_EQUAL(0,  toml::get_or(v2, opt));

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

BOOST_AUTO_TEST_CASE(test_find_or)
{
    // requires conversion int -> uint
    {
        toml::value v = toml::table{{"num", 42}};
        BOOST_CHECK_EQUAL(42u, toml::find_or(v, "num", 0u));
        BOOST_CHECK_EQUAL(0u,  toml::find_or(v, "foo", 0u));
    }
    // exact toml type
    {
        toml::value v1 = toml::table{{"key", 42  }};
        toml::value v2 = toml::table{{"key", 3.14}};
        toml::value v3 = toml::table{{"not", "key"}};

        BOOST_CHECK_EQUAL(42, toml::find_or(v1, "key", toml::integer(0)));
        BOOST_CHECK_EQUAL( 0, toml::find_or(v2, "key", toml::integer(0)));
        BOOST_CHECK_EQUAL( 0, toml::find_or(v3, "key", toml::integer(0)));

        toml::value v4 = toml::table{{"str", "foobar"}};
        toml::string s("bazqux");

        BOOST_CHECK_EQUAL("foobar", toml::find_or(v4, "str", s));
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(v1, "str", s));
    }
    // std::string
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key", 42}};

        std::string       s1("bazqux");
        const std::string s2("bazqux");

        BOOST_CHECK_EQUAL("foobar", toml::find_or(v1, "key", s1));
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(v2, "key", s1));

        std::string& v1r = toml::find_or(v1, "key", s1);
        std::string& s1r = toml::find_or(v2, "key", s1);

        BOOST_CHECK_EQUAL("foobar", v1r);
        BOOST_CHECK_EQUAL("bazqux", s1r);

        BOOST_CHECK_EQUAL("foobar", toml::find_or(v1, "key", s2));
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(v2, "key", s2));

        BOOST_CHECK_EQUAL("foobar", toml::find_or(std::move(v1), "key", std::move(s1)));
        s1 = "bazqux"; // restoring moved value
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(std::move(v2), "key", std::move(s1)));
    }
    // string literal
    {
        toml::value v1 = toml::table{{"key", "foobar"}};
        toml::value v2 = toml::table{{"key",42}};

        BOOST_CHECK_EQUAL("foobar", toml::find_or(v1, "key", "bazqux"));
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(v2, "key", "bazqux"));

        const char* lit = "bazqux";
        BOOST_CHECK_EQUAL("foobar", toml::find_or(v1, "key", lit));
        BOOST_CHECK_EQUAL("bazqux", toml::find_or(v2, "key", lit));
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

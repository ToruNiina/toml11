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
    {
        toml::value v1(42);
        toml::value v2(3.14);
        BOOST_CHECK_EQUAL(42, toml::get_or<int>(v1, 0));
        BOOST_CHECK_EQUAL(0,  toml::get_or<int>(v2, 0));
    }
}

BOOST_AUTO_TEST_CASE(test_expect)
{
    {
        toml::value v1(42);
        toml::value v2(3.14);
        BOOST_CHECK_EQUAL(42, toml::expect<int>(v1).unwrap_or(0));
        BOOST_CHECK_EQUAL( 0, toml::expect<int>(v2).unwrap_or(0));
        BOOST_CHECK_EQUAL("42",   toml::expect<int>(v1).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none")));
        BOOST_CHECK_EQUAL("none", toml::expect<int>(v2).map([](int i){return std::to_string(i);}).unwrap_or(std::string("none")));
    }
}

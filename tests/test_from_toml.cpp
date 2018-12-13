#define BOOST_TEST_MODULE "test_from_toml"
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

BOOST_AUTO_TEST_CASE(test_from_toml)
{
    toml::boolean    b = false;
    toml::integer    i = 0;
    toml::floating   f = 0.;
    toml::string     s;
    toml::local_date dt;
    toml::array      a;
    toml::table      t;
    {
        toml::value v(true);
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK_EQUAL(b, true);
    }
    {
        toml::value v(42);
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK_EQUAL(i, 42);
    }
    {
        toml::value v(3.14);
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK_EQUAL(f, 3.14);
    }
    {
        toml::value v("foo");

        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK_EQUAL(s, "foo");
    }
    {
        toml::value v(toml::local_date(2018, toml::month_t::Apr, 22));
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK(dt == toml::local_date(2018, toml::month_t::Apr, 22));
    }
    {
        toml::array ref{toml::value(42), toml::value(54)};
        toml::value v(ref);
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK(ref == a);
    }
    {
        toml::table ref{{"key1", 42}, {"key2", 3.14}};
        toml::value v(ref);
        toml::from_toml(std::tie(b, i, f, s, dt, a, t), v);
        BOOST_CHECK(ref == t);
    }
}


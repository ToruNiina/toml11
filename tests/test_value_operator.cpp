#define BOOST_TEST_MODULE "test_value_operator"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <map>
#include <list>

BOOST_AUTO_TEST_CASE(test_value_equal)
{
    toml::Boolean  b(true);
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

    toml::value v1(b);
    toml::value v2(i);
    toml::value v3(f);
    toml::value v4(s);
    toml::value v5(d);
    toml::value v6(a);
    toml::value v7(t);

    toml::value u1(b);
    toml::value u2(i);
    toml::value u3(f);
    toml::value u4(s);
    toml::value u5(d);
    toml::value u6(a);
    toml::value u7(t);

    const bool b1 = v1 == u1;
    const bool b2 = v2 == u2;
    const bool b3 = v3 == u3;
    const bool b4 = v4 == u4;
    const bool b5 = v5 == u5;
    const bool b6 = v6 == u6;
    const bool b7 = v7 == u7;

    BOOST_CHECK(b1);
    BOOST_CHECK(b2);
    BOOST_CHECK(b3);
    BOOST_CHECK(b4);
    BOOST_CHECK(b5);
    BOOST_CHECK(b6);
    BOOST_CHECK(b7);

    {
    const bool n1 = v1 != u1;
    const bool n2 = v2 != u2;
    const bool n3 = v3 != u3;
    const bool n4 = v4 != u4;
    const bool n5 = v5 != u5;
    const bool n6 = v6 != u6;
    const bool n7 = v7 != u7;

    BOOST_CHECK(!n1);
    BOOST_CHECK(!n2);
    BOOST_CHECK(!n3);
    BOOST_CHECK(!n4);
    BOOST_CHECK(!n5);
    BOOST_CHECK(!n6);
    BOOST_CHECK(!n7);
    }

    {
    const bool n1 = v1 == u2;
    const bool n2 = v2 == u3;
    const bool n3 = v3 == u4;
    const bool n4 = v4 == u5;
    const bool n5 = v5 == u6;
    const bool n6 = v6 == u7;
    const bool n7 = v7 == u1;

    BOOST_CHECK(!n1);
    BOOST_CHECK(!n2);
    BOOST_CHECK(!n3);
    BOOST_CHECK(!n4);
    BOOST_CHECK(!n5);
    BOOST_CHECK(!n6);
    BOOST_CHECK(!n7);
    }
}

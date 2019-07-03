#define BOOST_TEST_MODULE "test_expect"
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

#define BOOST_TEST_MODULE "test_acceptor"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/utility.hpp>
#include <vector>
#include <array>

BOOST_AUTO_TEST_CASE(test_resize)
{
    {
        typedef std::vector<int>  resizable_type;
        typedef std::array<int,1> non_resizable_type;
        BOOST_TEST(toml::detail::has_resize_method<resizable_type>::value);
        BOOST_TEST(!toml::detail::has_resize_method<non_resizable_type>::value);
    }
    {
        std::vector<int> v;
        toml::resize(v, 10);
        BOOST_TEST(v.size() == 10u);
    }
    {
        std::array<int, 15> a;
        toml::resize(a, 10);
        BOOST_TEST(a.size() == 15u);
    }
    {
        std::array<int, 15> a;
        BOOST_CHECK_THROW(toml::resize(a, 20), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(test_concat_to_string)
{
    const std::string cat = toml::concat_to_string("foo", "bar", 42);
    BOOST_TEST(cat == "foobar42");
}

BOOST_AUTO_TEST_CASE(test_from_string)
{
    {
        const std::string str("123");
        BOOST_TEST(toml::from_string<int>(str, 0) == 123);
    }
    {
        const std::string str("01");
        BOOST_TEST(toml::from_string<int>(str, 0) == 1);
    }
}

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
        BOOST_CHECK(toml::detail::has_resize_method<resizable_type>::value);
        BOOST_CHECK(!toml::detail::has_resize_method<non_resizable_type>::value);
    }

    {
        bool thrown = false;
        std::vector<int> v;
        try
        {
            toml::resize(v, 10);
        }
        catch(std::exception& ex)
        {
            thrown = true;
        }
        BOOST_CHECK(!thrown);
        BOOST_CHECK_EQUAL(v.size(), 10u);
    }

    {
        bool thrown = false;
        std::array<int, 15> a;
        try
        {
            toml::resize(a, 10);
        }
        catch(std::exception& ex)
        {
            thrown = true;
        }
        BOOST_CHECK(!thrown);
        BOOST_CHECK_EQUAL(a.size(), 15u);
    }

    {
        bool thrown = false;
        std::array<int, 15> a;
        try
        {
            toml::resize(a, 20);
        }
        catch(std::exception& ex)
        {
            thrown = true;
        }
        BOOST_CHECK(thrown);
    }
}

BOOST_AUTO_TEST_CASE(test_concat_to_string)
{
    const std::string cat = toml::concat_to_string("foo", "bar", 42);
    BOOST_CHECK(cat == "foobar42");
}

BOOST_AUTO_TEST_CASE(test_from_string)
{
    {
        const std::string str("123");
        BOOST_CHECK_EQUAL(toml::from_string<int>(str, 0), 123);
    }
    {
        const std::string str("01");
        BOOST_CHECK_EQUAL(toml::from_string<int>(str, 0), 1);
    }
}

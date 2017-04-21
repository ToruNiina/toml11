#define BOOST_TEST_MODULE "test_datetime"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/toml.hpp>

BOOST_AUTO_TEST_CASE(test_datetime_convertible)
{
    const auto now = std::chrono::system_clock::now();
    toml::Datetime d1(now);
    const std::chrono::system_clock::time_point cvt(d1);
    toml::Datetime d2(cvt);

    const auto time = std::chrono::system_clock::to_time_t(now);
    toml::Datetime d3(time);
    toml::Datetime d4(std::chrono::system_clock::from_time_t(time));

    BOOST_CHECK_EQUAL(d1, d2);
    BOOST_CHECK_EQUAL(d3, d4);
}

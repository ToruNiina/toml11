#define BOOST_TEST_MODULE "parse_datetime_test"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/parser.hpp>
#include "test_parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_time)
{
    TOML11_TEST_PARSE_EQUAL(parse_local_time, "07:32:00",        toml::local_time(7, 32, 0));
    TOML11_TEST_PARSE_EQUAL(parse_local_time, "07:32:00.99",     toml::local_time(7, 32, 0, 990, 0));
    TOML11_TEST_PARSE_EQUAL(parse_local_time, "07:32:00.999",    toml::local_time(7, 32, 0, 999, 0));
    TOML11_TEST_PARSE_EQUAL(parse_local_time, "07:32:00.999999", toml::local_time(7, 32, 0, 999, 999));
}

BOOST_AUTO_TEST_CASE(test_time_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "07:32:00",        toml::value(toml::local_time(7, 32, 0)));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "07:32:00.99",     toml::value(toml::local_time(7, 32, 0, 990, 0)));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "07:32:00.999",    toml::value(toml::local_time(7, 32, 0, 999, 0)));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "07:32:00.999999", toml::value(toml::local_time(7, 32, 0, 999, 999)));
}

BOOST_AUTO_TEST_CASE(test_date)
{
    TOML11_TEST_PARSE_EQUAL(parse_local_date,  "1979-05-27",
                            toml::local_date(1979, toml::month_t::May, 27));
}
BOOST_AUTO_TEST_CASE(test_date_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27",
                            value(toml::local_date(1979, toml::month_t::May, 27)));
}

BOOST_AUTO_TEST_CASE(test_datetime)
{
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27T07:32:00",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27T07:32:00.99",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27T07:32:00.999999",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999)));

    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27t07:32:00",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27t07:32:00.99",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27t07:32:00.999999",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999)));

    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27 07:32:00",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27 07:32:00.99",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_local_datetime, "1979-05-27 07:32:00.999999",
        toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999)));
}

BOOST_AUTO_TEST_CASE(test_datetime_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.99",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.999999",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999))));

    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27t07:32:00",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27t07:32:00.99",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27t07:32:00.999999",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999))));

    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27 07:32:00",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27 07:32:00.99",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 990, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27 07:32:00.999999",
        toml::value(toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27), toml::local_time(7, 32, 0, 999, 999))));
}

BOOST_AUTO_TEST_CASE(test_offset_datetime)
{
    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00Z",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0), toml::time_offset(0, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.99Z",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 990, 0), toml::time_offset(0, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.999999Z",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 999, 999), toml::time_offset(0, 0)));

    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00+09:00",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0), toml::time_offset(9, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.99+09:00",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 990, 0), toml::time_offset(9, 0)));
    TOML11_TEST_PARSE_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.999999+09:00",
        toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 999, 999), toml::time_offset(9, 0)));
}

BOOST_AUTO_TEST_CASE(test_offset_datetime_value)
{
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00Z",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0), toml::time_offset(0, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.99Z",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 990, 0), toml::time_offset(0, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.999999Z",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 999, 999), toml::time_offset(0, 0))));

    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00+09:00",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0), toml::time_offset(9, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.99+09:00",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 990, 0), toml::time_offset(9, 0))));
    TOML11_TEST_PARSE_EQUAL_VALUE(parse_value<toml::value>, "1979-05-27T07:32:00.999999+09:00",
        toml::value(toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                              toml::local_time(7, 32, 0, 999, 999), toml::time_offset(9, 0))));
}

#define BOOST_TEST_MODULE "test_to_toml"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <map>
#include <list>

BOOST_AUTO_TEST_CASE(test_value_boolean)
{
    toml::value v1 = toml::to_toml(true);
    toml::value v2 = toml::to_toml(false);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Boolean);
    BOOST_CHECK(v1.is(toml::value_t::Boolean));
    BOOST_CHECK(v2.is(toml::value_t::Boolean));
    BOOST_CHECK(v1.is<toml::Boolean>());
    BOOST_CHECK(v2.is<toml::Boolean>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Boolean>(), true);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Boolean>(), false);
}

BOOST_AUTO_TEST_CASE(test_value_integer)
{
    toml::value v1 = toml::to_toml(-42);
    toml::value v2 = toml::to_toml(42u);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Integer);
    BOOST_CHECK(v1.is(toml::value_t::Integer));
    BOOST_CHECK(v2.is(toml::value_t::Integer));
    BOOST_CHECK(v1.is<toml::Integer>());
    BOOST_CHECK(v2.is<toml::Integer>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Integer>(), -42);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Integer>(),  42u);
}

BOOST_AUTO_TEST_CASE(test_value_float)
{
    toml::value v1 = toml::to_toml(3.14);
    toml::value v2 = toml::to_toml(3.14f);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Float);
    BOOST_CHECK(v1.is(toml::value_t::Float));
    BOOST_CHECK(v2.is(toml::value_t::Float));
    BOOST_CHECK(v1.is<toml::Float>());
    BOOST_CHECK(v2.is<toml::Float>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Float>(), 3.14);
    BOOST_CHECK_CLOSE_FRACTION(v2.cast<toml::value_t::Float>(), 3.14, 1e-2);
}

BOOST_AUTO_TEST_CASE(test_value_string)
{
    toml::value v1 = toml::to_toml(std::string("foo"));
    toml::value v2 = toml::to_toml(std::string("foo"), toml::string_t::literal);
    toml::value v3 = toml::to_toml("foo");

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::String);
    BOOST_CHECK(v1.is(toml::value_t::String));
    BOOST_CHECK(v2.is(toml::value_t::String));
    BOOST_CHECK(v3.is(toml::value_t::String));
    BOOST_CHECK(v1.is<toml::String>());
    BOOST_CHECK(v2.is<toml::String>());
    BOOST_CHECK(v3.is<toml::String>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::String>(), "foo");
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::String>(), "foo");
    BOOST_CHECK_EQUAL(v3.cast<toml::value_t::String>(), "foo");
}

BOOST_AUTO_TEST_CASE(test_value_local_date)
{
    toml::value v1 = toml::to_toml(toml::local_date(2018, toml::month_t::Jan, 31));

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::LocalDate);
    BOOST_CHECK(v1.is(toml::value_t::LocalDate));
    BOOST_CHECK(v1.is<toml::LocalDate>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::LocalDate>(),
                      toml::local_date(2018, toml::month_t::Jan, 31));
}

BOOST_AUTO_TEST_CASE(test_value_local_time)
{
    toml::value v1 = toml::to_toml(toml::local_time(12, 30, 45));
    toml::value v2 = toml::to_toml(std::chrono::hours(12) + std::chrono::minutes(30) +
                   std::chrono::seconds(45));

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::LocalTime);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::LocalTime);
    BOOST_CHECK(v1.is(toml::value_t::LocalTime));
    BOOST_CHECK(v2.is(toml::value_t::LocalTime));
    BOOST_CHECK(v1.is<toml::LocalTime>());
    BOOST_CHECK(v2.is<toml::LocalTime>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::LocalTime>(),
                      toml::local_time(12, 30, 45));
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::LocalTime>(),
                      toml::local_time(12, 30, 45));
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::LocalTime>(),
                      v2.cast<toml::value_t::LocalTime>());
}

BOOST_AUTO_TEST_CASE(test_value_local_datetime)
{
    toml::value v1 = toml::to_toml(toml::local_datetime(
                toml::local_date(2018, toml::month_t::Jan, 31),
                toml::local_time(12, 30, 45)
                ));

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::LocalDatetime);
    BOOST_CHECK(v1.is(toml::value_t::LocalDatetime));
    BOOST_CHECK(v1.is<toml::LocalDatetime>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::LocalDatetime>(),
                      toml::local_datetime(
                          toml::local_date(2018, toml::month_t::Jan, 31),
                          toml::local_time(12, 30, 45)));
}

BOOST_AUTO_TEST_CASE(test_value_offset_datetime)
{
    toml::value v1 = toml::to_toml(toml::offset_datetime(
                toml::local_date(2018, toml::month_t::Jan, 31),
                toml::local_time(12, 30, 45),
                toml::time_offset(9, 0)
                ));

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::OffsetDatetime);
    BOOST_CHECK(v1.is(toml::value_t::OffsetDatetime));
    BOOST_CHECK(v1.is<toml::OffsetDatetime>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::OffsetDatetime>(),
            toml::offset_datetime(
                toml::local_date(2018, toml::month_t::Jan, 31),
                toml::local_time(12, 30, 45),
                toml::time_offset(9, 0)
                ));
}

BOOST_AUTO_TEST_CASE(test_value_array)
{
    std::vector<int> v{1,2,3,4,5};
    toml::value v1 = toml::to_toml(v);
    toml::value v2 = toml::to_toml(6,7,8,9,0);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Array);
    BOOST_CHECK(v1.is(toml::value_t::Array));
    BOOST_CHECK(v1.is<toml::Array>());

    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Array);
    BOOST_CHECK(v2.is(toml::value_t::Array));
    BOOST_CHECK(v2.is<toml::Array>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Array>().at(0).cast<toml::value_t::Integer>(), 1);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Array>().at(1).cast<toml::value_t::Integer>(), 2);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Array>().at(2).cast<toml::value_t::Integer>(), 3);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Array>().at(3).cast<toml::value_t::Integer>(), 4);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Array>().at(4).cast<toml::value_t::Integer>(), 5);

    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Array>().at(0).cast<toml::value_t::Integer>(), 6);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Array>().at(1).cast<toml::value_t::Integer>(), 7);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Array>().at(2).cast<toml::value_t::Integer>(), 8);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Array>().at(3).cast<toml::value_t::Integer>(), 9);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Array>().at(4).cast<toml::value_t::Integer>(), 0);
}

BOOST_AUTO_TEST_CASE(test_value_table)
{
    toml::value v1 = toml::to_toml({{"foo", 42}, {"bar", 3.14}, {"baz", "qux"}});

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Table);
    BOOST_CHECK(v1.is(toml::value_t::Table));
    BOOST_CHECK(v1.is<toml::Table>());

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Table>().at("foo").cast<toml::value_t::Integer>(),    42);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Table>().at("bar").cast<toml::value_t::Float>(),      3.14);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Table>().at("baz").cast<toml::value_t::String>().str, "qux");
}

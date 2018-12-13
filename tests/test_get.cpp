#define BOOST_TEST_MODULE "test_get"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/value.hpp>
#include <toml/get.hpp>
#include <map>
#include <unordered_map>
#include <list>
#include <deque>
#include <array>


BOOST_AUTO_TEST_CASE(test_get_exact)
{
    {
        toml::value v(true);
        BOOST_CHECK_EQUAL(true, toml::get<toml::boolean>(v));

        toml::get<toml::boolean>(v) = false;
        BOOST_CHECK_EQUAL(false, toml::get<toml::boolean>(v));
    }
    {
        toml::value v(42);
        BOOST_CHECK_EQUAL(toml::integer(42), toml::get<toml::integer>(v));

        toml::get<toml::integer>(v) = 54;
        BOOST_CHECK_EQUAL(toml::integer(54), toml::get<toml::integer>(v));
    }
    {
        toml::value v(3.14);
        BOOST_CHECK_EQUAL(toml::floating(3.14), toml::get<toml::floating>(v));

        toml::get<toml::floating>(v) = 2.71;
        BOOST_CHECK_EQUAL(toml::floating(2.71), toml::get<toml::floating>(v));
    }
    {
        toml::value v("foo");
        BOOST_CHECK_EQUAL(toml::string("foo", toml::string_t::basic),
                          toml::get<toml::string>(v));

        toml::get<toml::string>(v).str += "bar";
        BOOST_CHECK_EQUAL(toml::string("foobar", toml::string_t::basic),
                          toml::get<toml::string>(v));
    }
    {
        toml::value v("foo", toml::string_t::literal);
        BOOST_CHECK_EQUAL(toml::string("foo", toml::string_t::literal),
                          toml::get<toml::string>(v));

        toml::get<toml::string>(v).str += "bar";
        BOOST_CHECK_EQUAL(toml::string("foobar", toml::string_t::literal),
                          toml::get<toml::string>(v));
    }
    {
        toml::local_date d(2018, toml::month_t::Apr, 22);
        toml::value v(d);
        BOOST_CHECK(d == toml::get<toml::local_date>(v));

        toml::get<toml::local_date>(v).year = 2017;
        d.year = 2017;
        BOOST_CHECK(d == toml::get<toml::local_date>(v));
    }
    {
        toml::local_time t(12, 30, 45);
        toml::value v(t);
        BOOST_CHECK(t == toml::get<toml::local_time>(v));

        toml::get<toml::local_time>(v).hour = 9;
        t.hour = 9;
        BOOST_CHECK(t == toml::get<toml::local_time>(v));
    }
    {
        toml::local_datetime dt(toml::local_date(2018, toml::month_t::Apr, 22),
                                toml::local_time(12, 30, 45));
        toml::value v(dt);
        BOOST_CHECK(dt == toml::get<toml::local_datetime>(v));

        toml::get<toml::local_datetime>(v).date.year = 2017;
        dt.date.year = 2017;
        BOOST_CHECK(dt == toml::get<toml::local_datetime>(v));
    }
    {
        toml::offset_datetime dt(toml::local_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 30, 45)), toml::time_offset(9, 0));
        toml::value v(dt);
        BOOST_CHECK(dt == toml::get<toml::offset_datetime>(v));

        toml::get<toml::offset_datetime>(v).date.year = 2017;
        dt.date.year = 2017;
        BOOST_CHECK(dt == toml::get<toml::offset_datetime>(v));
    }
    {
        toml::array vec;
        vec.push_back(toml::value(42));
        vec.push_back(toml::value(54));
        toml::value v(vec);
        BOOST_CHECK(vec == toml::get<toml::array>(v));

        toml::get<toml::array>(v).push_back(toml::value(123));
        vec.push_back(toml::value(123));
        BOOST_CHECK(vec == toml::get<toml::array>(v));
    }
    {
        toml::table tab;
        tab["key1"] = toml::value(42);
        tab["key2"] = toml::value(3.14);
        toml::value v(tab);
        BOOST_CHECK(tab == toml::get<toml::table>(v));

        toml::get<toml::table>(v)["key3"] = toml::value(123);
        tab["key3"] = toml::value(123);
        BOOST_CHECK(tab == toml::get<toml::table>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_integer_type)
{
    {
        toml::value v(42);
        BOOST_CHECK_EQUAL(int(42),           toml::get<int          >(v));
        BOOST_CHECK_EQUAL(short(42),         toml::get<short        >(v));
        BOOST_CHECK_EQUAL(char(42),          toml::get<char         >(v));
        BOOST_CHECK_EQUAL(unsigned(42),      toml::get<unsigned     >(v));
        BOOST_CHECK_EQUAL(long(42),          toml::get<long         >(v));
        BOOST_CHECK_EQUAL(std::int64_t(42),  toml::get<std::int64_t >(v));
        BOOST_CHECK_EQUAL(std::uint64_t(42), toml::get<std::uint64_t>(v));
        BOOST_CHECK_EQUAL(std::int16_t(42),  toml::get<std::int16_t >(v));
        BOOST_CHECK_EQUAL(std::uint16_t(42), toml::get<std::uint16_t>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_floating_type)
{
    {
        toml::value v(3.14);
        BOOST_CHECK_EQUAL(static_cast<float      >(3.14), toml::get<float      >(v));
        BOOST_CHECK_EQUAL(static_cast<double     >(3.14), toml::get<double     >(v));
        BOOST_CHECK_EQUAL(static_cast<long double>(3.14), toml::get<long double>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_string_type)
{
    {
        toml::value v("foo", toml::string_t::basic);
        BOOST_CHECK_EQUAL("foo", toml::get<std::string>(v));
        toml::get<std::string>(v) += "bar";
        BOOST_CHECK_EQUAL("foobar", toml::get<std::string>(v));
    }
    {
        toml::value v("foo", toml::string_t::literal);
        BOOST_CHECK_EQUAL("foo", toml::get<std::string>(v));
        toml::get<std::string>(v) += "bar";
        BOOST_CHECK_EQUAL("foobar", toml::get<std::string>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_toml_array)
{
    toml::value v(toml::array(0));
    toml::get<toml::array>(v).push_back(toml::value(42));
    toml::get<toml::array>(v).push_back(toml::value(54));
    toml::get<toml::array>(v).push_back(toml::value(69));
    toml::get<toml::array>(v).push_back(toml::value(72));

    const std::vector<int>         vec = toml::get<std::vector<int>>(v);
    const std::list<short>         lst = toml::get<std::list<short>>(v);
    const std::deque<std::int64_t> deq = toml::get<std::deque<std::int64_t>>(v);

    BOOST_CHECK_EQUAL(42, vec.at(0));
    BOOST_CHECK_EQUAL(54, vec.at(1));
    BOOST_CHECK_EQUAL(69, vec.at(2));
    BOOST_CHECK_EQUAL(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    BOOST_CHECK_EQUAL(static_cast<short>(42), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(54), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(69), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(72), *(iter++));

    BOOST_CHECK_EQUAL(static_cast<std::int64_t>(42), deq.at(0));
    BOOST_CHECK_EQUAL(static_cast<std::int64_t>(54), deq.at(1));
    BOOST_CHECK_EQUAL(static_cast<std::int64_t>(69), deq.at(2));
    BOOST_CHECK_EQUAL(static_cast<std::int64_t>(72), deq.at(3));

    std::array<int, 4> ary = toml::get<std::array<int, 4>>(v);
    BOOST_CHECK_EQUAL(static_cast<int>(42), ary.at(0));
    BOOST_CHECK_EQUAL(static_cast<int>(54), ary.at(1));
    BOOST_CHECK_EQUAL(static_cast<int>(69), ary.at(2));
    BOOST_CHECK_EQUAL(static_cast<int>(72), ary.at(3));

    std::tuple<int, short, unsigned, long> tpl =
        toml::get<std::tuple<int, short, unsigned, long>>(v);
    BOOST_CHECK_EQUAL(static_cast<int     >(42), std::get<0>(tpl));
    BOOST_CHECK_EQUAL(static_cast<short   >(54), std::get<1>(tpl));
    BOOST_CHECK_EQUAL(static_cast<unsigned>(69), std::get<2>(tpl));
    BOOST_CHECK_EQUAL(static_cast<long    >(72), std::get<3>(tpl));

    toml::value p(toml::array{});
    toml::get<toml::array>(p).push_back(toml::value(3.14));
    toml::get<toml::array>(p).push_back(toml::value(2.71));
    std::pair<double, double> pr = toml::get<std::pair<double, double> >(p);
    BOOST_CHECK_EQUAL(3.14, pr.first);
    BOOST_CHECK_EQUAL(2.71, pr.second);
}

BOOST_AUTO_TEST_CASE(test_get_toml_array_of_array)
{
    toml::value v1(toml::array{});
    toml::get<toml::array>(v1).push_back(toml::value(42));
    toml::get<toml::array>(v1).push_back(toml::value(54));
    toml::get<toml::array>(v1).push_back(toml::value(69));
    toml::get<toml::array>(v1).push_back(toml::value(72));

    toml::value v2(toml::array{});
    toml::get<toml::array>(v2).push_back(toml::value("foo"));
    toml::get<toml::array>(v2).push_back(toml::value("bar"));
    toml::get<toml::array>(v2).push_back(toml::value("baz"));

    toml::value v(toml::array(2));
    toml::get<toml::array>(v).at(0) = v1;
    toml::get<toml::array>(v).at(1) = v2;

    std::pair<std::vector<int>, std::vector<std::string>> p =
        toml::get<std::pair<std::vector<int>, std::vector<std::string>>>(v);

    BOOST_CHECK_EQUAL(p.first.at(0), 42);
    BOOST_CHECK_EQUAL(p.first.at(1), 54);
    BOOST_CHECK_EQUAL(p.first.at(2), 69);
    BOOST_CHECK_EQUAL(p.first.at(3), 72);

    BOOST_CHECK_EQUAL(p.second.at(0), "foo");
    BOOST_CHECK_EQUAL(p.second.at(1), "bar");
    BOOST_CHECK_EQUAL(p.second.at(2), "baz");

    std::tuple<std::vector<int>, std::vector<std::string>> t =
        toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);

    BOOST_CHECK_EQUAL(std::get<0>(t).at(0), 42);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(1), 54);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(2), 69);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(3), 72);

    BOOST_CHECK_EQUAL(std::get<1>(t).at(0), "foo");
    BOOST_CHECK_EQUAL(std::get<1>(t).at(1), "bar");
    BOOST_CHECK_EQUAL(std::get<1>(t).at(2), "baz");
}

BOOST_AUTO_TEST_CASE(test_get_toml_table)
{
    toml::value v1(toml::table{
            {"key1", 1},
            {"key2", 2},
            {"key3", 3},
            {"key4", 4}
            });

    const auto v = toml::get<std::map<std::string, int>>(v1);
    BOOST_CHECK_EQUAL(v.at("key1"), 1);
    BOOST_CHECK_EQUAL(v.at("key2"), 2);
    BOOST_CHECK_EQUAL(v.at("key3"), 3);
    BOOST_CHECK_EQUAL(v.at("key4"), 4);
}


BOOST_AUTO_TEST_CASE(test_get_toml_local_date)
{
    toml::value v1(toml::local_date{2018, toml::month_t::Apr, 1});
    const auto date = std::chrono::system_clock::to_time_t(
            toml::get<std::chrono::system_clock::time_point>(v1));

    std::tm t;
    t.tm_year = 2018 - 1900;
    t.tm_mon  = 4    - 1;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    t.tm_isdst = -1;
    const auto c = std::mktime(&t);
    BOOST_CHECK_EQUAL(c, date);
}

BOOST_AUTO_TEST_CASE(test_get_toml_local_time)
{
    toml::value v1(toml::local_time{12, 30, 45});
    const auto time = toml::get<std::chrono::seconds>(v1);
    BOOST_CHECK(time == std::chrono::hours(12) +
                        std::chrono::minutes(30) + std::chrono::seconds(45));
}

BOOST_AUTO_TEST_CASE(test_get_toml_local_datetime)
{
    toml::value v1(toml::local_datetime(
                toml::local_date{2018, toml::month_t::Apr, 1},
                toml::local_time{12, 30, 45}));

    const auto date = std::chrono::system_clock::to_time_t(
            toml::get<std::chrono::system_clock::time_point>(v1));
    std::tm t;
    t.tm_year = 2018 - 1900;
    t.tm_mon  = 4    - 1;
    t.tm_mday = 1;
    t.tm_hour = 12;
    t.tm_min  = 30;
    t.tm_sec  = 45;
    t.tm_isdst = -1;
    const auto c = std::mktime(&t);
    BOOST_CHECK_EQUAL(c, date);
}

BOOST_AUTO_TEST_CASE(test_get_toml_offset_datetime)
{
    {
    toml::value v1(toml::offset_datetime(
                toml::local_date{2018, toml::month_t::Apr, 1},
                toml::local_time{12, 30, 0},
                toml::time_offset{9, 0}));
    //    2018-04-01T12:30:00+09:00
    // == 2018-04-01T03:30:00Z

    const auto date = toml::get<std::chrono::system_clock::time_point>(v1);
    const auto timet = std::chrono::system_clock::to_time_t(date);

    // get time_t as gmtime (2018-04-01T03:30:00Z)
    const auto tmp = std::gmtime(std::addressof(timet)); // XXX not threadsafe!
    BOOST_CHECK(tmp);
    const auto tm = *tmp;
    BOOST_CHECK_EQUAL(tm.tm_year + 1900, 2018);
    BOOST_CHECK_EQUAL(tm.tm_mon  + 1,       4);
    BOOST_CHECK_EQUAL(tm.tm_mday,           1);
    BOOST_CHECK_EQUAL(tm.tm_hour,           3);
    BOOST_CHECK_EQUAL(tm.tm_min,           30);
    BOOST_CHECK_EQUAL(tm.tm_sec,            0);
    }

    {
    toml::value v1(toml::offset_datetime(
                toml::local_date{2018, toml::month_t::Apr, 1},
                toml::local_time{12, 30, 0},
                toml::time_offset{-8, 0}));
    //    2018-04-01T12:30:00-08:00
    // == 2018-04-01T20:30:00Z

    const auto date = toml::get<std::chrono::system_clock::time_point>(v1);
    const auto timet = std::chrono::system_clock::to_time_t(date);

    // get time_t as gmtime (2018-04-01T03:30:00Z)
    const auto tmp = std::gmtime(std::addressof(timet)); // XXX not threadsafe!
    BOOST_CHECK(tmp);
    const auto tm = *tmp;
    BOOST_CHECK_EQUAL(tm.tm_year + 1900, 2018);
    BOOST_CHECK_EQUAL(tm.tm_mon  + 1,       4);
    BOOST_CHECK_EQUAL(tm.tm_mday,           1);
    BOOST_CHECK_EQUAL(tm.tm_hour,          20);
    BOOST_CHECK_EQUAL(tm.tm_min,           30);
    BOOST_CHECK_EQUAL(tm.tm_sec,            0);
    }
}


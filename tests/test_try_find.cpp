#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/value.hpp>
#include <toml11/try_find.hpp>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

#if defined(TOML11_HAS_STRING_VIEW)
#include <string_view>
#endif

namespace toml
{
namespace detail
{
std::tm localtime_s(const std::time_t* src);
std::tm gmtime_s(const std::time_t* src);
} // detail
} // toml

TEST_CASE("testing toml::try_find with toml type")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;
    using floating_type        = typename value_type::floating_type       ;
    using string_type          = typename value_type::string_type         ;
    using local_time_type      = typename value_type::local_time_type     ;
    using local_date_type      = typename value_type::local_date_type     ;
    using local_datetime_type  = typename value_type::local_datetime_type ;
    using offset_datetime_type = typename value_type::offset_datetime_type;
    using array_type           = typename value_type::array_type          ;
    using table_type           = typename value_type::table_type          ;

    {
        value_type v(toml::table{{"a", true}});
        CHECK_EQ(true, toml::try_find<boolean_type>(v, "a").unwrap());

        toml::try_find<boolean_type>(v, "a").unwrap() = false;
        CHECK_EQ(false, toml::try_find<boolean_type>(v, "a").unwrap());

        boolean_type x = toml::try_find<boolean_type>(std::move(v), "a").unwrap();
        CHECK_EQ(false, x);
    }
    {
        value_type v(toml::table{{"a", 42}});
        CHECK_EQ(integer_type(42), toml::try_find<integer_type>(v, "a").unwrap());

        toml::try_find<integer_type>(v, "a").unwrap() = 54;
        CHECK_EQ(integer_type(54), toml::try_find<integer_type>(v, "a").unwrap());

        integer_type x = toml::try_find<integer_type>(std::move(v), "a").unwrap();
        CHECK_EQ(integer_type(54), x);
    }
    {
        value_type v(toml::table{{"a", 3.14}});
        CHECK_EQ(floating_type(3.14), toml::try_find<floating_type>(v, "a").unwrap());

        toml::try_find<floating_type>(v, "a").unwrap() = 2.71;
        CHECK_EQ(floating_type(2.71), toml::try_find<floating_type>(v, "a").unwrap());

        floating_type x = toml::try_find<floating_type>(std::move(v), "a").unwrap();
        CHECK_EQ(floating_type(2.71), x);
    }
    {
        value_type v(toml::table{{"a", "foo"}});
        CHECK_EQ("foo", toml::try_find<string_type>(v, "a").unwrap());

        toml::try_find<string_type>(v, "a").unwrap() += "bar";
        CHECK_EQ("foobar", toml::try_find<string_type>(v, "a").unwrap());

        string_type x = toml::try_find<string_type>(std::move(v), "a").unwrap();
        CHECK_EQ("foobar", x);
    }
    {
        local_date_type d(2018, toml::month_t::Apr, 22);
        value_type v(toml::table{{"a", d}});
        CHECK_EQ(d, toml::try_find<local_date_type>(v, "a").unwrap());

        toml::try_find<local_date_type>(v, "a").unwrap().year = 2017;
        d.year = 2017;
        CHECK_EQ(d, toml::try_find<local_date_type>(v, "a").unwrap());

        local_date_type x = toml::try_find<local_date_type>(std::move(v), "a").unwrap();
        CHECK_EQ(d, x);
    }
    {
        local_time_type t(12, 30, 45);
        value_type v(toml::table{{"a", t}});
        CHECK_EQ(t, toml::try_find<local_time_type>(v, "a").unwrap());

        toml::try_find<local_time_type>(v, "a").unwrap().hour = 9;
        t.hour = 9;
        CHECK_EQ(t, toml::try_find<local_time_type>(v, "a").unwrap());

        local_time_type x = toml::try_find<local_time_type>(std::move(v), "a").unwrap();
        CHECK_EQ(t, x);
    }
    {
        local_datetime_type dt(toml::local_date(2018, toml::month_t::Apr, 22),
                               toml::local_time(12, 30, 45));
        value_type v(toml::table{{"a", dt}});
        CHECK_EQ(dt, toml::try_find<local_datetime_type>(v, "a").unwrap());

        toml::try_find<local_datetime_type>(v, "a").unwrap().date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::try_find<local_datetime_type>(v, "a").unwrap());

        toml::local_datetime x = toml::try_find<local_datetime_type>(std::move(v), "a").unwrap();
        CHECK_EQ(dt, x);
    }
    {
        offset_datetime_type dt(toml::local_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 30, 45)), toml::time_offset(9, 0));
        value_type v(toml::table{{"a", dt}});
        CHECK_EQ(dt, toml::try_find<offset_datetime_type>(v, "a").unwrap());

        toml::try_find<toml::offset_datetime>(v, "a").unwrap().date.year = 2017;
        dt.date.year = 2017;
        CHECK_EQ(dt, toml::try_find<offset_datetime_type>(v, "a").unwrap());

        offset_datetime_type x = toml::try_find<offset_datetime_type>(std::move(v), "a").unwrap();
        CHECK_EQ(dt, x);
    }
    {
        array_type vec;
        vec.push_back(value_type(42));
        vec.push_back(value_type(54));
        value_type v(toml::table{{"a", vec}});
        CHECK_EQ(vec, toml::try_find<array_type>(v, "a").unwrap());

        toml::try_find<array_type>(v, "a").unwrap().push_back(value_type(123));
        vec.push_back(value_type(123));
        CHECK_EQ(vec, toml::try_find<array_type>(v, "a").unwrap());

        array_type x = toml::try_find<array_type>(std::move(v), "a").unwrap();
        CHECK_EQ(vec, x);
    }
    {
        table_type tab;
        tab["key1"] = value_type(42);
        tab["key2"] = value_type(3.14);
        value_type v(toml::table{{"a", tab}});
        CHECK_EQ(tab, toml::try_find<table_type>(v, "a").unwrap());

        toml::try_find<table_type>(v, "a").unwrap()["key3"] = value_type(123);
        tab["key3"] = value_type(123);
        CHECK_EQ(tab, toml::try_find<table_type>(v, "a").unwrap());

        table_type x = toml::try_find<table_type>(std::move(v), "a").unwrap();
        CHECK_EQ(tab, x);
    }
    {
        value_type v1(toml::table{{"a", 42}});
        CHECK_EQ(toml::value(42), toml::try_find(v1, "a").unwrap());

        value_type v2(54);
        toml::try_find(v1, "a").unwrap() = v2;
        CHECK_EQ(v2, toml::try_find(v1, "a").unwrap());

        value_type x = toml::try_find(std::move(v1), "a").unwrap();
        CHECK_EQ(v2, x);
    }
}

TEST_CASE("testing try_find fails")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;

    // -----------------------------------------------------------------------
    // const-reference version
    {
        // value is not a table
        const toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(v, "key").is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        const toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<boolean_type>(v, "key").is_err());
    }
    {
        // the value corresponding to the key is not found
        const toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<integer_type>(v, "different_key").is_err());
    }
    {
        // the positive control.
        const toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<int>(v, "key").is_ok());
    }

    // -----------------------------------------------------------------------
    // reference version
    {
        // value is not a table
        toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(v, "key").is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<boolean_type>(v, "key").is_err());
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<integer_type>(v, "different_key").is_err());
    }
    {
        // the positive control.
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<int>(v, "key").is_ok());
    }

    // -----------------------------------------------------------------------
    // move version

    {
        // value is not a table
        toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(std::move(v), "key").is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<boolean_type>(std::move(v), "key").is_err());
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<integer_type>(std::move(v), "different_key").is_err());
    }
    {
        // the positive control.
        toml::value v = toml::table{{"key", 42}};
        CHECK_UNARY(toml::try_find<int>(std::move(v), "key").is_ok());
    }
}

TEST_CASE("testing toml::try_find(v, idx) throws")
{
    using value_type = toml::value;
    using boolean_type         = typename value_type::boolean_type        ;
    using integer_type         = typename value_type::integer_type        ;

    // -----------------------------------------------------------------------
    // const-reference version
    {
        // value is not an array
        const toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(v, 0).is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<boolean_type>(v, 0).is_err());
    }
    {
        // the value corresponding to the key is not found
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<integer_type>(v, 6).is_err());
    }
    {
        // the positive control.
        const toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<int>(v, 2).is_ok());
    }

    // -----------------------------------------------------------------------
    // non-const reference version
    {
        // value is not an array
        toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(v, 0).is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<boolean_type>(v, 0).is_err());
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<integer_type>(v, 6).is_err());
    }
    {
        // the positive control.
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<int>(v, 2).is_ok());
    }

    // -----------------------------------------------------------------------
    // move version
    {
        // value is not an array
        toml::value v(true);
        CHECK_UNARY(toml::try_find<boolean_type>(std::move(v), 0).is_err());
    }
    {
        // the value corresponding to the key is not the expected type
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<boolean_type>(std::move(v), 0).is_err());
    }
    {
        // the value corresponding to the key is not found
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<integer_type>(std::move(v), 6).is_err());
    }
    {
        // the positive control.
        toml::value v = toml::array{1, 2, 3, 4, 5};
        CHECK_UNARY(toml::try_find<int>(std::move(v), 2).is_ok());
    }
}

TEST_CASE("testing toml::try_find with recursive table/array")
{
    using value_type = toml::value;
    using integer_type         = typename value_type::integer_type        ;

    // recursively search tables
    {
        toml::value v = toml::table{
            {"a", toml::table{
                {"b", toml::table{
                    {"c", toml::table{
                        {"d", 42}
                    }}
                }}
            }}
        };
        CHECK_EQ(42, toml::try_find<int>(v, "a", "b", "c", "d").unwrap());

        // reference that can be used to modify the content
        auto num = toml::try_find<integer_type>(v, "a", "b", "c", "d");
        CHECK_UNARY(num.is_ok());
        num.unwrap() = 54;
        CHECK_EQ(54, toml::try_find<int>(v, "a", "b", "c", "d").unwrap());

        const std::string a("a"), b("b"), c("c"), d("d");
        auto num2 = toml::try_find<integer_type>(v, a, b, c, d);
        CHECK_UNARY(num2.is_ok());
        num2.unwrap() = 42;
        CHECK_EQ(42, toml::try_find<int>(v, a, b, c, d).unwrap());

        auto num3 = toml::try_find<integer_type>(v, a, "b", c, "d");
        CHECK_EQ(42, num3.unwrap());

        auto num4 = toml::try_find<integer_type>(std::move(v), a, b, c, d);
        CHECK_EQ(42, num4.unwrap());
    }
    // recursively search arrays
    {
        toml::value v = toml::array{
            toml::array{"array", "of", "string"},
            toml::array{toml::array{1, 2, 3}, toml::array{3.14, 2.71}}
        };
        CHECK_EQ("array" , toml::try_find<std::string>(v, 0, 0).unwrap());
        CHECK_EQ("of"    , toml::try_find<std::string>(v, 0, 1).unwrap());
        CHECK_EQ("string", toml::try_find<std::string>(v, 0, 2).unwrap());

        CHECK_EQ(1, toml::try_find<int>(v, 1, 0, 0).unwrap());
        CHECK_EQ(2, toml::try_find<int>(v, 1, 0, 1).unwrap());
        CHECK_EQ(3, toml::try_find<int>(v, 1, 0, 2).unwrap());

        CHECK_EQ(3.14, toml::try_find<double>(v, 1, 1, 0).unwrap());
        CHECK_EQ(2.71, toml::try_find<double>(v, 1, 1, 1).unwrap());

        // reference that can be used to modify the content
        auto num = toml::try_find<integer_type>(v, 1, 0, 2);
        num.unwrap() = 42;
        CHECK_EQ( 1, toml::try_find<int>(v, 1, 0, 0).unwrap());
        CHECK_EQ( 2, toml::try_find<int>(v, 1, 0, 1).unwrap());
        CHECK_EQ(42, toml::try_find<int>(v, 1, 0, 2).unwrap());

        // move value
        auto num2 = toml::try_find<integer_type>(std::move(v), 1, 0, 2);
        CHECK_EQ(42, num2.unwrap());
    }
    // recursively search mixtures
    {
        toml::value v = toml::table{{"array", toml::array{
                toml::array{1,   2,   3},
                toml::array{
                    toml::table{{"foo", "bar"}, {"baz", "qux"}},
                    toml::table{{"pi",   3.14}, {"e",    2.71}}
                }}
            }};

        CHECK_EQ(1, toml::try_find<int>(v, "array", 0, 0).unwrap());
        CHECK_EQ(2, toml::try_find<int>(v, "array", 0, 1).unwrap());
        CHECK_EQ(3, toml::try_find<int>(v, "array", 0, 2).unwrap());

        CHECK_EQ("bar", toml::try_find<std::string>(v, "array", 1, 0, "foo").unwrap());
        CHECK_EQ("qux", toml::try_find<std::string>(v, "array", 1, 0, "baz").unwrap());

        CHECK_EQ(3.14, toml::try_find<double>(v, "array", 1, 1, "pi").unwrap());
        CHECK_EQ(2.71, toml::try_find<double>(v, "array", 1, 1, "e" ).unwrap());

        const std::string ar("array");
        const auto ar_c = "array";

        const std::string pi("pi");
        const auto pi_c = "pi";

        CHECK_EQ(3.14, toml::try_find<double>(v, ar, 1, 1, "pi").unwrap());
        CHECK_EQ(3.14, toml::try_find<double>(v, ar, 1, 1, pi)  .unwrap());
        CHECK_EQ(3.14, toml::try_find<double>(v, ar, 1, 1, pi_c).unwrap());

        CHECK_EQ(3.14, toml::try_find<double>(v, ar_c, 1, 1, "pi").unwrap());
        CHECK_EQ(3.14, toml::try_find<double>(v, ar_c, 1, 1, pi)  .unwrap());
        CHECK_EQ(3.14, toml::try_find<double>(v, ar_c, 1, 1, pi_c).unwrap());

        CHECK_EQ(3.14, toml::try_find<double>(v, "array", 1, 1, pi)  .unwrap());
        CHECK_EQ(3.14, toml::try_find<double>(v, "array", 1, 1, pi_c).unwrap());
    }
}

TEST_CASE("testing toml::try_find integer conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", 42}};
        CHECK_EQ(int(42)          , toml::try_find<int          >(v, "key").unwrap());
        CHECK_EQ(short(42)        , toml::try_find<short        >(v, "key").unwrap());
        CHECK_EQ(char(42)         , toml::try_find<char         >(v, "key").unwrap());
        CHECK_EQ(unsigned(42)     , toml::try_find<unsigned     >(v, "key").unwrap());
        CHECK_EQ(long(42)         , toml::try_find<long         >(v, "key").unwrap());
        CHECK_EQ(std::int64_t(42) , toml::try_find<std::int64_t >(v, "key").unwrap());
        CHECK_EQ(std::uint64_t(42), toml::try_find<std::uint64_t>(v, "key").unwrap());
        CHECK_EQ(std::int16_t(42) , toml::try_find<std::int16_t >(v, "key").unwrap());
        CHECK_EQ(std::uint16_t(42), toml::try_find<std::uint16_t>(v, "key").unwrap());
        CHECK_EQ(std::uint16_t(42), toml::try_find<std::uint16_t>(std::move(v), "key").unwrap());
    }
}

TEST_CASE("testing toml::try_find floating conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", 3.14}};
        const double ref(3.14);
        CHECK_EQ(static_cast<float      >(ref), toml::try_find<float      >(v, "key").unwrap());
        CHECK_EQ(                         ref , toml::try_find<double     >(v, "key").unwrap());
        CHECK_EQ(static_cast<long double>(ref), toml::try_find<long double>(v, "key").unwrap());
        CHECK_EQ(static_cast<float      >(ref), toml::try_find<float      >(std::move(v), "key").unwrap());
    }
}

TEST_CASE("testing toml::try_find string conversion")
{
    using value_type = toml::value;

    {
        value_type v = toml::table{{"key", "foo"}};
        CHECK_EQ("foo", toml::try_find<std::string>(v, "key").unwrap());
        toml::try_find<std::string>(v, "key").unwrap() += "bar";
        CHECK_EQ("foobar", toml::try_find<std::string>(v, "key").unwrap());
    }

#if TOML11_CPLUSPLUS_STANDARD_VERSION >= 201703L
    {
        value_type v = toml::table{{"key", "foo"}};
        CHECK_EQ("foo", toml::try_find<std::string_view>(v, "key").unwrap());
    }
#endif
}

TEST_CASE("testing toml::try_find array conversion")
{
    using value_type = toml::value;

    value_type v = toml::table{{"key", toml::array{42, 54, 69, 72}}};

    const std::vector<int>         vec = toml::try_find<std::vector<int>>(v, "key").unwrap();
    const std::list<short>         lst = toml::try_find<std::list<short>>(v, "key").unwrap();
    const std::deque<std::int64_t> deq = toml::try_find<std::deque<std::int64_t>>(v, "key").unwrap();

    CHECK_EQ(42, vec.at(0));
    CHECK_EQ(54, vec.at(1));
    CHECK_EQ(69, vec.at(2));
    CHECK_EQ(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    CHECK_EQ(static_cast<short>(42), *(iter++));
    CHECK_EQ(static_cast<short>(54), *(iter++));
    CHECK_EQ(static_cast<short>(69), *(iter++));
    CHECK_EQ(static_cast<short>(72), *(iter++));

    CHECK_EQ(static_cast<std::int64_t>(42), deq.at(0));
    CHECK_EQ(static_cast<std::int64_t>(54), deq.at(1));
    CHECK_EQ(static_cast<std::int64_t>(69), deq.at(2));
    CHECK_EQ(static_cast<std::int64_t>(72), deq.at(3));

    std::array<int, 4> ary = toml::try_find<std::array<int, 4>>(v, "key").unwrap();
    CHECK_EQ(42, ary.at(0));
    CHECK_EQ(54, ary.at(1));
    CHECK_EQ(69, ary.at(2));
    CHECK_EQ(72, ary.at(3));

    std::tuple<int, short, unsigned, long> tpl =
        toml::try_find<std::tuple<int, short, unsigned, long>>(v, "key").unwrap();
    CHECK_EQ(                      42 , std::get<0>(tpl));
    CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
    CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
    CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));

    value_type p = toml::table{{"key", toml::array{3.14, 2.71}}};
    std::pair<double, double> pr = toml::try_find<std::pair<double, double> >(p, "key").unwrap();
    CHECK_EQ(3.14, pr.first);
    CHECK_EQ(2.71, pr.second);
}

TEST_CASE("testing toml::try_find array move conversion")
{
    using value_type = toml::value;

    value_type v1 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v2 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v3 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v4 = toml::table{{"key", toml::array{42, 54, 69, 72}}};
    value_type v5 = toml::table{{"key", toml::array{42, 54, 69, 72}}};

    const std::vector<int>         vec = toml::try_find<std::vector<int>>(std::move(v1), "key").unwrap();
    const std::list<short>         lst = toml::try_find<std::list<short>>(std::move(v2), "key").unwrap();
    const std::deque<std::int64_t> deq = toml::try_find<std::deque<std::int64_t>>(std::move(v3), "key").unwrap();

    CHECK_EQ(42, vec.at(0));
    CHECK_EQ(54, vec.at(1));
    CHECK_EQ(69, vec.at(2));
    CHECK_EQ(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    CHECK_EQ(static_cast<short>(42), *(iter++));
    CHECK_EQ(static_cast<short>(54), *(iter++));
    CHECK_EQ(static_cast<short>(69), *(iter++));
    CHECK_EQ(static_cast<short>(72), *(iter++));

    CHECK_EQ(static_cast<std::int64_t>(42), deq.at(0));
    CHECK_EQ(static_cast<std::int64_t>(54), deq.at(1));
    CHECK_EQ(static_cast<std::int64_t>(69), deq.at(2));
    CHECK_EQ(static_cast<std::int64_t>(72), deq.at(3));

    std::array<int, 4> ary = toml::try_find<std::array<int, 4>>(std::move(v4), "key").unwrap();
    CHECK_EQ(42, ary.at(0));
    CHECK_EQ(54, ary.at(1));
    CHECK_EQ(69, ary.at(2));
    CHECK_EQ(72, ary.at(3));

    std::tuple<int, short, unsigned, long> tpl =
        toml::try_find<std::tuple<int, short, unsigned, long>>(std::move(v5), "key").unwrap();
    CHECK_EQ(                      42 , std::get<0>(tpl));
    CHECK_EQ(static_cast<short   >(54), std::get<1>(tpl));
    CHECK_EQ(static_cast<unsigned>(69), std::get<2>(tpl));
    CHECK_EQ(static_cast<long    >(72), std::get<3>(tpl));

    value_type p = toml::table{{"key", toml::array{3.14, 2.71}}};
    std::pair<double, double> pr = toml::try_find<std::pair<double, double> >(std::move(p), "key").unwrap();
    CHECK_EQ(3.14, pr.first);
    CHECK_EQ(2.71, pr.second);
}

TEST_CASE("testing toml::try_find array of array conversion")
{
    using value_type = toml::value;

    value_type v1 = toml::array{42, 54, 69, 72};
    value_type v2 = toml::array{"foo", "bar", "baz"};
    value_type v = toml::table{{"key", toml::array{v1, v2}}};

    std::pair<std::vector<int>, std::vector<std::string>> p =
        toml::try_find<std::pair<std::vector<int>, std::vector<std::string>>>(v, "key").unwrap();

    CHECK_EQ(p.first.at(0), 42);
    CHECK_EQ(p.first.at(1), 54);
    CHECK_EQ(p.first.at(2), 69);
    CHECK_EQ(p.first.at(3), 72);

    CHECK_EQ(p.second.at(0), "foo");
    CHECK_EQ(p.second.at(1), "bar");
    CHECK_EQ(p.second.at(2), "baz");

    std::tuple<std::vector<int>, std::vector<std::string>> t =
        toml::try_find<std::tuple<std::vector<int>, std::vector<std::string>>>(v, "key").unwrap();

    CHECK_EQ(std::get<0>(t).at(0), 42);
    CHECK_EQ(std::get<0>(t).at(1), 54);
    CHECK_EQ(std::get<0>(t).at(2), 69);
    CHECK_EQ(std::get<0>(t).at(3), 72);

    CHECK_EQ(std::get<1>(t).at(0), "foo");
    CHECK_EQ(std::get<1>(t).at(1), "bar");
    CHECK_EQ(std::get<1>(t).at(2), "baz");
}

TEST_CASE("testing toml::try_find array of array move conversion")
{
    using value_type = toml::value;

    value_type a1 = toml::array{42, 54, 69, 72};
    value_type a2 = toml::array{"foo", "bar", "baz"};
    value_type v1 = toml::table{{"key", toml::array{a1, a2}}};
    value_type v2 = toml::table{{"key", toml::array{a1, a2}}};

    std::pair<std::vector<int>, std::vector<std::string>> p =
        toml::try_find<std::pair<std::vector<int>, std::vector<std::string>>>(std::move(v1), "key").unwrap();

    CHECK_EQ(p.first.at(0), 42);
    CHECK_EQ(p.first.at(1), 54);
    CHECK_EQ(p.first.at(2), 69);
    CHECK_EQ(p.first.at(3), 72);

    CHECK_EQ(p.second.at(0), "foo");
    CHECK_EQ(p.second.at(1), "bar");
    CHECK_EQ(p.second.at(2), "baz");

    std::tuple<std::vector<int>, std::vector<std::string>> t =
        toml::try_find<std::tuple<std::vector<int>, std::vector<std::string>>>(std::move(v2), "key").unwrap();

    CHECK_EQ(std::get<0>(t).at(0), 42);
    CHECK_EQ(std::get<0>(t).at(1), 54);
    CHECK_EQ(std::get<0>(t).at(2), 69);
    CHECK_EQ(std::get<0>(t).at(3), 72);

    CHECK_EQ(std::get<1>(t).at(0), "foo");
    CHECK_EQ(std::get<1>(t).at(1), "bar");
    CHECK_EQ(std::get<1>(t).at(2), "baz");
}

TEST_CASE("testing toml::try_find table conversion")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::table{
                {"key1", 1}, {"key2", 2}, {"key3", 3}, {"key4", 4}
            }}};
        const auto v = toml::try_find<std::map<std::string, int>>(v1, "key").unwrap();
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
    {
        value_type v1 = toml::table{{"key", toml::table{
                {"key1", 1}, {"key2", 2}, {"key3", 3}, {"key4", 4}
            }}};
        const auto v = toml::try_find<std::map<std::string, int>>(std::move(v1), "key").unwrap();
        CHECK_EQ(v.at("key1"), 1);
        CHECK_EQ(v.at("key2"), 2);
        CHECK_EQ(v.at("key3"), 3);
        CHECK_EQ(v.at("key4"), 4);
    }
}

TEST_CASE("testing toml::try_find local_date")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_date{2018, toml::month_t::Apr, 1}}};
        const auto date = std::chrono::system_clock::to_time_t(
                toml::try_find<std::chrono::system_clock::time_point>(v1, "key").unwrap());

        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 0;
        t.tm_min  = 0;
        t.tm_sec  = 0;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c, date);
    }
    {
        value_type v1 = toml::table{{"key", toml::local_date{2018, toml::month_t::Apr, 1}}};
        const auto date = std::chrono::system_clock::to_time_t(
                toml::try_find<std::chrono::system_clock::time_point>(std::move(v1), "key").unwrap());

        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 0;
        t.tm_min  = 0;
        t.tm_sec  = 0;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c, date);
    }
}

TEST_CASE("testing toml::try_find local_time")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_time{12, 30, 45}}};
        const auto time = toml::try_find<std::chrono::seconds>(v1, "key").unwrap();
        CHECK_EQ(time, std::chrono::hours(12) +
                            std::chrono::minutes(30) + std::chrono::seconds(45));
    }
    {
        value_type v1 = toml::table{{"key", toml::local_time{12, 30, 45}}};
        const auto time = toml::try_find<std::chrono::seconds>(std::move(v1), "key").unwrap();
        CHECK_EQ(time, std::chrono::hours(12) +
                            std::chrono::minutes(30) + std::chrono::seconds(45));
    }
}

TEST_CASE("testing toml::try_find local_datetime")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::local_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 45})}};

        const auto date = std::chrono::system_clock::to_time_t(
                toml::try_find<std::chrono::system_clock::time_point>(v1, "key").unwrap());
        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 12;
        t.tm_min  = 30;
        t.tm_sec  = 45;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c,  date);
    }
    {
        value_type v1 = toml::table{{"key", toml::local_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 45})}};

        const auto date = std::chrono::system_clock::to_time_t(
                toml::try_find<std::chrono::system_clock::time_point>(std::move(v1), "key").unwrap());
        std::tm t;
        t.tm_year = 2018 - 1900;
        t.tm_mon  = 4    - 1;
        t.tm_mday = 1;
        t.tm_hour = 12;
        t.tm_min  = 30;
        t.tm_sec  = 45;
        t.tm_isdst = -1;
        const auto c = std::mktime(&t);
        CHECK_EQ(c,  date);
    }
}

TEST_CASE("testing toml::try_find offset_datetime")
{
    using value_type = toml::value;
    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{9, 0})}};
        //    2018-04-01T12:30:00+09:00
        //,  2018-04-01T03:30:00Z

        const auto date = toml::try_find<std::chrono::system_clock::time_point>(v1, "key").unwrap();
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,            3);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }

    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{-8, 0})}};
        //    2018-04-01T12:30:00-08:00
        //,  2018-04-01T20:30:00Z

        const auto date = toml::try_find<std::chrono::system_clock::time_point>(v1, "key").unwrap();
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,           20);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }

    {
        value_type v1 = toml::table{{"key", toml::offset_datetime(
                    toml::local_date{2018, toml::month_t::Apr, 1},
                    toml::local_time{12, 30, 0},
                    toml::time_offset{-8, 0})}};
        //    2018-04-01T12:30:00-08:00
        //,  2018-04-01T20:30:00Z

        const auto date = toml::try_find<std::chrono::system_clock::time_point>(std::move(v1), "key").unwrap();
        const auto timet = std::chrono::system_clock::to_time_t(date);

        // get time_t as gmtime (2018-04-01T03:30:00Z)
        const auto tm = toml::detail::gmtime_s(std::addressof(timet));
        CHECK_EQ(tm.tm_year + 1900,  2018);
        CHECK_EQ(tm.tm_mon  + 1,        4);
        CHECK_EQ(tm.tm_mday,            1);
        CHECK_EQ(tm.tm_hour,           20);
        CHECK_EQ(tm.tm_min,            30);
        CHECK_EQ(tm.tm_sec,             0);
    }
}

#define BOOST_TEST_MODULE "test_literals"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <map>

BOOST_AUTO_TEST_CASE(test_file_as_literal)
{
    using namespace toml::literals::toml_literals;

    {
        const toml::value r{{"a", 42}, {"b", "baz"}};
        const toml::value v = u8R"(
            a = 42
            b = "baz"
        )"_toml;

        BOOST_CHECK_EQUAL(r, v);
    }
    {
        const toml::value r{
            {"c", 3.14},
            {"table", toml::table{{"a", 42}, {"b", "baz"}}}
        };
        const toml::value v = u8R"(
            c = 3.14
            [table]
            a = 42
            b = "baz"
        )"_toml;

        BOOST_CHECK_EQUAL(r, v);
    }
    {
        const toml::value r{
            {"table", toml::table{{"a", 42}, {"b", "baz"}}}
        };
        const toml::value v = u8R"(
            [table]
            a = 42
            b = "baz"
        )"_toml;

        BOOST_CHECK_EQUAL(r, v);
    }
}

BOOST_AUTO_TEST_CASE(test_value_as_literal)
{
    using namespace toml::literals::toml_literals;

    {
        const toml::value v1 = u8"true"_toml;
        const toml::value v2 = u8"false"_toml;

        BOOST_CHECK(v1.is_boolean());
        BOOST_CHECK(v2.is_boolean());
        BOOST_CHECK(toml::get<bool>(v1));
        BOOST_CHECK(!toml::get<bool>(v2));
    }
    {
        const toml::value v1 = u8"123_456"_toml;
        const toml::value v2 = u8"0b0010"_toml;
        const toml::value v3 = u8"0xDEADBEEF"_toml;

        BOOST_CHECK(v1.is_integer());
        BOOST_CHECK(v2.is_integer());
        BOOST_CHECK(v3.is_integer());
        BOOST_CHECK_EQUAL(toml::get<toml::integer>(v1), 123456);
        BOOST_CHECK_EQUAL(toml::get<toml::integer>(v2), 2);
        BOOST_CHECK_EQUAL(toml::get<toml::integer>(v3), 0xDEADBEEF);
    }
    {
        const toml::value v1 = u8"3.1415"_toml;
        const toml::value v2 = u8"6.02e+23"_toml;

        BOOST_CHECK(v1.is_floating());
        BOOST_CHECK(v2.is_floating());
        BOOST_CHECK_CLOSE(toml::get<double>(v1), 3.1415,  0.00001);
        BOOST_CHECK_CLOSE(toml::get<double>(v2), 6.02e23, 0.0001);
    }
    {
        const toml::value v1 = u8R"("foo")"_toml;
        const toml::value v2 = u8R"('foo')"_toml;
        const toml::value v3 = u8R"("""foo""")"_toml;
        const toml::value v4 = u8R"('''foo''')"_toml;

        BOOST_CHECK(v1.is_string());
        BOOST_CHECK(v2.is_string());
        BOOST_CHECK(v3.is_string());
        BOOST_CHECK(v4.is_string());
        BOOST_CHECK_EQUAL(toml::get<std::string>(v1), "foo");
        BOOST_CHECK_EQUAL(toml::get<std::string>(v2), "foo");
        BOOST_CHECK_EQUAL(toml::get<std::string>(v3), "foo");
        BOOST_CHECK_EQUAL(toml::get<std::string>(v4), "foo");
    }
    {
        const toml::value v1 = u8R"([1,2,3])"_toml;

        BOOST_CHECK(v1.is_array());
        BOOST_CHECK((toml::get<std::vector<int>>(v1) == std::vector<int>{1,2,3}));

        const toml::value v2 = u8R"([1,])"_toml;

        BOOST_CHECK(v2.is_array());
        BOOST_CHECK((toml::get<std::vector<int>>(v2) == std::vector<int>{1}));

        const toml::value v3 = u8R"([[1,]])"_toml;
        BOOST_CHECK(v3.is_array());
        BOOST_CHECK((toml::get<std::vector<int>>(toml::get<toml::array>(v3).front()) == std::vector<int>{1}));

        const toml::value v4 = u8R"([[1],])"_toml;
        BOOST_CHECK(v4.is_array());
        BOOST_CHECK((toml::get<std::vector<int>>(toml::get<toml::array>(v4).front()) == std::vector<int>{1}));
    }
    {
        const toml::value v1 = u8R"({a = 42})"_toml;

        BOOST_CHECK(v1.is_table());
        BOOST_CHECK((toml::get<std::map<std::string,int>>(v1) ==
                               std::map<std::string,int>{{"a", 42}}));
    }
    {
        const toml::value v1 = u8"1979-05-27"_toml;

        BOOST_CHECK(v1.is_local_date());
        BOOST_CHECK_EQUAL(toml::get<toml::local_date>(v1),
                          toml::local_date(1979, toml::month_t::May, 27));
    }
    {
        const toml::value v1 = u8"12:00:00"_toml;

        BOOST_CHECK(v1.is_local_time());
        BOOST_CHECK(toml::get<std::chrono::hours>(v1) == std::chrono::hours(12));
    }
    {
        const toml::value v1 = u8"1979-05-27T07:32:00"_toml;
        BOOST_CHECK(v1.is_local_datetime());
        BOOST_CHECK_EQUAL(toml::get<toml::local_datetime>(v1),
            toml::local_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                 toml::local_time(7, 32, 0)));
    }
    {
        const toml::value v1 = "1979-05-27T07:32:00Z"_toml;
        BOOST_CHECK(v1.is_offset_datetime());
        BOOST_CHECK_EQUAL(toml::get<toml::offset_datetime>(v1),
            toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                  toml::local_time(7, 32, 0), toml::time_offset(0, 0)));
    }
}

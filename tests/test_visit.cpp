#define BOOST_TEST_MODULE "test_visit"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif

#include <toml.hpp>
#include <map>
#include <deque>

using test_value_types = std::tuple<
    toml::value,
    toml::basic_value<toml::preserve_comments>,
    toml::basic_value<toml::discard_comments,  std::map, std::deque>,
    toml::basic_value<toml::preserve_comments, std::map, std::deque>
>;

template<typename Value>
struct visitor1
{
    std::string operator()(const toml::boolean&)              const {return "boolean";}
    std::string operator()(const toml::integer&)              const {return "integer";}
    std::string operator()(const toml::floating&)             const {return "floating";}
    std::string operator()(const toml::string&)               const {return "string";}
    std::string operator()(const toml::local_time&)           const {return "local_time";}
    std::string operator()(const toml::local_date&)           const {return "local_date";}
    std::string operator()(const toml::local_datetime&)       const {return "local_datetime";}
    std::string operator()(const toml::offset_datetime&)      const {return "offset_datetime";}
    std::string operator()(const typename Value::array_type&) const {return "array";}
    std::string operator()(const typename Value::table_type&) const {return "table";}
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_visit_one, value_type, test_value_types)
{
    {
        const value_type v(true);
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "boolean");
    }
    {
        const value_type v(42);
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "integer");
    }
    {
        const value_type v(3.14);
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "floating");
    }
    {
        const value_type v("foo");
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "string");
    }
    {
        const value_type v(toml::local_date(2018, toml::month_t::Apr, 22));
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "local_date");
    }
    {
        const value_type v(toml::local_time(12, 34, 56));
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "local_time");
    }
    {
        const value_type v(toml::local_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 34, 56)));
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "local_datetime");
    }
    {
        const value_type v(toml::offset_datetime(
                    toml::local_date(2018, toml::month_t::Apr, 22),
                    toml::local_time(12, 34, 56),
                    toml::time_offset(9, 0)));
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "offset_datetime");
    }
    {
        const value_type v{1,2,3,4,5};
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "array");
    }
    {
        const value_type v{
            {"foo", 42}, {"bar", "baz"}
        };
        BOOST_TEST(toml::visit(visitor1<value_type>{}, v) == "table");
    }
}

template<typename Value>
struct visitor2
{
    template<typename T1, typename T2>
    std::string operator()(const T1& v1, const T2& v2) const
    {
        visitor1<Value> vis;
        return vis(v1) + "+" + vis(v2);
    }
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_visit_two, value_type, test_value_types)
{
    std::vector<value_type> vs;
    vs.push_back(value_type(true));
    vs.push_back(value_type(42));
    vs.push_back(value_type(3.14));
    vs.push_back(value_type("foo"));
    vs.push_back(value_type(toml::local_date(2018, toml::month_t::Apr, 22)));
    vs.push_back(value_type(toml::local_time(12, 34, 56)));
    vs.push_back(value_type(toml::local_datetime(
                                toml::local_date(2018, toml::month_t::Apr, 22),
                                toml::local_time(12, 34, 56))));
    vs.push_back(value_type(toml::offset_datetime(
                                toml::local_date(2018, toml::month_t::Apr, 22),
                                toml::local_time(12, 34, 56),
                                toml::time_offset(9, 0))));
    vs.push_back(value_type{1,2,3,4,5});
    vs.push_back(value_type{{"foo", 42}, {"bar", "baz"}});

    for(const auto& v1 : vs)
    {
        const auto t1 = toml::visit(visitor1<value_type>{}, v1);
        for(const auto& v2 : vs)
        {
            const auto t2 = toml::visit(visitor1<value_type>{}, v2);
            BOOST_TEST(toml::visit(visitor2<value_type>{}, v1, v2) ==
                       t1 + "+" + t2);
        }
    }
}

template<typename Value>
struct visitor3
{
    template<typename T1, typename T2, typename T3>
    std::string operator()(const T1& v1, const T2& v2, const T3& v3) const
    {
        visitor1<Value> vis;
        return vis(v1) + "+" + vis(v2) + "+" + vis(v3);
    }
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_visit_three, value_type, test_value_types)
{
    std::vector<value_type> vs;
    vs.push_back(value_type(true));
    vs.push_back(value_type(42));
    vs.push_back(value_type(3.14));
    vs.push_back(value_type("foo"));
    vs.push_back(value_type(toml::local_date(2018, toml::month_t::Apr, 22)));
    vs.push_back(value_type(toml::local_time(12, 34, 56)));
    vs.push_back(value_type(toml::local_datetime(
                                toml::local_date(2018, toml::month_t::Apr, 22),
                                toml::local_time(12, 34, 56))));
    vs.push_back(value_type(toml::offset_datetime(
                                toml::local_date(2018, toml::month_t::Apr, 22),
                                toml::local_time(12, 34, 56),
                                toml::time_offset(9, 0))));
    vs.push_back(value_type{1,2,3,4,5});
    vs.push_back(value_type{{"foo", 42}, {"bar", "baz"}});

    for(const auto& v1 : vs)
    {
        const auto t1 = toml::visit(visitor1<value_type>{}, v1);
        for(const auto& v2 : vs)
        {
            const auto t2 = toml::visit(visitor1<value_type>{}, v2);
            for(const auto& v3 : vs)
            {
                const auto t3 = toml::visit(visitor1<value_type>{}, v3);
                BOOST_TEST(toml::visit(visitor3<value_type>{}, v1, v2, v3) ==
                           t1 + "+" + t2 + "+" + t3);
            }
        }
    }
}

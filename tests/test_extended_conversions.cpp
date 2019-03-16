#define BOOST_TEST_MODULE "test_extended_conversions"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>

namespace extlib
{
struct foo
{
    int a;
    std::string b;
};
struct bar
{
    int a;
    std::string b;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int>(v, "a");
        this->b = toml::find<std::string>(v, "b");
        return ;
    }

    toml::table into_toml() const
    {
        return toml::table{{"a", this->a}, {"b", this->b}};
    }
};
} // extlib

namespace toml
{
template<>
struct from<extlib::foo>
{
    static extlib::foo from_toml(const toml::value& v)
    {
        return extlib::foo{toml::find<int>(v, "a"), toml::find<std::string>(v, "b")};
    }
};

template<>
struct into<extlib::foo>
{
    static toml::table into_toml(const extlib::foo& f)
    {
        return toml::table{{"a", f.a}, {"b", f.b}};
    }
};
} // toml


BOOST_AUTO_TEST_CASE(test_conversion_by_member_methods)
{
    const toml::value v{{"a", 42}, {"b", "baz"}};

    const auto foo = toml::get<extlib::foo>(v);
    BOOST_TEST(foo.a == 42);
    BOOST_TEST(foo.b == "baz");

    const toml::value v2(foo);

    BOOST_TEST(v == v2);
}

BOOST_AUTO_TEST_CASE(test_conversion_by_specialization)
{
    const toml::value v{{"a", 42}, {"b", "baz"}};

    const auto bar = toml::get<extlib::bar>(v);
    BOOST_TEST(bar.a == 42);
    BOOST_TEST(bar.b == "baz");

    const toml::value v2(bar);

    BOOST_TEST(v == v2);
}

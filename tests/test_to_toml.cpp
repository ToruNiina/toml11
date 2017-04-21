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

BOOST_AUTO_TEST_CASE(test_to_toml_exact)
{
    toml::Boolean  b(true);
    toml::Integer  i(42);
    toml::Float    f(3.14);
    toml::String   s("hoge");
    toml::Datetime d(std::chrono::system_clock::now());
    toml::Array    a;
    a.emplace_back(2);
    a.emplace_back(7);
    a.emplace_back(1);
    a.emplace_back(8);
    a.emplace_back(2);
    toml::Table    t;
    t.emplace("val1", true);
    t.emplace("val2", 42);
    t.emplace("val3", 3.14);
    t.emplace("val4", "piyo");

    auto v1 = toml::to_toml(b);
    auto v2 = toml::to_toml(i);
    auto v3 = toml::to_toml(f);
    auto v4 = toml::to_toml(s);
    auto v5 = toml::to_toml(d);
    auto v6 = toml::to_toml(a);
    auto v7 = toml::to_toml(t);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v4.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(v5.type(), toml::value_t::Datetime);
    BOOST_CHECK_EQUAL(v6.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(v7.type(), toml::value_t::Table);

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Boolean >(), b);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Integer >(), i);
    BOOST_CHECK_EQUAL(v3.cast<toml::value_t::Float   >(), f);
    BOOST_CHECK_EQUAL(v4.cast<toml::value_t::String  >(), s);
    const auto& ar = v6.cast<toml::value_t::Array>();
    BOOST_CHECK_EQUAL(ar.at(0).cast<toml::value_t::Integer>(), a.at(0).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(ar.at(1).cast<toml::value_t::Integer>(), a.at(1).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(ar.at(2).cast<toml::value_t::Integer>(), a.at(2).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(ar.at(3).cast<toml::value_t::Integer>(), a.at(3).cast<toml::value_t::Integer>());
    BOOST_CHECK_EQUAL(ar.at(4).cast<toml::value_t::Integer>(), a.at(4).cast<toml::value_t::Integer>());
}

BOOST_AUTO_TEST_CASE(test_to_toml_castable)
{
    auto v1 = toml::to_toml(true);
    auto v2 = toml::to_toml(42ul);
    auto v3 = toml::to_toml(3.14f);
    auto v4 = toml::to_toml("hoge");

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v4.type(), toml::value_t::String);

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Boolean >(), true);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Integer >(), 42);
    BOOST_CHECK_CLOSE_FRACTION(v3.cast<toml::value_t::Float   >(), 3.14, 1e-5);
    BOOST_CHECK_EQUAL(v4.cast<toml::value_t::String  >(), "hoge");
}

BOOST_AUTO_TEST_CASE(test_to_toml_initializer_list)
{
    toml::value v1 = toml::to_toml({3,1,4,1,5});
    toml::value v2 = toml::to_toml({{"hoge", 1}, {"piyo", 3.14}, {"fuga", "string"}});

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Table);

    const auto& ar = v1.cast<toml::value_t::Array>();
    BOOST_CHECK_EQUAL(ar.at(0).cast<toml::value_t::Integer>(), 3);
    BOOST_CHECK_EQUAL(ar.at(1).cast<toml::value_t::Integer>(), 1);
    BOOST_CHECK_EQUAL(ar.at(2).cast<toml::value_t::Integer>(), 4);
    BOOST_CHECK_EQUAL(ar.at(3).cast<toml::value_t::Integer>(), 1);
    BOOST_CHECK_EQUAL(ar.at(4).cast<toml::value_t::Integer>(), 5);

    const auto& tb = v2.cast<toml::value_t::Table>();

    BOOST_CHECK_EQUAL(tb.at("hoge").type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(tb.at("piyo").type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(tb.at("fuga").type(), toml::value_t::String);

    BOOST_CHECK_EQUAL(tb.at("hoge").cast<toml::value_t::Integer>(), 1);
    BOOST_CHECK_CLOSE_FRACTION(tb.at("piyo").cast<toml::value_t::Float>(), 3.14, 1e-3);
    BOOST_CHECK_EQUAL(tb.at("fuga").cast<toml::value_t::String>(),  "string");

}




#define BOOST_TEST_MODULE "test_value"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <map>
#include <list>

BOOST_AUTO_TEST_CASE(test_value_exact_constructor)
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

    toml::value v1(b);
    toml::value v2(i);
    toml::value v3(f);
    toml::value v4(s);
    toml::value v5(d);
    toml::value v6(a);
    toml::value v7(t);

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

BOOST_AUTO_TEST_CASE(test_value_convertible_constructor)
{
    int            i(42);
    float          f(3.14);
    const char*    s = "hoge";

    toml::value v1(i);
    toml::value v2(f);
    toml::value v3(s);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::String);

    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Integer >(), i);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Float   >(), f);
    BOOST_CHECK_EQUAL(v3.cast<toml::value_t::String  >(), s);
}

BOOST_AUTO_TEST_CASE(test_value_copy_move_constructor)
{
    toml::Array a;
    toml::Table t;
    toml::value v1(true);
    toml::value v2(42);
    toml::value v3(3.14);
    toml::value v4("hoge");
    toml::value v5(std::chrono::system_clock::now());
    toml::value v6(a);
    toml::value v7(t);

    toml::value u1(v1);
    toml::value u2(v2);
    toml::value u3(v3);
    toml::value u4(v4);
    toml::value u5(v5);
    toml::value u6(v6);
    toml::value u7(v7);

    BOOST_CHECK_EQUAL(u1.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(u2.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(u3.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(u4.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(u5.type(), toml::value_t::Datetime);
    BOOST_CHECK_EQUAL(u6.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(u7.type(), toml::value_t::Table);

    BOOST_CHECK_EQUAL(u1.cast<toml::value_t::Boolean >(), true);
    BOOST_CHECK_EQUAL(u2.cast<toml::value_t::Integer >(), 42);
    BOOST_CHECK_EQUAL(u3.cast<toml::value_t::Float   >(), 3.14);
    BOOST_CHECK_EQUAL(u4.cast<toml::value_t::String  >(), "hoge");

    toml::value w1(std::move(v1));
    toml::value w2(std::move(v2));
    toml::value w3(std::move(v3));
    toml::value w4(std::move(v4));
    toml::value w5(std::move(v5));
    toml::value w6(std::move(v6));
    toml::value w7(std::move(v7));

    BOOST_CHECK_EQUAL(w1.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(w2.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(w3.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(w4.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(w5.type(), toml::value_t::Datetime);
    BOOST_CHECK_EQUAL(w6.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(w7.type(), toml::value_t::Table);

    BOOST_CHECK_EQUAL(w1.cast<toml::value_t::Boolean >(), true);
    BOOST_CHECK_EQUAL(w2.cast<toml::value_t::Integer >(), 42);
    BOOST_CHECK_EQUAL(w3.cast<toml::value_t::Float   >(), 3.14);
    BOOST_CHECK_EQUAL(w4.cast<toml::value_t::String  >(), "hoge");
}

BOOST_AUTO_TEST_CASE(test_value_copy_move_substitution)
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

    toml::value v1(b);
    toml::value v2(i);
    toml::value v3(f);
    toml::value v4(s);
    toml::value v5(d);
    toml::value v6(a);
    toml::value v7(t);

    v1 = i;
    v2 = f;
    v3 = s;
    v4 = d;
    v5 = a;
    v6 = t;
    v7 = b;

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Integer);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(v4.type(), toml::value_t::Datetime);
    BOOST_CHECK_EQUAL(v5.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(v6.type(), toml::value_t::Table);
    BOOST_CHECK_EQUAL(v7.type(), toml::value_t::Boolean);

    BOOST_CHECK_EQUAL(v7.cast<toml::value_t::Boolean >(), b);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Integer >(), i);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::Float   >(), f);
    BOOST_CHECK_EQUAL(v3.cast<toml::value_t::String  >(), s);

    v1 = std::move(f);
    v2 = std::move(s);
    v3 = std::move(d);
    v4 = std::move(a);
    v5 = std::move(t);
    v6 = std::move(b);
    v7 = std::move(i);

    BOOST_CHECK_EQUAL(v1.type(), toml::value_t::Float);
    BOOST_CHECK_EQUAL(v2.type(), toml::value_t::String);
    BOOST_CHECK_EQUAL(v3.type(), toml::value_t::Datetime);
    BOOST_CHECK_EQUAL(v4.type(), toml::value_t::Array);
    BOOST_CHECK_EQUAL(v5.type(), toml::value_t::Table);
    BOOST_CHECK_EQUAL(v6.type(), toml::value_t::Boolean);
    BOOST_CHECK_EQUAL(v7.type(), toml::value_t::Integer);

    BOOST_CHECK_EQUAL(v6.cast<toml::value_t::Boolean >(), b);
    BOOST_CHECK_EQUAL(v7.cast<toml::value_t::Integer >(), i);
    BOOST_CHECK_EQUAL(v1.cast<toml::value_t::Float   >(), f);
    BOOST_CHECK_EQUAL(v2.cast<toml::value_t::String  >(), s);
}

BOOST_AUTO_TEST_CASE(test_value_initializer_list)
{
    toml::value v1{3,1,4,1,5};
    toml::value v2{{"hoge", 1}, {"piyo", 3.14}, {"fuga", "string"}};

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

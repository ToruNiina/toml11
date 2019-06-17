#define BOOST_TEST_MODULE "test_serialize_file"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_CASE(test_example)
{
    const auto data = toml::parse("toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1.toml");
        ofs << std::setw(80) << data;
    }

    auto serialized = toml::parse("tmp1.toml");
    {
        auto& owner = toml::find<toml::table>(serialized, "owner");
        auto& bio   = toml::get<std::string>(owner.at("bio"));
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_example_with_comment)
{
    const auto data = toml::parse<toml::preserve_comments>("toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1_com.toml");
        ofs << std::setw(80) << data;
    }

    auto serialized = toml::parse<toml::preserve_comments>("tmp1_com.toml");
    {
        auto& owner = toml::find<typename decltype(serialized)::table_type>(serialized, "owner");
        auto& bio   = toml::get<std::string>(owner.at("bio"));
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_CHECK(data == serialized);
    {
        std::ofstream ofs("tmp1_com1.toml");
        ofs << std::setw(80) << serialized;
    }
}

BOOST_AUTO_TEST_CASE(test_fruit)
{
    const auto data = toml::parse("toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse("tmp2.toml");
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_fruit_with_comments)
{
    const auto data = toml::parse<toml::preserve_comments>("toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2_com.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::preserve_comments>("tmp2_com.toml");
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example)
{
    const auto data = toml::parse("toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse("tmp3.toml");
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example_with_comment)
{
    const auto data = toml::parse<toml::preserve_comments>("toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3_com.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::preserve_comments>("tmp3_com.toml");
    {
        std::ofstream ofs("tmp3_com1.toml");
        ofs << std::setw(80) << serialized;
    }
    BOOST_CHECK(data == serialized);
}

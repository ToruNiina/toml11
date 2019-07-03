#define BOOST_TEST_MODULE "test_serialize_file"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <deque>
#include <map>
#include <iostream>
#include <fstream>

template<typename Comment,
         template<typename ...> class Table,
         template<typename ...> class Array>
bool has_comment_inside(const toml::basic_value<Comment, Table, Array>& v)
{
    if(!v.comments().empty())
    {
        return false;
    }
    // v itself does not have a comment.
    if(v.is_array())
    {
        for(const auto& x : v.as_array())
        {
            if(has_comment_inside(x))
            {
                return false;
            }
        }
    }
    if(v.is_table())
    {
        for(const auto& x : v.as_table())
        {
            if(has_comment_inside(x.second))
            {
                return false;
            }
        }
    }
    return true;
}

BOOST_AUTO_TEST_CASE(test_example)
{
    const auto data = toml::parse("toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1.toml");
        ofs << std::setw(80) << data;
    }

    auto serialized = toml::parse("tmp1.toml");
    {
        auto& owner = toml::find(serialized, "owner");
        auto& bio   = toml::find<std::string>(owner, "bio");
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_example_map_dq)
{
    const auto data = toml::parse<toml::discard_comments, std::map, std::deque>(
            "toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1_map_dq.toml");
        ofs << std::setw(80) << data;
    }

    auto serialized = toml::parse<toml::discard_comments, std::map, std::deque>(
            "tmp1_map_dq.toml");
    {
        auto& owner = toml::find(serialized, "owner");
        auto& bio   = toml::find<std::string>(owner, "bio");
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_TEST(data == serialized);
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
        auto& owner = toml::find(serialized, "owner");
        auto& bio   = toml::find<std::string>(owner, "bio");
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_TEST(data == serialized);
    {
        std::ofstream ofs("tmp1_com1.toml");
        ofs << std::setw(80) << serialized;
    }
}

BOOST_AUTO_TEST_CASE(test_example_with_comment_nocomment)
{
    {
        const auto data = toml::parse<toml::preserve_comments>("toml/tests/example.toml");
        {
            std::ofstream ofs("tmp1_com_nocomment.toml");
            ofs << std::setw(80) << toml::nocomment << data;
        }
        const auto serialized = toml::parse<toml::preserve_comments>("tmp1_com_nocomment.toml");
        // check no comment exist
        BOOST_TEST(!has_comment_inside(serialized));
    }
    {
        const auto data_nocomment = toml::parse("toml/tests/example.toml");
        auto serialized = toml::parse("tmp1_com_nocomment.toml");
        {
            auto& owner = toml::find(serialized, "owner");
            auto& bio   = toml::find<std::string>(owner, "bio");
            const auto CR = std::find(bio.begin(), bio.end(), '\r');
            if(CR != bio.end())
            {
                bio.erase(CR);
            }
        }
        // check collectly serialized
        BOOST_TEST(data_nocomment == serialized);
    }
}

BOOST_AUTO_TEST_CASE(test_example_with_comment_map_dq)
{
    const auto data = toml::parse<toml::preserve_comments, std::map, std::deque>(
            "toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1_com_map_dq.toml");
        ofs << std::setw(80) << data;
    }

    auto serialized = toml::parse<toml::preserve_comments, std::map, std::deque>(
            "tmp1_com_map_dq.toml");
    {
        auto& owner = toml::find(serialized, "owner");
        auto& bio   = toml::find<std::string>(owner, "bio");
        const auto CR = std::find(bio.begin(), bio.end(), '\r');
        if(CR != bio.end())
        {
            bio.erase(CR);
        }
    }
    BOOST_TEST(data == serialized);
    {
        std::ofstream ofs("tmp1_com1_map_dq.toml");
        ofs << std::setw(80) << serialized;
    }
}

BOOST_AUTO_TEST_CASE(test_example_with_comment_map_dq_nocomment)
{
    {
        const auto data = toml::parse<toml::preserve_comments, std::map, std::deque>("toml/tests/example.toml");
        {
            std::ofstream ofs("tmp1_com_map_dq_nocomment.toml");
            ofs << std::setw(80) << toml::nocomment << data;
        }
        const auto serialized = toml::parse<toml::preserve_comments, std::map, std::deque>("tmp1_com_map_dq_nocomment.toml");
        BOOST_TEST(!has_comment_inside(serialized));
    }
    {
        const auto data_nocomment = toml::parse("toml/tests/example.toml");
        auto serialized = toml::parse("tmp1_com_map_dq_nocomment.toml");
        {
            auto& owner = toml::find(serialized, "owner");
            auto& bio   = toml::find<std::string>(owner, "bio");
            const auto CR = std::find(bio.begin(), bio.end(), '\r');
            if(CR != bio.end())
            {
                bio.erase(CR);
            }
        }
        BOOST_TEST(data_nocomment == serialized);
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
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_fruit_map_dq)
{
    const auto data = toml::parse<toml::discard_comments, std::map, std::deque>(
            "toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::discard_comments, std::map, std::deque>(
            "tmp2.toml");
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_fruit_with_comments)
{
    const auto data = toml::parse<toml::preserve_comments>("toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2_com.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::preserve_comments>("tmp2_com.toml");
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_fruit_with_comments_map_dq)
{
    const auto data = toml::parse<toml::preserve_comments, std::map, std::deque>(
        "toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2_com.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::preserve_comments, std::map, std::deque>("tmp2_com.toml");
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example)
{
    const auto data = toml::parse("toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse("tmp3.toml");
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example_map_dq)
{
    const auto data = toml::parse<toml::discard_comments, std::map, std::deque>(
            "toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::discard_comments, std::map, std::deque>(
            "tmp3.toml");
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example_with_comment)
{
    const auto data = toml::parse<toml::preserve_comments, std::map, std::deque>(
            "toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3_com.toml");
        ofs << std::setw(80) << data;
    }
    const auto serialized = toml::parse<toml::preserve_comments, std::map, std::deque>(
            "tmp3_com.toml");
    {
        std::ofstream ofs("tmp3_com1.toml");
        ofs << std::setw(80) << serialized;
    }
    BOOST_TEST(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_format_key)
{
    {
        const toml::key key("normal_bare-key");
        BOOST_TEST("normal_bare-key" == toml::format_key(key));
    }
    {
        const toml::key key("key.include.dots");
        BOOST_TEST("\"key.include.dots\"" == toml::format_key(key));
    }
    {
        const toml::key key("key-include-unicode-\xE3\x81\x82");
        BOOST_TEST("\"key-include-unicode-\xE3\x81\x82\"" == toml::format_key(key));
    }
    {
        const toml::key key("special-chars-\\-\"-\b-\f-\r-\n-\t");
        BOOST_TEST("\"special-chars-\\\\-\\\"-\\b-\\f-\\r-\\n-\\t\"" == toml::format_key(key));
    }
}

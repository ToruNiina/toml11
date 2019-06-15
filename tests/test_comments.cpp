#define BOOST_TEST_MODULE "test_comments"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif

#include <toml.hpp>

BOOST_AUTO_TEST_CASE(test_comment_before)
{
    {
        const std::string file = u8R"(
            # comment for a.
            a = 42
            # comment for b.
            b = "baz"
        )";
        std::istringstream iss(file);
        const auto v = toml::parse<toml::preserve_comments>(iss);

        const auto& a = toml::find(v, "a");
        const auto& b = toml::find(v, "b");

        BOOST_CHECK_EQUAL(a.comments().size(), 1u);
        BOOST_CHECK_EQUAL(a.comments().front(), u8" comment for a.");
        BOOST_CHECK_EQUAL(b.comments().size(), 1u);
        BOOST_CHECK_EQUAL(b.comments().front(), u8" comment for b.");
    }
    {
        const std::string file = u8R"(
            # comment for a.
            # another comment for a.
            a = 42
            # comment for b.
            # also comment for b.
            b = "baz"
        )";

        std::istringstream iss(file);
        const auto v = toml::parse<toml::preserve_comments>(iss);

        const auto& a = toml::find(v, "a");
        const auto& b = toml::find(v, "b");

        BOOST_CHECK_EQUAL(a.comments().size(), 2u);
        BOOST_CHECK_EQUAL(a.comments().front(), u8" comment for a.");
        BOOST_CHECK_EQUAL(a.comments().back(),  u8" another comment for a.");
        BOOST_CHECK_EQUAL(b.comments().size(), 2u);
        BOOST_CHECK_EQUAL(b.comments().front(), u8" comment for b.");
        BOOST_CHECK_EQUAL(b.comments().back(),  u8" also comment for b.");
    }
}

BOOST_AUTO_TEST_CASE(test_comment_inline)
{
    using namespace toml::literals::toml_literals;
    {
        const std::string file = u8R"(
            a = 42    # comment for a.
            b = "baz" # comment for b.
        )";

        std::istringstream iss(file);
        const auto v = toml::parse<toml::preserve_comments>(iss);

        const auto& a = toml::find(v, "a");
        const auto& b = toml::find(v, "b");

        BOOST_CHECK_EQUAL(a.comments().size(), 1u);
        BOOST_CHECK_EQUAL(a.comments().front(), u8" comment for a.");
        BOOST_CHECK_EQUAL(b.comments().size(), 1u);
        BOOST_CHECK_EQUAL(b.comments().front(), u8" comment for b.");
    }
    {
        const std::string file = u8R"(
            a = [
                42,
            ] # comment for a.
            b = [
                "bar", # this is not a comment for b, but "bar"
            ] # this is a comment for b.
        )";

        std::istringstream iss(file);
        const auto v = toml::parse<toml::preserve_comments>(iss);

        const auto& a  = toml::find(v, "a");
        const auto& b  = toml::find(v, "b");
        const auto& b0 = b.as_array().at(0);

        BOOST_CHECK_EQUAL(a.comments().size(), 1u);
        BOOST_CHECK_EQUAL(a.comments().front(), u8" comment for a.");
        BOOST_CHECK_EQUAL(b.comments().size(), 1u);
        BOOST_CHECK_EQUAL(b.comments().front(), u8" this is a comment for b.");
        BOOST_CHECK_EQUAL(b0.comments().size(), 1u);
        BOOST_CHECK_EQUAL(b0.comments().front(),
                          u8" this is not a comment for b, but \"bar\"");
    }
}

BOOST_AUTO_TEST_CASE(test_comment_both)
{
    using namespace toml::literals::toml_literals;
    {
        const std::string file = u8R"(
            # comment for a.
            a = 42 # inline comment for a.
            # comment for b.
            b = "baz" # inline comment for b.
            # comment for c.
            c = [ # this comment will be ignored
                # comment for the first element.
                10 # this also.
            ] # another comment for c.
        )";

        std::istringstream iss(file);
        const auto v = toml::parse<toml::preserve_comments>(iss);

        const auto& a  = toml::find(v, "a");
        const auto& b  = toml::find(v, "b");
        const auto& c  = toml::find(v, "c");
        const auto& c0 = c.as_array().at(0);

        BOOST_CHECK_EQUAL(a.comments().size(), 2u);
        BOOST_CHECK_EQUAL(a.comments().front(), u8" comment for a.");
        BOOST_CHECK_EQUAL(a.comments().back(),  u8" inline comment for a.");
        BOOST_CHECK_EQUAL(b.comments().size(), 2u);
        BOOST_CHECK_EQUAL(b.comments().front(), u8" comment for b.");
        BOOST_CHECK_EQUAL(b.comments().back(),  u8" inline comment for b.");

        BOOST_CHECK_EQUAL(c.comments().size(), 2u);
        BOOST_CHECK_EQUAL(c.comments().front(), u8" comment for c.");
        BOOST_CHECK_EQUAL(c.comments().back(),  u8" another comment for c.");

        BOOST_CHECK_EQUAL(c0.comments().size(), 2u);
        BOOST_CHECK_EQUAL(c0.comments().front(), u8" comment for the first element.");
        BOOST_CHECK_EQUAL(c0.comments().back(),  u8" this also.");
    }
}

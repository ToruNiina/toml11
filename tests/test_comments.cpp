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
    using namespace toml::literals::toml_literals;
    {
        const toml::value v = u8R"(
            # comment for a.
            a = 42
            # comment for b.
            b = "baz"
        )"_toml;

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_before(), u8"# comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_before(), u8"# comment for b.");
        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_inline(), "");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_inline(), "");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment(), u8"# comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment(), u8"# comment for b.");
    }

    {
        const toml::value v = u8R"(
            # comment for a.
            # another comment for a.
            a = 42
            # comment for b.
            # also comment for b.
            b = "baz"
        )"_toml;

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_before(), u8R"(# comment for a.
# another comment for a.)");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_before(), u8R"(# comment for b.
# also comment for b.)");
        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_inline(), u8"");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_inline(), u8"");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment(), u8R"(# comment for a.
# another comment for a.)");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment(), u8R"(# comment for b.
# also comment for b.)");
    }
}

BOOST_AUTO_TEST_CASE(test_comment_inline)
{
    using namespace toml::literals::toml_literals;
    {
        const toml::value v = u8R"(
            a = 42    # comment for a.
            b = "baz" # comment for b.
        )"_toml;

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_before(), "");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_before(), "");
        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_inline(), u8"# comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_inline(), u8"# comment for b.");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment(), u8"# comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment(), u8"# comment for b.");
    }
    {
        const toml::value v = u8R"(
            a = [    # comment for a.
                42,
            ] # this also.
            b = [ # comment for b.
                "bar",
            ]
            c = [
                3.14, # this is not a comment for c, but 3.14.
            ] # comment for c.
        )"_toml;

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_before(), "");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_before(), "");
        BOOST_CHECK_EQUAL(toml::find(v, "c").comment_before(), "");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_inline(), u8R"(# comment for a.
# this also.)");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_inline(), u8"# comment for b.");
        BOOST_CHECK_EQUAL(toml::find(v, "c").comment_inline(), u8"# comment for c.");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment(),  u8R"(# comment for a.
# this also.)");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment(), u8"# comment for b.");
        BOOST_CHECK_EQUAL(toml::find(v, "c").comment(), u8"# comment for c.");

        const auto& c0 = toml::find<toml::array>(v, "c").at(0);
        BOOST_CHECK_EQUAL(c0.comment(), u8"# this is not a comment for c, but 3.14.");
    }
}

BOOST_AUTO_TEST_CASE(test_comment_both)
{
    using namespace toml::literals::toml_literals;
    {
        const toml::value v = u8R"(
            # comment for a.
            a = 42 # inline comment for a.
            # comment for b.
            b = "baz" # inline comment for b.
        )"_toml;

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_before(), "# comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_before(), "# comment for b.");
        BOOST_CHECK_EQUAL(toml::find(v, "a").comment_inline(), "# inline comment for a.");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment_inline(), "# inline comment for b.");

        BOOST_CHECK_EQUAL(toml::find(v, "a").comment(), u8R"(# comment for a.
# inline comment for a.)");
        BOOST_CHECK_EQUAL(toml::find(v, "b").comment(), u8R"(# comment for b.
# inline comment for b.)");
    }
}

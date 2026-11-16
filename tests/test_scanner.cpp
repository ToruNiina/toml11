#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "utility.hpp"

#include <toml11/parser.hpp>
#include <toml11/scanner.hpp>
#include <toml11/serializer.hpp>

#include <string>
#include <vector>

TEST_CASE("testing scanner: character")
{
    auto loc = toml::detail::make_temporary_location("\t \tA");

    toml::detail::character tab('\t');
    toml::detail::character space(' ');

    CHECK_UNARY( tab  .scan(loc).is_ok()); // loc += 1
    CHECK_UNARY(!tab  .scan(loc).is_ok());
    CHECK_UNARY( space.scan(loc).is_ok());
    CHECK_UNARY( tab  .scan(loc).is_ok());
    CHECK_UNARY(!tab  .scan(loc).is_ok());
    CHECK_UNARY(!space.scan(loc).is_ok());
}

TEST_CASE("testing scanner: character_either")
{
    auto loc = toml::detail::make_temporary_location("\t \t01\0");

    toml::detail::character_either wschar("\t ");
    toml::detail::character_either digit01("01");

    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY( wschar .scan(loc).is_ok());
    CHECK_UNARY(!wschar .scan(loc).is_ok());
    CHECK_UNARY( digit01.scan(loc).is_ok());
    CHECK_UNARY( digit01.scan(loc).is_ok());
    CHECK_UNARY(!digit01.scan(loc).is_ok());
    CHECK_UNARY(!wschar .scan(loc).is_ok());
}

TEST_CASE("testing scanner: character_in_range")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::character_in_range alpha('a', 'z');
    toml::detail::character_in_range digit('0', '9');

    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY( alpha.scan(loc).is_ok());
    CHECK_UNARY(!alpha.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY( digit.scan(loc).is_ok());
    CHECK_UNARY(!digit.scan(loc).is_ok());
}

TEST_CASE("testing scanner: literal")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    CHECK_UNARY( abcd.scan(loc).is_ok());
    CHECK_UNARY(!abcd.scan(loc).is_ok());
    CHECK_UNARY( z123.scan(loc).is_ok());
    CHECK_UNARY(!z123.scan(loc).is_ok());
    CHECK_UNARY(!abcd.scan(loc).is_ok());
}

TEST_CASE("testing scanner: sequence")
{
    auto loc = toml::detail::make_temporary_location("abcdz12349");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    toml::detail::sequence seq(abcd, z123);

    CHECK_UNARY( seq.scan(loc).is_ok());
    CHECK_EQ( loc.current(), toml::detail::location::char_type('4'));

    auto loc2 = toml::detail::make_temporary_location("abcde12349");

    CHECK_UNARY(!seq.scan(loc2).is_ok());
}

TEST_CASE("testing scanner: either")
{
    auto loc = toml::detail::make_temporary_location("abcdz123");

    toml::detail::literal abcd("abcd");
    toml::detail::literal z123("z123");

    toml::detail::either e(abcd, z123);

    CHECK_UNARY( e.scan(loc).is_ok());
    CHECK_EQ( loc.current(), toml::detail::location::char_type('z'));

    CHECK_UNARY( e.scan(loc).is_ok());
    CHECK_UNARY( loc.eof() );
}

TEST_CASE("testing scanner: repeat_exact")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_exact r(2, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }

    {
        auto loc = toml::detail::make_temporary_location("foofoobar");

        toml::detail::repeat_exact r(3, toml::detail::literal("foo"));

        CHECK_UNARY( ! r.scan(loc).is_ok());
    }
}

TEST_CASE("testing scanner: repeat_at_least")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_at_least r(1, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::repeat_at_least r(2, toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('b'));
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");

        toml::detail::repeat_at_least r(3, toml::detail::literal("foo"));

        CHECK_UNARY( ! r.scan(loc).is_ok());
    }
}

TEST_CASE("testing scanner: maybe")
{
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::maybe r(toml::detail::literal("foo"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('f'));
        CHECK_EQ( loc.get_location(), 3);
    }
    {
        auto loc = toml::detail::make_temporary_location("foofoobar");
        toml::detail::maybe r(toml::detail::literal("bar"));

        CHECK_UNARY( r.scan(loc).is_ok());
        CHECK_EQ( loc.current(), toml::detail::location::char_type('f'));
        CHECK_EQ( loc.get_location(), 0);
    }
}

TEST_CASE("multiline spacer preserves comments indentation and location")
{
    struct Case
    {
        std::string text;
        std::size_t consumed;
        bool newline;
        toml::indent_char indent_type;
        std::int32_t indent;
        std::vector<std::string> comments;
    };
    const std::vector<Case> cases = {
        {"# first\n  value", 10, true, toml::indent_char::space, 2, {"# first"}},
        {"# first\r\n\tvalue", 10, true, toml::indent_char::tab, 1, {"# first"}},
        {"# first\n# second\nvalue", 17, true, toml::indent_char::none, 0, {"# first", "# second"}},
        {"# first\n\nvalue", 9, true, toml::indent_char::none, 0, {}},
        {" \tvalue", 2, false, toml::indent_char::tab, 1, {}},
        {"  # unfinished", 2, false, toml::indent_char::space, 2, {}}
    };
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        for (const auto& test : cases)
        {
            CAPTURE(test.text);
            auto loc = toml::detail::make_temporary_location(test.text);
            toml::detail::context<toml::type_config> ctx(toml::spec::v(1, 0, 0));
            const auto spacer = toml::detail::skip_multiline_spacer(loc, ctx);
            REQUIRE(spacer.has_value());
            CHECK(loc.get_location() == test.consumed);
            CHECK(spacer.value().newline_found == test.newline);
            CHECK(spacer.value().indent_type == test.indent_type);
            CHECK(spacer.value().indent == test.indent);
            const auto& comments = spacer.value().comments;
            CHECK(std::vector<std::string>(comments.begin(), comments.end()) == test.comments);
            CHECK(loc.current() == test.text[test.consumed]);
        }
    }
}

TEST_CASE("multiline spacer failure restores the original position")
{
    for (const auto& text : {"value", "# no newline", "\rvalue", ""})
    {
        auto loc = toml::detail::make_temporary_location(text);
        const auto first = loc;
        toml::detail::context<toml::type_config> ctx(toml::spec::v(1, 0, 0));
        const auto spacer = toml::detail::skip_multiline_spacer(loc, ctx, true);
        CHECK_FALSE(spacer.has_value());
        CHECK(loc == first);
    }
}

TEST_CASE("multiline spacer cache follows all specification flags")
{
    const std::string text = std::string("# control ") + '\x01' + "\nvalue";
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        for (const bool allow : {false, true, false})
        {
            auto spec = toml::spec::v(1, 0, 0);
            spec.ext_allow_control_characters_in_comments = allow;
            toml::detail::context<toml::type_config> ctx(spec);
            auto loc = toml::detail::make_temporary_location(text);
            const auto spacer = toml::detail::skip_multiline_spacer(loc, ctx);
            CHECK(spacer.has_value() == allow);
            CHECK(loc.get_location() == (allow ? text.find('\n') + 1 : 0));
            if (allow)
            {
                REQUIRE(spacer.has_value());
                const auto& comments = spacer.value().comments;
                REQUIRE(comments.size() == 1);
                CHECK(*comments.begin() == text.substr(0, text.find('\n')));
                CHECK(loc.line_number() == 2);
                CHECK(loc.column_number() == 1);
            }
        }
    }
}

TEST_CASE("cached scanners preserve parse and format across specification changes")
{
    const std::string text = "# root\r\n\r\nroot = [1, # element\r\n 2]\r\n  [next]\r\nvalue = { key = 3 }\r\n";
    for (const auto& spec : {toml::spec::v(1, 0, 0), toml::spec::v(1, 1, 0), toml::spec::v(1, 0, 0)})
    {
        const auto value = toml::parse_str(text, spec);
        CHECK(value.at("root").as_array().size() == 2);
        CHECK(value.at("root").at(1).as_integer() == 2);
        CHECK(value.at("next").at("value").at("key").as_integer() == 3);
        const auto formatted = toml::format(value, spec);
        CHECK(toml::parse_str(formatted, spec) == value);
        CHECK(formatted.find("# element") != std::string::npos);
        CHECK_THROWS_AS(toml::parse_str("a = 1 b = 2\n", spec), toml::syntax_error);
    }
}

TEST_CASE("cached table prefix preserves diagnostic positions")
{
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        const auto result = toml::try_parse_str("# header\r\n[first]\r\na = 1 b = 2\r\n");
        REQUIRE(result.is_err());
        const auto& errors = result.as_err();
        REQUIRE(errors.size() == 1);
        REQUIRE(errors.front().locations().size() == 1);
        const auto& position = errors.front().locations().front().first;
        CHECK(position.first_line_number() == 3);
        CHECK(position.first_column_number() == 7);
        CHECK(position.first_line() == "a = 1 b = 2\r");
    }
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing multiline table")
{
    const auto table_fmt = [](toml::indent_char c, std::int32_t body, std::int32_t name) {
        toml::table_format_info fmt;
        fmt.fmt = toml::table_format::multiline;
        fmt.indent_type    = c;
        fmt.body_indent    = body;
        fmt.name_indent    = name;
        fmt.closing_indent = 0;
        return fmt;
    };

    toml::value v(toml::table{
            {"a", 42},
            {"b", 3.14},
            {"c", "foobar"},
        });

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 0, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 2, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::space, 2, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));


    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 0, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 0, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 2, 0);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));

    v.as_table_fmt() = table_fmt(toml::indent_char::tab, 2, 2);
    CHECK_EQ(v, toml::parse_str(toml::format(v)));
}

TEST_CASE("testing multiline table")
{
    const auto table_fmt = [](toml::indent_char c, std::int32_t body, std::int32_t name) {
        toml::table_format_info fmt;
        fmt.fmt = toml::table_format::dotted;
        fmt.indent_type    = c;
        fmt.body_indent    = body;
        fmt.name_indent    = name;
        fmt.closing_indent = 0;
        return fmt;
    };

    {
        toml::value v(toml::table{});
        v["a"]["b"]["c"] = "foobar";
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").at("b").as_table_fmt() = table_fmt(toml::indent_char::space, 0, 0);
        CHECK_EQ(v, toml::parse_str(toml::format(v)));

        v.at("a").as_table_fmt().fmt = toml::table_format::multiline;
        CHECK_EQ(v, toml::parse_str(toml::format(v)));
    }
}

TEST_CASE_TEMPLATE("testing multiline inline table keys", Value,
                   toml::value, toml::ordered_value)
{
    const auto spec = toml::spec::v(1, 1, 0);
    const std::vector<std::pair<std::string, std::string>> keys = {
        {"bare-key", "bare-key"},
        {"a.b", "\"a.b\""},
        {"two words", "\"two words\""},
        {"", "\"\""},
        {"a\"b", "\"a\\\"b\""},
        {"a\\b", "\"a\\\\b\""},
        {"a\nb", "\"a\\nb\""},
    };
    for(const auto& key : keys)
    {
        Value inner(typename Value::table_type{{key.first, 42}});
        inner.as_table_fmt().fmt = toml::table_format::multiline_oneline;
        const Value root(typename Value::table_type{{"config", inner}});
        const auto text = toml::format(root, spec);
        CHECK(text.find(key.second + " = 42") != std::string::npos);
        CHECK_EQ(toml::parse_str<typename Value::config_type>(text, spec), root);
    }
}

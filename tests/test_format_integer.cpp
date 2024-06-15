#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>
#include <toml11/types.hpp>

TEST_CASE("testing decimal")
{
    const auto decimal_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::dec;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    CHECK_EQ(          "0",  toml::format(toml::value(        0, decimal_fmt(1, 0))));
    CHECK_EQ(       "1234",  toml::format(toml::value(     1234, decimal_fmt(0, 0))));
    CHECK_EQ(      "-1234",  toml::format(toml::value(    -1234, decimal_fmt(0, 0))));
    CHECK_EQ(    "1_2_3_4",  toml::format(toml::value(     1234, decimal_fmt(4, 1))));
    CHECK_EQ(   "-1_2_3_4",  toml::format(toml::value(    -1234, decimal_fmt(5, 1))));
    CHECK_EQ("123_456_789",  toml::format(toml::value(123456789, decimal_fmt(9, 3))));
}

TEST_CASE("testing hex")
{
    const auto hex_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::hex;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    CHECK_EQ("0xdeadbeef",  toml::format(toml::value(0xDEADBEEF, hex_fmt(8, 0))));
    CHECK_EQ("0xdead_beef", toml::format(toml::value(0xDEADBEEF, hex_fmt(8, 4))));
    CHECK_EQ("0xff",        toml::format(toml::value(0xFF,       hex_fmt(2, 0))));
    CHECK_EQ("0x00ff",      toml::format(toml::value(0xFF,       hex_fmt(4, 0))));
    CHECK_EQ("0x0000ff",    toml::format(toml::value(0xFF,       hex_fmt(6, 0))));
}

TEST_CASE("testing oct")
{
    const auto oct_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::oct;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };
    CHECK_EQ("0o644",     toml::format(toml::value(64*6+8*4+4, oct_fmt(3, 0))));
    CHECK_EQ("0o7_7_7",   toml::format(toml::value(64*7+8*7+7, oct_fmt(3, 1))));
    CHECK_EQ("0o000644",  toml::format(toml::value(64*6+8*4+4, oct_fmt(6, 0))));
}

TEST_CASE("testing bin")
{
    const auto bin_fmt = [](std::size_t w, std::size_t s) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::bin;
        fmt.width = w;
        fmt.spacer = s;
        return fmt;
    };

    CHECK_EQ("0b1000",      toml::format(toml::value(8, bin_fmt(4, 0))));
    CHECK_EQ("0b00001000",  toml::format(toml::value(8, bin_fmt(8, 0))));
    CHECK_EQ("0b0000_1000", toml::format(toml::value(8, bin_fmt(8, 4))));
}

TEST_CASE("testing decimal with suffix")
{
    toml::spec sp = toml::spec::v(1,0,0);
    sp.ext_num_suffix = true;

    const auto decimal_fmt = [](std::size_t w, std::size_t s, std::string sfx) {
        toml::integer_format_info fmt;
        fmt.fmt = toml::integer_format::dec;
        fmt.width = w;
        fmt.spacer = s;
        fmt.suffix = sfx;
        return fmt;
    };

    CHECK_EQ(      "0_J",    toml::format(toml::value(        0, decimal_fmt(1, 0, "J")),    sp));
    CHECK_EQ(   "1234_kcal", toml::format(toml::value(     1234, decimal_fmt(0, 0, "kcal")), sp));
    CHECK_EQ("1_2_3_4_μm",   toml::format(toml::value(     1234, decimal_fmt(4, 1, "μm")),   sp));
}

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml11/location.hpp>
#include <toml11/region.hpp>

TEST_CASE("testing location")
{
    const auto first = toml::detail::make_temporary_location("hogefuga");

    auto loc = first;
    loc.advance(4);

    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY( ! loc.eof());
    CHECK_EQ(loc.current(), 'f');
    CHECK_EQ(loc.line_number(), 1);
    CHECK_EQ(loc.column_number(), 5);

    const auto loc2 = loc;
    CHECK_EQ(loc, loc2);
    CHECK_UNARY(loc == loc2);
    CHECK_UNARY(!(loc != loc2));

    loc.advance(4);
    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY(loc.eof());
    CHECK_EQ(loc.current(), '\0');
    CHECK_EQ(loc.line_number(), 1);
    CHECK_EQ(loc.column_number(), 9);

    CHECK_NE(loc, loc2);
    CHECK_UNARY(!(loc == loc2));
    CHECK_UNARY(loc != loc2);

    CHECK_EQ(toml::detail::count(first, loc, 'e'), 1);
    CHECK_EQ(toml::detail::count(first, loc, 'g'), 2);
}

TEST_CASE("testing multiline location")
{
    const auto first = toml::detail::make_temporary_location("hoge\nfuga");
    auto loc = first;
    loc.advance(5);

    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY( ! loc.eof());
    CHECK_EQ(loc.current(), 'f');
    CHECK_EQ(loc.line_number(), 2);
    CHECK_EQ(loc.column_number(), 1);

    const auto newline_r = toml::detail::rfind(first, loc, '\n');
    CHECK_UNARY(newline_r.is_ok());

    const auto newline = newline_r.as_ok();
    CHECK_EQ(newline.current(), '\n');

    loc.advance(4);
    CHECK_UNARY(loc.is_ok());
    CHECK_UNARY(loc.eof());
    CHECK_EQ(loc.current(), '\0');
    CHECK_EQ(loc.line_number(), 2);
    CHECK_EQ(loc.column_number(), 5);
}

TEST_CASE("source name survives copies of locations and regions")
{
    const auto loc = toml::detail::make_temporary_location("x");
    const auto copied_loc = loc;
    CHECK_EQ(copied_loc.source_name(), "internal temporary");

    const auto next = toml::detail::next(loc);
    const toml::detail::region span(loc, next);
    CHECK_EQ(span.source_name(), "internal temporary");

    const toml::detail::region point(loc);
    CHECK_EQ(point.source_name(), "internal temporary");

    const toml::detail::region manual;
    CHECK_EQ(manual.source_name(), "");

    const auto span_x = [] {
        const auto loc = toml::detail::make_temporary_location("x");
        const auto next = toml::detail::next(loc);
        return toml::detail::region(loc, next);
   }();

    CHECK_EQ(span_x.source_name(), "internal temporary");
}

TEST_CASE("source name and region survive destruction of original locations")
{
    const std::string name(256, 'n');
    const auto span = [&name]()
    {
        const auto src = std::make_shared<const toml::detail::location::container_type>(
            toml::detail::location::container_type{'a', '\n', 'b', 'c'});
        toml::detail::location first(src, name);
        first.advance(2);
        auto copied = first;
        auto moved = std::move(copied);
        first = moved;
        auto last = first;
        last.advance(2);
        return toml::detail::region(first, last);
    }();
    auto copied = span;
    auto moved = std::move(copied);
    toml::detail::region assigned;
    assigned = moved;
    toml::detail::region move_assigned;
    move_assigned = std::move(assigned);
    const toml::detail::region* regions[] = {&span, &moved, &move_assigned};
    for (const auto* region : regions)
    {
        CHECK_EQ(region->source_name(), name);
        CHECK_EQ(region->as_string(), "bc");
        CHECK_EQ(region->first_line_number(), 2);
        CHECK_EQ(region->first_column_number(), 1);
        CHECK_EQ(region->last_line_number(), 2);
        CHECK_EQ(region->last_column_number(), 3);
    }
}


#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <toml.hpp>

#include <clocale>
#include <fstream>
#include <sstream>

TEST_CASE("testing toml::parse(file)")
{
    using namespace toml::literals::toml_literals;

    const std::string spec_example(R"(# This is a TOML document

title = "TOML Example"

[owner]
name = "Tom Preston-Werner"
dob = 1979-05-27T07:32:00-08:00

[database]
enabled = true
ports = [ 8000, 8001, 8002 ]
data = [ ["delta", "phi"], [3.14] ]
temp_targets = { cpu = 79.5, case = 72.0 }

[servers]

[servers.alpha]
ip = "10.0.0.1"
role = "frontend"

[servers.beta]
ip = "10.0.0.2"
role = "backend"
)");

    {
        std::ofstream ofs("test_parse.toml");
        ofs << spec_example;
    }

    const auto u = toml::parse_str(spec_example);
    const auto v = toml::parse("test_parse.toml");

    CHECK_EQ(u, v);

    CHECK_EQ(toml::find<int>(v, "database", "ports", 0), 8000);
    CHECK_EQ(toml::find<int>(v, "database", "ports", 1), 8001);
    CHECK_EQ(toml::find<int>(v, "database", "ports", 2), 8002);
}

namespace
{

template<typename TC>
void check_stream_error(std::istream& input)
{
    SUBCASE("try_parse returns an I/O error")
    {
        const auto result = toml::try_parse<TC>(input, "input.toml");
        REQUIRE(result.is_err());
        REQUIRE_EQ(result.unwrap_err().size(), 1);
        CHECK_NE(result.unwrap_err().front().title().find("input.toml"),
                 std::string::npos);
        CHECK(result.unwrap_err().front().locations().empty());
    }
    SUBCASE("parse throws an I/O error")
    {
        CHECK_THROWS_AS(toml::parse<TC>(input, "input.toml"), toml::file_io_error);
    }
}

struct failing_streambuf : std::stringbuf
{
    enum class failure { tell, seek_end, seek_back, short_read };

    explicit failing_streambuf(failure fail_at)
        : std::stringbuf("answer = 42\n"), fail_at_(fail_at)
    {}

    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode mode) override
    {
        if(fail_at_ == failure::tell ||
           (fail_at_ == failure::seek_end && dir == std::ios_base::end))
        {
            return pos_type(off_type(-1));
        }
        return std::stringbuf::seekoff(off, dir, mode);
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override
    {
        if(fail_at_ == failure::seek_back)
        {
            return pos_type(off_type(-1));
        }
        return std::stringbuf::seekpos(pos, mode);
    }

    std::streamsize xsgetn(char* dest, std::streamsize count) override
    {
        if(fail_at_ == failure::short_read && count > 0)
        {
            --count;
        }
        return std::stringbuf::xsgetn(dest, count);
    }

    failure fail_at_;
};

} // namespace

TEST_CASE_TEMPLATE("rejecting unopened input streams", TC,
                   toml::type_config, toml::ordered_type_config)
{
    std::ifstream input;
    check_stream_error<TC>(input);
}

TEST_CASE_TEMPLATE("rejecting input streams with failed state", TC,
                   toml::type_config, toml::ordered_type_config)
{
    std::istringstream input("answer = 42\n");
    SUBCASE("failbit")
    {
        input.setstate(std::ios_base::failbit);
        check_stream_error<TC>(input);
    }
    SUBCASE("badbit")
    {
        input.setstate(std::ios_base::badbit);
        check_stream_error<TC>(input);
    }
    SUBCASE("eofbit")
    {
        input.setstate(std::ios_base::eofbit);
        check_stream_error<TC>(input);
    }
}

TEST_CASE_TEMPLATE("rejecting failed input stream operations", TC,
                   toml::type_config, toml::ordered_type_config)
{
    const auto check = [](failing_streambuf::failure fail_at) {
        failing_streambuf buffer(fail_at);
        std::istream input(&buffer);
        check_stream_error<TC>(input);
    };
    SUBCASE("tell")       {check(failing_streambuf::failure::tell);}
    SUBCASE("seek end")   {check(failing_streambuf::failure::seek_end);}
    SUBCASE("seek back")  {check(failing_streambuf::failure::seek_back);}
    SUBCASE("short read") {check(failing_streambuf::failure::short_read);}
}

TEST_CASE_TEMPLATE("parsing valid input streams", TC,
                   toml::type_config, toml::ordered_type_config)
{
    SUBCASE("empty stream")
    {
        std::istringstream input;
        CHECK(toml::parse<TC>(input).as_table().empty());
    }
    SUBCASE("current position")
    {
        std::istringstream input("ignored\nanswer = 42\n");
        input.seekg(8);
        CHECK_EQ(toml::find<int>(toml::parse<TC>(input), "answer"), 42);
    }
    SUBCASE("position at end")
    {
        std::istringstream input("answer = 42\n");
        input.seekg(0, std::ios_base::end);
        CHECK(toml::parse<TC>(input).as_table().empty());
    }
    SUBCASE("invalid TOML still raises a syntax error")
    {
        std::istringstream input("answer = [\n");
        CHECK_THROWS_AS(toml::parse<TC>(input), toml::syntax_error);
    }
}

TEST_CASE("preserving standard stream exceptions")
{
    failing_streambuf buffer(failing_streambuf::failure::short_read);
    std::istream input(&buffer);
    input.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    SUBCASE("try_parse")
    {
        CHECK_THROWS_AS(toml::try_parse(input), std::ios_base::failure);
    }
    SUBCASE("parse")
    {
        CHECK_THROWS_AS(toml::parse(input), std::ios_base::failure);
    }
}

//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_PARSER_HPP
#define TOML11_PARSER_HPP
#include "result.hpp"
#include "region.hpp"
#include "combinator.hpp"
#include "lexer.hpp"
#include "types.hpp"
#include "value.hpp"
#include <fstream>
#include <sstream>
#include <cstring>

namespace toml
{
namespace detail
{

template<typename Container>
result<std::pair<boolean, region<Container>>, std::string>
parse_boolean(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_boolean::invoke(loc))
    {
        const auto reg = token.unwrap();
        if     (reg.str() == "true")  {return ok(std::make_pair(true,  reg));}
        else if(reg.str() == "false") {return ok(std::make_pair(false, reg));}
        else // internal error.
        {
            throw toml::internal_error(format_underline(
                "[error] toml::parse_boolean: internal error",
                {{std::addressof(reg), "invalid token"}}));
        }
    }
    loc.reset(first); //rollback
    return err(format_underline("[error] toml::parse_boolean: ",
               {{std::addressof(loc), "the next token is not a boolean"}}));
}

template<typename Container>
result<std::pair<integer, region<Container>>, std::string>
parse_binary_integer(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_bin_int::invoke(loc))
    {
        auto str = token.unwrap().str();
        assert(str.size() > 2); // minimum -> 0b1
        integer retval(0), base(1);
        for(auto i(str.rbegin()), e(str.rend() - 2); i!=e; ++i)
        {
            if     (*i == '1'){retval += base; base *= 2;}
            else if(*i == '0'){base *= 2;}
            else if(*i == '_'){/* do nothing. */}
            else // internal error.
            {
                throw toml::internal_error(format_underline(
                    "[error] toml::parse_integer: internal error",
                    {{std::addressof(token.unwrap()), "invalid token"}}));
            }
        }
        return ok(std::make_pair(retval, token.unwrap()));
    }
    loc.reset(first);
    return err(format_underline("[error] toml::parse_binary_integer:",
               {{std::addressof(loc), "the next token is not an integer"}}));
}

template<typename Container>
result<std::pair<integer, region<Container>>, std::string>
parse_octal_integer(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_oct_int::invoke(loc))
    {
        auto str = token.unwrap().str();
        str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
        str.erase(str.begin()); str.erase(str.begin()); // remove `0o` prefix

        std::istringstream iss(str);
        integer retval(0);
        iss >> std::oct >> retval;
        return ok(std::make_pair(retval, token.unwrap()));
    }
    loc.reset(first);
    return err(format_underline("[error] toml::parse_octal_integer:",
               {{std::addressof(loc), "the next token is not an integer"}}));
}

template<typename Container>
result<std::pair<integer, region<Container>>, std::string>
parse_hexadecimal_integer(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_hex_int::invoke(loc))
    {
        auto str = token.unwrap().str();
        str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
        str.erase(str.begin()); str.erase(str.begin()); // remove `0x` prefix

        std::istringstream iss(str);
        integer retval(0);
        iss >> std::hex >> retval;
        return ok(std::make_pair(retval, token.unwrap()));
    }
    loc.reset(first);
    return err(format_underline("[error] toml::parse_hexadecimal_integer",
               {{std::addressof(loc), "the next token is not an integer"}}));
}

template<typename Container>
result<std::pair<integer, region<Container>>, std::string>
parse_integer(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first != loc.end() && *first == '0')
    {
        const auto second = std::next(first);
        if(second == loc.end()) // the token is just zero.
        {
            return ok(std::make_pair(0, region<Container>(loc, first, second)));
        }

        if(*second == 'b') {return parse_binary_integer     (loc);} // 0b1100
        if(*second == 'o') {return parse_octal_integer      (loc);} // 0o775
        if(*second == 'x') {return parse_hexadecimal_integer(loc);} // 0xC0FFEE

        if(std::isdigit(*second))
        {
            return err(format_underline("[error] toml::parse_integer: "
                "leading zero in an Integer is not allowed.",
                {{std::addressof(loc), "leading zero"}}));
        }
        else if(std::isalpha(*second))
        {
             return err(format_underline("[error] toml::parse_integer: "
                "unknown integer prefix appeared.",
                {{std::addressof(loc), "none of 0x, 0o, 0b"}}));
        }
    }

    if(const auto token = lex_dec_int::invoke(loc))
    {
        auto str = token.unwrap().str();
        str.erase(std::remove(str.begin(), str.end(), '_'), str.end());

        std::istringstream iss(str);
        integer retval(0);
        iss >> retval;
        return ok(std::make_pair(retval, token.unwrap()));
    }
    loc.reset(first);
    return err(format_underline("[error] toml::parse_integer: ",
               {{std::addressof(loc), "the next token is not an integer"}}));
}

template<typename Container>
result<std::pair<floating, region<Container>>, std::string>
parse_floating(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_float::invoke(loc))
    {
        auto str = token.unwrap().str();
        if(str == "inf" || str == "+inf")
        {
            if(std::numeric_limits<floating>::has_infinity)
            {
                return ok(std::make_pair(
                    std::numeric_limits<floating>::infinity(), token.unwrap()));
            }
            else
            {
                throw std::domain_error("toml::parse_floating: inf value found"
                    " but the current environment does not support inf. Please"
                    " make sure that the floating-point implementation conforms"
                    " IEEE 754/ISO 60559 international standard.");
            }
        }
        else if(str == "-inf")
        {
            if(std::numeric_limits<floating>::has_infinity)
            {
                return ok(std::make_pair(
                    -std::numeric_limits<floating>::infinity(), token.unwrap()));
            }
            else
            {
                throw std::domain_error("toml::parse_floating: inf value found"
                    " but the current environment does not support inf. Please"
                    " make sure that the floating-point implementation conforms"
                    " IEEE 754/ISO 60559 international standard.");
            }
        }
        else if(str == "nan" || str == "+nan")
        {
            if(std::numeric_limits<floating>::has_quiet_NaN)
            {
                return ok(std::make_pair(
                    std::numeric_limits<floating>::quiet_NaN(), token.unwrap()));
            }
            else if(std::numeric_limits<floating>::has_signaling_NaN)
            {
                return ok(std::make_pair(
                    std::numeric_limits<floating>::signaling_NaN(), token.unwrap()));
            }
            else
            {
                throw std::domain_error("toml::parse_floating: NaN value found"
                    " but the current environment does not support NaN. Please"
                    " make sure that the floating-point implementation conforms"
                    " IEEE 754/ISO 60559 international standard.");
            }
        }
        else if(str == "-nan")
        {
            if(std::numeric_limits<floating>::has_quiet_NaN)
            {
                return ok(std::make_pair(
                    -std::numeric_limits<floating>::quiet_NaN(), token.unwrap()));
            }
            else if(std::numeric_limits<floating>::has_signaling_NaN)
            {
                return ok(std::make_pair(
                    -std::numeric_limits<floating>::signaling_NaN(), token.unwrap()));
            }
            else
            {
                throw std::domain_error("toml::parse_floating: NaN value found"
                    " but the current environment does not support NaN. Please"
                    " make sure that the floating-point implementation conforms"
                    " IEEE 754/ISO 60559 international standard.");
            }
        }
        str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
        std::istringstream iss(str);
        floating v(0.0);
        iss >> v;
        return ok(std::make_pair(v, token.unwrap()));
    }
    loc.reset(first);
    return err(format_underline("[error] toml::parse_floating: ",
               {{std::addressof(loc), "the next token is not a float"}}));
}

template<typename Container, typename Container2>
std::string read_utf8_codepoint(const region<Container>& reg,
              /* for err msg */ const location<Container2>& loc)
{
    const auto str = reg.str().substr(1);
    std::uint_least32_t codepoint;
    std::istringstream iss(str);
    iss >> std::hex >> codepoint;

    const auto to_char = [](const int i) noexcept -> char {
        const auto uc = static_cast<unsigned char>(i);
        return *reinterpret_cast<const char*>(std::addressof(uc));
    };

    std::string character;
    if(codepoint < 0x80) // U+0000 ... U+0079 ; just an ASCII.
    {
        character += static_cast<char>(codepoint);
    }
    else if(codepoint < 0x800) //U+0080 ... U+07FF
    {
        // 110yyyyx 10xxxxxx; 0x3f == 0b0011'1111
        character += to_char(0xC0| codepoint >> 6);
        character += to_char(0x80|(codepoint & 0x3F));
    }
    else if(codepoint < 0x10000) // U+0800...U+FFFF
    {
        if(0xD800 <= codepoint && codepoint <= 0xDFFF)
        {
            throw syntax_error(format_underline("[error] "
                "toml::read_utf8_codepoint: codepoints in the range "
                "[0xD800, 0xDFFF] are not valid UTF-8.", {{
                    std::addressof(loc), "not a valid UTF-8 codepoint"
                }}));
        }
        assert(codepoint < 0xD800 || 0xDFFF < codepoint);
        // 1110yyyy 10yxxxxx 10xxxxxx
        character += to_char(0xE0| codepoint >> 12);
        character += to_char(0x80|(codepoint >> 6 & 0x3F));
        character += to_char(0x80|(codepoint      & 0x3F));
    }
    else if(codepoint < 0x110000) // U+010000 ... U+10FFFF
    {
        // 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
        character += to_char(0xF0| codepoint >> 18);
        character += to_char(0x80|(codepoint >> 12 & 0x3F));
        character += to_char(0x80|(codepoint >> 6  & 0x3F));
        character += to_char(0x80|(codepoint       & 0x3F));
    }
    else // out of UTF-8 region
    {
        throw syntax_error(format_underline("[error] toml::read_utf8_codepoint:"
            " input codepoint is too large.",
            {{std::addressof(loc), "should be in [0x00..0x10FFFF]"}}));
    }
    return character;
}

template<typename Container>
result<std::string, std::string> parse_escape_sequence(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first == loc.end() || *first != '\\')
    {
        return err(format_underline("[error]: toml::parse_escape_sequence: ", {{
            std::addressof(loc), "the next token is not a backslash \"\\\""}}));
    }
    loc.advance();
    switch(*loc.iter())
    {
        case '\\':{loc.advance(); return ok(std::string("\\"));}
        case '"' :{loc.advance(); return ok(std::string("\""));}
        case 'b' :{loc.advance(); return ok(std::string("\b"));}
        case 't' :{loc.advance(); return ok(std::string("\t"));}
        case 'n' :{loc.advance(); return ok(std::string("\n"));}
        case 'f' :{loc.advance(); return ok(std::string("\f"));}
        case 'r' :{loc.advance(); return ok(std::string("\r"));}
        case 'u' :
        {
            if(const auto token = lex_escape_unicode_short::invoke(loc))
            {
                return ok(read_utf8_codepoint(token.unwrap(), loc));
            }
            else
            {
                return err(format_underline("[error] parse_escape_sequence: "
                           "invalid token found in UTF-8 codepoint uXXXX.",
                           {{std::addressof(loc), "here"}}));
            }
        }
        case 'U':
        {
            if(const auto token = lex_escape_unicode_long::invoke(loc))
            {
                return ok(read_utf8_codepoint(token.unwrap(), loc));
            }
            else
            {
                return err(format_underline("[error] parse_escape_sequence: "
                           "invalid token found in UTF-8 codepoint Uxxxxxxxx",
                           {{std::addressof(loc), "here"}}));
            }
        }
    }

    const auto msg = format_underline("[error] parse_escape_sequence: "
           "unknown escape sequence appeared.", {{std::addressof(loc),
           "escape sequence is one of \\, \", b, t, n, f, r, uxxxx, Uxxxxxxxx"}},
           /* Hints = */{"if you want to write backslash as just one backslash, "
           "use literal string like: regex    = '<\\i\\c*\\s*>'"});
    loc.reset(first);
    return err(msg);
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_ml_basic_string(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_ml_basic_string::invoke(loc))
    {
        auto inner_loc = loc;
        inner_loc.reset(first);

        std::string retval;
        retval.reserve(token.unwrap().size());

        auto delim = lex_ml_basic_string_delim::invoke(inner_loc);
        if(!delim)
        {
            throw internal_error(format_underline("[error] "
                "parse_ml_basic_string: invalid token",
                {{std::addressof(inner_loc), "should be \"\"\""}}));
        }
        // immediate newline is ignored (if exists)
        /* discard return value */ lex_newline::invoke(inner_loc);

        delim = none();
        while(!delim)
        {
            using lex_unescaped_seq = repeat<
                either<lex_ml_basic_unescaped, lex_newline>, unlimited>;
            if(auto unescaped = lex_unescaped_seq::invoke(inner_loc))
            {
                retval += unescaped.unwrap().str();
            }
            if(auto escaped = parse_escape_sequence(inner_loc))
            {
                retval += escaped.unwrap();
            }
            if(auto esc_nl = lex_ml_basic_escaped_newline::invoke(inner_loc))
            {
                // ignore newline after escape until next non-ws char
            }
            if(inner_loc.iter() == inner_loc.end())
            {
                throw internal_error(format_underline("[error] "
                    "parse_ml_basic_string: unexpected end of region",
                    {{std::addressof(inner_loc), "not sufficient token"}}));
            }
            delim = lex_ml_basic_string_delim::invoke(inner_loc);
        }
        return ok(std::make_pair(toml::string(retval), token.unwrap()));
    }
    else
    {
        loc.reset(first);
        return err(format_underline("[error] toml::parse_ml_basic_string: "
                   "the next token is not a multiline string",
                   {{std::addressof(loc), "here"}}));
    }
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_basic_string(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_basic_string::invoke(loc))
    {
        auto inner_loc = loc;
        inner_loc.reset(first);

        auto quot = lex_quotation_mark::invoke(inner_loc);
        if(!quot)
        {
            throw internal_error(format_underline("[error] parse_basic_string: "
                "invalid token", {{std::addressof(inner_loc), "should be \""}}));
        }

        std::string retval;
        retval.reserve(token.unwrap().size());

        quot = none();
        while(!quot)
        {
            using lex_unescaped_seq = repeat<lex_basic_unescaped, unlimited>;
            if(auto unescaped = lex_unescaped_seq::invoke(inner_loc))
            {
                retval += unescaped.unwrap().str();
            }
            if(auto escaped = parse_escape_sequence(inner_loc))
            {
                retval += escaped.unwrap();
            }
            if(inner_loc.iter() == inner_loc.end())
            {
                throw internal_error(format_underline("[error] "
                    "parse_ml_basic_string: unexpected end of region",
                    {{std::addressof(inner_loc), "not sufficient token"}}));
            }
            quot = lex_quotation_mark::invoke(inner_loc);
        }
        return ok(std::make_pair(toml::string(retval), token.unwrap()));
    }
    else
    {
        loc.reset(first); // rollback
        return err(format_underline("[error] toml::parse_basic_string: "
                   "the next token is not a string",
                   {{std::addressof(loc), "here"}}));
    }
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_ml_literal_string(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_ml_literal_string::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto open = lex_ml_literal_string_delim::invoke(inner_loc);
        if(!open)
        {
            throw internal_error(format_underline("[error] "
                "parse_ml_literal_string: invalid token",
                {{std::addressof(inner_loc), "should be '''"}}));
        }
        // immediate newline is ignored (if exists)
        /* discard return value */ lex_newline::invoke(inner_loc);

        const auto body = lex_ml_literal_body::invoke(inner_loc);

        const auto close = lex_ml_literal_string_delim::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "parse_ml_literal_string: invalid token",
                {{std::addressof(inner_loc), "should be '''"}}));
        }
        return ok(std::make_pair(
                  toml::string(body.unwrap().str(), toml::string_t::literal),
                  token.unwrap()));
    }
    else
    {
        loc.reset(first); // rollback
        return err(format_underline("[error] toml::parse_ml_literal_string: "
                   "the next token is not a multiline literal string",
                   {{std::addressof(loc), "here"}}));
    }
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_literal_string(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_literal_string::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto open = lex_apostrophe::invoke(inner_loc);
        if(!open)
        {
            throw internal_error(format_underline("[error] "
                "parse_literal_string: invalid token",
                {{std::addressof(inner_loc), "should be '"}}));
        }

        const auto body = repeat<lex_literal_char, unlimited>::invoke(inner_loc);

        const auto close = lex_apostrophe::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "parse_literal_string: invalid token",
                {{std::addressof(inner_loc), "should be '"}}));
        }
        return ok(std::make_pair(
                  toml::string(body.unwrap().str(), toml::string_t::literal),
                  token.unwrap()));
    }
    else
    {
        loc.reset(first); // rollback
        return err(format_underline("[error] toml::parse_literal_string: "
                   "the next token is not a literal string",
                   {{std::addressof(loc), "here"}}));
    }
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_string(location<Container>& loc)
{
    if(loc.iter() != loc.end() && *(loc.iter()) == '"')
    {
        if(loc.iter() + 1 != loc.end() && *(loc.iter() + 1) == '"' &&
           loc.iter() + 2 != loc.end() && *(loc.iter() + 2) == '"')
        {
            return parse_ml_basic_string(loc);
        }
        else
        {
            return parse_basic_string(loc);
        }
    }
    else if(loc.iter() != loc.end() && *(loc.iter()) == '\'')
    {
        if(loc.iter() + 1 != loc.end() && *(loc.iter() + 1) == '\'' &&
           loc.iter() + 2 != loc.end() && *(loc.iter() + 2) == '\'')
        {
            return parse_ml_literal_string(loc);
        }
        else
        {
            return parse_literal_string(loc);
        }
    }
    return err(format_underline("[error] toml::parse_string: ",
                {{std::addressof(loc), "the next token is not a string"}}));
}

template<typename Container>
result<std::pair<local_date, region<Container>>, std::string>
parse_local_date(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_local_date::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto y = lex_date_fullyear::invoke(inner_loc);
        if(!y || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != '-')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_inner_local_date: invalid year format",
                {{std::addressof(inner_loc), "should be `-`"}}));
        }
        inner_loc.advance();
        const auto m = lex_date_month::invoke(inner_loc);
        if(!m || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != '-')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_date: invalid month format",
                {{std::addressof(inner_loc), "should be `-`"}}));
        }
        inner_loc.advance();
        const auto d = lex_date_mday::invoke(inner_loc);
        if(!d)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_date: invalid day format",
                {{std::addressof(inner_loc), "here"}}));
        }
        return ok(std::make_pair(local_date(
            static_cast<std::int16_t>(from_string<int>(y.unwrap().str(), 0)),
            static_cast<month_t>(
                static_cast<std::int8_t>(from_string<int>(m.unwrap().str(), 0)-1)),
            static_cast<std::int8_t>(from_string<int>(d.unwrap().str(), 0))),
            token.unwrap()));
    }
    else
    {
        loc.reset(first);
        return err(format_underline("[error]: toml::parse_local_date: ",
            {{std::addressof(loc), "the next token is not a local_date"}}));
    }
}

template<typename Container>
result<std::pair<local_time, region<Container>>, std::string>
parse_local_time(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_local_time::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto h = lex_time_hour::invoke(inner_loc);
        if(!h || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != ':')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_time: invalid year format",
                {{std::addressof(inner_loc), "should be `:`"}}));
        }
        inner_loc.advance();
        const auto m = lex_time_minute::invoke(inner_loc);
        if(!m || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != ':')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_time: invalid month format",
                {{std::addressof(inner_loc), "should be `:`"}}));
        }
        inner_loc.advance();
        const auto s = lex_time_second::invoke(inner_loc);
        if(!s)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_time: invalid second format",
                {{std::addressof(inner_loc), "here"}}));
        }
        local_time time(
            from_string<int>(h.unwrap().str(), 0),
            from_string<int>(m.unwrap().str(), 0),
            from_string<int>(s.unwrap().str(), 0), 0, 0);

        const auto before_secfrac = inner_loc.iter();
        if(const auto secfrac = lex_time_secfrac::invoke(inner_loc))
        {
            auto sf = secfrac.unwrap().str();
            sf.erase(sf.begin()); // sf.front() == '.'
            switch(sf.size() % 3)
            {
                case 2:  sf += '0';  break;
                case 1:  sf += "00"; break;
                case 0:  break;
                default: break;
            }
            if(sf.size() >= 6)
            {
                time.millisecond = from_string<std::uint16_t>(sf.substr(0, 3), 0u);
                time.microsecond = from_string<std::uint16_t>(sf.substr(3, 3), 0u);
            }
            else if(sf.size() >= 3)
            {
                time.millisecond = from_string<std::uint16_t>(sf, 0u);
                time.microsecond = 0u;
            }
        }
        else
        {
            if(before_secfrac != inner_loc.iter())
            {
                throw internal_error(format_underline("[error]: "
                    "toml::parse_local_time: invalid subsecond format",
                    {{std::addressof(inner_loc), "here"}}));
            }
        }
        return ok(std::make_pair(time, token.unwrap()));
    }
    else
    {
        loc.reset(first);
        return err(format_underline("[error]: toml::parse_local_time: ",
            {{std::addressof(loc), "the next token is not a local_time"}}));
    }
}

template<typename Container>
result<std::pair<local_datetime, region<Container>>, std::string>
parse_local_datetime(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_local_date_time::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());
        const auto date = parse_local_date(inner_loc);
        if(!date || inner_loc.iter() == inner_loc.end())
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_datetime: invalid datetime format",
                {{std::addressof(inner_loc), "date, not datetime"}}));
        }
        const char delim = *(inner_loc.iter());
        if(delim != 'T' && delim != 't' && delim != ' ')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_datetime: invalid datetime format",
                {{std::addressof(inner_loc), "should be `T` or ` ` (space)"}}));
        }
        inner_loc.advance();
        const auto time = parse_local_time(inner_loc);
        if(!time)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_datetime: invalid datetime format",
                {{std::addressof(inner_loc), "invalid time fomrat"}}));
        }
        return ok(std::make_pair(
            local_datetime(date.unwrap().first, time.unwrap().first),
            token.unwrap()));
    }
    else
    {
        loc.reset(first);
        return err(format_underline("[error]: toml::parse_local_datetime: ",
            {{std::addressof(loc), "the next token is not a local_datetime"}}));
    }
}

template<typename Container>
result<std::pair<offset_datetime, region<Container>>, std::string>
parse_offset_datetime(location<Container>& loc)
{
    const auto first = loc.iter();
    if(const auto token = lex_offset_date_time::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());
        const auto datetime = parse_local_datetime(inner_loc);
        if(!datetime || inner_loc.iter() == inner_loc.end())
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_offset_datetime: invalid datetime format",
                {{std::addressof(inner_loc), "date, not datetime"}}));
        }
        time_offset offset(0, 0);
        if(const auto ofs = lex_time_numoffset::invoke(inner_loc))
        {
            const auto str = ofs.unwrap().str();
            if(str.front() == '+')
            {
                offset.hour   = static_cast<std::int8_t>(from_string<int>(str.substr(1,2), 0));
                offset.minute = static_cast<std::int8_t>(from_string<int>(str.substr(4,2), 0));
            }
            else
            {
                offset.hour   = -static_cast<std::int8_t>(from_string<int>(str.substr(1,2), 0));
                offset.minute = -static_cast<std::int8_t>(from_string<int>(str.substr(4,2), 0));
            }
        }
        else if(*inner_loc.iter() != 'Z' && *inner_loc.iter() != 'z')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_offset_datetime: invalid datetime format",
                {{std::addressof(inner_loc), "should be `Z` or `+HH:MM`"}}));
        }
        return ok(std::make_pair(offset_datetime(datetime.unwrap().first, offset),
                                 token.unwrap()));
    }
    else
    {
        loc.reset(first);
        return err(format_underline("[error]: toml::parse_offset_datetime: ",
            {{std::addressof(loc), "the next token is not a offset_datetime"}}));
    }
}

template<typename Container>
result<std::pair<key, region<Container>>, std::string>
parse_simple_key(location<Container>& loc)
{
    if(const auto bstr = parse_basic_string(loc))
    {
        return ok(std::make_pair(bstr.unwrap().first.str, bstr.unwrap().second));
    }
    if(const auto lstr = parse_literal_string(loc))
    {
        return ok(std::make_pair(lstr.unwrap().first.str, lstr.unwrap().second));
    }
    if(const auto bare = lex_unquoted_key::invoke(loc))
    {
        const auto reg = bare.unwrap();
        return ok(std::make_pair(reg.str(), reg));
    }
    return err(format_underline("[error] toml::parse_simple_key: ",
            {{std::addressof(loc), "the next token is not a simple key"}}));
}

// dotted key become vector of keys
template<typename Container>
result<std::pair<std::vector<key>, region<Container>>, std::string>
parse_key(location<Container>& loc)
{
    const auto first = loc.iter();
    // dotted key -> foo.bar.baz whitespaces are allowed
    if(const auto token = lex_dotted_key::invoke(loc))
    {
        const auto reg = token.unwrap();
        location<std::string> inner_loc(loc.name(), reg.str());
        std::vector<key> keys;

        while(inner_loc.iter() != inner_loc.end())
        {
            lex_ws::invoke(inner_loc);
            if(const auto k = parse_simple_key(inner_loc))
            {
                keys.push_back(k.unwrap().first);
            }
            else
            {
                throw internal_error(format_underline("[error] "
                    "toml::detail::parse_key: dotted key contains invalid key",
                    {{std::addressof(inner_loc), k.unwrap_err()}}));
            }

            lex_ws::invoke(inner_loc);
            if(inner_loc.iter() == inner_loc.end())
            {
                break;
            }
            else if(*inner_loc.iter() == '.')
            {
                inner_loc.advance(); // to skip `.`
            }
            else
            {
                throw internal_error(format_underline("[error] toml::parse_key: "
                    "dotted key contains invalid key ",
                    {{std::addressof(inner_loc), "should be `.`"}}));
            }
        }
        return ok(std::make_pair(keys, reg));
    }
    loc.reset(first);

    // simple key -> foo
    if(const auto smpl = parse_simple_key(loc))
    {
        return ok(std::make_pair(std::vector<key>(1, smpl.unwrap().first),
                                 smpl.unwrap().second));
    }
    return err(format_underline("[error] toml::parse_key: ",
                {{std::addressof(loc), "is not a valid key"}}));
}

// forward-decl to implement parse_array and parse_table
template<typename Value, typename Container>
result<Value, std::string> parse_value(location<Container>&);

template<typename Value, typename Container>
result<std::pair<typename Value::array_type, region<Container>>, std::string>
parse_array(location<Container>& loc)
{
    using value_type = Value;
    using array_type = typename value_type::array_type;

    const auto first = loc.iter();
    if(loc.iter() == loc.end())
    {
        return err("[error] toml::parse_array: input is empty");
    }
    if(*loc.iter() != '[')
    {
        return err("[error] toml::parse_array: token is not an array");
    }
    loc.advance();

    using lex_ws_comment_newline = repeat<
        either<lex_wschar, lex_newline, lex_comment>, unlimited>;

    array_type retval;
    while(loc.iter() != loc.end())
    {
        lex_ws_comment_newline::invoke(loc); // skip

        if(loc.iter() != loc.end() && *loc.iter() == ']')
        {
            loc.advance(); // skip ']'
            return ok(std::make_pair(retval,
                      region<Container>(loc, first, loc.iter())));
        }

        if(auto val = parse_value<value_type>(loc))
        {
            if(!retval.empty() && retval.front().type() != val.as_ok().type())
            {
                auto array_start_loc = loc;
                array_start_loc.reset(first);

                throw syntax_error(format_underline("[error] toml::parse_array: "
                    "type of elements should be the same each other.", {
                        {std::addressof(array_start_loc), "array starts here"},
                        {
                            std::addressof(get_region(retval.front())),
                            "value has type " + stringize(retval.front().type())
                        },
                        {
                            std::addressof(get_region(val.unwrap())),
                            "value has different type, " + stringize(val.unwrap().type())
                        }
                    }));
            }
            retval.push_back(std::move(val.unwrap()));
        }
        else
        {
            auto array_start_loc = loc;
            array_start_loc.reset(first);

            throw syntax_error(format_underline("[error] toml::parse_array: "
                "value having invalid format appeared in an array", {
                    {std::addressof(array_start_loc), "array starts here"},
                    {std::addressof(loc), "it is not a valid value."}
                }));
        }

        using lex_array_separator = sequence<maybe<lex_ws>, character<','>>;
        const auto sp = lex_array_separator::invoke(loc);
        if(!sp)
        {
            lex_ws_comment_newline::invoke(loc);
            if(loc.iter() != loc.end() && *loc.iter() == ']')
            {
                loc.advance(); // skip ']'
                return ok(std::make_pair(retval,
                          region<Container>(loc, first, loc.iter())));
            }
            else
            {
                auto array_start_loc = loc;
                array_start_loc.reset(first);

                throw syntax_error(format_underline("[error] toml::parse_array:"
                    " missing array separator `,` after a value", {
                        {std::addressof(array_start_loc), "array starts here"},
                        {std::addressof(loc),             "should be `,`"}
                    }));
            }
        }
    }
    loc.reset(first);
    throw syntax_error(format_underline("[error] toml::parse_array: "
            "array did not closed by `]`",
            {{std::addressof(loc), "should be closed"}}));
}

template<typename Value, typename Container>
result<std::pair<std::pair<std::vector<key>, region<Container>>, Value>, std::string>
parse_key_value_pair(location<Container>& loc)
{
    using value_type = Value;

    const auto first = loc.iter();
    auto key_reg = parse_key(loc);
    if(!key_reg)
    {
        std::string msg = std::move(key_reg.unwrap_err());
        // if the next token is keyvalue-separator, it means that there are no
        // key. then we need to show error as "empty key is not allowed".
        if(const auto keyval_sep = lex_keyval_sep::invoke(loc))
        {
            loc.reset(first);
            msg = format_underline("[error] toml::parse_key_value_pair: "
                "empty key is not allowed.",
                {{std::addressof(loc), "key expected before '='"}});
        }
        return err(std::move(msg));
    }

    const auto kvsp = lex_keyval_sep::invoke(loc);
    if(!kvsp)
    {
        std::string msg;
        // if the line contains '=' after the invalid sequence, possibly the
        // error is in the key (like, invalid character in bare key).
        const auto line_end = std::find(loc.iter(), loc.end(), '\n');
        if(std::find(loc.iter(), line_end, '=') != line_end)
        {
            msg = format_underline("[error] toml::parse_key_value_pair: "
                "invalid format for key",
                {{std::addressof(loc), "invalid character in key"}},
                {"Did you forget '.' to separate dotted-key?",
                "Allowed characters for bare key are [0-9a-zA-Z_-]."});
        }
        else // if not, the error is lack of key-value separator.
        {
            msg = format_underline("[error] toml::parse_key_value_pair: "
                "missing key-value separator `=`",
                {{std::addressof(loc), "should be `=`"}});
        }
        loc.reset(first);
        return err(std::move(msg));
    }

    const auto after_kvsp = loc.iter(); // err msg
    auto val = parse_value<value_type>(loc);
    if(!val)
    {
        std::string msg;
        loc.reset(after_kvsp);
        // check there is something not a comment/whitespace after `=`
        if(sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>::invoke(loc))
        {
            loc.reset(after_kvsp);
            msg = format_underline("[error] toml::parse_key_value_pair: "
                    "missing value after key-value separator '='",
                    {{std::addressof(loc), "expected value, but got nothing"}});
        }
        else // there is something not a comment/whitespace, so invalid format.
        {
            msg = std::move(val.unwrap_err());
        }
        loc.reset(first);
        return err(msg);
    }
    return ok(std::make_pair(std::move(key_reg.unwrap()),
                             std::move(val.unwrap())));
}

// for error messages.
template<typename InputIterator>
std::string format_dotted_keys(InputIterator first, const InputIterator last)
{
    static_assert(std::is_same<key,
        typename std::iterator_traits<InputIterator>::value_type>::value,"");

    std::string retval(*first++);
    for(; first != last; ++first)
    {
        retval += '.';
        retval += *first;
    }
    return retval;
}

// forward decl for is_valid_forward_table_definition
template<typename Container>
result<std::pair<std::vector<key>, region<Container>>, std::string>
parse_table_key(location<Container>& loc);

// The following toml file is allowed.
// ```toml
// [a.b.c]     # here, table `a` has element `b`.
// foo = "bar"
// [a]         # merge a = {baz = "qux"} to a = {b = {...}}
// baz = "qux"
// ```
// But the following is not allowed.
// ```toml
// [a]
// b.c.foo = "bar"
// [a]             # error! the same table [a] defined!
// baz = "qux"
// ```
// The following is neither allowed.
// ```toml
// a = { b.c.foo = "bar"}
// [a]             # error! the same table [a] defined!
// baz = "qux"
// ```
// Here, it parses region of `tab->at(k)` as a table key and check the depth
// of the key. If the key region points deeper node, it would be allowed.
// Otherwise, the key points the same node. It would be rejected.
template<typename Value, typename Iterator>
bool is_valid_forward_table_definition(const Value& fwd,
        Iterator key_first, Iterator key_curr, Iterator key_last)
{
    location<std::string> def("internal", detail::get_region(fwd).str());
    if(const auto tabkeys = parse_table_key(def))
    {
        // table keys always contains all the nodes from the root.
        const auto& tks = tabkeys.unwrap().first;
        if(std::size_t(std::distance(key_first, key_last)) == tks.size() &&
           std::equal(tks.begin(), tks.end(), key_first))
        {
            // the keys are equivalent. it is not allowed.
            return false;
        }
        // the keys are not equivalent. it is allowed.
        return true;
    }
    if(const auto dotkeys = parse_key(def))
    {
        // consider the following case.
        // [a]
        // b.c = {d = 42}
        // [a.b.c]
        // e = 2.71
        // this defines the table [a.b.c] twice. no?

        // a dotted key starts from the node representing a table in which the
        // dotted key belongs to.
        const auto& dks = dotkeys.unwrap().first;
        if(std::size_t(std::distance(key_curr, key_last)) == dks.size() &&
           std::equal(dks.begin(), dks.end(), key_curr))
        {
            // the keys are equivalent. it is not allowed.
            return false;
        }
        // the keys are not equivalent. it is allowed.
        return true;
    }
    return false;
}

template<typename Value, typename InputIterator, typename Container>
result<bool, std::string>
insert_nested_key(typename Value::table_type& root, const Value& v,
                  InputIterator iter, const InputIterator last,
                  region<Container> key_reg,
                  const bool is_array_of_table = false)
{
    static_assert(std::is_same<key,
        typename std::iterator_traits<InputIterator>::value_type>::value,"");

    using value_type = Value;
    using table_type = typename value_type::table_type;
    using array_type = typename value_type::array_type;

    const auto first = iter;
    assert(iter != last);

    table_type* tab = std::addressof(root);
    for(; iter != last; ++iter) // search recursively
    {
        const key& k = *iter;
        if(std::next(iter) == last) // k is the last key
        {
            // XXX if the value is array-of-tables, there can be several
            //     tables that are in the same array. in that case, we need to
            //     find the last element and insert it to there.
            if(is_array_of_table)
            {
                if(tab->count(k) == 1) // there is already an array of table
                {
                    if(tab->at(k).is_table())
                    {
                        // show special err msg for conflicting table
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last),
                            "\") cannot be defined"), {
                                {std::addressof(get_region(tab->at(k))),
                                 "table already defined"},
                                {std::addressof(get_region(v)),
                                 "this conflicts with the previous table"}
                            }));
                    }
                    else if(!(tab->at(k).is_array()))
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") collides with"
                            " existing value"), {
                                {std::addressof(get_region(tab->at(k))),
                                 concat_to_string("this ", tab->at(k).type(),
                                                  " value already exists")},
                                {std::addressof(get_region(v)),
                                 "while inserting this array-of-tables"}
                            }));
                    }
                    // the above if-else-if checks tab->at(k) is an array
                    auto& a = tab->at(k).as_array();
                    if(!(a.front().is_table()))
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") collides with"
                            " existing value"), {
                                {std::addressof(get_region(tab->at(k))),
                                 concat_to_string("this ", tab->at(k).type(),
                                                  " value already exists")},
                                {std::addressof(get_region(v)),
                                 "while inserting this array-of-tables"}
                            }));
                    }
                    // avoid conflicting array of table like the following.
                    // ```toml
                    // a = [{b = 42}] # define a as an array of *inline* tables
                    // [[a]]          # a is an array of *multi-line* tables
                    // b = 54
                    // ```
                    // Here, from the type information, these cannot be detected
                    // bacause inline table is also a table.
                    // But toml v0.5.0 explicitly says it is invalid. The above
                    // array-of-tables has a static size and appending to the
                    // array is invalid.
                    // In this library, multi-line table value has a region
                    // that points to the key of the table (e.g. [[a]]). By
                    // comparing the first two letters in key, we can detect
                    // the array-of-table is inline or multiline.
                    if(detail::get_region(a.front()).str().substr(0,2) != "[[")
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") collides with"
                            " existing array-of-tables"), {
                                {std::addressof(get_region(tab->at(k))),
                                 concat_to_string("this ", tab->at(k).type(),
                                                  " value has static size")},
                                {std::addressof(get_region(v)),
                                 "appending it to the statically sized array"}
                            }));
                    }
                    a.push_back(v);
                    return ok(true);
                }
                else // if not, we need to create the array of table
                {
                    value_type aot(array_type(1, v), key_reg);
                    tab->insert(std::make_pair(k, aot));
                    return ok(true);
                }
            } // end if(array of table)

            if(tab->count(k) == 1)
            {
                if(tab->at(k).is_table() && v.is_table())
                {
                    if(!is_valid_forward_table_definition(
                                tab->at(k), first, iter, last))
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: table (\"",
                            format_dotted_keys(first, last),
                            "\") already exists."), {
                                {std::addressof(get_region(tab->at(k))),
                                 "table already exists here"},
                                {std::addressof(get_region(v)),
                                 "table defined twice"}
                            }));
                    }
                    // to allow the following toml file.
                    // [a.b.c]
                    // d = 42
                    // [a]
                    // e = 2.71
                    auto& t = tab->at(k).as_table();
                    for(const auto& kv : v.as_table())
                    {
                        t[kv.first] = kv.second;
                    }
                    detail::change_region(tab->at(k), key_reg);
                    return ok(true);
                }
                else if(v.is_table()                     &&
                        tab->at(k).is_array()            &&
                        tab->at(k).as_array().size() > 0 &&
                        tab->at(k).as_array().front().is_table())
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: array of tables (\"",
                        format_dotted_keys(first, last), "\") already exists."), {
                            {std::addressof(get_region(tab->at(k))),
                             "array of tables defined here"},
                            {std::addressof(get_region(v)),
                            "table conflicts with the previous array of table"}
                        }));
                }
                else
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: value (\"",
                        format_dotted_keys(first, last), "\") already exists."), {
                            {std::addressof(get_region(tab->at(k))),
                             "value already exists here"},
                            {std::addressof(get_region(v)),
                             "value defined twice"}
                        }));
                }
            }
            tab->insert(std::make_pair(k, v));
            return ok(true);
        }
        else
        {
            // if there is no corresponding value, insert it first.
            // related: you don't need to write
            // # [x]
            // # [x.y]
            // to write
            // [x.y.z]
            if(tab->count(k) == 0)
            {
                (*tab)[k] = value_type(table_type{}, key_reg);
            }

            // type checking...
            if(tab->at(k).is_table())
            {
                tab = std::addressof((*tab)[k].as_table());
            }
            else if(tab->at(k).is_array()) // inserting to array-of-tables?
            {
                auto& a = (*tab)[k].as_array();
                if(!a.back().is_table())
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: target (",
                        format_dotted_keys(first, std::next(iter)),
                        ") is neither table nor an array of tables"), {
                            {std::addressof(get_region(a.back())),
                             concat_to_string("actual type is ", a.back().type())},
                            {std::addressof(get_region(v)), "inserting this"}
                        }));
                }
                tab = std::addressof(a.back().as_table());
            }
            else
            {
                throw syntax_error(format_underline(concat_to_string(
                    "[error] toml::insert_value: target (",
                    format_dotted_keys(first, std::next(iter)),
                    ") is neither table nor an array of tables"), {
                        {std::addressof(get_region(tab->at(k))),
                         concat_to_string("actual type is ", tab->at(k).type())},
                        {std::addressof(get_region(v)), "inserting this"}
                    }));
            }
        }
    }
    return err(std::string("toml::detail::insert_nested_key: never reach here"));
}

template<typename Value, typename Container>
result<std::pair<typename Value::table_type, region<Container>>, std::string>
parse_inline_table(location<Container>& loc)
{
    using value_type = Value;
    using table_type = typename value_type::table_type;

    const auto first = loc.iter();
    table_type retval;
    if(!(loc.iter() != loc.end() && *loc.iter() == '{'))
    {
        return err(format_underline("[error] toml::parse_inline_table: ",
            {{std::addressof(loc), "the next token is not an inline table"}}));
    }
    loc.advance();
    // it starts from "{". it should be formatted as inline-table
    while(loc.iter() != loc.end())
    {
        maybe<lex_ws>::invoke(loc);
        if(loc.iter() != loc.end() && *loc.iter() == '}')
        {
            loc.advance(); // skip `}`
            return ok(std::make_pair(
                        retval, region<Container>(loc, first, loc.iter())));
        }

        const auto kv_r = parse_key_value_pair<value_type>(loc);
        if(!kv_r)
        {
            return err(kv_r.unwrap_err());
        }
        const std::vector<key>&  keys    = kv_r.unwrap().first.first;
        const region<Container>& key_reg = kv_r.unwrap().first.second;
        const value_type&        val     = kv_r.unwrap().second;

        const auto inserted =
            insert_nested_key(retval, val, keys.begin(), keys.end(), key_reg);
        if(!inserted)
        {
            throw internal_error("[error] toml::parse_inline_table: "
                "failed to insert value into table: " + inserted.unwrap_err());
        }

        using lex_table_separator = sequence<maybe<lex_ws>, character<','>>;
        const auto sp = lex_table_separator::invoke(loc);
        if(!sp)
        {
            maybe<lex_ws>::invoke(loc);
            if(loc.iter() != loc.end() && *loc.iter() == '}')
            {
                loc.advance(); // skip `}`
                return ok(std::make_pair(
                            retval, region<Container>(loc, first, loc.iter())));
            }
            else if(*loc.iter() == '#' || *loc.iter() == '\r' || *loc.iter() == '\n')
            {
                throw syntax_error(format_underline("[error] "
                    "toml::parse_inline_table: missing curly brace `}`",
                    {{std::addressof(loc), "should be `}`"}}));
            }
            else
            {
                throw syntax_error(format_underline("[error] "
                    "toml::parse_inline_table: missing table separator `,` ",
                    {{std::addressof(loc), "should be `,`"}}));
            }
        }
    }
    loc.reset(first);
    throw syntax_error(format_underline("[error] toml::parse_inline_table: "
            "inline table did not closed by `}`",
            {{std::addressof(loc), "should be closed"}}));
}

template<typename Container>
result<value_t, std::string> guess_number_type(const location<Container>& l)
{
    // This function tries to find some (common) mistakes by checking characters
    // that follows the last character of a value. But it is often difficult
    // because some non-newline characters can appear after a value. E.g.
    // spaces, tabs, commas (in an array or inline table), closing brackets
    // (of an array or inline table), comment-sign (#). Since this function
    // does not parse further, those characters are always allowed to be there.
    location<Container> loc = l;

    if(lex_offset_date_time::invoke(loc)) {return ok(value_t::offset_datetime);}
    loc.reset(l.iter());

    if(lex_local_date_time::invoke(loc))
    {
        // bad offset may appear after this.
        if(loc.iter() != loc.end() && (*loc.iter() == '+' || *loc.iter() == '-'
                    || *loc.iter() == 'Z' || *loc.iter() == 'z'))
        {
            return err(format_underline("[error] bad offset: should be [+-]HH:MM or Z",
                        {{std::addressof(loc), "[+-]HH:MM or Z"}},
                        {"pass: +09:00, -05:30", "fail: +9:00, -5:30"}));
        }
        return ok(value_t::local_datetime);
    }
    loc.reset(l.iter());

    if(lex_local_date::invoke(loc))
    {
        // bad time may appear after this.
        // A space is allowed as a delimiter between local time. But there are
        // both cases in which a space becomes valid or invalid.
        // - invalid: 2019-06-16 7:00:00
        // - valid  : 2019-06-16 07:00:00
        if(loc.iter() != loc.end())
        {
            const auto c = *loc.iter();
            if(c == 'T' || c == 't')
            {
                return err(format_underline("[error] bad time: should be HH:MM:SS.subsec",
                        {{std::addressof(loc), "HH:MM:SS.subsec"}},
                        {"pass: 1979-05-27T07:32:00, 1979-05-27 07:32:00.999999",
                         "fail: 1979-05-27T7:32:00, 1979-05-27 17:32"}));
            }
            if('0' <= c && c <= '9')
            {
                return err(format_underline("[error] bad time: missing T",
                        {{std::addressof(loc), "T or space required here"}},
                        {"pass: 1979-05-27T07:32:00, 1979-05-27 07:32:00.999999",
                         "fail: 1979-05-27T7:32:00, 1979-05-27 7:32"}));
            }
            if(c == ' ' && std::next(loc.iter()) != loc.end() &&
                ('0' <= *std::next(loc.iter()) && *std::next(loc.iter())<= '9'))
            {
                loc.advance();
                return err(format_underline("[error] bad time: should be HH:MM:SS.subsec",
                        {{std::addressof(loc), "HH:MM:SS.subsec"}},
                        {"pass: 1979-05-27T07:32:00, 1979-05-27 07:32:00.999999",
                         "fail: 1979-05-27T7:32:00, 1979-05-27 7:32"}));
            }
        }
        return ok(value_t::local_date);
    }
    loc.reset(l.iter());

    if(lex_local_time::invoke(loc)) {return ok(value_t::local_time);}
    loc.reset(l.iter());

    if(lex_float::invoke(loc))
    {
        if(loc.iter() != loc.end() && *loc.iter() == '_')
        {
            return err(format_underline("[error] bad float: `_` should be surrounded by digits",
                        {{std::addressof(loc), "here"}},
                        {"pass: +1.0, -2e-2, 3.141_592_653_589, inf, nan",
                         "fail: .0, 1., _1.0, 1.0_, 1_.0, 1.0__0"}));
        }
        return ok(value_t::floating);
    }
    loc.reset(l.iter());

    if(lex_integer::invoke(loc))
    {
        if(loc.iter() != loc.end())
        {
            const auto c = *loc.iter();
            if(c == '_')
            {
                return err(format_underline("[error] bad integer: `_` should be surrounded by digits",
                            {{std::addressof(loc), "here"}},
                            {"pass: -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755",
                             "fail: 1__000, 0123"}));
            }
            if('0' <= c && c <= '9')
            {
                // leading zero. point '0'
                loc.retrace();
                return err(format_underline("[error] bad integer: leading zero",
                            {{std::addressof(loc), "here"}},
                            {"pass: -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755",
                             "fail: 1__000, 0123"}));
            }
            if(c == ':' || c == '-')
            {
                return err(format_underline("[error] bad datetime: invalid format",
                            {{std::addressof(loc), "here"}},
                            {"pass: 1979-05-27T07:32:00-07:00, 1979-05-27 07:32:00.999999Z",
                             "fail: 1979-05-27T7:32:00-7:00, 1979-05-27 7:32-00:30"}));
            }
            if(c == '.' || c == 'e' || c == 'E')
            {
                return err(format_underline("[error] bad float: invalid format",
                            {{std::addressof(loc), "here"}},
                            {"pass: +1.0, -2e-2, 3.141_592_653_589, inf, nan",
                             "fail: .0, 1., _1.0, 1.0_, 1_.0, 1.0__0"}));
            }
        }
        return ok(value_t::integer);
    }
    if(loc.iter() != loc.end() && *loc.iter() == '.')
    {
        return err(format_underline("[error] bad float: invalid format",
                {{std::addressof(loc), "integer part required before this"}},
                {"pass: +1.0, -2e-2, 3.141_592_653_589, inf, nan",
                 "fail: .0, 1., _1.0, 1.0_, 1_.0, 1.0__0"}));
    }
    if(loc.iter() != loc.end() && *loc.iter() == '_')
    {
        return err(format_underline("[error] bad number: `_` should be surrounded by digits",
                {{std::addressof(loc), "`_` is not surrounded by digits"}},
                {"pass: -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755",
                 "fail: 1__000, 0123"}));
    }
    return err(format_underline("[error] bad format: unknown value appeared",
                {{std::addressof(loc), "here"}}));
}

template<typename Container>
result<value_t, std::string> guess_value_type(const location<Container>& loc)
{
    switch(*loc.iter())
    {
        case '"' : {return ok(value_t::string);  }
        case '\'': {return ok(value_t::string);  }
        case 't' : {return ok(value_t::boolean); }
        case 'f' : {return ok(value_t::boolean); }
        case '[' : {return ok(value_t::array);   }
        case '{' : {return ok(value_t::table);   }
        case 'i' : {return ok(value_t::floating);} // inf.
        case 'n' : {return ok(value_t::floating);} // nan.
        default  : {return guess_number_type(loc);}
    }
}

template<typename Value, typename Container>
result<Value, std::string> parse_value(location<Container>& loc)
{
    using value_type = Value;

    const auto first = loc.iter();
    if(first == loc.end())
    {
        return err(format_underline("[error] toml::parse_value: input is empty",
                   {{std::addressof(loc), ""}}));
    }

    const auto type = guess_value_type(loc);
    if(!type)
    {
        return err(type.unwrap_err());
    }
    switch(type.unwrap())
    {
        case value_t::boolean        : {return parse_boolean(loc);        }
        case value_t::integer        : {return parse_integer(loc);        }
        case value_t::floating       : {return parse_floating(loc);       }
        case value_t::string         : {return parse_string(loc);         }
        case value_t::offset_datetime: {return parse_offset_datetime(loc);}
        case value_t::local_datetime : {return parse_local_datetime(loc); }
        case value_t::local_date     : {return parse_local_date(loc);     }
        case value_t::local_time     : {return parse_local_time(loc);     }
        case value_t::array          : {return parse_array<value_type>(loc);       }
        case value_t::table          : {return parse_inline_table<value_type>(loc);}
        default:
        {
            const auto msg = format_underline("[error] toml::parse_value: "
                    "unknown token appeared", {{std::addressof(loc), "unknown"}});
            loc.reset(first);
            return err(msg);
        }
    }
}

template<typename Container>
result<std::pair<std::vector<key>, region<Container>>, std::string>
parse_table_key(location<Container>& loc)
{
    if(auto token = lex_std_table::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto open = lex_std_table_open::invoke(inner_loc);
        if(!open || inner_loc.iter() == inner_loc.end())
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: no `[`",
                {{std::addressof(inner_loc), "should be `[`"}}));
        }
        // to skip [ a . b . c ]
        //          ^----------- this whitespace
        lex_ws::invoke(inner_loc);
        const auto keys = parse_key(inner_loc);
        if(!keys)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: invalid key",
                {{std::addressof(inner_loc), "not key"}}));
        }
        // to skip [ a . b . c ]
        //                    ^-- this whitespace
        lex_ws::invoke(inner_loc);
        const auto close = lex_std_table_close::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: no `]`",
                {{std::addressof(inner_loc), "should be `]`"}}));
        }

        // after [table.key], newline or EOF(empty table) requried.
        if(loc.iter() != loc.end())
        {
            using lex_newline_after_table_key =
                sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>;
            const auto nl = lex_newline_after_table_key::invoke(loc);
            if(!nl)
            {
                throw syntax_error(format_underline("[error] "
                    "toml::parse_table_key: newline required after [table.key]",
                    {{std::addressof(loc), "expected newline"}}));
            }
        }
        return ok(std::make_pair(keys.unwrap().first, token.unwrap()));
    }
    else
    {
        return err(format_underline("[error] toml::parse_table_key: "
            "not a valid table key", {{std::addressof(loc), "here"}}));
    }
}

template<typename Container>
result<std::pair<std::vector<key>, region<Container>>, std::string>
parse_array_table_key(location<Container>& loc)
{
    if(auto token = lex_array_table::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());

        const auto open = lex_array_table_open::invoke(inner_loc);
        if(!open || inner_loc.iter() == inner_loc.end())
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_array_table_key: no `[[`",
                {{std::addressof(inner_loc), "should be `[[`"}}));
        }
        lex_ws::invoke(inner_loc);
        const auto keys = parse_key(inner_loc);
        if(!keys)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_array_table_key: invalid key",
                {{std::addressof(inner_loc), "not a key"}}));
        }
        lex_ws::invoke(inner_loc);
        const auto close = lex_array_table_close::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: no `]]`",
                {{std::addressof(inner_loc), "should be `]]`"}}));
        }

        // after [[table.key]], newline or EOF(empty table) requried.
        if(loc.iter() != loc.end())
        {
            using lex_newline_after_table_key =
                sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>;
            const auto nl = lex_newline_after_table_key::invoke(loc);
            if(!nl)
            {
                throw syntax_error(format_underline("[error] toml::"
                    "parse_array_table_key: newline required after [[table.key]]",
                    {{std::addressof(loc), "expected newline"}}));
            }
        }
        return ok(std::make_pair(keys.unwrap().first, token.unwrap()));
    }
    else
    {
        return err(format_underline("[error] toml::parse_array_table_key: "
            "not a valid table key", {{std::addressof(loc), "here"}}));
    }
}

// parse table body (key-value pairs until the iter hits the next [tablekey])
template<typename Value, typename Container>
result<typename Value::table_type, std::string>
parse_ml_table(location<Container>& loc)
{
    using value_type = Value;
    using table_type = typename value_type::table_type;

    const auto first = loc.iter();
    if(first == loc.end())
    {
        return ok(table_type{});
    }

    // XXX at lest one newline is needed.
    using skip_line = repeat<
        sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>, at_least<1>>;
    skip_line::invoke(loc);

    table_type tab;
    while(loc.iter() != loc.end())
    {
        lex_ws::invoke(loc);
        const auto before = loc.iter();
        if(const auto tmp = parse_array_table_key(loc)) // next table found
        {
            loc.reset(before);
            return ok(tab);
        }
        if(const auto tmp = parse_table_key(loc)) // next table found
        {
            loc.reset(before);
            return ok(tab);
        }

        if(const auto kv = parse_key_value_pair<value_type>(loc))
        {
            const std::vector<key>&  keys    = kv.unwrap().first.first;
            const region<Container>& key_reg = kv.unwrap().first.second;
            const value_type&        val     = kv.unwrap().second;
            const auto inserted =
                insert_nested_key(tab, val, keys.begin(), keys.end(), key_reg);
            if(!inserted)
            {
                return err(inserted.unwrap_err());
            }
        }
        else
        {
            return err(kv.unwrap_err());
        }

        // comment lines are skipped by the above function call.
        // However, since the `skip_line` requires at least 1 newline, it fails
        // if the file ends with ws and/or comment without newline.
        // `skip_line` matches `ws? + comment? + newline`, not `ws` or `comment`
        // itself. To skip the last ws and/or comment, call lexers.
        // It does not matter if these fails, so the return value is discarded.
        lex_ws::invoke(loc);
        lex_comment::invoke(loc);

        // skip_line is (whitespace? comment? newline)_{1,}. multiple empty lines
        // and comments after the last key-value pairs are allowed.
        const auto newline = skip_line::invoke(loc);
        if(!newline && loc.iter() != loc.end())
        {
            const auto before2 = loc.iter();
            lex_ws::invoke(loc); // skip whitespace
            const auto msg = format_underline("[error] toml::parse_table: "
                "invalid line format", {{std::addressof(loc), concat_to_string(
                "expected newline, but got '", show_char(*loc.iter()), "'.")}});
            loc.reset(before2);
            return err(msg);
        }

        // the skip_lines only matches with lines that includes newline.
        // to skip the last line that includes comment and/or whitespace
        // but no newline, call them one more time.
        lex_ws::invoke(loc);
        lex_comment::invoke(loc);
    }
    return ok(tab);
}

template<typename Value, typename Container>
result<Value, std::string> parse_toml_file(location<Container>& loc)
{
    using value_type = Value;
    using table_type = typename value_type::table_type;

    const auto first = loc.iter();
    if(first == loc.end())
    {
        return ok(value_type(table_type{}));
    }

    // put the first line as a region of a file
    const region<Container> file(loc, loc.iter(),
            std::find(loc.iter(), loc.end(), '\n'));

    // The first successive comments that are separated from the first value
    // by an empty line are for a file itself.
    // ```toml
    // # this is a comment for a file.
    //
    // key = "the first value"
    // ```
    // ```toml
    // # this is a comment for "the first value".
    // key = "the first value"
    // ```
    std::vector<std::string> comments;
    using lex_first_comments = sequence<
        repeat<sequence<maybe<lex_ws>, lex_comment, lex_newline>, at_least<1>>,
        sequence<maybe<lex_ws>, lex_newline>
        >;
    if(const auto token = lex_first_comments::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());
        while(inner_loc.iter() != inner_loc.end())
        {
            maybe<lex_ws>::invoke(inner_loc); // remove ws if exists
            if(lex_newline::invoke(inner_loc))
            {
                assert(inner_loc.iter() == inner_loc.end());
                break; // empty line found.
            }
            auto com = lex_comment::invoke(inner_loc).unwrap().str();
            com.erase(com.begin()); // remove # sign
            comments.push_back(std::move(com));
            lex_newline::invoke(inner_loc);
        }
    }

    table_type data;
    // root object is also a table, but without [tablename]
    if(auto tab = parse_ml_table<value_type>(loc))
    {
        data = std::move(tab.unwrap());
    }
    else // failed (empty table is regarded as success in parse_ml_table)
    {
        return err(tab.unwrap_err());
    }
    while(loc.iter() != loc.end())
    {
        // here, the region of [table] is regarded as the table-key because
        // the table body is normally too big and it is not so informative
        // if the first key-value pair of the table is shown in the error
        // message.
        if(const auto tabkey = parse_array_table_key(loc))
        {
            const auto tab = parse_ml_table<value_type>(loc);
            if(!tab){return err(tab.unwrap_err());}

            const auto& keys = tabkey.unwrap().first;
            const auto& reg  = tabkey.unwrap().second;

            const auto inserted = insert_nested_key(data,
                    value_type(tab.unwrap(), reg),
                    keys.begin(), keys.end(), reg,
                    /*is_array_of_table=*/ true);
            if(!inserted) {return err(inserted.unwrap_err());}

            continue;
        }
        if(const auto tabkey = parse_table_key(loc))
        {
            const auto tab = parse_ml_table<value_type>(loc);
            if(!tab){return err(tab.unwrap_err());}

            const auto& keys = tabkey.unwrap().first;
            const auto& reg  = tabkey.unwrap().second;

            const auto inserted = insert_nested_key(data,
                value_type(tab.unwrap(), reg), keys.begin(), keys.end(), reg);
            if(!inserted) {return err(inserted.unwrap_err());}

            continue;
        }
        return err(format_underline("[error]: toml::parse_toml_file: "
            "unknown line appeared", {{std::addressof(loc), "unknown format"}}));
    }

    Value v(std::move(data), file);
    v.comments() = comments;

    return ok(std::move(v));
}

} // detail

template<typename                     Comment = ::toml::discard_comments,
         template<typename ...> class Table   = std::unordered_map,
         template<typename ...> class Array   = std::vector>
basic_value<Comment, Table, Array>
parse(std::istream& is, const std::string& fname = "unknown file")
{
    using value_type = basic_value<Comment, Table, Array>;

    const auto beg = is.tellg();
    is.seekg(0, std::ios::end);
    const auto end = is.tellg();
    const auto fsize = end - beg;
    is.seekg(beg);

    // read whole file as a sequence of char
    assert(fsize >= 0);
    std::vector<char> letters(static_cast<std::size_t>(fsize));
    is.read(letters.data(), fsize);

    detail::location<std::vector<char>>
        loc(std::move(fname), std::move(letters));

    // skip BOM if exists.
    // XXX component of BOM (like 0xEF) exceeds the representable range of
    // signed char, so on some (actually, most) of the environment, these cannot
    // be compared to char. However, since we are always out of luck, we need to
    // check our chars are equivalent to BOM. To do this, first we need to
    // convert char to unsigned char to guarantee the comparability.
    if(loc.source()->size() >= 3)
    {
        std::array<unsigned char, 3> BOM;
        std::memcpy(BOM.data(), loc.source()->data(), 3);
        if(BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
        {
            loc.advance(3); // BOM found. skip.
        }
    }

    const auto data = detail::parse_toml_file<value_type>(loc);
    if(!data)
    {
        throw syntax_error(data.unwrap_err());
    }
    return data.unwrap();
}

template<typename                     Comment = ::toml::discard_comments,
         template<typename ...> class Table   = std::unordered_map,
         template<typename ...> class Array   = std::vector>
basic_value<Comment, Table, Array> parse(const std::string& fname)
{
    std::ifstream ifs(fname.c_str(), std::ios_base::binary);
    if(!ifs.good())
    {
        throw std::runtime_error("toml::parse: file open error -> " + fname);
    }
    return parse<Comment, Table, Array>(ifs, fname);
}

} // toml
#endif// TOML11_PARSER_HPP

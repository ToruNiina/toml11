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
#include <iostream>
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
                "[error] toml::parse_boolean: internal error", reg,
                "invalid token"));
        }
    }
    loc.iter() = first; //rollback
    return err(std::string("[error] toml::parse_boolean: "
                           "the next token is not a boolean"));
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
                    token.unwrap(), "invalid token"));
            }
        }
        return ok(std::make_pair(retval, token.unwrap()));
    }
    loc.iter() = first;
    return err(std::string("[error] toml::parse_binary_integer:"
                           "the next token is not an integer"));
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
    loc.iter() = first;
    return err(std::string("[error] toml::parse_octal_integer:"
                           "the next token is not an integer"));
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
    loc.iter() = first;
    return err(std::string("[error] toml::parse_hexadecimal_integer"
                           "the next token is not an integer"));
}

template<typename Container>
result<std::pair<integer, region<Container>>, std::string>
parse_integer(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first != loc.end() && *first == '0')
    {
        if(const auto bin = parse_binary_integer     (loc)) {return bin;}
        if(const auto oct = parse_octal_integer      (loc)) {return oct;}
        if(const auto hex = parse_hexadecimal_integer(loc)) {return hex;}
        // else, maybe just zero.
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
    loc.iter() = first;
    return err(std::string("[error] toml::parse_integer: "
                           "the next token is not an integer"));
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
    loc.iter() = first;
    return err(std::string("[error] toml::parse_floating: "
                           "the next token is not a float"));
}

template<typename Container>
std::string read_utf8_codepoint(const region<Container>& reg)
{
    const auto str = reg.str().substr(1);
    std::uint_least32_t codepoint;
    std::istringstream iss(str);
    iss >> std::hex >> codepoint;

    std::string character;
    if(codepoint < 0x80) // U+0000 ... U+0079 ; just an ASCII.
    {
        character += static_cast<char>(codepoint);
    }
    else if(codepoint < 0x800) //U+0080 ... U+07FF
    {
        // 110yyyyx 10xxxxxx; 0x3f == 0b0011'1111
        character += static_cast<unsigned char>(0xC0| codepoint >> 6);
        character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
    }
    else if(codepoint < 0x10000) // U+0800...U+FFFF
    {
        // 1110yyyy 10yxxxxx 10xxxxxx
        character += static_cast<unsigned char>(0xE0| codepoint >> 12);
        character += static_cast<unsigned char>(0x80|(codepoint >> 6 & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint      & 0x3F));
    }
    else if(codepoint < 0x200000) // U+10000 ... U+1FFFFF
    {
        if(0x10FFFF < codepoint) // out of Unicode region
        {
            std::cerr << format_underline(concat_to_string("[warning] "
                    "input codepoint (", str, ") is too large to decode as "
                    "a unicode character. The result may not be able to render "
                    "to your screen."), reg, "should be in [0x00..0x10FFFF]")
                      << std::endl;
        }
        // 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
        character += static_cast<unsigned char>(0xF0| codepoint >> 18);
        character += static_cast<unsigned char>(0x80|(codepoint >> 12 & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint >> 6  & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint       & 0x3F));
    }
    else // out of UTF-8 region
    {
        throw std::range_error(format_underline(concat_to_string("[error] "
                "input codepoint (", str, ") is too large to encode as utf-8."),
                reg, "should be in [0x00..0x10FFFF]"));
    }
    return character;
}

template<typename Container>
result<std::string, std::string> parse_escape_sequence(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first == loc.end() || *first != '\\')
    {
        return err(std::string("[error]: toml::parse_escape_sequence: "
                    "the next token is not an escape sequence \"\\\""));
    }
    ++loc.iter();
    switch(*loc.iter())
    {
        case '\\':{++loc.iter(); return ok(std::string("\\"));}
        case '"' :{++loc.iter(); return ok(std::string("\""));}
        case 'b' :{++loc.iter(); return ok(std::string("\b"));}
        case 't' :{++loc.iter(); return ok(std::string("\t"));}
        case 'n' :{++loc.iter(); return ok(std::string("\n"));}
        case 'f' :{++loc.iter(); return ok(std::string("\f"));}
        case 'r' :{++loc.iter(); return ok(std::string("\r"));}
        case 'u' :
        {
            if(const auto token = lex_escape_unicode_short::invoke(loc))
            {
                return ok(read_utf8_codepoint(token.unwrap()));
            }
            else
            {
                return err(format_underline("[error] parse_escape_sequence: "
                           "invalid token found in UTF-8 codepoint uXXXX.",
                           loc, token.unwrap_err()));
            }
        }
        case 'U':
        {
            if(const auto token = lex_escape_unicode_long::invoke(loc))
            {
                return ok(read_utf8_codepoint(token.unwrap()));
            }
            else
            {
                return err(format_underline("[error] parse_escape_sequence: "
                           "invalid token found in UTF-8 codepoint Uxxxxxxxx",
                           loc, token.unwrap_err()));
            }
        }
    }

    const auto msg = format_underline("[error] parse_escape_sequence: "
           "unknown escape sequence appeared.", loc, "escape sequence is one of"
           " \\, \", b, t, n, f, r, uxxxx, Uxxxxxxxx", {"if you want to write "
           "backslash as just one backslash, use literal string like:",
           "regex    = '<\\i\\c*\\s*>'"});
    loc.iter() = first;
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
        inner_loc.iter() = first;

        std::string retval;
        retval.reserve(token.unwrap().size());

        auto delim = lex_ml_basic_string_delim::invoke(inner_loc);
        if(!delim)
        {
            throw internal_error(format_underline("[error] "
                "parse_ml_basic_string: invalid token",
                inner_loc, "should be \"\"\""));
        }
        // immediate newline is ignored (if exists)
        /* discard return value */ lex_newline::invoke(inner_loc);

        delim = err("tmp");
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
                    inner_loc, "not sufficient token"));
            }
            delim = lex_ml_basic_string_delim::invoke(inner_loc);
        }
        return ok(std::make_pair(toml::string(retval), token.unwrap()));
    }
    else
    {
        loc.iter() = first;
        return err(token.unwrap_err());
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
        inner_loc.iter() = first;

        auto quot = lex_quotation_mark::invoke(inner_loc);
        if(!quot)
        {
            throw internal_error(format_underline("[error] parse_basic_string: "
                "invalid token", inner_loc, "should be \""));
        }

        std::string retval;
        retval.reserve(token.unwrap().size());

        quot = err("tmp");
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
                    inner_loc, "not sufficient token"));
            }
            quot = lex_quotation_mark::invoke(inner_loc);
        }
        return ok(std::make_pair(toml::string(retval), token.unwrap()));
    }
    else
    {
        loc.iter() = first; // rollback
        return err(token.unwrap_err());
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
                inner_loc, "should be '''"));
        }
        // immediate newline is ignored (if exists)
        /* discard return value */ lex_newline::invoke(inner_loc);

        const auto body = lex_ml_literal_body::invoke(inner_loc);

        const auto close = lex_ml_literal_string_delim::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "parse_ml_literal_string: invalid token",
                inner_loc, "should be '''"));
        }
        return ok(std::make_pair(
                  toml::string(body.unwrap().str(), toml::string_t::literal),
                  token.unwrap()));
    }
    else
    {
        loc.iter() = first; // rollback
        return err(token.unwrap_err());
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
                inner_loc, "should be '"));
        }

        const auto body = repeat<lex_literal_char, unlimited>::invoke(inner_loc);

        const auto close = lex_apostrophe::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "parse_literal_string: invalid token",
                inner_loc, "should be '"));
        }
        return ok(std::make_pair(
                  toml::string(body.unwrap().str(), toml::string_t::literal),
                  token.unwrap()));
    }
    else
    {
        loc.iter() = first; // rollback
        return err(token.unwrap_err());
    }
}

template<typename Container>
result<std::pair<toml::string, region<Container>>, std::string>
parse_string(location<Container>& loc)
{
    if(const auto rslt = parse_ml_basic_string(loc))   {return rslt;}
    if(const auto rslt = parse_ml_literal_string(loc)) {return rslt;}
    if(const auto rslt = parse_basic_string(loc))      {return rslt;}
    if(const auto rslt = parse_literal_string(loc))    {return rslt;}
    return err(std::string("[error] toml::parse_string: "
                "the next token is not a string"));
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
                inner_loc, y.map_err_or_else([](const std::string& msg) {
                        return msg;
                    }, "should be `-`")));
        }
        ++inner_loc.iter();
        const auto m = lex_date_month::invoke(inner_loc);
        if(!m || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != '-')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_date: invalid month format",
                inner_loc, m.map_err_or_else([](const std::string& msg) {
                        return msg;
                    }, "should be `-`")));
        }
        ++inner_loc.iter();
        const auto d = lex_date_mday::invoke(inner_loc);
        if(!d)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_date: invalid day format",
                inner_loc, d.unwrap_err()));
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
        loc.iter() = first;
        return err(std::string("[error]: toml::parse_local_date: "
                    "the next token is not a local_date"));
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
                inner_loc, h.map_err_or_else([](const std::string& msg) {
                        return msg;
                    }, "should be `:`")));
        }
        ++inner_loc.iter();
        const auto m = lex_time_minute::invoke(inner_loc);
        if(!m || inner_loc.iter() == inner_loc.end() || *inner_loc.iter() != ':')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_time: invalid month format",
                inner_loc, m.map_err_or_else([](const std::string& msg) {
                        return msg;
                    }, "should be `:`")));
        }
        ++inner_loc.iter();
        const auto s = lex_time_second::invoke(inner_loc);
        if(!s)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_time: invalid second format",
                inner_loc, s.unwrap_err()));
        }
        local_time time(
            static_cast<std::int8_t>(from_string<int>(h.unwrap().str(), 0)),
            static_cast<std::int8_t>(from_string<int>(m.unwrap().str(), 0)),
            static_cast<std::int8_t>(from_string<int>(s.unwrap().str(), 0)), 0, 0);

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
                time.millisecond = from_string<std::int16_t>(sf.substr(0, 3), 0);
                time.microsecond = from_string<std::int16_t>(sf.substr(3, 3), 0);
            }
            else if(sf.size() >= 3)
            {
                time.millisecond = from_string<std::int16_t>(sf, 0);
                time.microsecond = 0;
            }
        }
        else
        {
            if(before_secfrac != inner_loc.iter())
            {
                throw internal_error(format_underline("[error]: "
                    "toml::parse_local_time: invalid subsecond format",
                    inner_loc, secfrac.unwrap_err()));
            }
        }
        return ok(std::make_pair(time, token.unwrap()));
    }
    else
    {
        loc.iter() = first;
        return err(std::string("[error]: toml::parse_local_time: "
                    "the next token is not a local_time"));
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
                inner_loc, date.map_err_or_else([](const std::string& msg){
                        return msg;
                    }, "date, not datetime")));
        }
        const char delim = *(inner_loc.iter()++);
        if(delim != 'T' && delim != 't' && delim != ' ')
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_datetime: invalid datetime format",
                inner_loc, "should be `T` or ` ` (space)"));
        }
        const auto time = parse_local_time(inner_loc);
        if(!time)
        {
            throw internal_error(format_underline("[error]: "
                "toml::parse_local_datetime: invalid datetime format",
                inner_loc, "invalid time fomrat"));
        }
        return ok(std::make_pair(
            local_datetime(date.unwrap().first, time.unwrap().first),
            token.unwrap()));
    }
    else
    {
        loc.iter() = first;
        return err(std::string("[error]: toml::parse_local_datetime: "
                    "the next token is not a local_datetime"));
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
                inner_loc, datetime.map_err_or_else([](const std::string& msg){
                        return msg;
                    }, "date, not datetime")));
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
                inner_loc, "should be `Z` or `+HH:MM`"));
        }
        return ok(std::make_pair(offset_datetime(datetime.unwrap().first, offset),
                                 token.unwrap()));
    }
    else
    {
        loc.iter() = first;
        return err(std::string("[error]: toml::parse_offset_datetime: "
                    "the next token is not a local_datetime"));
    }
}

template<typename Container>
result<key, std::string> parse_simple_key(location<Container>& loc)
{
    if(const auto bstr = parse_basic_string(loc))
    {
        return ok(bstr.unwrap().first.str);
    }
    if(const auto lstr = parse_literal_string(loc))
    {
        return ok(lstr.unwrap().first.str);
    }
    if(const auto bare = lex_unquoted_key::invoke(loc))
    {
        return ok(bare.unwrap().str());
    }
    return err(std::string("[error] toml::parse_simple_key: "
        "the next token is not a simple key"));
}

// dotted key become vector of keys
template<typename Container>
result<std::vector<key>, std::string> parse_key(location<Container>& loc)
{
    const auto first = loc.iter();
    // dotted key -> foo.bar.baz whitespaces are allowed
    if(const auto token = lex_dotted_key::invoke(loc))
    {
        location<std::string> inner_loc(loc.name(), token.unwrap().str());
        std::vector<key> keys;

        while(inner_loc.iter() != inner_loc.end())
        {
            lex_ws::invoke(inner_loc);
            if(const auto k = parse_simple_key(inner_loc))
            {
                keys.push_back(k.unwrap());
            }
            else
            {
                throw internal_error(format_underline("[error] "
                    "toml::detail::parse_key: dotted key contains invalid key",
                    inner_loc, k.unwrap_err()));
            }

            lex_ws::invoke(inner_loc);
            if(inner_loc.iter() == inner_loc.end())
            {
                break;
            }
            else if(*inner_loc.iter() == '.')
            {
                ++inner_loc.iter(); // to skip `.`
            }
            else
            {
                throw internal_error(format_underline("[error] toml::parse_key: "
                    "dotted key contains invalid key ", inner_loc,
                    "should be `.`"));
            }
        }
        return ok(keys);
    }
    loc.iter() = first;

    // simple key -> foo
    if(const auto smpl = parse_simple_key(loc))
    {
        return ok(std::vector<key>(1, smpl.unwrap()));
    }
    return err(std::string("[error] toml::parse_key: "
                           "the next token is not a key"));
}

// forward-decl to implement parse_array and parse_table
template<typename Container>
result<value, std::string> parse_value(location<Container>&);

template<typename Container>
result<std::pair<array, region<Container>>, std::string>
parse_array(location<Container>& loc)
{
    const auto first = loc.iter();
    if(loc.iter() == loc.end())
    {
        return err("[error] toml::parse_array: input is empty");
    }
    if(*loc.iter() != '[')
    {
        return err("[error] toml::parse_array: token is not an array");
    }
    ++loc.iter();

    using lex_ws_comment_newline = repeat<
        either<lex_wschar, lex_newline, lex_comment>, unlimited>;

    array retval;
    while(loc.iter() != loc.end())
    {
        lex_ws_comment_newline::invoke(loc); // skip

        if(loc.iter() != loc.end() && *loc.iter() == ']')
        {
            ++loc.iter(); // skip ']'
            return ok(std::make_pair(retval,
                      region<Container>(loc, first, loc.iter())));
        }

        if(auto val = parse_value(loc))
        {
            if(!retval.empty() && retval.front().type() != val.as_ok().type())
            {
                throw syntax_error(format_underline(
                    "[error] toml::parse_array: type of elements should be the "
                    "same each other.", region<Container>(loc, first, loc.iter()),
                    "inhomogeneous types"));
            }
            retval.push_back(std::move(val.unwrap()));
        }
        else
        {
            return err(val.unwrap_err());
        }

        using lex_array_separator = sequence<maybe<lex_ws>, character<','>>;
        const auto sp = lex_array_separator::invoke(loc);
        if(!sp)
        {
            lex_ws_comment_newline::invoke(loc);
            if(loc.iter() != loc.end() && *loc.iter() == ']')
            {
                ++loc.iter(); // skip ']'
                return ok(std::make_pair(retval,
                          region<Container>(loc, first, loc.iter())));
            }
            else
            {
                throw syntax_error(format_underline("[error] toml::parse_array:"
                    " missing array separator `,`", loc, "should be `,`"));
            }
        }
    }
    loc.iter() = first;
    throw syntax_error(format_underline("[error] toml::parse_array: "
            "array did not closed by `]`", loc, "should be closed"));
}

template<typename Container>
result<std::pair<std::vector<key>, value>, std::string>
parse_key_value_pair(location<Container>& loc)
{
    const auto first = loc.iter();
    auto key = parse_key(loc);
    if(!key)
    {
        std::string msg = std::move(key.unwrap_err());
        // if the next token is keyvalue-separator, it means that there are no
        // key. then we need to show error as "empty key is not allowed".
        if(const auto keyval_sep = lex_keyval_sep::invoke(loc))
        {
            loc.iter() = first;
            msg = format_underline("[error] toml::parse_key_value_pair: "
                "empty key is not allowed.", loc, "key expected before '='");
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
                "invalid format for key", loc, "invalid character in key", {
                "Did you forget '.' to separate dotted-key?",
                "Allowed characters for bare key are [0-9a-zA-Z_-]."});
        }
        else // if not, the error is lack of key-value separator.
        {
            msg = format_underline("[error] toml::parse_key_value_pair: "
                "missing key-value separator `=`", loc, "should be `=`");
        }
        loc.iter() = first;
        return err(std::move(msg));
    }

    const auto after_kvsp = loc.iter(); // err msg
    auto val = parse_value(loc);
    if(!val)
    {
        std::string msg;
        loc.iter() = after_kvsp;
        if(sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>::invoke(loc))
        {
            loc.iter() = after_kvsp;
            msg = format_underline("[error] toml::parse_key_value_pair: "
                    "missing value after key-value separator '='", loc,
                    "expected value, but got nothing");
        }
        else
        {
            msg = format_underline("[error] toml::parse_key_value_pair: "
                    "invalid value format", loc, val.unwrap_err());
        }
        loc.iter() = first;
        return err(msg);
    }
    return ok(std::make_pair(std::move(key.unwrap()), std::move(val.unwrap())));
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

template<typename InputIterator>
result<bool, std::string>
insert_nested_key(table& root, const toml::value& v,
                  InputIterator iter, const InputIterator last,
                  const bool is_array_of_table = false)
{
    static_assert(std::is_same<key,
        typename std::iterator_traits<InputIterator>::value_type>::value,"");

    const auto first = iter;
    assert(iter != last);

    table* tab = std::addressof(root);
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
                    if(tab->at(k).is(value_t::Table))
                    {
                        // show special err msg for conflicting table
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") cannot insert"
                            "ed"), get_region(tab->at(k)), "table already defined",
                            get_region(v), "this conflicts with the previous table"));
                    }
                    else if(!(tab->at(k).is(value_t::Array)))
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") collides with"
                            " existing value"), get_region(tab->at(k)),
                            concat_to_string("this ", tab->at(k).type(),
                            " value already exists"), get_region(v),
                            "while inserting this array-of-tables"));
                    }
                    array& a = tab->at(k).template cast<toml::value_t::Array>();
                    if(!(a.front().is(value_t::Table)))
                    {
                        throw syntax_error(format_underline(concat_to_string(
                            "[error] toml::insert_value: array of table (\"",
                            format_dotted_keys(first, last), "\") collides with"
                            " existing value"), get_region(tab->at(k)),
                            concat_to_string("this ", tab->at(k).type(),
                            " value already exists"), get_region(v),
                            "while inserting this array-of-tables"));
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
                            " existing array-of-tables"), get_region(tab->at(k)),
                            concat_to_string("this ", tab->at(k).type(),
                            " value has static size"), get_region(v),
                            "appending this to the statically sized array"));
                    }
                    a.push_back(v);
                    return ok(true);
                }
                else // if not, we need to create the array of table
                {
                    toml::value aot(v); // copy region info from table to array
                    // update content by an array with one element
                    detail::assign_keeping_region(aot,
                            ::toml::value(toml::array(1, v)));

                    tab->insert(std::make_pair(k, aot));
                    return ok(true);
                }
            }
            if(tab->count(k) == 1)
            {
                if(tab->at(k).is(value_t::Table) && v.is(value_t::Table))
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: table (\"",
                        format_dotted_keys(first, last), "\") already exists."),
                        get_region(tab->at(k)), "table already exists here",
                        get_region(v), "table defined twice"));
                }
                else if(v.is(value_t::Table)                          &&
                        tab->at(k).is(value_t::Array)                 &&
                        tab->at(k).cast<value_t::Array>().size() > 0  &&
                        tab->at(k).cast<value_t::Array>().front().is(value_t::Table))
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: array of tables (\"",
                        format_dotted_keys(first, last), "\") already exists."),
                        get_region(tab->at(k)), "array of tables defined here",
                        get_region(v), "table conflicts with the previous array"
                        " of table"));
                }
                else
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: value (\"",
                        format_dotted_keys(first, last), "\") already exists."),
                        get_region(tab->at(k)), "value already exists here",
                        get_region(v), "value inserted twice"));
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
                // the region of [x.y] is the same as [x.y.z].
                (*tab)[k] = v; // copy region_info_
                detail::assign_keeping_region((*tab)[k],
                        ::toml::value(::toml::table{}));
            }

            // type checking...
            if(tab->at(k).is(value_t::Table))
            {
                tab = std::addressof((*tab)[k].template cast<value_t::Table>());
            }
            else if(tab->at(k).is(value_t::Array)) // inserting to array-of-tables?
            {
                array& a = (*tab)[k].template cast<value_t::Array>();
                if(!a.back().is(value_t::Table))
                {
                    throw syntax_error(format_underline(concat_to_string(
                        "[error] toml::insert_value: target (",
                        format_dotted_keys(first, std::next(iter)),
                        ") is neither table nor an array of tables"),
                        get_region(a.back()), concat_to_string("actual type is ",
                        a.back().type()), get_region(v), "inserting this"));
                }
                tab = std::addressof(a.back().template cast<value_t::Table>());
            }
            else
            {
                throw syntax_error(format_underline(concat_to_string(
                    "[error] toml::insert_value: target (",
                    format_dotted_keys(first, std::next(iter)),
                    ") is neither table nor an array of tables"),
                    get_region(tab->at(k)), concat_to_string("actual type is ",
                    tab->at(k).type()), get_region(v), "inserting this"));
            }
        }
    }
    return err(std::string("toml::detail::insert_nested_key: never reach here"));
}

template<typename Container>
result<std::pair<table, region<Container>>, std::string>
parse_inline_table(location<Container>& loc)
{
    const auto first = loc.iter();
    table retval;
    if(!(loc.iter() != loc.end() && *loc.iter() == '{'))
    {
        return err(std::string("[error] toml::parse_inline_table: "
                               "the next token is not an inline table"));
    }
    ++loc.iter();
    // it starts from "{". it should be formatted as inline-table
    while(loc.iter() != loc.end())
    {
        maybe<lex_ws>::invoke(loc);
        if(loc.iter() != loc.end() && *loc.iter() == '}')
        {
            ++loc.iter(); // skip `}`
            return ok(std::make_pair(
                        retval, region<Container>(loc, first, loc.iter())));
        }

        const auto kv_r = parse_key_value_pair(loc);
        if(!kv_r)
        {
            return err(kv_r.unwrap_err());
        }
        const std::vector<key>& keys = kv_r.unwrap().first;
        const value&            val  = kv_r.unwrap().second;

        const auto inserted =
            insert_nested_key(retval, val, keys.begin(), keys.end());
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
                ++loc.iter(); // skip `}`
                return ok(std::make_pair(
                            retval, region<Container>(loc, first, loc.iter())));
            }
            else
            {
                throw syntax_error(format_underline("[error] "
                    "toml:::parse_inline_table: missing table separator `,` ",
                    loc, "should be `,`"));
            }
        }
    }
    loc.iter() = first;
    throw syntax_error(format_underline("[error] toml::parse_inline_table: "
            "inline table did not closed by `}`", loc, "should be closed"));
}

template<typename Container>
result<value, std::string> parse_value(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first == loc.end())
    {
        return err(std::string("[error] toml::parse_value: input is empty"));
    }
    if(auto r = parse_string         (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_array          (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_inline_table   (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_boolean        (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_offset_datetime(loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_local_datetime (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_local_date     (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_local_time     (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_floating       (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}
    if(auto r = parse_integer        (loc))
    {return ok(value(std::move(r.unwrap().first), std::move(r.unwrap().second)));}

    const auto msg = format_underline("[error] toml::parse_value: "
            "unknown token appeared", loc, "unknown");
    loc.iter() = first;
    return err(msg);
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
                "toml::parse_table_key: no `[`", inner_loc, "should be `[`"));
        }
        // to skip [ a . b . c ]
        //          ^----------- this whitespace
        lex_ws::invoke(inner_loc);
        const auto keys = parse_key(inner_loc);
        if(!keys)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: invalid key", inner_loc, "not key"));
        }
        // to skip [ a . b . c ]
        //                    ^-- this whitespace
        lex_ws::invoke(inner_loc);
        const auto close = lex_std_table_close::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: no `]`", inner_loc, "should be `]`"));
        }
        return ok(std::make_pair(keys.unwrap(), token.unwrap()));
    }
    else
    {
        return err(token.unwrap_err());
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
                "toml::parse_array_table_key: no `[[`", inner_loc,
                "should be `[[`"));
        }
        lex_ws::invoke(inner_loc);
        const auto keys = parse_key(inner_loc);
        if(!keys)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_array_table_key: invalid key", inner_loc,
                "not key"));
        }
        lex_ws::invoke(inner_loc);
        const auto close = lex_array_table_close::invoke(inner_loc);
        if(!close)
        {
            throw internal_error(format_underline("[error] "
                "toml::parse_table_key: no `]]`", inner_loc, "should be `]]`"));
        }
        return ok(std::make_pair(keys.unwrap(), token.unwrap()));
    }
    else
    {
        return err(token.unwrap_err());
    }
}

// parse table body (key-value pairs until the iter hits the next [tablekey])
template<typename Container>
result<table, std::string> parse_ml_table(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first == loc.end())
    {
        return err(std::string("toml::parse_ml_table: input is empty"));
    }

    // XXX at lest one newline is needed.
    using skip_line = repeat<
        sequence<maybe<lex_ws>, maybe<lex_comment>, lex_newline>, at_least<1>>;
    skip_line::invoke(loc);

    table tab;
    while(loc.iter() != loc.end())
    {
        lex_ws::invoke(loc);
        const auto before = loc.iter();
        if(const auto tmp = parse_array_table_key(loc)) // next table found
        {
            loc.iter() = before;
            return ok(tab);
        }
        if(const auto tmp = parse_table_key(loc)) // next table found
        {
            loc.iter() = before;
            return ok(tab);
        }

        if(const auto kv = parse_key_value_pair(loc))
        {
            const std::vector<key>& keys = kv.unwrap().first;
            const value&            val  = kv.unwrap().second;
            const auto inserted =
                insert_nested_key(tab, val, keys.begin(), keys.end());
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
            const auto before = loc.iter();
            lex_ws::invoke(loc); // skip whitespace
            const auto msg = format_underline("[error] toml::parse_table: "
                "invalid line format", loc, concat_to_string(
                "expected newline, but got '", show_char(*loc.iter()), "'."));
            loc.iter() = before;
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

template<typename Container>
result<table, std::string> parse_toml_file(location<Container>& loc)
{
    const auto first = loc.iter();
    if(first == loc.end())
    {
        return err(std::string("toml::detail::parse_toml_file: input is empty"));
    }

    table data;
    /* root object is also table, but without [tablename] */
    if(auto tab = parse_ml_table(loc))
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
            const auto tab = parse_ml_table(loc);
            if(!tab){return err(tab.unwrap_err());}

            const auto& keys = tabkey.unwrap().first;
            const auto& reg  = tabkey.unwrap().second;

            const auto inserted = insert_nested_key(data,
                    toml::value(tab.unwrap(), reg),
                    keys.begin(), keys.end(), /*is_array_of_table=*/ true);
            if(!inserted) {return err(inserted.unwrap_err());}

            continue;
        }
        if(const auto tabkey = parse_table_key(loc))
        {
            const auto tab = parse_ml_table(loc);
            if(!tab){return err(tab.unwrap_err());}

            const auto& keys = tabkey.unwrap().first;
            const auto& reg  = tabkey.unwrap().second;

            const auto inserted = insert_nested_key(data,
                    toml::value(tab.unwrap(), reg), keys.begin(), keys.end());
            if(!inserted) {return err(inserted.unwrap_err());}

            continue;
        }
        return err(format_underline("[error]: toml::parse_toml_file: "
                "unknown line appeared", loc, "unknown format"));
    }
    return ok(data);
}

} // detail

inline table parse(std::istream& is, std::string fname = "unknown file")
{
    const auto beg = is.tellg();
    is.seekg(0, std::ios::end);
    const auto end = is.tellg();
    const auto fsize = end - beg;
    is.seekg(beg);

    // read whole file as a sequence of char
    std::vector<char> letters(fsize);
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
            loc.iter() += 3; // BOM found. skip.
        }
    }

    const auto data = detail::parse_toml_file(loc);
    if(!data)
    {
        throw syntax_error(data.unwrap_err());
    }
    return data.unwrap();
}

inline table parse(const std::string& fname)
{
    std::ifstream ifs(fname.c_str(), std::ios_base::binary);
    if(!ifs.good())
    {
        throw std::runtime_error("toml::parse: file open error -> " + fname);
    }
    return parse(ifs, fname);
}

} // toml
#endif// TOML11_PARSER_HPP

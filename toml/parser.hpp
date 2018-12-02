#ifndef TOML11_PARSER
#define TOML11_PARSER
#include "value.hpp"
#include "acceptor.hpp"
#include "result.hpp"
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace toml
{

struct parse_escape_sequence
{
    typedef toml::character value_type;
    typedef toml::String    string_type;
    typedef result<string_type, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter, Iterator end)
    {
        const auto beg = iter;
        if(iter == end || *iter != '\\')
            return std::make_pair(err("not an escape sequence"), iter);
        ++iter;
        switch(*iter)
        {
            case '\\': return std::make_pair(ok(string_type("\\")), std::next(iter));
            case '"' : return std::make_pair(ok(string_type("\"")), std::next(iter));
            case 'b' : return std::make_pair(ok(string_type("\b")), std::next(iter));
            case 't' : return std::make_pair(ok(string_type("\t")), std::next(iter));
            case 'n' : return std::make_pair(ok(string_type("\n")), std::next(iter));
            case 'f' : return std::make_pair(ok(string_type("\f")), std::next(iter));
            case 'r' : return std::make_pair(ok(string_type("\r")), std::next(iter));
            case 'u' :
            {
                if(std::distance(iter, end) < 5)
                    throw std::make_pair(iter, syntax_error(
                        "invalid escape sequence: " + std::string(beg, end)));
                return std::make_pair(ok(
                    utf8_to_char(make_codepoint(string_type(iter+1, iter+5)))),
                    iter+5);
            }
            case 'U':
            {
                if(std::distance(iter, end) < 8)
                    throw std::make_pair(iter, syntax_error(
                        "invalid escape sequence: " + std::string(beg, end)));
                return std::make_pair(ok(
                    utf8_to_char(make_codepoint(string_type(iter+1, iter+9)))),
                    iter+9);
            }
            default: throw std::make_pair(iter, syntax_error(
                        "unkwnon escape sequence: " + std::string(iter, end)));
        }
    }

    static unsigned int make_codepoint(string_type str)
    {
        unsigned int codepoint;
        std::basic_istringstream<value_type> iss(str);
        iss >> std::hex >> codepoint;
        return codepoint;
    }

    static string_type utf8_to_char(const unsigned int codepoint)
    {
        string_type character;
        if(codepoint < 0x80)
        {
            character += static_cast<unsigned char>(codepoint);
        }
        else if(codepoint < 0x800)
        {
            character += static_cast<unsigned char>(0xC0| codepoint >> 6);
            character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        else if(codepoint < 0x10000)
        {
            character += static_cast<unsigned char>(0xE0| codepoint >>12);
            character += static_cast<unsigned char>(0x80|(codepoint >>6&0x3F));
            character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        else
        {
            character += static_cast<unsigned char>(0xF0| codepoint >>18);
            character += static_cast<unsigned char>(0x80|(codepoint >>12&0x3F));
            character += static_cast<unsigned char>(0x80|(codepoint >>6 &0x3F));
            character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        return character;
    }
};

struct parse_basic_inline_string
{
    typedef toml::character value_type;
    typedef result<toml::String, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_basic_inline_string<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        if(std::distance(iter, end) < 2)
            throw internal_error("is_basic_inline_string");

        toml::String result; result.reserve(std::distance(iter, end)-2);
        ++iter;
        const Iterator last = std::prev(end); // ignore '"'
        while(iter != last)
        {
            if(*iter == '\\')
            {
                auto r = parse_escape_sequence::invoke(iter, last);
                if(!r.first.is_ok())
                    throw internal_error("parse_basic_inline_string");
                result += r.first.unwrap();
                iter = r.second;
            }
            else
            {
                result += *iter;
                ++iter;
            }
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_basic_multiline_string
{
    typedef toml::character value_type;
    typedef toml::String    string_type;
    typedef result<string_type, std::string> result_type;

    typedef is_chain_of<is_character<value_type, '\\'>, is_newline<value_type>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<value_type>, is_newline<value_type>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_basic_multiline_string<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        if(std::distance(iter, end) < 6)
            throw internal_error("is_basic_inline_string");

        toml::String result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        const Iterator last = end - 3;
        iter = is_newline<value_type>::invoke(iter, last);
        while(iter != last)
        {
            if(*iter == '\\')
            {
                if(is_line_ending_backslash::invoke(iter, last) != iter)
                {
                    iter = ws_nl_after_backslash_remover::invoke(std::next(iter), last);
                }
                else
                {
                    auto r = parse_escape_sequence::invoke(iter, last);
                    if(!r.first.is_ok())
                        throw internal_error("parse_basic_inline_string");
                    result += r.first.unwrap();
                    iter = r.second;
                }
            }
            else
            {
                result.push_back(*iter);
                ++iter;
            }
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_literal_inline_string
{
    typedef toml::character value_type;
    typedef result<toml::String, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_literal_inline_string<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        if(std::distance(iter, end) < 2)
            throw internal_error("is_literal_inline_string");

        toml::String result; result.reserve(std::distance(iter, end)-2);
        ++iter;
        const Iterator last = end - 1;
        while(iter != last)
        {
            result.push_back(*iter);
            ++iter;
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_literal_multiline_string
{
    typedef toml::character value_type;
    typedef result<toml::String, std::string> result_type;

    typedef is_chain_of<is_character<value_type, '\\'>, is_newline<value_type>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<value_type>, is_newline<value_type>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_literal_multiline_string<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        if(std::distance(iter, end) < 6)
            throw internal_error("is_literal_multiline_string");

        toml::String result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        const Iterator last = end - 3;
        iter = is_newline<value_type>::invoke(iter, last); // trim first newline if exist
        while(iter != last)
        {
            result.push_back(*iter);
            ++iter;
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_string
{
    typedef toml::character value_type;
    typedef result<toml::String, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        std::pair<result_type, Iterator> result(err(""), Iterator());
        if((result = parse_basic_inline_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_basic_multiline_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_literal_inline_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_literal_multiline_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else
            return std::make_pair(err("does not match anything"), iter);
    }
};

struct parse_integer
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type> string_type;
    typedef result<toml::Integer, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_integer<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](value_type c){return c != '_';});
        return std::make_pair(ok(std::stoll(result)), end);
    }
};

struct parse_float
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type> string_type;
    typedef result<toml::Float, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_float<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](value_type c){return c != '_';});
        try{
            return std::make_pair(ok(std::stod(result)), end);
        }
        catch(std::out_of_range& oor)
        {
            std::cout << "extremely large Float value appeared: "
                      << result << "; it is negrected" << std::endl;
            return std::make_pair(ok(0.0), end);
        }
    }
};

struct parse_boolean
{
    typedef toml::character value_type;
    typedef result<toml::Boolean, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_boolean<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        return std::make_pair(ok(std::distance(iter, end) == 4), end);
    }
};

struct parse_local_time
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type>  string_type;
    typedef result<toml::Datetime, std::string> result_type;
    typedef typename toml::Datetime::number_type number_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::character>, N>;
    typedef is_character<toml::character, ':'> delim;
    typedef is_character<toml::character, '.'> fract;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_local_time<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        toml::Datetime result;
        result.hour   = std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
        iter = delim::invoke(nums<2>::invoke(iter, end), end);
        result.minute = std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
        iter = delim::invoke(nums<2>::invoke(iter, end), end);
        result.second = std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
        iter = fract::invoke(nums<2>::invoke(iter, end), end);
        if(iter == end)
        {
            result.millisecond = 0.0;
            result.microsecond = 0.0;
        }
        else if(std::distance(iter, end) <= 3)
        {
            result.millisecond = parse_number(iter, end);
            result.microsecond = 0.0;
        }
        else
        {
            result.millisecond = parse_number(iter, iter + 3);
            result.microsecond = parse_number(iter + 3, end);
        }
        result.offset_hour   = toml::Datetime::nooffset;
        result.offset_minute = toml::Datetime::nooffset;
        result.year  = toml::Datetime::undef;
        result.month = toml::Datetime::undef;
        result.day   = toml::Datetime::undef;
        return std::make_pair(ok(result), end);
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static number_type parse_number(Iterator iter, Iterator end)
    {
        if(std::distance(iter, end) > 3) end = iter + 3;
        string_type str(iter, end);
        while(str.size() < 3){str += '0';}
        return std::stoi(str);
    }
};

struct parse_local_date
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type> string_type;
    typedef result<toml::Datetime, std::string> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<value_type>, N>;
    typedef is_character<value_type, '-'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_local_date<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        toml::Datetime result;
        result.year   = std::stoi(string_type(iter, nums<4>::invoke(iter, end)));
        iter = delim::invoke(nums<4>::invoke(iter, end), end);
        result.month  = std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
        iter = delim::invoke(nums<2>::invoke(iter, end), end);
        result.day    = std::stoi(string_type(iter, nums<2>::invoke(iter, end)));

        result.offset_hour   = toml::Datetime::nooffset;
        result.offset_minute = toml::Datetime::nooffset;
        result.hour   = toml::Datetime::undef;
        result.minute = toml::Datetime::undef;
        result.second = toml::Datetime::undef;
        result.millisecond = toml::Datetime::undef;
        result.microsecond = toml::Datetime::undef;
        return std::make_pair(ok(result), end);
    }
};

struct parse_local_date_time
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type> string_type;
    typedef result<toml::Datetime, std::string> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::character>, N>;
    typedef is_character<toml::character, 'T'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_local_date_time<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        auto ld = parse_local_date::invoke(iter, end);
        if(!ld.first.is_ok())
            throw std::make_pair(iter, syntax_error("invalid local datetime"));
        toml::Datetime result(ld.first.unwrap());
        iter = delim::invoke(ld.second, end);// 'T'

        const auto time = parse_local_time::invoke(iter, end);
        result.hour          = time.first.unwrap().hour;
        result.minute        = time.first.unwrap().minute;
        result.second        = time.first.unwrap().second;
        result.millisecond   = time.first.unwrap().millisecond;
        result.microsecond   = time.first.unwrap().microsecond;
        result.offset_hour   = toml::Datetime::nooffset;
        result.offset_minute = toml::Datetime::nooffset;
        return std::make_pair(ok(result), end);
    }
};

struct parse_offset_date_time
{
    typedef toml::character value_type;
    typedef std::basic_string<value_type> string_type;
    typedef result<toml::Datetime, std::string> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::character>, N>;
    typedef is_character<toml::character, ':'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_offset_date_time<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        auto ldt = parse_local_date_time::invoke(iter, end);
        if(!ldt.first.is_ok())
            throw std::make_pair(iter, syntax_error("invalid offset datetime"));
        toml::Datetime result(ldt.first.unwrap());
        iter = ldt.second;
        if(*iter == 'Z')
        {
            result.offset_hour   = 0;
            result.offset_minute = 0;
        }
        else
        {
            if(*iter != '+' && *iter != '-')
                throw std::make_pair(iter, syntax_error("invalid offset-datetime"));
            const int sign = (*iter == '-') ? -1 : 1;
            ++iter;
            result.offset_hour   = sign *
                std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
            iter = delim::invoke(nums<2>::invoke(iter, end), end);
            result.offset_minute = sign *
                std::stoi(string_type(iter, nums<2>::invoke(iter, end)));
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_datetime
{
    typedef toml::character value_type;
    typedef result<toml::Datetime, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        std::pair<result_type, Iterator> result(err(""), Iterator());
        if((result = parse_offset_date_time::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_local_date_time::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_local_date::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_local_time::invoke(iter, range_end)).first.is_ok())
            return result;
        else
            return std::make_pair(err("does not match anything"), iter);
    }
};

template<typename acceptorT, typename parserT>
struct parse_fixed_type_array
{
    typedef toml::character value_type;
    typedef result<toml::Array, std::string> result_type;
    typedef acceptorT acceptor_type;
    typedef parserT   parser_type;
    typedef is_skippable_in_array<value_type> skippable;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_fixed_type_array<value_type, acceptorT>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        toml::Array result;
        const Iterator last = std::prev(end);
        iter = skippable::invoke(std::next(iter), last);
        while(iter != last)
        {
            const Iterator tmp = acceptor_type::invoke(iter, last);
            if(tmp == iter)
                throw std::make_pair(iter, syntax_error("parse_array"));
            auto next = parser_type::invoke(iter, last);
            if(!next.first.is_ok())
                throw std::make_pair(iter, syntax_error("parse_array"));
            result.emplace_back(next.first.unwrap());
            iter = tmp;
            iter = skippable::invoke(iter, last);
            iter = is_character<value_type, ','>::invoke(iter, last);
            iter = skippable::invoke(iter, last);
        }
        return std::make_pair(ok(result), end);
    }
};

template<typename charT>
struct parse_inline_table;

template<typename charT>
struct parse_array
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::character>::value, "");
    typedef result<toml::Array, std::string> result_type;
    typedef is_skippable_in_array<value_type> skippable;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        if(iter == is_array<value_type>::invoke(iter, range_end))
            return std::make_pair(err("input is not an array"), iter);

        std::pair<result_type, Iterator> result(err(""), Iterator());
        if((result = parse_fixed_type_array<is_boolean<value_type>,
                     parse_boolean>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_string<value_type>,
                parse_string>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_datetime<value_type>,
                parse_datetime>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_float<value_type>,
                parse_float>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_integer<value_type>,
                parse_integer>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_array<value_type>,
                parse_array<value_type>>::invoke(iter, range_end)).first.is_ok()) return result;
        else if((result = parse_fixed_type_array<is_inline_table<value_type>,
                parse_inline_table<value_type>>::invoke(iter, range_end)).first.is_ok())
            return result;
        else if(skippable::invoke(std::next(iter), range_end) == // empty
                std::prev(is_array<value_type>::invoke(iter, range_end))
                ) return std::make_pair(
                    ok(toml::Array{}), is_array<value_type>::invoke(iter, range_end));
        else throw std::make_pair(iter, syntax_error("no valid array here"));
    }
};

template<typename charT>
struct parse_value
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::character>::value, "");
    typedef result<toml::value, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        std::pair<result_type, Iterator> result(err(""), Iterator());
        if((result = parse_boolean::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_datetime::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_float::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_integer::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_array<value_type>::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_inline_table<value_type>::invoke(iter, range_end)).first.is_ok())
            return result;
        else
            return std::make_pair(err("does not match any value type"), iter);
    }
};

struct parse_barekey
{
    typedef toml::character value_type;
    typedef result<toml::key, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_barekey<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);
        return std::make_pair(ok(toml::key(iter, end)), end);
    }
};

struct parse_key
{
    typedef toml::character value_type;
    typedef result<toml::key, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        std::pair<result_type, Iterator> result(err(""), Iterator());
        if((result = parse_barekey::invoke(iter, range_end)).first.is_ok())
            return result;
        else if((result = parse_string::invoke(iter, range_end)).first.is_ok())
            return result;
        else return std::make_pair(err("does not match anything"), iter);
    }
};

template<typename charT>
struct parse_key_value_pair
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::character>::value, "");
    typedef result<std::pair<toml::key, toml::value>, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        auto tmp_key = parse_key::invoke(iter, range_end);
        if(!tmp_key.first.is_ok())
            return std::make_pair(err(tmp_key.first.unwrap_err()), iter);
        iter = is_any_num_of_ws<charT>::invoke(tmp_key.second, range_end);
        if(*iter != '=')
            throw std::make_pair(iter, syntax_error("invalid key value pair"));
        iter = is_any_num_of_ws<charT>::invoke(std::next(iter), range_end);

        auto tmp_value = parse_value<toml::character>::invoke(iter, range_end);
        if(!tmp_value.first.is_ok())
            throw std::make_pair(iter, syntax_error("invalid key value pair"));

        iter = tmp_value.second;

        return std::make_pair(ok(std::make_pair(
                    tmp_key.first.unwrap(), tmp_value.first.unwrap())),
                is_any_num_of_ws<charT>::invoke(tmp_value.second, range_end));
    }
};

template<typename charT>
struct parse_inline_table
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::character>::value, "");
    typedef result<toml::Table, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end = is_inline_table<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        iter = is_any_num_of_ws<value_type>::invoke(std::next(iter), range_end);

        const Iterator last = std::prev(end);
        toml::Table result;
        while(iter != last)
        {
            auto tmp = parse_key_value_pair<value_type>::invoke(iter, last);
            if(!tmp.first.is_ok())
                throw std::make_pair(iter, syntax_error("parse_inline_table"));

            result.emplace(tmp.first.unwrap());
            iter = tmp.second;

            iter = is_any_num_of_ws<value_type>::invoke(iter, last);
            iter = is_character<value_type, ','>::invoke(iter, last);
            iter = is_any_num_of_ws<value_type>::invoke(iter, last);
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_table_definition
{
    typedef toml::character value_type;
    typedef result<std::vector<toml::key>, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_table_definition<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        std::vector<toml::key> result;
        result.reserve(std::count(iter, end, '.')+1);

        const Iterator last = std::prev(end);
        iter = is_any_num_of_ws<value_type>::invoke(iter, last);
        iter = is_any_num_of_ws<value_type>::invoke(std::next(iter), last);

        auto tmp = parse_key::invoke(iter, last);
        if(!tmp.first.is_ok())
            throw std::make_pair(iter, syntax_error("table definition"));
        result.emplace_back(tmp.first.unwrap());
        iter = is_any_num_of_ws<value_type>::invoke(tmp.second, last);

        while(iter != last)
        {
            iter = is_character<value_type, '.'>::invoke(iter, last);
            iter = is_any_num_of_ws<value_type>::invoke(iter, last);

            tmp = parse_key::invoke(iter, last);
            if(!tmp.first.is_ok())
                throw std::make_pair(iter, syntax_error("table definition"));
            result.emplace_back(tmp.first.unwrap());
            iter = is_any_num_of_ws<value_type>::invoke(tmp.second, last);
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_array_of_table_definition
{
    typedef toml::character value_type;
    typedef result<std::vector<toml::key>, std::string> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator>
    invoke(Iterator iter, Iterator range_end)
    {
        const Iterator end =
            is_array_of_table_definition<value_type>::invoke(iter, range_end);
        if(iter == end) return std::make_pair(err("input is empty"), iter);

        if(std::distance(iter, end) < 5)
            throw std::make_pair(iter, syntax_error("invalid array_of_table definition"));

        std::vector<toml::key> result;
        result.reserve(std::count(iter, end, '.')+1);

        const Iterator last = end - 2;
        iter = is_any_num_of_ws<value_type>::invoke(iter, last) + 2;
        iter = is_any_num_of_ws<value_type>::invoke(iter, last);

        auto tmp = parse_key::invoke(iter, last);
        if(!tmp.first.is_ok())
            throw std::make_pair(iter, syntax_error("array of table definition"));
        result.emplace_back(tmp.first.unwrap());
        iter = is_any_num_of_ws<value_type>::invoke(tmp.second, last);

        while(iter != last)
        {
            iter = is_character<value_type, '.'>::invoke(iter, last);
            iter = is_any_num_of_ws<value_type>::invoke(iter, last);

            tmp  = parse_key::invoke(iter, last);
            if(!tmp.first.is_ok())
                throw std::make_pair(iter, syntax_error("array of table definition"));
            result.emplace_back(tmp.first.unwrap());
            iter = is_any_num_of_ws<value_type>::invoke(tmp.second, last);
        }
        return std::make_pair(ok(result), end);
    }
};

struct parse_data
{
    typedef toml::character value_type;
    typedef toml::Table result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, const Iterator end)
    {
        toml::Table result;
        auto noname = parse_table_contents(iter, end);
        result = std::move(noname.first);
        iter = skip_empty(noname.second, end);

        while(iter != end)
        {
            iter = skip_empty(iter, end);
            std::pair<::toml::result<std::vector<toml::key>, std::string>, Iterator>
                tabname(err(""), Iterator());
            if((tabname = parse_table_definition::invoke(iter, end)).first.is_ok())
            {
                auto contents = parse_table_contents(tabname.second, end);
                push_table(result, std::move(contents.first),
                           tabname.first.unwrap().begin(), tabname.first.unwrap().end());
                iter = contents.second;
            }
            else if((tabname = parse_array_of_table_definition::invoke(iter, end)).first.is_ok())
            {
                auto contents = parse_table_contents(tabname.second, end);
                push_array_of_table(result, std::move(contents.first),
                           tabname.first.unwrap().begin(), tabname.first.unwrap().end());
                iter = contents.second;
            }
            else
                throw std::make_pair(iter, syntax_error("parse_data: unknown line"));
        }
        return result;
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator
    skip_empty(Iterator iter, Iterator end)
    {
        while(iter != end)
        {
            if(*iter == '#')
            {
                while(iter != end &&
                      iter == is_newline<value_type>::invoke(iter, end)){++iter;}
            }
            else if(iter == is_newline<value_type>::invoke(iter, end) &&
                    iter == is_whitespace<value_type>::invoke(iter, end))
            {
                return iter;
            }
            else
            {
                ++iter;
            }
        }
        return iter;
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<toml::Table, Iterator>
    parse_table_contents(Iterator iter, Iterator end)
    {
        toml::Table table;
        iter = skip_empty(iter, end);
        while(iter != end)
        {
            auto kv = parse_key_value_pair<value_type>::invoke(iter, end);
            if(!kv.first.is_ok()) return std::make_pair(table, iter);

            table.emplace(kv.first.unwrap());
            iter = kv.second;
            iter = skip_empty(iter, end);
        }
        return std::make_pair(table, iter);
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     toml::key>::value>::type>
    static void
    push_table(toml::Table& data, toml::Table&& v, Iterator iter, Iterator end)
    {
        if(iter == std::prev(end))
        {
            if(data.count(*iter) == 1)
                throw syntax_error("duplicate key: " + *iter);
            data.emplace(*iter, std::move(v));
            return;
        }

        if(data.count(*iter) == 0)
        {
            data.emplace(*iter, toml::Table());
            return push_table(data[*iter].template cast<value_t::Table>(),
                              std::move(v), std::next(iter), end);
        }
        else if(data[*iter].type() == value_t::Table)
        {
            return push_table(data[*iter].template cast<value_t::Table>(),
                              std::move(v), std::next(iter), end);
        }
        else if(data[*iter].type() == value_t::Array)
        {
            auto& ar = data[*iter].template cast<value_t::Array>();
            if(ar.empty()) ar.emplace_back(toml::Table{});
            if(ar.back().type() != value_t::Table)
                throw syntax_error("assign table into array having non-table type: " + *iter);
            return push_table(ar.back().template cast<value_t::Table>(),
                              std::move(v), std::next(iter), end);
        }
        else
            throw syntax_error("assign table into not table: " + *iter);
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     toml::key>::value>::type>
    static void
    push_array_of_table(toml::Table& data, toml::Table&& v,
                        Iterator iter, Iterator end)
    {
        //XXX Iterator::value_type == toml::key
        if(iter == std::prev(end))
        {
            if(data.count(*iter) == 0)
                data.emplace(*iter, toml::Array());
            else if(data.at(*iter).type() != value_t::Array)
                throw syntax_error("duplicate key: " + *iter);

            data[*iter].template cast<value_t::Array>().emplace_back(std::move(v));
            return;
        }

        if(data.count(*iter) == 0)
        {
            data.emplace(*iter, toml::Table());
            return push_array_of_table(data[*iter].template cast<value_t::Table>(),
                                       std::move(v), std::next(iter), end);
        }
        else if(data[*iter].type() == value_t::Table)
        {
            return push_array_of_table(data[*iter].template cast<value_t::Table>(),
                              std::move(v), std::next(iter), end);
        }
        else if(data[*iter].type() == value_t::Array)
        {
            auto& ar = data[*iter].template cast<value_t::Array>();
            if(ar.empty()) ar.emplace_back(toml::Table{});
            if(ar.back().type() != value_t::Table)
                throw syntax_error("assign table into array having non-table type: " + *iter);
            return push_array_of_table(ar.back().template cast<value_t::Table>(),
                              std::move(v), std::next(iter), end);
        }
        else
            throw syntax_error("assign array of table into not table: " + *iter);
    }

};

template<typename traits = std::char_traits<toml::character>>
toml::Table parse(std::basic_istream<toml::character, traits>& is)
{
    const auto initial = is.tellg();
    is.seekg(0, std::ios::end);
    const auto eofpos  = is.tellg();
    const std::size_t size = eofpos - initial;
    is.seekg(initial);
    std::vector<toml::character> contents(size);
    typedef std::vector<toml::character>::const_iterator iterator_type;
    is.read(contents.data(), size);
    try
    {
        return parse_data::invoke(contents.cbegin(), contents.cend());
    }
    catch(std::pair<iterator_type, syntax_error> iter_except)
    {
        std::cerr << "toml syntax error." << std::endl;
        auto iter = iter_except.first;
        const std::size_t nline = 1 + std::count(contents.cbegin(), iter, '\n');
        std::cerr << "processing at line " << nline << std::endl;
        while(*iter != '\n' && iter != contents.cbegin()){--iter;}
        ++iter;
        while(*iter != '\n' && iter != contents.cend())
        {
            std::cerr << *iter; ++iter;
        }
        std::cerr << std::endl;

        throw iter_except.second;
    }
}

inline toml::Table parse(const char* filename)
{
    std::ifstream ifs(filename, std::ios_base::in | std::ios_base::binary);
    if(!ifs.good())
    {
        throw std::runtime_error("file open error: " + std::string(filename));
    }
    return parse(ifs);
}

template<typename charT, typename traits = std::char_traits<charT>>
inline toml::Table parse(const std::basic_string<charT, traits>& filename)
{
    std::ifstream ifs(filename, std::ios_base::in | std::ios_base::binary);
    if(!ifs.good())
    {
        throw std::runtime_error("file open error: " + filename);
    }
    return parse(ifs);
}

}// toml
#endif// TOML11_PARSER

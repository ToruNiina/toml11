#ifndef TOML11_PARSER
#define TOML11_PARSER
#include "value.hpp"
#include "acceptor.hpp"
#include <algorithm>

namespace toml
{

template<typename charT>
struct parse_barekey
{
    typedef charT value_type;
    typedef toml::key result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default key type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        return result_type(iter, end);
    }
};

template<typename charT>
struct parse_escape_sequence
{
    typedef charT value_type;
    typedef toml::String string_type;
    typedef string_type result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    //XXX this changes iterator position!
    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator& iter)
    {
        assert(*iter == '\\');
        ++iter;
        switch(*iter)
        {
            case '\\': ++iter; return "\\";
            case '"' : ++iter; return "\"";
            case 'b' : ++iter; return "\b";
            case 't' : ++iter; return "\t";
            case 'n' : ++iter; return "\n";
            case 'f' : ++iter; return "\f";
            case 'r' : ++iter; return "\r";
            case 'u' :
            {
                string_type tmp(iter+1, iter+5);
                std::advance(iter, 5);
                return utf8_to_char(make_codepoint(tmp));
            }
            case 'U':
            {
                string_type tmp(iter+1, iter+9);
                std::advance(iter, 9);
                return utf8_to_char(make_codepoint(tmp));
            }
            default: throw syntax_error("unkwnon escape sequence");
        }
    }

    static unsigned int make_codepoint(string_type str)
    {
        unsigned int codepoint;
        std::basic_istringstream<charT> iss(str);
        iss >> std::hex >> codepoint;
        return codepoint;
    }

    static result_type utf8_to_char(const unsigned int codepoint)
    {
        result_type charactor;
        if(codepoint < 0x80)
        {
            charactor += static_cast<unsigned char>(codepoint);
        }
        else if(codepoint < 0x800)
        {
            charactor += static_cast<unsigned char>(0xC0| codepoint >> 6);
            charactor += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        else if(codepoint < 0x10000)
        {
            charactor += static_cast<unsigned char>(0xE0| codepoint >>12);
            charactor += static_cast<unsigned char>(0x80|(codepoint >>6&0x3F));
            charactor += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        else
        {
            charactor += static_cast<unsigned char>(0xF0| codepoint >>18);
            charactor += static_cast<unsigned char>(0x80|(codepoint >>12&0x3F));
            charactor += static_cast<unsigned char>(0x80|(codepoint >>6 &0x3F));
            charactor += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
        }
        return charactor;
    }
};

template<typename charT>
struct parse_basic_inline_string
{
    typedef charT value_type;
    typedef toml::String result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        if(std::distance(iter, end) < 2)
            throw internal_error("no basic inline string here");
        result_type result; result.reserve(std::distance(iter, end)-2);
        ++iter; --end; // ignore '"'
        while(iter != end)
        {
            if(*iter == '\\')
            {
                result += parse_escape_sequence<charT>::invoke(iter);
            }
            else
            {
                result.push_back(*iter);
                ++iter;
            }
        }
        return result;
    }
};

template<typename charT>
struct parse_basic_multiline_string
{
    typedef charT value_type;
    typedef toml::String result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    typedef is_chain_of<is_charactor<charT, '\\'>, is_newline<charT>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<charT>, is_newline<charT>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        if(std::distance(iter, end) < 6)
            throw internal_error("no basic multiline string here");
        result_type result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        std::advance(end, -3);
        iter = is_newline<charT>::invoke(iter); // the first newline will be trimmed
        while(iter != end)
        {
            if(*iter == '\\')
            {
                if(is_line_ending_backslash::invoke(iter) != iter)
                {
                    iter = ws_nl_after_backslash_remover::invoke(std::next(iter));
                }
                else
                {
                    result += parse_escape_sequence<charT>::invoke(iter);
                }
            }
            else
            {
                result.push_back(*iter);
                ++iter;
            }
        }
        return result;
    }
};

template<typename charT>
struct parse_literal_inline_string
{
    typedef charT value_type;
    typedef toml::String result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        if(std::distance(iter, end) < 2)
            throw internal_error("no literal multiline string here");
        result_type result; result.reserve(std::distance(iter, end)-2);
        std::advance(iter, 1);
        std::advance(end, -1);
        while(iter != end)
        {
            result.push_back(*iter);
            ++iter;
        }
        return result;
    }
};

template<typename charT>
struct parse_literal_multiline_string
{
    typedef charT value_type;
    typedef toml::String result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    typedef is_chain_of<is_charactor<charT, '\\'>, is_newline<charT>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<charT>, is_newline<charT>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        if(std::distance(iter, end) < 6)
            throw internal_error("no literal multiline string here");
        result_type result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        std::advance(end, -3);
        iter = is_newline<charT>::invoke(iter); // the first newline will be trimmed
        while(iter != end)
        {
            result.push_back(*iter);
            ++iter;
        }
        return result;
    }
};

template<typename charT>
struct parse_string
{
    typedef charT value_type;
    typedef toml::String result_type;
    static_assert(std::is_same<value_type, result_type::value_type
            >::value, "char type is different from default String type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        Iterator tmp = is_basic_inline_string<charT>::invoke(iter);
        if(iter != tmp)
            return parse_basic_inline_string<charT>::invoke(iter, tmp);
        tmp = is_basic_multiline_string<charT>::invoke(iter);
        if(iter != tmp)
            return parse_basic_multiline_string<charT>::invoke(iter, tmp);
        tmp = is_literal_inline_string<charT>::invoke(iter);
        if(iter != tmp)
            return parse_literal_inline_string<charT>::invoke(iter, tmp);
        tmp = is_literal_multiline_string<charT>::invoke(iter);
        if(iter != tmp)
            return parse_literal_multiline_string<charT>::invoke(iter, tmp);
        throw internal_error("no string here");
    }
};


template<typename charT>
struct parse_integer
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Integer result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](charT c){return c != '_';});
        return std::stoi(result);
    }
};

template<typename charT>
struct parse_float
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Float result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](charT c){return c != '_';});
        return std::stod(result);
    }
};

template<typename charT>
struct parse_boolean
{
    typedef charT value_type;
    typedef toml::Boolean result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        return (std::distance(iter, end) == 4);
    }
};

template<typename charT>
struct parse_local_time
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Datetime result_type;
    typedef typename result_type::number_type number_type;
    typedef is_repeat_of<is_number<charT>, 2> nums;
    typedef is_charactor<charT, ':'> delim;
    typedef is_charactor<charT, '.'> fract;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        result_type result;
        result.hour   = std::stoi(string_type(iter, nums::invoke(iter)));
        iter = delim::invoke(nums::invoke(iter));
        result.minute = std::stoi(string_type(iter, nums::invoke(iter)));
        iter = delim::invoke(nums::invoke(iter));
        result.second = std::stoi(string_type(iter, nums::invoke(iter)));
        iter = fract::invoke(nums::invoke(iter));
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
        result.offset_hour   = result_type::nooffset;
        result.offset_minute = result_type::nooffset;
        result.year  = result_type::undef;
        result.month = result_type::undef;
        result.day   = result_type::undef;
        return result;
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

template<typename charT>
struct parse_local_date
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Datetime result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<charT>, N>;
    typedef is_charactor<charT, '-'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        result_type result;
        result.year   = std::stoi(string_type(iter, nums<4>::invoke(iter)));
        iter = delim::invoke(nums<4>::invoke(iter));
        result.month  = std::stoi(string_type(iter, nums<2>::invoke(iter)));
        iter = delim::invoke(nums<2>::invoke(iter));
        result.day    = std::stoi(string_type(iter, nums<2>::invoke(iter)));

        result.offset_hour   = result_type::nooffset;
        result.offset_minute = result_type::nooffset;
        result.hour   = result_type::undef;
        result.minute = result_type::undef;
        result.second = result_type::undef;
        result.millisecond = result_type::undef;
        result.microsecond = result_type::undef;
        return result;
    }
};

template<typename charT>
struct parse_local_date_time
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Datetime result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<charT>, N>;
    typedef is_charactor<charT, 'T'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        const Iterator date = is_local_date<charT>::invoke(iter);
        result_type result = parse_local_date<charT>::invoke(iter, date);
        iter = delim::invoke(date);// 'T'
        const result_type time = parse_local_time<charT>::invoke(iter, end);
        result.hour          = time.hour;
        result.minute        = time.minute;
        result.second        = time.second;
        result.millisecond   = time.millisecond;
        result.microsecond   = time.microsecond;
        result.offset_hour   = result_type::nooffset;
        result.offset_minute = result_type::nooffset;
        return result;
    }
};

template<typename charT>
struct parse_offset_date_time
{
    typedef charT value_type;
    typedef std::basic_string<value_type> string_type;
    typedef toml::Datetime result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<charT>, N>;
    typedef is_charactor<charT, ':'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        const Iterator datetime = is_local_date_time<charT>::invoke(iter);
        result_type result = parse_local_date_time<charT>::invoke(iter, datetime);
        iter = datetime;
        if(*iter == 'Z')
        {
            result.offset_hour   = 0;
            result.offset_minute = 0;
        }
        else
        {
            if(*iter != '+' && *iter != '-')
                throw syntax_error("invalid offset-datetime");
            const int sign = (*iter == '-') ? -1 : 1;
            ++iter;
            result.offset_hour   = sign *
                std::stoi(string_type(iter, nums<2>::invoke(iter)));
            iter = delim::invoke(nums<2>::invoke(iter));
            result.offset_minute = sign *
                std::stoi(string_type(iter, nums<2>::invoke(iter)));
        }
        return result;
    }
};

template<typename charT>
struct parse_datetime
{
    typedef charT value_type;
    typedef toml::Datetime result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static result_type invoke(Iterator iter, Iterator end)
    {
        Iterator tmp = is_offset_date_time<value_type>::invoke(iter);
        if(tmp != iter) return parse_offset_date_time<value_type>::invoke(iter);
        tmp = is_local_date_time<value_type>::invoke(iter);
        if(tmp != iter) return parse_local_date_time<value_type>::invoke(iter);
        tmp = is_local_date<value_type>::invoke(iter);
        if(tmp != iter) return parse_local_date<value_type>::invoke(iter);
        tmp = is_local_time<value_type>::invoke(iter);
        if(tmp != iter) return parse_local_time<value_type>::invoke(iter);
        throw internal_error("no datetime here");
    }
};


}// toml
#endif// TOML11_PARSER

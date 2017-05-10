#ifndef TOML11_PARSER
#define TOML11_PARSER
#include "value.hpp"
#include "acceptor.hpp"
#include <algorithm>
#include <iostream>

namespace toml
{

namespace detail
{

// it is just an inferior vertion of boost/std::optional
template<typename T>
struct result
{
    result() : ok_(false){}
    ~result() = default;
    result(const result& rhs) = default;
    result(result&& rhs)      = default;
    result& operator=(const result& rhs) = default;
    result& operator=(result&& rhs)      = default;

    result(const T& v) : ok_(true), value_(v){}
    result(T&& v) : ok_(true), value_(std::move(v)){}
    result& operator=(const T& rhs){ok_ = true; value_ = rhs; return *this;}
    result& operator=(T&& rhs)     {ok_ = true; value_ = rhs; return *this;}

    bool ok() const {return ok_;}
    operator bool() const {return ok_;}

    T&       get()       {if(!ok_) throw std::logic_error("result"); return value_;}
    T const& get() const {if(!ok_) throw std::logic_error("result"); return value_;}
    T&&      move()
    {if(!ok_) throw std::logic_error("result"); ok_ = false; return std::move(value_);}

  private:
    bool ok_;
    T value_;
};


}//detail

struct parse_escape_sequence
{
    typedef toml::charactor value_type;
    typedef toml::String    string_type;
    typedef detail::result<string_type> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        if(*iter != '\\') return std::make_pair(result_type{}, iter);
        ++iter;
        switch(*iter)
        {
            case '\\': return std::make_pair(string_type("\\"), std::next(iter));
            case '"' : return std::make_pair(string_type("\""), std::next(iter));
            case 'b' : return std::make_pair(string_type("\b"), std::next(iter));
            case 't' : return std::make_pair(string_type("\t"), std::next(iter));
            case 'n' : return std::make_pair(string_type("\n"), std::next(iter));
            case 'f' : return std::make_pair(string_type("\f"), std::next(iter));
            case 'r' : return std::make_pair(string_type("\r"), std::next(iter));
            case 'u' :
                return std::make_pair(utf8_to_char(make_codepoint(
                                      string_type(iter+1, iter+5))), iter+5);
            case 'U':
                return std::make_pair(utf8_to_char(make_codepoint(
                                      string_type(iter+1, iter+9))), iter+9);
            default: throw syntax_error("unkwnon escape sequence");
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
        string_type charactor;
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

struct parse_basic_inline_string
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::String> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_basic_inline_string<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
        if(std::distance(iter, end) < 2)
            throw internal_error("is_basic_inline_string");

        toml::String result; result.reserve(std::distance(iter, end)-2);
        ++iter;
        const Iterator last = std::prev(end); // ignore '"'
        while(iter != last)
        {
            if(*iter == '\\')
            {
                auto r = parse_escape_sequence::invoke(iter);
                if(!r.first.ok()) throw internal_error("parse_basic_inline_string");
                result += r.first.move();
                iter = r.second;
            }
            else
            {
                result += *iter;
                ++iter;
            }
        }
        return std::make_pair(result, end);
    }
};

struct parse_basic_multiline_string
{
    typedef toml::charactor value_type;
    typedef toml::String    string_type;
    typedef detail::result<string_type> result_type;

    typedef is_chain_of<is_charactor<value_type, '\\'>, is_newline<value_type>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<value_type>, is_newline<value_type>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_basic_multiline_string<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
        if(std::distance(iter, end) < 6)
            throw internal_error("is_basic_inline_string");

        toml::String result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        const Iterator last = end - 3;
        iter = is_newline<value_type>::invoke(iter); // trim first newline if exists
        while(iter != last)
        {
            if(*iter == '\\')
            {
                if(is_line_ending_backslash::invoke(iter) != iter)
                {
                    iter = ws_nl_after_backslash_remover::invoke(std::next(iter));
                }
                else
                {
                    auto r = parse_escape_sequence::invoke(iter);
                    if(!r.first.ok()) throw internal_error("parse_basic_inline_string");
                    result += r.first.move();
                    iter = r.second;
                }
            }
            else
            {
                result.push_back(*iter);
                ++iter;
            }
        }
        return std::make_pair(result, end);
    }
};

struct parse_literal_inline_string
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::String> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_literal_inline_string<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
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
        return std::make_pair(result, end);
    }
};

struct parse_literal_multiline_string
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::String> result_type;

    typedef is_chain_of<is_charactor<value_type, '\\'>, is_newline<value_type>>
            is_line_ending_backslash;
    typedef is_repeat_of<is_one_of<is_whitespace<value_type>, is_newline<value_type>>,
                         repeat_infinite()> ws_nl_after_backslash_remover;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_literal_multiline_string<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
        if(std::distance(iter, end) < 6)
            throw internal_error("is_literal_multiline_string");

        toml::String result; result.reserve(std::distance(iter, end)-6);
        std::advance(iter, 3);
        const Iterator last = end - 3;
        iter = is_newline<value_type>::invoke(iter); // trim first newline if exist
        while(iter != last)
        {
            result.push_back(*iter);
            ++iter;
        }
        return std::make_pair(result, end);
    }
};

struct parse_string
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::String> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        std::pair<result_type, Iterator> result;
        if((result = parse_basic_inline_string::invoke(iter)).first.ok())
            return result;
        else if((result = parse_basic_multiline_string::invoke(iter)).first.ok())
            return result;
        else if((result = parse_literal_inline_string::invoke(iter)).first.ok())
            return result;
        else if((result = parse_literal_multiline_string::invoke(iter)).first.ok())
            return result;
        else
            return std::make_pair(result_type{}, iter);
    }
};

//
// template<typename charT>
// struct parse_integer
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Integer result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         string_type result; result.resize(std::distance(iter, end));
//         std::copy_if(iter, end, result.begin(), [](charT c){return c != '_';});
//         return std::stoi(result);
//     }
// };
//
// template<typename charT>
// struct parse_float
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Float result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         string_type result; result.resize(std::distance(iter, end));
//         std::copy_if(iter, end, result.begin(), [](charT c){return c != '_';});
//         return std::stod(result);
//     }
// };
//
// template<typename charT>
// struct parse_boolean
// {
//     typedef charT value_type;
//     typedef toml::Boolean result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         return (std::distance(iter, end) == 4);
//     }
// };
//
// template<typename charT>
// struct parse_local_time
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Datetime result_type;
//     typedef typename result_type::number_type number_type;
//     typedef is_repeat_of<is_number<charT>, 2> nums;
//     typedef is_charactor<charT, ':'> delim;
//     typedef is_charactor<charT, '.'> fract;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         result.hour   = std::stoi(string_type(iter, nums::invoke(iter)));
//         iter = delim::invoke(nums::invoke(iter));
//         result.minute = std::stoi(string_type(iter, nums::invoke(iter)));
//         iter = delim::invoke(nums::invoke(iter));
//         result.second = std::stoi(string_type(iter, nums::invoke(iter)));
//         iter = fract::invoke(nums::invoke(iter));
//         if(iter == end)
//         {
//             result.millisecond = 0.0;
//             result.microsecond = 0.0;
//         }
//         else if(std::distance(iter, end) <= 3)
//         {
//             result.millisecond = parse_number(iter, end);
//             result.microsecond = 0.0;
//         }
//         else
//         {
//             result.millisecond = parse_number(iter, iter + 3);
//             result.microsecond = parse_number(iter + 3, end);
//         }
//         result.offset_hour   = result_type::nooffset;
//         result.offset_minute = result_type::nooffset;
//         result.year  = result_type::undef;
//         result.month = result_type::undef;
//         result.day   = result_type::undef;
//         return result;
//     }
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static number_type parse_number(Iterator iter, Iterator end)
//     {
//         if(std::distance(iter, end) > 3) end = iter + 3;
//         string_type str(iter, end);
//         while(str.size() < 3){str += '0';}
//         return std::stoi(str);
//     }
// };
//
// template<typename charT>
// struct parse_local_date
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Datetime result_type;
//     template<std::size_t N>
//     using nums = is_repeat_of<is_number<charT>, N>;
//     typedef is_charactor<charT, '-'> delim;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         result.year   = std::stoi(string_type(iter, nums<4>::invoke(iter)));
//         iter = delim::invoke(nums<4>::invoke(iter));
//         result.month  = std::stoi(string_type(iter, nums<2>::invoke(iter)));
//         iter = delim::invoke(nums<2>::invoke(iter));
//         result.day    = std::stoi(string_type(iter, nums<2>::invoke(iter)));
//
//         result.offset_hour   = result_type::nooffset;
//         result.offset_minute = result_type::nooffset;
//         result.hour   = result_type::undef;
//         result.minute = result_type::undef;
//         result.second = result_type::undef;
//         result.millisecond = result_type::undef;
//         result.microsecond = result_type::undef;
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_local_date_time
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Datetime result_type;
//     template<std::size_t N>
//     using nums = is_repeat_of<is_number<charT>, N>;
//     typedef is_charactor<charT, 'T'> delim;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         const Iterator date = is_local_date<charT>::invoke(iter);
//         result_type result = parse_local_date<charT>::invoke(iter, date);
//         iter = delim::invoke(date);// 'T'
//         const result_type time = parse_local_time<charT>::invoke(iter, end);
//         result.hour          = time.hour;
//         result.minute        = time.minute;
//         result.second        = time.second;
//         result.millisecond   = time.millisecond;
//         result.microsecond   = time.microsecond;
//         result.offset_hour   = result_type::nooffset;
//         result.offset_minute = result_type::nooffset;
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_offset_date_time
// {
//     typedef charT value_type;
//     typedef std::basic_string<value_type> string_type;
//     typedef toml::Datetime result_type;
//     template<std::size_t N>
//     using nums = is_repeat_of<is_number<charT>, N>;
//     typedef is_charactor<charT, ':'> delim;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         const Iterator datetime = is_local_date_time<charT>::invoke(iter);
//         result_type result = parse_local_date_time<charT>::invoke(iter, datetime);
//         iter = datetime;
//         if(*iter == 'Z')
//         {
//             result.offset_hour   = 0;
//             result.offset_minute = 0;
//         }
//         else
//         {
//             if(*iter != '+' && *iter != '-')
//                 throw syntax_error("invalid offset-datetime");
//             const int sign = (*iter == '-') ? -1 : 1;
//             ++iter;
//             result.offset_hour   = sign *
//                 std::stoi(string_type(iter, nums<2>::invoke(iter)));
//             iter = delim::invoke(nums<2>::invoke(iter));
//             result.offset_minute = sign *
//                 std::stoi(string_type(iter, nums<2>::invoke(iter)));
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_datetime
// {
//     typedef charT value_type;
//     typedef toml::Datetime result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         Iterator tmp = is_offset_date_time<value_type>::invoke(iter);
//         if(tmp != iter) return parse_offset_date_time<value_type>::invoke(iter, tmp);
//         tmp = is_local_date_time<value_type>::invoke(iter);
//         if(tmp != iter) return parse_local_date_time<value_type>::invoke(iter, tmp);
//         tmp = is_local_date<value_type>::invoke(iter);
//         if(tmp != iter) return parse_local_date<value_type>::invoke(iter, tmp);
//         tmp = is_local_time<value_type>::invoke(iter);
//         if(tmp != iter) return parse_local_time<value_type>::invoke(iter, tmp);
//         throw internal_error("no datetime here");
//     }
// };
//
// template<typename charT, typename acceptorT, typename parserT>
// struct parse_fixed_type_array
// {
//     typedef charT value_type;
//     typedef toml::Array result_type;
//     typedef acceptorT acceptor_type;
//     typedef parserT   parser_type;
//     typedef is_skippable_in_array<charT> skippable;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         --end;
//         assert(*iter == '[' && *end == ']');
//         ++iter;
//         iter = skippable::invoke(iter);
//         while(iter != end)
//         {
//             const Iterator tmp = acceptor_type::invoke(iter);
//             result.emplace_back(parser_type::invoke(iter, tmp));
//             iter = tmp;
//             iter = skippable::invoke(iter);
//             iter = is_charactor<charT, ','>::invoke(iter);
//             iter = skippable::invoke(iter);
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_inline_table;
//
// template<typename charT>
// struct parse_array
// {
//     typedef charT value_type;
//     typedef toml::Array result_type;
//     typedef is_skippable_in_array<charT> skippable;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         const Iterator init = skippable::invoke(std::next(iter));
//         if(is_boolean<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_boolean<charT>,
//                        parse_boolean<charT>>::invoke(iter, end);
//
//         if(is_integer<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_integer<charT>,
//                        parse_integer<charT>>::invoke(iter, end);
//
//         if(is_float<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_float<charT>,
//                        parse_float<charT>>::invoke(iter, end);
//
//         if(is_string<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_string<charT>,
//                        parse_string<charT>>::invoke(iter, end);
//
//         if(is_datetime<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_datetime<charT>,
//                        parse_datetime<charT>>::invoke(iter, end);
//
//         if(is_array<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_array<charT>,
//                        parse_array<charT>>::invoke(iter, end);
//
//         if(is_inline_table<charT>::invoke(init) != init)
//             return parse_fixed_type_array<charT, is_inline_table<charT>,
//                         parse_inline_table<charT>>::invoke(iter, end);
//
//         if(is_skippable_in_array<charT>::invoke(init) == std::prev(end))
//             return result_type{}; // empty
//
//         throw internal_error("no valid array here");
//     }
// };
//
// template<typename charT>
// struct parse_value
// {
//     typedef charT value_type;
//     typedef toml::value result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         if(iter != is_string<charT>::invoke(iter))
//             return result_type(parse_string<charT>::invoke(iter, end));
//         else if(iter != is_datetime<charT>::invoke(iter))
//             return result_type(parse_datetime<charT>::invoke(iter, end));
//         else if(iter != is_float<charT>::invoke(iter))
//             return result_type(parse_float<charT>::invoke(iter, end));
//         else if(iter != is_integer<charT>::invoke(iter))
//             return result_type(parse_integer<charT>::invoke(iter, end));
//         else if(iter != is_boolean<charT>::invoke(iter))
//             return result_type(parse_boolean<charT>::invoke(iter, end));
//         else if(iter != is_array<charT>::invoke(iter))
//             return result_type(parse_array<charT>::invoke(iter, end));
//         else if(iter != is_inline_table<charT>::invoke(iter))
//             return result_type(parse_inline_table<charT>::invoke(iter, end));
//
//         throw internal_error("no valid value here");
//     }
// };
//
// template<typename charT>
// struct parse_barekey
// {
//     typedef charT value_type;
//     typedef toml::key result_type;
//     static_assert(std::is_same<value_type, result_type::value_type
//             >::value, "char type is different from default key type");
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         return result_type(iter, end);
//     }
// };
//
// template<typename charT>
// struct parse_key
// {
//     typedef charT value_type;
//     typedef toml::key result_type;
//     static_assert(std::is_same<value_type, result_type::value_type
//             >::value, "char type is different from default key type");
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         if(iter != is_barekey<charT>::invoke(iter))
//             return parse_barekey<charT>::invoke(iter, end);
//         else if(iter != is_string<charT>::invoke(iter))
//             return parse_string<charT>::invoke(iter, end);
//         throw internal_error("no valid key here");
//     }
// };
//
// template<typename charT>
// struct parse_key_value_pair
// {
//     typedef charT value_type;
//     typedef std::pair<toml::key, toml::value> result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         Iterator tmp = is_key<charT>::invoke(iter);
//         const toml::key k = parse_key<charT>::invoke(iter, tmp);
//         iter = tmp;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//         assert(*iter == '='); ++iter;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//         tmp = is_value<charT>::invoke(iter);
//         const toml::value v = parse_value<charT>::invoke(iter, tmp);
//         return std::make_pair(k, v);
//     }
// };
//
// template<typename charT>
// struct parse_inline_table
// {
//     typedef charT value_type;
//     typedef toml::Table result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         --end;
//         assert(*iter == '{' && *end == '}');
//         ++iter;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//
//         result_type result;
//
//         while(iter != end)
//         {
//             Iterator tmp = is_key_value_pair<charT>::invoke(iter);
//             result.emplace(parse_key_value_pair<charT>::invoke(iter, tmp));
//             iter = tmp;
//
//             iter = is_any_num_of_ws<charT>::invoke(iter);
//             iter = is_charactor<charT, ','>::invoke(iter);
//             iter = is_any_num_of_ws<charT>::invoke(iter);
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_table_contents
// {
//     typedef charT value_type;
//     typedef toml::Table result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         while(iter != end)
//         {
//             iter = is_empty_lines<charT>::invoke(iter);
//
//             Iterator tmp = is_key_value_pair<charT>::invoke(iter);
//             result.emplace(parse_key_value_pair<charT>::invoke(iter, tmp));
//             iter = is_one_of<is_comment<charT>, is_newline<charT>>::invoke(tmp);
//
//             iter = is_empty_lines<charT>::invoke(iter);
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_standard_table_definition
// {
//     typedef charT value_type;
//     typedef std::vector<toml::key> result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//         --end;
//         assert(*iter == '[' && *end == ']');
//         ++iter;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//
//         Iterator tmp = is_key<charT>::invoke(iter);
//         result.emplace_back(parse_key<charT>::invoke(iter, tmp));
//         iter = is_any_num_of_ws<charT>::invoke(tmp);
//
//         while(iter != end)
//         {
//             iter = is_charactor<charT, '.'>::invoke(iter);
//             iter = is_any_num_of_ws<charT>::invoke(iter);
//
//             tmp  = is_key<charT>::invoke(iter);
//             result.emplace_back(parse_key<charT>::invoke(iter, tmp));
//             iter = is_any_num_of_ws<charT>::invoke(tmp);
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_array_of_table_definition
// {
//     typedef charT value_type;
//     typedef std::vector<toml::key> result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//         --end;
//         assert(*iter == '[' && *std::next(iter) == '[' &&
//                *end  == ']' && *std::prev(end) == ']');
//         ++iter; ++iter; --end;
//
//         iter = is_any_num_of_ws<charT>::invoke(iter);
//
//         Iterator tmp = is_key<charT>::invoke(iter);
//         result.emplace_back(parse_key<charT>::invoke(iter, tmp));
//         iter = is_any_num_of_ws<charT>::invoke(tmp);
//
//         while(iter != end)
//         {
//             iter = is_charactor<charT, '.'>::invoke(iter);
//             iter = is_any_num_of_ws<charT>::invoke(iter);
//
//             tmp  = is_key<charT>::invoke(iter);
//             result.emplace_back(parse_key<charT>::invoke(iter, tmp));
//             iter = is_any_num_of_ws<charT>::invoke(tmp);
//         }
//         return result;
//     }
// };
//
// template<typename charT>
// struct parse_data
// {
//     typedef charT value_type;
//     typedef toml::Table result_type;
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      value_type>::value>::type>
//     static result_type invoke(Iterator iter, Iterator end)
//     {
//         result_type result;
//
//         Iterator tmp = is_table_contents<charT>::invoke(iter);
//         if(tmp != iter)
//         {
//             result = parse_table_contents<charT>::invoke(iter, tmp);
//         }
//
//         while(iter != end)
//         {
//             if(iter != (tmp = is_table_definition<charT>::invoke(iter)))
//             {
//                 auto k = parse_standard_table_definition<charT>::invoke(iter, tmp);
//                 iter = tmp;
//                 tmp  = is_table_contents<charT>::invoke(iter);
//                 auto tab = parse_table_contents<charT>::invoke(iter, tmp);
//                 push_table(result, std::move(tab), std::begin(k), std::end(k));
//             }
//             else if(iter != (tmp=is_array_of_table_definition<charT>::invoke(iter)))
//             {
//                 auto k = parse_array_of_table_definition<charT>::invoke(iter, tmp);
//                 iter = tmp;
//                 tmp  = is_table_contents<charT>::invoke(iter);
//                 auto tab = parse_table_contents<charT>::invoke(iter, tmp);
//                 push_array_of_table(result, std::move(tab),
//                                     std::begin(k), std::end(k));
//             }
//             else
//                 throw internal_error("no array ");
//         }
//         return result;
//     }
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      toml::key>::value>::type>
//     static void
//     push_table(toml::Table& data, toml::Table&& v, Iterator iter, Iterator end)
//     {
//         if(iter == std::prev(end))
//         {
//             if(data.count(*iter) == 1)
//                 throw syntax_error("duplicate key");
//             data.emplace(*iter, std::move(v));
//         }
//
//         if(data.count(*iter) == 0)
//             data.emplace(*iter, toml::Table());
//         else if(data[*iter].type() != value_t::Table)
//             throw syntax_error("duplicate key");
//
//         return push_table(data[*iter].template cast<value_t::Table>(),
//                           std::move(v), std::next(iter), end);
//     }
//
//     template<typename Iterator, class = typename std::enable_if<
//         std::is_same<typename std::iterator_traits<Iterator>::value_type,
//                      toml::key>::value>::type>
//     static void
//     push_array_of_table(toml::Table& data, toml::Table&& v,
//                         Iterator iter, Iterator end)
//     {
//         if(iter == std::prev(end))
//         {
//             if(data.count(*iter) == 0)
//                 data.emplace(*iter, toml::Array());
//             else if(data.at(*iter).type() != value_t::Array)
//                 throw syntax_error("duplicate key");
//
//             data[*iter].template cast<value_t::Array>().emplace_back(std::move(v));
//         }
//
//         if(data.count(*iter) == 0)
//             data.emplace(*iter, toml::Table());
//         else if(data[*iter].type() != value_t::Table)
//             throw syntax_error("duplicate key");
//
//         return push_array_of_table(data[*iter].template cast<value_t::Table>(),
//                                    std::move(v), std::next(iter), end);
//     }
//
// };

}// toml
#endif// TOML11_PARSER

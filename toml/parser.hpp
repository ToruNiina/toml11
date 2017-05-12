#ifndef TOML11_PARSER
#define TOML11_PARSER
#include "value.hpp"
#include "acceptor.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>

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

    template<typename U>
    result& operator=(const result<U>& u) {ok_ = u.ok(); if(ok_)value_ = u.move(); return *this;}
    template<typename U>
    result& operator=(result<U>&& u) {ok_ = u.ok(); if(ok_)value_ = u.move(); return *this;}
    template<typename U>
    result(const result<U>& u): ok_(u.ok()){if(ok_)value_ = u.get();}
    template<typename U>
    result(result<U>&& u): ok_(u.ok()){if(ok_)value_ = u.move();}

    bool ok() const {return ok_;}
    operator bool() const {return ok_;}

    T&       get()       {if(!ok_) throw std::logic_error("result::get"); return value_;}
    T const& get() const {if(!ok_) throw std::logic_error("result::get"); return value_;}
    T&&      move()
    {if(!ok_) throw std::logic_error("result::move"); ok_ = false; return std::move(value_);}

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

struct parse_integer
{
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type> string_type;
    typedef detail::result<toml::Integer> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_integer<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](value_type c){return c != '_';});
        return std::make_pair(std::stoll(result), end);
    }
};

struct parse_float
{
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type> string_type;
    typedef detail::result<toml::Float> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_float<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        string_type result; result.resize(std::distance(iter, end));
        std::copy_if(iter, end, result.begin(), [](value_type c){return c != '_';});
        try{
            return std::make_pair(std::stod(result), end);
        }
        catch(std::out_of_range& oor)
        {
            std::cout << "extremely large Float value appeared: "
                      << result << "; it is negrected" << std::endl;
            return std::make_pair(0, end);
        }
    }
};

struct parse_boolean
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::Boolean> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_boolean<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
        return std::make_pair((std::distance(iter, end) == 4), end);
    }
};

struct parse_local_time
{
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type>  string_type;
    typedef detail::result<toml::Datetime> result_type;
    typedef typename toml::Datetime::number_type number_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::charactor>, N>;
    typedef is_charactor<toml::charactor, ':'> delim;
    typedef is_charactor<toml::charactor, '.'> fract;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_local_time<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        toml::Datetime result;
        result.hour   = std::stoi(string_type(iter, nums<2>::invoke(iter)));
        iter = delim::invoke(nums<2>::invoke(iter));
        result.minute = std::stoi(string_type(iter, nums<2>::invoke(iter)));
        iter = delim::invoke(nums<2>::invoke(iter));
        result.second = std::stoi(string_type(iter, nums<2>::invoke(iter)));
        iter = fract::invoke(nums<2>::invoke(iter));
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
        return std::make_pair(result, end);
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
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type> string_type;
    typedef detail::result<toml::Datetime> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<value_type>, N>;
    typedef is_charactor<value_type, '-'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_local_date<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        toml::Datetime result;
        result.year   = std::stoi(string_type(iter, nums<4>::invoke(iter)));
        iter = delim::invoke(nums<4>::invoke(iter));
        result.month  = std::stoi(string_type(iter, nums<2>::invoke(iter)));
        iter = delim::invoke(nums<2>::invoke(iter));
        result.day    = std::stoi(string_type(iter, nums<2>::invoke(iter)));

        result.offset_hour   = toml::Datetime::nooffset;
        result.offset_minute = toml::Datetime::nooffset;
        result.hour   = toml::Datetime::undef;
        result.minute = toml::Datetime::undef;
        result.second = toml::Datetime::undef;
        result.millisecond = toml::Datetime::undef;
        result.microsecond = toml::Datetime::undef;
        return std::make_pair(result, end);
    }
};

struct parse_local_date_time
{
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type> string_type;
    typedef detail::result<toml::Datetime> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::charactor>, N>;
    typedef is_charactor<toml::charactor, 'T'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_local_date_time<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        auto ld = parse_local_date::invoke(iter);
        if(!ld.first.ok()) throw syntax_error("invalid local datetime");
        toml::Datetime result(ld.first.move());
        iter = delim::invoke(ld.second);// 'T'

        const auto time = parse_local_time::invoke(iter);
        result.hour          = time.first.get().hour;
        result.minute        = time.first.get().minute;
        result.second        = time.first.get().second;
        result.millisecond   = time.first.get().millisecond;
        result.microsecond   = time.first.get().microsecond;
        result.offset_hour   = toml::Datetime::nooffset;
        result.offset_minute = toml::Datetime::nooffset;
        return std::make_pair(result, end);
    }
};

struct parse_offset_date_time
{
    typedef toml::charactor value_type;
    typedef std::basic_string<value_type> string_type;
    typedef detail::result<toml::Datetime> result_type;
    template<std::size_t N>
    using nums = is_repeat_of<is_number<toml::charactor>, N>;
    typedef is_charactor<toml::charactor, ':'> delim;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_offset_date_time<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        auto ldt = parse_local_date_time::invoke(iter);
        if(!ldt.first.ok()) throw syntax_error("invalid offset datetime");
        toml::Datetime result(ldt.first.move());
        iter = ldt.second;
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
        return std::make_pair(result, end);
    }
};

struct parse_datetime
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::Datetime> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        std::pair<result_type, Iterator> result;
        if((result = parse_offset_date_time::invoke(iter)).first.ok())
            return result;
        else if((result = parse_local_date_time::invoke(iter)).first.ok())
            return result;
        else if((result = parse_local_date::invoke(iter)).first.ok())
            return result;
        else if((result = parse_local_time::invoke(iter)).first.ok())
            return result;
        else
            return std::make_pair(result_type{}, iter);
    }
};

template<typename acceptorT, typename parserT>
struct parse_fixed_type_array
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::Array> result_type;
    typedef acceptorT acceptor_type;
    typedef parserT   parser_type;
    typedef is_skippable_in_array<value_type> skippable;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_fixed_type_array<value_type, acceptorT>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        toml::Array result;
        const Iterator last = std::prev(end);
        iter = skippable::invoke(std::next(iter));
        while(iter != last)
        {
            const Iterator tmp = acceptor_type::invoke(iter);
            if(tmp == iter) throw syntax_error("parse_array");
            auto next = parser_type::invoke(iter);
            if(!next.first.ok()) throw syntax_error("parse_array");
            result.emplace_back(next.first.move());
            iter = tmp;
            iter = skippable::invoke(iter);
            iter = is_charactor<value_type, ','>::invoke(iter);
            iter = skippable::invoke(iter);
        }
        return std::make_pair(result, end);
    }
};

template<typename charT>
struct parse_inline_table;

template<typename charT>
struct parse_array
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::charactor>::value, "");
    typedef detail::result<toml::Array> result_type;
    typedef is_skippable_in_array<value_type> skippable;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        if(iter == is_array<value_type>::invoke(iter))
            return std::make_pair(result_type{}, iter);

        std::pair<result_type, Iterator> result;
        if((result = parse_fixed_type_array<is_boolean<value_type>,
                     parse_boolean>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_string<value_type>,
                parse_string>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_datetime<value_type>,
                parse_datetime>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_float<value_type>,
                parse_float>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_integer<value_type>,
                parse_integer>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_array<value_type>,
                parse_array<value_type>>::invoke(iter)).first.ok()) return result;
        else if((result = parse_fixed_type_array<is_inline_table<value_type>,
                parse_inline_table<value_type>>::invoke(iter)).first.ok())
            return result;
        else if(skippable::invoke(std::next(iter)) == // empty
                std::prev(is_array<value_type>::invoke(iter))
                ) return std::make_pair(
                    toml::Array{}, is_array<value_type>::invoke(iter));
        else throw syntax_error("no valid array here");
    }
};

template<typename charT>
struct parse_value
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::charactor>::value, "");
    typedef detail::result<toml::value> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        std::pair<result_type, Iterator> result;
        if((result = parse_boolean::invoke(iter)).first.ok())
            return result;
        else if((result = parse_string::invoke(iter)).first.ok())
            return result;
        else if((result = parse_datetime::invoke(iter)).first.ok())
            return result;
        else if((result = parse_float::invoke(iter)).first.ok())
            return result;
        else if((result = parse_integer::invoke(iter)).first.ok())
            return result;
        else if((result = parse_array<value_type>::invoke(iter)).first.ok())
            return result;
        else if((result = parse_inline_table<value_type>::invoke(iter)).first.ok())
            return result;
        else
            return std::make_pair(result_type{}, iter);
    }
};

struct parse_barekey
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::key> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_barekey<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);
        return std::make_pair(toml::key(iter, end), end);
    }
};

struct parse_key
{
    typedef toml::charactor value_type;
    typedef detail::result<toml::key> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        std::pair<result_type, Iterator> result;
        if((result = parse_barekey::invoke(iter)).first.ok())
            return result;
        else if((result = parse_string::invoke(iter)).first.ok())
            return result;
        else return std::make_pair(result_type{}, iter);
    }
};

template<typename charT>
struct parse_key_value_pair
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::charactor>::value, "");
    typedef detail::result<std::pair<toml::key, toml::value>> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        auto tmp_key = parse_key::invoke(iter);
        if(!tmp_key.first.ok())
            return std::make_pair(result_type{}, iter);
        iter = is_any_num_of_ws<charT>::invoke(tmp_key.second);
        if(*iter != '=') throw syntax_error("invalid key value pair");
        iter = is_any_num_of_ws<charT>::invoke(std::next(iter));

        auto tmp_value = parse_value<toml::charactor>::invoke(iter);
        if(!tmp_value.first.ok())
            throw syntax_error("invalid key value pair");

        iter = tmp_value.second;

        return std::make_pair(std::make_pair(
                    tmp_key.first.move(), tmp_value.first.move()),
                is_any_num_of_ws<charT>::invoke(tmp_value.second));
    }
};

template<typename charT>
struct parse_inline_table
{
    typedef charT value_type;
    static_assert(std::is_same<charT, toml::charactor>::value, "");
    typedef detail::result<toml::Table> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_inline_table<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        iter = is_any_num_of_ws<value_type>::invoke(std::next(iter));

        const Iterator last = std::prev(end);
        toml::Table result;
        while(iter != last)
        {
            auto tmp = parse_key_value_pair<value_type>::invoke(iter);
            if(!tmp.first.ok()) throw syntax_error("parse_inline_table");

            result.emplace(tmp.first.move());
            iter = tmp.second;

            iter = is_any_num_of_ws<value_type>::invoke(iter);
            iter = is_charactor<value_type, ','>::invoke(iter);
            iter = is_any_num_of_ws<value_type>::invoke(iter);
        }
        return std::make_pair(result, end);
    }
};

struct parse_table_definition
{
    typedef toml::charactor value_type;
    typedef detail::result<std::vector<toml::key>> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_table_definition<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        std::vector<toml::key> result;
        result.reserve(std::count(iter, end, '.')+1);

        const Iterator last = std::prev(end);
        iter = is_any_num_of_ws<value_type>::invoke(iter);
        iter = is_any_num_of_ws<value_type>::invoke(std::next(iter));

        auto tmp = parse_key::invoke(iter);
        if(!tmp.first.ok()) throw syntax_error("table definition");
        result.emplace_back(tmp.first.move());
        iter = is_any_num_of_ws<value_type>::invoke(tmp.second);

        while(iter != last)
        {
            iter = is_charactor<value_type, '.'>::invoke(iter);
            iter = is_any_num_of_ws<value_type>::invoke(iter);

            tmp = parse_key::invoke(iter);
            if(!tmp.first.ok()) throw syntax_error("table definition");
            result.emplace_back(tmp.first.move());
            iter = is_any_num_of_ws<value_type>::invoke(tmp.second);
        }
        return std::make_pair(result, end);
    }
};

struct parse_array_of_table_definition
{
    typedef toml::charactor value_type;
    typedef detail::result<std::vector<toml::key>> result_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static std::pair<result_type, Iterator> invoke(Iterator iter)
    {
        const Iterator end = is_array_of_table_definition<value_type>::invoke(iter);
        if(iter == end) return std::make_pair(result_type{}, iter);

        std::vector<toml::key> result;
        result.reserve(std::count(iter, end, '.')+1);

        const Iterator last = end - 2;
        iter = is_any_num_of_ws<value_type>::invoke(iter) + 2;
        iter = is_any_num_of_ws<value_type>::invoke(iter);

        auto tmp = parse_key::invoke(iter);
        if(!tmp.first.ok()) throw syntax_error("array of table definition");
        result.emplace_back(tmp.first.move());
        iter = is_any_num_of_ws<value_type>::invoke(tmp.second);

        while(iter != last)
        {
            iter = is_charactor<value_type, '.'>::invoke(iter);
            iter = is_any_num_of_ws<value_type>::invoke(iter);

            tmp  = parse_key::invoke(iter);
            if(!tmp.first.ok()) throw syntax_error("array of table definition");
            result.emplace_back(tmp.first.move());
            iter = is_any_num_of_ws<value_type>::invoke(tmp.second);
        }
        return std::make_pair(result, end);
    }
};

struct parse_data
{
    typedef toml::charactor value_type;
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
            std::pair<detail::result<std::vector<toml::key>>, Iterator> tabname;
            if((tabname = parse_table_definition::invoke(iter)).first.ok())
            {
                auto contents = parse_table_contents(tabname.second, end);
                push_table(result, std::move(contents.first),
                           tabname.first.get().begin(), tabname.first.get().end());
                iter = contents.second;
            }
            else if((tabname = parse_array_of_table_definition::invoke(iter)).first.ok())
            {
                auto contents = parse_table_contents(tabname.second, end);
                push_array_of_table(result, std::move(contents.first),
                           tabname.first.get().begin(), tabname.first.get().end());
                iter = contents.second;
            }
            else
                throw syntax_error("parse_data: unknown line");
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
                      iter == is_newline<value_type>::invoke(iter)){++iter;}
            }
            else if(iter == is_newline<value_type>::invoke(iter) &&
                    iter == is_whitespace<value_type>::invoke(iter))
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
            auto kv = parse_key_value_pair<value_type>::invoke(iter);
            if(!kv.first.ok()) return std::make_pair(table, iter);

            table.emplace(kv.first.move());
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
            data.emplace(*iter, toml::Table());
        else if(data[*iter].type() != value_t::Table)
            throw syntax_error("duplicate key: " + *iter);

        return push_table(data[*iter].template cast<value_t::Table>(),
                          std::move(v), std::next(iter), end);
    }

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     toml::key>::value>::type>
    static void
    push_array_of_table(toml::Table& data, toml::Table&& v,
                        Iterator iter, Iterator end)
    {
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
            data.emplace(*iter, toml::Table());
        else if(data[*iter].type() != value_t::Table)
            throw syntax_error("duplicate key: " + *iter);

        return push_array_of_table(data[*iter].template cast<value_t::Table>(),
                                   std::move(v), std::next(iter), end);
    }

};

template<typename traits = std::char_traits<toml::charactor>>
toml::Table parse(std::basic_istream<toml::charactor, traits>& is)
{
    const auto initial = is.tellg();
    is.seekg(0, std::ios::end);
    const auto eofpos  = is.tellg();
    const std::size_t size = eofpos - initial;
    is.seekg(initial);
    std::vector<toml::charactor> contents(size);
    is.read(contents.data(), size);
    return parse_data::invoke(contents.cbegin(), contents.cend());
}

toml::Table parse(const std::string& filename)
{
    std::ifstream ifs(filename);
    if(!ifs.good()) throw std::runtime_error("file open error: " + filename);
    return parse(ifs);
}


}// toml
#endif// TOML11_PARSER

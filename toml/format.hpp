#ifndef TOML11_FORMAT
#define TOML11_FORMAT
#include "value.hpp"
#include <type_traits>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <locale>

namespace toml
{

// synopsis
// toml::format("key", value, toml::make_inline(80))
// toml::format("key", value, toml::forceinline)
// std::cout << toml::make_inline(80) << value;
// std::cout << toml::forceinline << value;

template<typename traits = std::char_traits<toml::character>,
         typename alloc = std::allocator<toml::character>>
std::basic_string<toml::character, traits, alloc>
format(const value& v);

template<typename traits = std::char_traits<toml::character>,
         typename alloc = std::allocator<toml::character>>
std::basic_string<toml::character, traits, alloc>
format(const value& v, std::size_t mk);

template<typename traits = std::char_traits<toml::character>,
         typename alloc = std::allocator<toml::character>>
std::basic_string<toml::character, traits, alloc>
format(const toml::key& k, const value& v);

template<typename traits = std::char_traits<toml::character>,
         typename alloc = std::allocator<toml::character>>
std::basic_string<toml::character, traits, alloc>
format(const toml::key& k, const value& v, std::size_t mk);

template<value_t Type>
struct format_impl;

template<> struct format_impl<value_t::Boolean>
{
    typedef detail::toml_default_type<value_t::Boolean>::type type;

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        return val ? "true" : "false";
    }
};

template<> struct format_impl<value_t::Integer>
{
    typedef detail::toml_default_type<value_t::Integer>::type type;

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        return std::to_string(val);
    }
};

template<> struct format_impl<value_t::Float>
{
    typedef detail::toml_default_type<value_t::Float>::type type;

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        std::basic_ostringstream<toml::character> oss;
        oss << std::showpoint << val;
        if(oss.str().back() == '.') oss << '0';
        return oss.str();
    }
};

template<> struct format_impl<value_t::String>
{
    typedef detail::toml_default_type<value_t::String>::type type;

    std::size_t max_length;

    format_impl() : max_length(80){}
    format_impl(std::size_t mx) : max_length(mx){}

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        auto tmp = make_inline(val);
        if(max_length == std::numeric_limits<std::size_t>::max() ||
           tmp.size() <= max_length) return tmp;
        return convert_multiline(std::move(tmp));
    }

  private:

    std::basic_string<toml::character>
    make_inline(const std::basic_string<toml::character>& val)
    {
        std::basic_string<toml::character> str;
        str += '"';
        for(const auto& c : val)
        {
            if('\0' < c && c < '\31')
            {
                switch(c)
                {
                    case '\b': str += "\\b"; break;
                    case '\t': str += "\\t"; break;
                    case '\n': str += "\\n"; break;
                    case '\f': str += "\\f"; break;
                    case '\r': str += "\\r"; break;
                    default:
                    {
                        str += 'u';
                        std::basic_ostringstream<toml::character> oss;
                        oss << std::setw(4) << std::setfill('0') << std::hex
                            << static_cast<std::int8_t>(c);
                        auto hexdig = oss.str();
                        std::transform(hexdig.begin(), hexdig.end(), hexdig.begin(), ::toupper);
                        str += oss.str();
                        break;
                    }
                }
            }
            else if(c == '"')
            {
                str += "\\\"";
            }
            else if(c == '\\')
            {
                str += "\\\\";
            }
            else
            {
                str += c;
            }
        }
        str += '"';
        return str;
    }

    std::basic_string<toml::character>
    convert_multiline(std::basic_string<toml::character>&& val)
    {
        std::basic_string<toml::character> str; str.reserve(val.size() + 6);
        str += "\"\"\"\n";
        std::size_t current = 0;
        for(auto iter = val.begin()+1; iter != val.end()-1; ++iter)
        {
            if(*iter != '\\')
            {
                if(current + 1 == max_length) str += "\\\n";
                str += *iter; continue;
            }
            assert(std::next(iter) < val.end()-1);
            if(*std::next(iter) == 'u')
            {
                if(current + 5 == max_length) str += "\\\n";
                assert(iter + 5 < val.end()-1);
                str += *iter; ++iter;  // u
                str += *iter; ++iter;  // 0
                str += *iter; ++iter;  // 1
                str += *iter; ++iter;  // 2
                str += *iter; continue;// 3
            }
            if(current + 2 == max_length) str += "\\\n";
            str += *iter; ++iter; str += *iter;
        }
        str += "\"\"\"";
        return str;
    }

};

template<> struct format_impl<value_t::Datetime>
{
    typedef detail::toml_default_type<value_t::Datetime>::type type;

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        std::basic_ostringstream<toml::character> oss;
        oss << val;
        return oss.str();
    }
};

// TODO max length!
template<> struct format_impl<value_t::Array>
{
    typedef detail::toml_default_type<value_t::Array>::type type;

    std::size_t max_length;

    format_impl() : max_length(80){}
    format_impl(std::size_t mx) : max_length(mx){}

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        std::basic_string<toml::character> retval;
        retval += '[';
        for(const auto& item : val)
        {
            auto tmp = format(val, max_length - 1);
            retval += tmp;
            retval += ", ";
            if(tmp.size() * 2 > max_length) retval += '\n';
        }
        retval += ']';
        return retval;
    }
};

// TODO max length && inline!
template<> struct format_impl<value_t::Table>
{
    typedef detail::toml_default_type<value_t::Table>::type type;

    std::size_t max_length;

    format_impl() : max_length(80){}
    format_impl(std::size_t mx) : max_length(mx){}

    std::basic_string<toml::character>
    operator()(const type& val)
    {
        std::basic_string<toml::character> retval;
        for(const auto& item : val)
        {
            retval += item.first;
            retval += " = ";
            retval += format(item.second);
            retval += '\n';
        }
        return retval;
    }
};

template<typename traits, typename alloc>
std::basic_string<toml::character, traits, alloc>
format(const value& v)
{
    switch(v.type())
    {
        case value_t::Boolean : return format_impl<value_t::Boolean >{}(v.template cast<value_t::Boolean >());
        case value_t::Integer : return format_impl<value_t::Integer >{}(v.template cast<value_t::Integer >());
        case value_t::Float   : return format_impl<value_t::Float   >{}(v.template cast<value_t::Float   >());
        case value_t::String  : return format_impl<value_t::String  >{}(v.template cast<value_t::String  >());
        case value_t::Datetime: return format_impl<value_t::Datetime>{}(v.template cast<value_t::Datetime>());
        case value_t::Array   : return format_impl<value_t::Array   >{}(v.template cast<value_t::Array   >());
        case value_t::Table   : return format_impl<value_t::Table   >{}(v.template cast<value_t::Table   >());
        case value_t::Empty   : throw std::runtime_error("toml::format: empty value");
        case value_t::Unknown : throw std::runtime_error("toml::format: unknown value");
        default: throw std::logic_error("toml::format: unknown enum value");
    }
}

template<typename traits, typename alloc>
std::basic_string<toml::character, traits, alloc>
format(const value& v, std::size_t inl)
{
    switch(v.type())
    {
        case value_t::Boolean : return format_impl<value_t::Boolean >{}(v.template cast<value_t::Boolean>());
        case value_t::Integer : return format_impl<value_t::Integer >{}(v.template cast<value_t::Integer>());
        case value_t::Float   : return format_impl<value_t::Float   >{}(v.template cast<value_t::Float>());
        case value_t::String  : return format_impl<value_t::String  >{inl}(v.template cast<value_t::String>());
        case value_t::Datetime: return format_impl<value_t::Datetime>{}(v.template cast<value_t::Datetime>());
        case value_t::Array   : return format_impl<value_t::Array   >{inl}(v.template cast<value_t::Array>());
        case value_t::Table   : return format_impl<value_t::Table   >{inl}(v.template cast<value_t::Table>());
        case value_t::Empty   : throw std::runtime_error("toml::format: empty value");
        case value_t::Unknown : throw std::runtime_error("toml::format: unknown value");
        default: throw std::logic_error("toml::format: unknown enum value");
    }
}

template<typename traits, typename alloc>
std::basic_string<toml::character, traits, alloc>
format(std::basic_string<toml::character, traits, alloc> key, const value& val)
{
    std::basic_string<toml::character, traits, alloc> retval(std::move(key));
    retval += " = ";
    retval += format(val);
    return retval;
}

template<typename traits, typename alloc>
std::basic_string<toml::character, traits, alloc>
format(std::basic_string<toml::character, traits, alloc> key, const value& val, std::size_t mk)
{
    std::basic_string<toml::character, traits, alloc> retval(std::move(key));
    retval += " = ";
    retval += format(val, mk);
    return retval;
}


// ----------------------------- stream operators -----------------------------

namespace detail
{

template<typename T = std::size_t>
struct inline_limit
{
    static_assert(std::is_same<T, std::size_t>::value, "do not instantiate this");
    static const int index;
    T limit;
    inline_limit() = default;
    ~inline_limit() = default;
    constexpr inline_limit(T i): limit(i){}
    constexpr operator T() const {return limit;}

    static void callback(std::ios_base::event ev, std::ios_base& ios, int idx)
    {
        void*& info = ios.pword(idx);
        switch (ev)
        {
            case std::ios_base::erase_event:
            {
                delete static_cast<std::size_t*>(info);
                break;
            }
            case std::ios_base::copyfmt_event:
            {
                info = new std::size_t(*static_cast<std::size_t*>(info));
                break;
            }
            case std::ios_base::imbue_event:
            {
                break;
            }
        }
    }
};

template<typename T>
const int inline_limit<T>::index = std::ios_base::xalloc();

} //detail

template<typename sizeT, typename traits = std::char_traits<toml::character>>
std::basic_ostream<toml::character, traits>&
operator<<(std::basic_ostream<toml::character, traits>& os,
           const detail::inline_limit<sizeT>& inl)
{
    void*& info = os.pword(detail::inline_limit<sizeT>::index);
    if(!os.bad())
    {
        if(info == nullptr)
        {
            os.register_callback(detail::inline_limit<sizeT>::callback,
                                 detail::inline_limit<sizeT>::index);
            info = new std::size_t(inl.limit);
        }
        else
        {
            *static_cast<std::size_t*>(info) = inl.limit;
        }
    }
    return os;
}

constexpr static detail::inline_limit<std::size_t> forceinline(
        std::numeric_limits<std::size_t>::max());

inline detail::inline_limit<std::size_t> make_inline(std::size_t sz)
{
    return detail::inline_limit<std::size_t>(sz);
}

template<typename T, typename traits = std::char_traits<toml::character>>
std::basic_ostream<toml::character, traits>&
operator<<(std::basic_ostream<toml::character, traits>& os,
           const toml::value& v)
{
    std::size_t* info =
        static_cast<std::size_t*>(os.pword(detail::inline_limit<std::size_t>::index));
    return os << (info == nullptr ? toml::format(v) : toml::format(v, *info));
}

}
#endif // TOML11_FORMAT

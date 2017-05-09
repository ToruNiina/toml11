#ifndef TOML11_FORMAT
#define TOML11_FORMAT
#include "value.hpp"
#include <sstream>

namespace toml
{

template<typename traits = std::char_traits<toml::charactor>,
         typename alloc = std::allocator<toml::charactor>>
std::basic_string<toml::charactor, traits, alloc>
format(const value& v);

template<value_t Type>
struct format_impl;

template<> struct format_impl<value_t::Boolean>
{
    typedef detail::toml_default_type<value_t::Boolean>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        return val ? "true" : "false";
    }
};

template<> struct format_impl<value_t::Integer>
{
    typedef detail::toml_default_type<value_t::Integer>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        return std::to_string(val);
    }
};

template<> struct format_impl<value_t::Float>
{
    typedef detail::toml_default_type<value_t::Float>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        std::basic_ostringstream<toml::charactor> oss;
        oss << std::showpoint << val;
        if(oss.str().back() == '.') oss << '0';
        return oss.str();
    }
};

template<> struct format_impl<value_t::String>
{
    typedef detail::toml_default_type<value_t::String>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        //TODO escape some charactors!
        return val;
    }
};

template<> struct format_impl<value_t::Datetime>
{
    typedef detail::toml_default_type<value_t::Datetime>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        std::basic_ostringstream<toml::charactor> oss;
        oss << val;
        return oss.str();
    }
};

// TODO max length!
template<> struct format_impl<value_t::Array>
{
    typedef detail::toml_default_type<value_t::Array>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        std::basic_string<toml::charactor> retval;
        retval += '[';
        for(const auto&& item : val)
        {
            retval += format(val);
            retval += ", ";
        }
        retval += ']';
        return ;
    }
};

// TODO max length && inline!
template<> struct format_impl<value_t::Table>
{
    typedef detail::toml_default_type<value_t::Table>::type type;

    static std::basic_string<toml::charactor>
    invoke(const type& val)
    {
        std::basic_string<toml::charactor> retval;
        for(const auto&& item : val)
        {
            retval += val.first;
            retval += " = "
            retval += format(val.second);
            retval += '\n';
        }
        return ;
    }
};


template<typename traits, typename alloc>
std::basic_string<toml::charactor, traits, alloc>
format(const value& v)
{
    switch(v.type())
    {
        case value_t::Boolean : return format_impl<value_t::Boolean >::invoke(v.template cast<value_t::Boolean >());
        case value_t::Integer : return format_impl<value_t::Integer >::invoke(v.template cast<value_t::Integer >());
        case value_t::Float   : return format_impl<value_t::Float   >::invoke(v.template cast<value_t::Float   >());
        case value_t::String  : return format_impl<value_t::String  >::invoke(v.template cast<value_t::String  >());
        case value_t::Datetime: return format_impl<value_t::Datetime>::invoke(v.template cast<value_t::Datetime>());
        case value_t::Array   : return format_impl<value_t::Array   >::invoke(v.template cast<value_t::Array   >());
        case value_t::Table   : return format_impl<value_t::Table   >::invoke(v.template cast<value_t::Table   >());
        case value_t::Empty   : return format_impl<value_t::Empty   >::invoke(v.template cast<value_t::Empty   >());
        case value_t::Unknown : return format_impl<value_t::Unknown >::invoke(v.template cast<value_t::Unknown >());
        default throw std::logic_error("toml::format: unknown enum value");
    }
}

template<typename charT = charactor, typename traits = std::char_traits<charT>,
         typename alloc = std::allocator<charT>>
std::basic_string<charT, traits, alloc>
format(std::basic_string<charT, traits, alloc>&& key, const value& val)
{
    std::basic_string<charT, traits, alloc> retval(
            std::forward<std::basic_string<charT, traits, alloc>>(key));
    retval += " = ";
    retval += format(val);
    return retval;
}

}
#endif // TOML11_FORMAT

#ifndef TOML11_TYPES_H
#define TOML11_TYPES_H
#include "datetime.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace toml
{

using character = char;

class value;
using key = std::basic_string<character>;

using Boolean  = bool;
using Integer  = std::int64_t;
using Float    = double;
using String   = std::basic_string<character>;
using Datetime = basic_datetime<unsigned int, int>;
using Array    = std::vector<value>;
using Table    = std::unordered_map<key, value>;

// alias for snake_case, consistency with STL/Boost, toml::key, toml::value
using boolean  = Boolean;
using integer  = Integer;
using floating = Float;  // XXX float is keyword
using string   = String;
using datetime = Datetime;
using array    = Array;
using table    = Table;

enum class value_t : std::uint8_t
{
    Boolean  = 1,
    Integer  = 2,
    Float    = 3,
    String   = 4,
    Datetime = 5,
    Array    = 6,
    Table    = 7,
    Empty    = 0,
    Unknown  = 255,
};

template<typename charT = character, typename traits = std::char_traits<charT>>
inline std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean : os << "Boolean";  return os;
        case toml::value_t::Integer : os << "Integer";  return os;
        case toml::value_t::Float   : os << "Float";    return os;
        case toml::value_t::String  : os << "String";   return os;
        case toml::value_t::Datetime: os << "Datetime"; return os;
        case toml::value_t::Array   : os << "Array";    return os;
        case toml::value_t::Table   : os << "Table";    return os;
        case toml::value_t::Empty   : os << "Empty";    return os;
        case toml::value_t::Unknown : os << "Unknown";  return os;
        default                     : os << "Nothing";  return os;
    }
}

template<typename charT = character, typename traits = std::char_traits<charT>,
         typename alloc = std::allocator<charT>>
inline std::basic_string<charT, traits, alloc>
stringize(value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean : return "Boolean";
        case toml::value_t::Integer : return "Integer";
        case toml::value_t::Float   : return "Float";
        case toml::value_t::String  : return "String";
        case toml::value_t::Datetime: return "Datetime";
        case toml::value_t::Array   : return "Array";
        case toml::value_t::Table   : return "Table";
        case toml::value_t::Empty   : return "Empty";
        case toml::value_t::Unknown : return "Unknown";
        default                     : return "Nothing";
    }
}

namespace detail
{

template<typename T>
constexpr inline value_t check_type()
{
    return std::is_same<unwrap_t<T>,        toml::Boolean >::value ? value_t::Boolean :
           std::is_integral<unwrap_t<T>>::value                    ? value_t::Integer :
           std::is_floating_point<unwrap_t<T>>::value              ? value_t::Float   :
           std::is_convertible<unwrap_t<T>, toml::String  >::value ? value_t::String  :
           std::is_convertible<unwrap_t<T>, toml::Datetime>::value ? value_t::Datetime:
           std::is_convertible<unwrap_t<T>, toml::Array   >::value ? value_t::Array   :
           std::is_convertible<unwrap_t<T>, toml::Table   >::value ? value_t::Table   :
           value_t::Unknown;
}

constexpr inline bool is_valid(value_t vt)
{
    return vt != value_t::Unknown;
}

template<value_t t> struct toml_default_type{};
template<> struct toml_default_type<value_t::Boolean >{typedef Boolean  type;};
template<> struct toml_default_type<value_t::Integer >{typedef Integer  type;};
template<> struct toml_default_type<value_t::Float   >{typedef Float    type;};
template<> struct toml_default_type<value_t::String  >{typedef String   type;};
template<> struct toml_default_type<value_t::Datetime>{typedef Datetime type;};
template<> struct toml_default_type<value_t::Array   >{typedef Array    type;};
template<> struct toml_default_type<value_t::Table   >{typedef Table    type;};
template<> struct toml_default_type<value_t::Empty   >{typedef void     type;};
template<> struct toml_default_type<value_t::Unknown >{typedef void     type;};

template<typename T>
struct is_exact_toml_type : disjunction<
    std::is_same<T, Boolean >,
    std::is_same<T, Integer >,
    std::is_same<T, Float   >,
    std::is_same<T, String  >,
    std::is_same<T, Datetime>,
    std::is_same<T, Array   >,
    std::is_same<T, Table   >
    >{};

template<typename T>
struct is_map : conjunction<
    has_iterator<T>,
    has_value_type<T>,
    has_key_type<T>,
    has_mapped_type<T>
    >{};

template<typename T>
struct is_container : conjunction<
    negation<is_map<T>>,
    negation<std::is_same<T, String>>,
    has_iterator<T>,
    has_value_type<T>
    >{};

} // detail
} // toml
#endif// TOML11_TYPES_H

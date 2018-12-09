#ifndef TOML11_TYPES_H
#define TOML11_TYPES_H
#include "datetime.hpp"
#include "traits.hpp"
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
using Datetime = offset_datetime;
using Array    = std::vector<value>;
using Table    = std::unordered_map<key, value>;

// alias for snake_case, consistency with STL/Boost, toml::key, toml::value
using boolean  = Boolean;
using integer  = Integer;
using floating = Float;  // XXX float is keyword
using string   = String;
// these are defined in datetime.hpp
// offset_datetime
// local_datetime
// local_date
// local_time
using array    = Array;
using table    = Table;

enum class value_t : std::uint8_t
{
    Empty          = 0,
    Boolean        = 1,
    Integer        = 2,
    Float          = 3,
    String         = 4,
    OffsetDatetime = 5,
    LocalDatetime  = 6,
    LocalDate      = 7,
    LocalTime      = 8,
    Array          = 9,
    Table          = 10,
    Unknown        = 255,
};

constexpr inline bool is_valid(value_t vt)
{
    return vt != value_t::Unknown;
}

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean      : os << "boolean";         return os;
        case toml::value_t::Integer      : os << "integer";         return os;
        case toml::value_t::Float        : os << "float";           return os;
        case toml::value_t::String       : os << "string";          return os;
        case toml::value_t::Datetime     : os << "offset_datetime"; return os;
        case toml::value_t::LocalDatetime: os << "local_datetime";  return os;
        case toml::value_t::LocalDate    : os << "local_date";      return os;
        case toml::value_t::LocalTime    : os << "local_time";      return os;
        case toml::value_t::Array        : os << "array";           return os;
        case toml::value_t::Table        : os << "table";           return os;
        case toml::value_t::Empty        : os << "empty";           return os;
        case toml::value_t::Unknown      : os << "unknown";         return os;
        default                          : os << "nothing";         return os;
    }
}

template<typename charT = character, typename traits = std::char_traits<charT>,
         typename alloc = std::allocator<charT>>
inline std::basic_string<charT, traits, alloc> stringize(value_t t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
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

template<value_t t> struct toml_default_type;
template<> struct toml_default_type<value_t::Boolean >     {typedef Boolean         type;};
template<> struct toml_default_type<value_t::Integer >     {typedef Integer         type;};
template<> struct toml_default_type<value_t::Float   >     {typedef Float           type;};
template<> struct toml_default_type<value_t::String  >     {typedef String          type;};
template<> struct toml_default_type<value_t::Datetime>     {typedef offset_datetime type;};
template<> struct toml_default_type<value_t::LocalDatetime>{typedef local_datetime  type;};
template<> struct toml_default_type<value_t::LocalDate>    {typedef local_date      type;};
template<> struct toml_default_type<value_t::LocalTime>    {typedef local_time      type;};
template<> struct toml_default_type<value_t::Array   >     {typedef Array           type;};
template<> struct toml_default_type<value_t::Table   >     {typedef Table           type;};
template<> struct toml_default_type<value_t::Empty   >     {typedef void            type;};
template<> struct toml_default_type<value_t::Unknown >     {typedef void            type;};

template<typename T>
struct is_exact_toml_type : disjunction<
    std::is_same<T, Boolean >,
    std::is_same<T, Integer >,
    std::is_same<T, Float   >,
    std::is_same<T, String  >,
    std::is_same<T, offset_datetime>,
    std::is_same<T, local_datetime>,
    std::is_same<T, local_date>,
    std::is_same<T, local_time>,
    std::is_same<T, Array   >,
    std::is_same<T, Table   >
    >{};
template<typename T> struct is_exact_toml_type<T&>               : is_exact_toml_type<T>{};
template<typename T> struct is_exact_toml_type<T const&>         : is_exact_toml_type<T>{};
template<typename T> struct is_exact_toml_type<T volatile&>      : is_exact_toml_type<T>{};
template<typename T> struct is_exact_toml_type<T const volatile&>: is_exact_toml_type<T>{};

template<typename T>
struct is_map : conjunction<
    has_iterator<T>,
    has_value_type<T>,
    has_key_type<T>,
    has_mapped_type<T>
    >{};
template<typename T> struct is_map<T&>                : is_map<T>{};
template<typename T> struct is_map<T const&>          : is_map<T>{};
template<typename T> struct is_map<T volatile&>       : is_map<T>{};
template<typename T> struct is_map<T const volatile&> : is_map<T>{};

template<typename T>
struct is_container : conjunction<
    negation<is_map<T>>,
    negation<std::is_same<T, std::string>>,
    has_iterator<T>,
    has_value_type<T>
    >{};
template<typename T> struct is_container<T&>                : is_container<T>{};
template<typename T> struct is_container<T const&>          : is_container<T>{};
template<typename T> struct is_container<T volatile&>       : is_container<T>{};
template<typename T> struct is_container<T const volatile&> : is_container<T>{};


} // detail
} // toml
#endif// TOML11_TYPES_H

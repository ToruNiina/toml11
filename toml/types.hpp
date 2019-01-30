//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_TYPES_H
#define TOML11_TYPES_H
#include "datetime.hpp"
#include "string.hpp"
#include "traits.hpp"
#include <vector>
#include <unordered_map>

namespace toml
{

using character = char;

class value;
using key = std::string;

using Boolean        = bool;
using Integer        = std::int64_t;
using Float          = double;
using String         = ::toml::string;
using Datetime       = offset_datetime;
using OffsetDatetime = offset_datetime;
using LocalDatetime  = local_datetime;
using LocalDate      = local_date;
using LocalTime      = local_time;
using Array          = std::vector<value>;
using Table          = std::unordered_map<key, value>;

// alias for snake_case, consistency with STL/Boost, toml::key, toml::value
using boolean  = Boolean;
using integer  = Integer;
using floating = Float;   // XXX `float` is keyword. we can't use it here
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

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean       : os << "boolean";         return os;
        case toml::value_t::Integer       : os << "integer";         return os;
        case toml::value_t::Float         : os << "float";           return os;
        case toml::value_t::String        : os << "string";          return os;
        case toml::value_t::OffsetDatetime: os << "offset_datetime"; return os;
        case toml::value_t::LocalDatetime : os << "local_datetime";  return os;
        case toml::value_t::LocalDate     : os << "local_date";      return os;
        case toml::value_t::LocalTime     : os << "local_time";      return os;
        case toml::value_t::Array         : os << "array";           return os;
        case toml::value_t::Table         : os << "table";           return os;
        case toml::value_t::Empty         : os << "empty";           return os;
        case toml::value_t::Unknown       : os << "unknown";         return os;
        default                           : os << "nothing";         return os;
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
    using type = typename std::remove_cv<
                 typename std::remove_reference<T>::type
                 >::type;
    return std::is_same<type, toml::boolean>::value                         ? value_t::Boolean        :
           std::is_integral<type>::value                                    ? value_t::Integer        :
           std::is_floating_point<type>::value                              ? value_t::Float          :
           std::is_same<type, std::string>::value                           ? value_t::String         :
           std::is_same<type, toml::string>::value                          ? value_t::String         :
           std::is_same<type, toml::local_date>::value                      ? value_t::LocalDate      :
           std::is_same<type, toml::local_time>::value                      ? value_t::LocalTime      :
           is_chrono_duration<type>::value                                  ? value_t::LocalTime      :
           std::is_same<type, toml::local_datetime>::value                  ? value_t::LocalDatetime  :
           std::is_same<type, toml::offset_datetime>::value                 ? value_t::OffsetDatetime :
           std::is_same<type, std::chrono::system_clock::time_point>::value ? value_t::OffsetDatetime :
           std::is_convertible<type, toml::array>::value                    ? value_t::Array          :
           std::is_convertible<type, toml::table>::value                    ? value_t::Table          :
           value_t::Unknown;
}

constexpr inline bool is_valid(value_t vt)
{
    return vt != value_t::Unknown;
}

template<value_t t> struct toml_default_type;
template<> struct toml_default_type<value_t::Boolean >      {typedef boolean         type;};
template<> struct toml_default_type<value_t::Integer >      {typedef integer         type;};
template<> struct toml_default_type<value_t::Float   >      {typedef floating        type;};
template<> struct toml_default_type<value_t::String  >      {typedef string          type;};
template<> struct toml_default_type<value_t::OffsetDatetime>{typedef offset_datetime type;};
template<> struct toml_default_type<value_t::LocalDatetime> {typedef local_datetime  type;};
template<> struct toml_default_type<value_t::LocalDate>     {typedef local_date      type;};
template<> struct toml_default_type<value_t::LocalTime>     {typedef local_time      type;};
template<> struct toml_default_type<value_t::Array   >      {typedef array           type;};
template<> struct toml_default_type<value_t::Table   >      {typedef table           type;};
template<> struct toml_default_type<value_t::Empty   >      {typedef void            type;};
template<> struct toml_default_type<value_t::Unknown >      {typedef void            type;};

template<typename T> struct toml_value_t      {static constexpr value_t value = value_t::Unknown       ;};
template<> struct toml_value_t<Boolean       >{static constexpr value_t value = value_t::Boolean       ;};
template<> struct toml_value_t<Integer       >{static constexpr value_t value = value_t::Integer       ;};
template<> struct toml_value_t<Float         >{static constexpr value_t value = value_t::Float         ;};
template<> struct toml_value_t<String        >{static constexpr value_t value = value_t::String        ;};
template<> struct toml_value_t<OffsetDatetime>{static constexpr value_t value = value_t::OffsetDatetime;};
template<> struct toml_value_t<LocalDatetime >{static constexpr value_t value = value_t::LocalDatetime ;};
template<> struct toml_value_t<LocalDate     >{static constexpr value_t value = value_t::LocalDate     ;};
template<> struct toml_value_t<LocalTime     >{static constexpr value_t value = value_t::LocalTime     ;};
template<> struct toml_value_t<Array         >{static constexpr value_t value = value_t::Array         ;};
template<> struct toml_value_t<Table         >{static constexpr value_t value = value_t::Table         ;};

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

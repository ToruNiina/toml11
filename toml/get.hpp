#ifndef TOML11_GET
#define TOML11_GET
#include "value.hpp"
#include <algorithm>

namespace toml
{

template<typename T, typename std::enable_if<
    detail::is_exact_toml_type<T>::value, std::nullptr_t>::type = nullptr>
inline T& get(value& v)
{
    constexpr value_t kind = detail::check_type<T>();
    return v.cast<kind>();
}

template<typename T, typename std::enable_if<
    detail::is_exact_toml_type<T>::value, std::nullptr_t>::type = nullptr>
inline T const& get(const value& v)
{
    constexpr value_t kind = detail::check_type<T>();
    return v.cast<kind>();
}

template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>,
    detail::negation<std::is_same<T, bool>>, std::is_integral<T>
    >::value, std::nullptr_t>::type = nullptr>
inline T get(const value& v)
{
    return static_cast<T>(v.cast<value_t::Integer>());
}
template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, std::is_floating_point<T>
    >::value, std::nullptr_t>::type = nullptr>
inline T get(const value& v)
{
    return static_cast<T>(v.cast<value_t::Float>());
}

// array-like type
template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, detail::is_container<T>
    >::value, std::nullptr_t>::type = nullptr>
T get(const value& v)
{
    const auto& ar = v.cast<value_t::Array>();
    T tmp;
    try
    {
        ::toml::resize(tmp, ar.size());
    }
    catch(std::invalid_argument& iv)
    {
        throw type_error("toml::get: static array: size is not enough");
    }
    std::transform(ar.cbegin(), ar.cend(), tmp.begin(),
        [](value const& elem){return get<typename T::value_type>(elem);});
    return tmp;
}

// table-like case
template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, detail::is_map<T>
    >::value, std::nullptr_t>::type = nullptr>
T get(const toml::value& v)
{
    const auto& tb = v.cast<value_t::Table>();
    T tmp;
    for(const auto& kv : tb){tmp.insert(kv);}
    return tmp;
}

//  get_or -----------------------------------------------------------------

template<typename T>
inline typename std::remove_cv<typename std::remove_reference<T>::type>::type
get_or(const toml::Table& tab, const toml::key& ky, T&& opt)
{
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get<typename std::remove_cv<
        typename std::remove_reference<T>::type>::type>(tab.find(ky)->second);
}

} // toml
#endif// TOML11_GET

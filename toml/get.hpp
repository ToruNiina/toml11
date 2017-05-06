#ifndef TOML11_GET
#define TOML11_GET
#include "value.hpp"
#include <algorithm>

namespace toml
{

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
inline T get(const toml::value& v)
{
    return static_cast<T>(v.cast<vT>());
}

// array case
template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
T get(const toml::value& v)
{
    if(v.type() != value_t::Array)
        throw type_error("get: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Array"));
    const auto& ar = v.cast<value_t::Array>();
    T tmp;
    try
    {
        toml::resize(tmp, ar.size());
    }
    catch(std::invalid_argument& iv)
    {
        throw toml::type_error("toml::get: static array size is not enough");
    }
    std::transform(ar.cbegin(), ar.cend(), tmp.begin(),
        [](toml::value const& elem){return get<typename T::value_type>(elem);});
    return tmp;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
T get(const toml::value& v)
{
    if(v.type() != value_t::Table)
        throw type_error("get: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Table"));
    T tmp;
    const auto& tb = v.cast<value_t::Table>();
    for(const auto& kv : tb){tmp.insert(kv);}
    return tmp;
}

} // toml
#endif// TOML11_GET

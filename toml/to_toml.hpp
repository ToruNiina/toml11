#ifndef TOML11_TO_TOML
#define TOML11_TO_TOML
#include "value.hpp"

namespace toml
{

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
inline toml::value to_toml(T&& x)
{
    return toml::value(std::forward<T>(x));
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
toml::value to_toml(T&& x)
{
    toml::Array tmp; tmp.reserve(std::distance(std::begin(x), std::end(x)));
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
        tmp.emplace_back(*iter);
    return toml::value(std::move(tmp));
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
toml::value to_toml(T&& x)
{
    toml::Table tmp;
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
        tmp.emplace(iter->first, to_toml(iter->second));
    return toml::value(std::move(tmp));
}

template<typename T>
inline toml::value to_toml(std::initializer_list<T> init)
{
    return toml::value(std::move(init));
}

inline toml::value
to_toml(std::initializer_list<std::pair<std::string, toml::value>> init)
{
    return toml::value(std::move(init));
}

} // toml
#endif // TOML11_TO_TOML

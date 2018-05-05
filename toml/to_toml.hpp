#ifndef TOML11_TO_TOML
#define TOML11_TO_TOML
#include "value.hpp"

namespace toml
{

template<typename T, typename std::enable_if<
    detail::is_exact_toml_type<T>::value, std::nullptr_t>::type = nullptr>
inline value to_toml(const T& x)
{
    return value(x);
}

template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, std::is_integral<T>
    >::value, std::nullptr_t>::type = nullptr>
inline value to_toml(const T& x)
{
    return value(::toml::Integer(x));
}

template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, std::is_floating_point<T>
    >::value, std::nullptr_t>::type = nullptr>
inline value to_toml(const T& x)
{
    return value(::toml::Float(x));
}

inline value to_toml(const char* str)
{
    return value(::toml::String(str));
}

template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, detail::is_container<T>
    >::value, std::nullptr_t>::type = nullptr>
value to_toml(const T& x)
{
    Array tmp;
    tmp.reserve(std::distance(std::begin(x), std::end(x)));
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
    {
        tmp.emplace_back(*iter);
    }
    return value(std::move(tmp));
}

template<typename T, typename std::enable_if<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<T>>, detail::is_map<T>
    >::value, std::nullptr_t>::type = nullptr>
value to_toml(const T& x)
{
    Table tmp;
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
    {
        tmp.emplace(iter->first, to_toml(iter->second));
    }
    return value(std::move(tmp));
}

template<typename T>
inline value to_toml(std::initializer_list<T> init)
{
    return value(std::move(init));
}

inline value to_toml(std::initializer_list<std::pair<std::string, value>> init)
{
    return value(std::move(init));
}

template<typename T>
inline value to_toml(const value& x)
{
    return x;
}

} // toml
#endif // TOML11_TO_TOML

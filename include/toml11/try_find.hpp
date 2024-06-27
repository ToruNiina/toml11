#ifndef TOML11_TRY_FIND_HPP
#define TOML11_TRY_FIND_HPP

#include <algorithm>

#include "try_get.hpp"
#include "utility.hpp"
#include "value.hpp"

#if defined(TOML11_HAS_STRING_VIEW)
#include <string_view>
#endif

namespace toml
{

// ----------------------------------------------------------------------------
// find<T>(value, key);

template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC> const&>()))
try_find(const basic_value<TC>& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    const auto res = v.try_at(ky);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(res.as_ok());
}

template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC>&>()))
try_find(basic_value<TC>& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    auto res = v.try_at(ky);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(res.as_ok());
}

template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC>&&>()))
try_find(basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    auto res = v.try_at(ky);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(std::move(res.as_ok()));
}

// ----------------------------------------------------------------------------
// find<T>(value, idx)

template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC> const&>()))
try_find(const basic_value<TC>& v, const std::size_t idx) noexcept
{
    const auto res = v.try_at(idx);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(res.as_ok());
}
template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC>&>()))
try_find(basic_value<TC>& v, const std::size_t idx) noexcept
{
    auto res = v.try_at(idx);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(res.as_ok());
}
template<typename T, typename TC>
decltype(try_get<T>(std::declval<basic_value<TC>&&>()))
try_find(basic_value<TC>&& v, const std::size_t idx) noexcept
{
    auto res = v.try_at(idx);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_get<T>(std::move(res.as_ok()));
}

// ----------------------------------------------------------------------------
// find(value, key/idx), w/o conversion

template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<basic_value<TC>>, error_info>>
try_find(basic_value<TC>& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    return v.try_at(ky);
}
template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<const basic_value<TC>>, error_info>>
try_find(basic_value<TC> const& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    return v.try_at(ky);
}
template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<basic_value<TC>, error_info>>
try_find(basic_value<TC>&& v, const typename basic_value<TC>::key_type& ky) noexcept
{
    auto res = v.try_at(ky);
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return ok(basic_value<TC>(std::move(res.as_ok())));
}

template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<basic_value<TC>>, error_info>>
try_find(basic_value<TC>& v, const std::size_t idx) noexcept
{
    return v.try_at(idx);
}
template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<const basic_value<TC>>, error_info>>
try_find(basic_value<TC> const& v, const std::size_t idx) noexcept
{
    return v.try_at(idx);
}
template<typename TC>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<basic_value<TC>, error_info>>
try_find(basic_value<TC>&& v, const std::size_t idx) noexcept
{
    auto res = v.try_at(idx);
    if(res.is_err())
    {
        return err(std::move(res.as_err()));
    }
    return ok(basic_value<TC>(std::move(res.as_ok())));
}

// --------------------------------------------------------------------------
// toml::find(toml::value, toml::key, Ts&& ... keys) w/o conversion

template<typename TC, typename K1, typename K2, typename ... Ks>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<const basic_value<TC>>, error_info>>
try_find(const basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find(res.as_ok(), detail::key_cast<TC>(k2), ks...);
}
template<typename TC, typename K1, typename K2, typename ... Ks>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<std::reference_wrapper<basic_value<TC>>, error_info>>
try_find(basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find(res.as_ok(), detail::key_cast<TC>(k2), ks...);
}
template<typename TC, typename K1, typename K2, typename ... Ks>
cxx::enable_if_t<detail::is_type_config<TC>::value,
    result<basic_value<TC>, error_info>>
try_find(basic_value<TC>&& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find(std::move(res.as_ok()), detail::key_cast<TC>(k2), ks...);
}

// ----------------------------------------------------------------------------
// find<T>(v, keys...)

template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
decltype(::toml::try_get<T>(std::declval<const basic_value<TC>&>()))
try_find(const basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find<T>(res.as_ok(), detail::key_cast<TC>(k2), ks...);
}
template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
decltype(::toml::try_get<T>(std::declval<basic_value<TC>&>()))
try_find(basic_value<TC>& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find<T>(res.as_ok(), detail::key_cast<TC>(k2), ks...);
}
template<typename T, typename TC, typename K1, typename K2, typename ... Ks>
decltype(::toml::try_get<T>(std::declval<basic_value<TC>&&>()))
try_find(basic_value<TC>&& v, const K1& k1, const K2& k2, const Ks& ... ks) noexcept
{
    auto res = v.try_at(detail::key_cast<TC>(k1));
    if(res.is_err())
    {
        return err(res.as_err());
    }
    return try_find<T>(std::move(res.as_ok()), detail::key_cast<TC>(k2), ks...);
}

} // toml
#endif // TOML11_FIND_HPP

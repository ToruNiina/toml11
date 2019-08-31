//     Copyright Toru Niina 2019.
// Distributed under the MIT License.
#ifndef TOML11_FIND_HPP
#define TOML11_FIND_HPP
#include "get.hpp"

namespace toml
{

// ----------------------------------------------------------------------------
// these overloads do not require to set T. and returns value itself.
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V> const& find(const basic_value<C, M, V>& v, const key& ky)
{
    const auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>& find(basic_value<C, M, V>& v, const key& ky)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>&& find(basic_value<C, M, V>&& v, const key& ky)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return std::move(tab.at(ky));
}

// ----------------------------------------------------------------------------
// find<T>(value, key);

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V> const&>()))
find(const basic_value<C, M, V>& v, const key& ky)
{
    const auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&>()))
find(basic_value<C, M, V>& v, const key& ky)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&&>()))
find(basic_value<C, M, V>&& v, const key& ky)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(std::move(tab.at(ky)));
}

// --------------------------------------------------------------------------
// toml::find(toml::value, toml::key, Ts&& ... keys)

template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, const basic_value<C, M, V>&>
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, basic_value<C, M, V>&>
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, basic_value<C, M, V>&&>
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<const basic_value<C, M, V>&>()))>
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<basic_value<C, M, V>&>()))>
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<basic_value<C, M, V>&&>()))>
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

// ===========================================================================
// find_or(value, key, fallback)

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V> const&
find_or(const basic_value<C, M, V>& v, const key& ky,
        const basic_value<C, M, V>& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return tab.at(ky);
}

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, basic_value<C, M, V>& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return tab[ky];
}

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>
find_or(basic_value<C, M, V>&& v, const toml::key& ky, basic_value<C, M, V>&& opt)
{
    if(!v.is_table()) {return opt;}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return opt;}
    return std::move(tab[ky]);
}

// ---------------------------------------------------------------------------
// exact types (return type can be a reference)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> const&
find_or(const basic_value<C, M, V>& v, const key& ky, const T& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, T& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab[ky], opt);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&&
find_or(basic_value<C, M, V>&& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return opt;}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(std::move(tab[ky]), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// std::string (return type can be a reference)

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string> const&
find_or(const basic_value<C, M, V>& v, const key& ky, const T& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, T& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string>
find_or(basic_value<C, M, V>&& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::forward<T>(opt);}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get_or(std::move(tab.at(ky)), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// string literal (deduced as std::string)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_string_literal<typename std::remove_reference<T>::type>::value,
    std::string>
find_or(const basic_value<C, M, V>& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::string(opt);}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return std::string(opt);}
    return get_or(tab.at(ky), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// others (require type conversion and return type cannot be lvalue reference)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    // T is not an exact toml type
    detail::negation<detail::is_exact_toml_type<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        basic_value<C, M, V>>>,
    // T is not std::string
    detail::negation<std::is_same<std::string,
        typename std::remove_cv<typename std::remove_reference<T>::type>::type>>,
    // T is not a string literal
    detail::negation<detail::is_string_literal<
        typename std::remove_reference<T>::type>>
    >::value, typename std::remove_cv<typename std::remove_reference<T>::type>::type>
find_or(const basic_value<C, M, V>& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::forward<T>(opt);}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get_or(tab.at(ky), std::forward<T>(opt));
}

// ============================================================================
// expect

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
result<T, std::string> expect(const basic_value<C, M, V>& v) noexcept
{
    try
    {
        return ok(get<T>(v));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
result<T, std::string>
expect(const basic_value<C, M, V>& v, const toml::key& k) noexcept
{
    try
    {
        return ok(find<T>(v, k));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}
template<typename T, typename Table>
detail::enable_if_t<detail::conjunction<
    detail::is_map<Table>, detail::is_basic_value<typename Table::mapped_type>
    >::value, result<T, std::string>>
expect(const Table& t, const toml::key& k,
       std::string tablename = "unknown table") noexcept
{
    try
    {
        return ok(find<T>(t, k, std::move(tablename)));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}


} // toml
#endif// TOML11_FIND_HPP

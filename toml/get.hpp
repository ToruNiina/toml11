//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_GET_HPP
#define TOML11_GET_HPP
#include "from.hpp"
#include "result.hpp"
#include "value.hpp"
#include <algorithm>

namespace toml
{

// ============================================================================
// exact toml::* type

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> &
get(basic_value<C, M, V>& v)
{
    return v.template cast<detail::type_to_enum<T, basic_value<C, M, V>>::value>();
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> const&
get(const basic_value<C, M, V>& v)
{
    return v.template cast<detail::type_to_enum<T, basic_value<C, M, V>>::value>();
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> &&
get(basic_value<C, M, V>&& v)
{
    return std::move(v).template cast<detail::type_to_enum<T, basic_value<C, M, V>>::value>();
}

// ============================================================================
// T == toml::value; identity transformation.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, basic_value<C, M, V>>::value, T>&
get(basic_value<C, M, V>& v)
{
    return v;
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, basic_value<C, M, V>>::value, T> const&
get(const basic_value<C, M, V>& v)
{
    return v;
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, basic_value<C, M, V>>::value, T> &&
get(basic_value<C, M, V>&& v)
{
    return std::move(v);
}

// ============================================================================
// T == toml::basic_value<C2, M2, V2>; basic_value -> basic_value

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<detail::conjunction<detail::is_basic_value<T>,
    detail::negation<std::is_same<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    return T(v);
}

// ============================================================================
// integer convertible from toml::Integer

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<detail::conjunction<
    std::is_integral<T>,                            // T is integral
    detail::negation<std::is_same<T, bool>>,        // but not bool
    detail::negation<                               // but not toml::integer
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    return static_cast<T>(v.template cast<value_t::integer>());
}

// ============================================================================
// floating point convertible from toml::Float

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<detail::conjunction<
    std::is_floating_point<T>,                      // T is floating_point
    detail::negation<                               // but not toml::floating
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    return static_cast<T>(v.template cast<value_t::floating>());
}

// ============================================================================
// std::string; toml uses its own toml::string, but it should be convertible to
// std::string seamlessly

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, std::string>::value, std::string>&
get(basic_value<C, M, V>& v)
{
    return v.template cast<value_t::string>().str;
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, std::string>::value, std::string> const&
get(const basic_value<C, M, V>& v)
{
    return v.template cast<value_t::string>().str;
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, std::string>::value, std::string> const&
get(basic_value<C, M, V>&& v)
{
    return std::move(v.template cast<value_t::string>().str);
}

// ============================================================================
// std::string_view

#if __cplusplus >= 201703L
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<std::is_same<T, std::string_view>::value, std::string_view>
get(const basic_value<C, M, V>& v)
{
    return std::string_view(v.template cast<value_t::string>().str);
}
#endif

// ============================================================================
// std::chrono::duration from toml::local_time.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<detail::is_chrono_duration<T>::value, T>
get(const basic_value<C, M, V>& v)
{
    return std::chrono::duration_cast<T>(
            std::chrono::nanoseconds(v.template cast<value_t::local_time>()));
}

// ============================================================================
// std::chrono::system_clock::time_point from toml::datetime variants

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
inline detail::enable_if_t<
    std::is_same<std::chrono::system_clock::time_point, T>::value, T>
get(const basic_value<C, M, V>& v)
{
    switch(v.type())
    {
        case value_t::local_date:
        {
            return std::chrono::system_clock::time_point(
                    v.template cast<value_t::local_date>());
        }
        case value_t::local_datetime:
        {
            return std::chrono::system_clock::time_point(
                    v.template cast<value_t::local_datetime>());
        }
        case value_t::offset_datetime:
        {
            return std::chrono::system_clock::time_point(
                    v.template cast<value_t::offset_datetime>());
        }
        default:
        {
            throw type_error(detail::format_underline("[error] toml::value "
                "bad_cast to std::chrono::system_clock::time_point", {
                    {std::addressof(detail::get_region(v)),
                     concat_to_string("the actual type is ", v.type())}
                }));
        }
    }
}

// ============================================================================
// forward declaration to use this recursively. ignore this and go ahead.

// array-like type with resize(N) method
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_container<T>,      // T is container
    detail::has_resize_method<T>, // T::resize(N) works
    detail::negation<             // but not toml::array
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>&);

// array-like type with resize(N) method
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_container<T>,                        // T is container
    detail::negation<detail::has_resize_method<T>>, // no T::resize() exists
    detail::negation<                               // not toml::array
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>&);

// std::pair<T1, T2>
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_std_pair<T>::value, T>
get(const basic_value<C, M, V>&);

// std::tuple<T1, T2, ...>
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_std_tuple<T>::value, T>
get(const basic_value<C, M, V>&);

// map-like classes
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_map<T>, // T is map
    detail::negation<  // but not toml::table
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>&);

// T.from_toml(v)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::negation<                         // not a toml::* type
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>,
    detail::has_from_toml_method<T, C, M, V>, // but has from_toml(toml::value)
    std::is_default_constructible<T>          // and default constructible
    >::value, T>
get(const basic_value<C, M, V>&);

// toml::from<T>::from_toml(v)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         std::size_t S = sizeof(::toml::into<T>)>
T get(const basic_value<C, M, V>&);

// ============================================================================
// array-like types; most likely STL container, like std::vector, etc.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_container<T>,      // T is container
    detail::has_resize_method<T>, // T::resize(N) works
    detail::negation<             // but not toml::array
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    using value_type = typename T::value_type;
    const auto& ar = v.template cast<value_t::array>();
    T container;
    container.resize(ar.size());
    std::transform(ar.cbegin(), ar.cend(), container.begin(),
                   [](const value& x){return ::toml::get<value_type>(x);});
    return container;
}

// ============================================================================
// array-like types; but does not have resize(); most likely std::array.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_container<T>,                        // T is container
    detail::negation<detail::has_resize_method<T>>, // no T::resize() exists
    detail::negation<                              // but not toml::array
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    using value_type = typename T::value_type;
    const auto& ar = v.template cast<value_t::array>();

    T container;
    if(ar.size() != container.size())
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[erorr] toml::get specified container size is ", container.size(),
            " but there are ", ar.size(), " elements in toml array."), {
                {std::addressof(detail::get_region(v)), "here"}
            }));
    }
    std::transform(ar.cbegin(), ar.cend(), container.begin(),
                   [](const value& x){return ::toml::get<value_type>(x);});
    return container;
}

// ============================================================================
// std::pair.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_std_pair<T>::value, T>
get(const basic_value<C, M, V>& v)
{
    using first_type  = typename T::first_type;
    using second_type = typename T::second_type;

    const auto& ar = v.template cast<value_t::array>();
    if(ar.size() != 2)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[erorr] toml::get specified std::pair but there are ", ar.size(),
            " elements in toml array."), {
                {std::addressof(detail::get_region(v)), "here"}
            }));
    }
    return std::make_pair(::toml::get<first_type >(ar.at(0)),
                          ::toml::get<second_type>(ar.at(1)));
}

// ============================================================================
// std::tuple.

namespace detail
{
template<typename T, typename Array, std::size_t ... I>
T get_tuple_impl(const Array& a, index_sequence<I...>)
{
    return std::make_tuple(
        ::toml::get<typename std::tuple_element<I, T>::type>(a.at(I))...);
}
} // detail

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_std_tuple<T>::value, T>
get(const basic_value<C, M, V>& v)
{
    const auto& ar = v.template cast<value_t::array>();
    if(ar.size() != std::tuple_size<T>::value)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[erorr] toml::get specified std::tuple with ",
            std::tuple_size<T>::value, "elements, but there are ", ar.size(),
            " elements in toml array."), {
                {std::addressof(detail::get_region(v)), "here"}
            }));
    }
    return detail::get_tuple_impl<T>(ar,
            detail::make_index_sequence<std::tuple_size<T>::value>{});
}

// ============================================================================
// map-like types; most likely STL map, like std::map or std::unordered_map.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::is_map<T>, // T is map
    detail::negation<  // but not toml::array
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    using key_type    = typename T::key_type;
    using mapped_type = typename T::mapped_type;
    static_assert(std::is_convertible<std::string, key_type>::value,
                  "toml::get only supports map type of which key_type is "
                  "convertible from std::string.");
    T map;
    for(const auto& kv : v.template cast<value_t::table>())
    {
        map[key_type(kv.first)] = ::toml::get<mapped_type>(kv.second);
    }
    return map;
}

// ============================================================================
// user-defined, but compatible types.

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::negation<                         // not a toml::* type
        detail::is_exact_toml_type<T, basic_value<C, M, V>>>,
    detail::has_from_toml_method<T, C, M, V>, // but has from_toml(toml::value) memfn
    std::is_default_constructible<T>          // and default constructible
    >::value, T>
get(const basic_value<C, M, V>& v)
{
    T ud;
    ud.from_toml(v);
    return ud;
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         std::size_t>
T get(const basic_value<C, M, V>& v)
{
    return ::toml::from<T>::from_toml(v);
}

// ============================================================================
// find and get

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
const basic_value<C, M, V>&
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
basic_value<C, M, V>&
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
basic_value<C, M, V>&&
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
decltype(::toml::get<T>(std::declval<const basic_value<C, M, V>&>()))
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&>()))
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&&>()))
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

// ============================================================================
// get_or(value, fallback)

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V> const&
get_or(const basic_value<C, M, V>& v, const basic_value<C, M, V>&)
{
    return v;
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>&
get_or(basic_value<C, M, V>& v, basic_value<C, M, V>&)
{
    return v;
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>
get_or(basic_value<C, M, V>&& v, basic_value<C, M, V>&&)
{
    return v;
}

// ----------------------------------------------------------------------------
// specialization for the exact toml types (return type becomes lvalue ref)

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> const&
get_or(const basic_value<C, M, V>& v, const T& opt)
{
    try
    {
        return get<typename std::remove_cv<
            typename std::remove_reference<T>::type>::type>(v);
    }
    catch(...)
    {
        return opt;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&
get_or(basic_value<C, M, V>& v, T& opt)
{
    try
    {
        return get<typename std::remove_cv<
            typename std::remove_reference<T>::type>::type>(v);
    }
    catch(...)
    {
        return opt;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&&
get_or(basic_value<C, M, V>&& v, T&& opt)
{
    try
    {
        return get<typename std::remove_cv<
            typename std::remove_reference<T>::type>::type>(v);
    }
    catch(...)
    {
        return opt;
    }
}

// ----------------------------------------------------------------------------
// specialization for std::string (return type becomes lvalue ref)

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type,
    std::string>::value, std::string> const&
get_or(const basic_value<C, M, V>& v, T&& opt)
{
    try
    {
        return v.template cast<value_t::string>().str;
    }
    catch(...)
    {
        return opt;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string>&
get_or(basic_value<C, M, V>& v, T& opt)
{
    try
    {
        return v.template cast<value_t::string>().str;
    }
    catch(...)
    {
        return opt;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type,
    std::string>::value, std::string>
get_or(basic_value<C, M, V>&& v, T&& opt)
{
    try
    {
        return std::move(v.template cast<value_t::string>().str);
    }
    catch(...)
    {
        return std::forward<T>(opt);
    }
}

// ----------------------------------------------------------------------------
// specialization for string literal

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::is_string_literal<
    typename std::remove_reference<T>::type>::value, std::string>
get_or(const basic_value<C, M, V>& v, T&& opt)
{
    try
    {
        return std::move(v.template cast<value_t::string>().str);
    }
    catch(...)
    {
        return std::string(opt);
    }
}

// ----------------------------------------------------------------------------
// others (require type conversion and return type cannot be lvalue reference)

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        basic_value<C, M, V>>>,
    detail::negation<std::is_same<std::string,
        typename std::remove_cv<typename std::remove_reference<T>::type>::type>>,
    detail::negation<detail::is_string_literal<
        typename std::remove_reference<T>::type>>
    >::value, typename std::remove_reference<T>::type>
get_or(const basic_value<C, M, V>& v, T&& opt)
{
    try
    {
        return get<typename std::remove_cv<
            typename std::remove_reference<T>::type>::type>(v);
    }
    catch(...)
    {
        return std::forward<T>(opt);
    }
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
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return std::string(opt);}
    return get_or(tab.at(ky), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// others (require type conversion and return type cannot be lvalue reference)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    detail::negation<detail::is_exact_toml_type<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        basic_value<C, M, V>>>,
    detail::negation<std::is_same<std::string,
        typename std::remove_cv<typename std::remove_reference<T>::type>::type>>,
    detail::negation<detail::is_string_literal<
        typename std::remove_reference<T>::type>>
    >::value, T>
find_or(const basic_value<C, M, V>& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
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
#endif// TOML11_GET

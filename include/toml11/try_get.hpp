#ifndef TOML11_TRY_GET_HPP
#define TOML11_TRY_GET_HPP

#include <algorithm>

#include "error_info.hpp"
#include "from.hpp"
#include "types.hpp"
#include "value.hpp"

#if defined(TOML11_HAS_STRING_VIEW)
#include <string_view>
#endif // string_view

namespace toml
{

// ============================================================================
// T is toml::value; identity transformation.

template<typename T, typename TC>
cxx::enable_if_t<std::is_same<T, basic_value<TC>>::value,
    result<std::reference_wrapper<T>, error_info>>
try_get(basic_value<TC>& v) noexcept
{
    return ok(std::ref(v));
}

template<typename T, typename TC>
cxx::enable_if_t<std::is_same<T, basic_value<TC>>::value,
    result<std::reference_wrapper<const T>, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    return ok(std::cref(v));
}

template<typename T, typename TC>
cxx::enable_if_t<std::is_same<T, basic_value<TC>>::value,
    result<T, error_info>>
try_get(basic_value<TC>&& v) noexcept
{
    return ok(basic_value<TC>(std::move(v)));
}

// ============================================================================
// exact toml::* type

template<typename T, typename TC>
cxx::enable_if_t<detail::is_exact_toml_type<T, basic_value<TC>>::value,
    result<std::reference_wrapper<T>, error_info>>
try_get(basic_value<TC>& v) noexcept
{
    constexpr auto ty = detail::type_to_enum<T, basic_value<TC>>::value;
    if(v.type() == ty)
    {
        return ok(std::ref(detail::getter<TC, ty>::get_nothrow(v)));
    }
    else
    {
        return err(detail::make_type_error(v, "toml::try_get()", ty));
    }
}

template<typename T, typename TC>
cxx::enable_if_t<detail::is_exact_toml_type<T, basic_value<TC>>::value,
    result<std::reference_wrapper<const T>, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    constexpr auto ty = detail::type_to_enum<T, basic_value<TC>>::value;
    if(v.type() == ty)
    {
        return ok(std::cref(detail::getter<TC, ty>::get_nothrow(v)));
    }
    else
    {
        return err(detail::make_type_error(v, "toml::try_get()", ty));
    }
}

template<typename T, typename TC>
cxx::enable_if_t<detail::is_exact_toml_type<T, basic_value<TC>>::value,
    result<T, error_info>>
try_get(basic_value<TC>&& v) noexcept
{
    constexpr auto ty = detail::type_to_enum<T, basic_value<TC>>::value;
    if(v.type() == ty)
    {
        return ok(detail::getter<TC, ty>::get_nothrow(std::move(v)));
    }
    else
    {
        return err(detail::make_type_error(v, "toml::try_get()", ty));
    }
}

// ============================================================================
// T is toml::basic_value<U>

template<typename T, typename TC>
cxx::enable_if_t<cxx::conjunction<
        detail::is_basic_value<T>,
        cxx::negation<std::is_same<T, basic_value<TC>>>
    >::value, result<T, error_info>>
try_get(basic_value<TC> v) noexcept
{
    return ok(T(std::move(v)));
}

// ============================================================================
// integer convertible from toml::value::integer_type

template<typename T, typename TC>
cxx::enable_if_t<cxx::conjunction<
        std::is_integral<T>,
        cxx::negation<std::is_same<T, bool>>,
        detail::is_not_toml_type<T, basic_value<TC>>,
        cxx::negation<detail::has_from_toml_method<T, TC>>,
        cxx::negation<detail::has_specialized_from<T>>
    >::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.is_integer())
    {
        return ok(static_cast<T>(v.as_integer(std::nothrow)));
    }
    else
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::integer));
    }
}

// ============================================================================
// floating point convertible from toml::value::floating_type

template<typename T, typename TC>
cxx::enable_if_t<cxx::conjunction<
        std::is_floating_point<T>,
        detail::is_not_toml_type<T, basic_value<TC>>,
        cxx::negation<detail::has_from_toml_method<T, TC>>,
        cxx::negation<detail::has_specialized_from<T>>
    >::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.is_floating())
    {
        return ok(static_cast<T>(v.as_floating(std::nothrow)));
    }
    else
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::floating));
    }
}

// ============================================================================
// std::string_view

#if defined(TOML11_HAS_STRING_VIEW)

template<typename T, typename TC>
cxx::enable_if_t<std::is_same<T, std::string_view>::value,
    result<std::string_view, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.is_string())
    {
        return ok(std::string_view(v.as_string(std::nothrow)));
    }
    else
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::string));
    }
}

#endif // string_view

// ============================================================================
// std::chrono::duration from toml::local_time

template<typename T, typename TC>
cxx::enable_if_t<detail::is_chrono_duration<T>::value,
    result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.is_local_time())
    {
        return ok(std::chrono::duration_cast<T>(
                std::chrono::nanoseconds(v.as_local_time(std::nothrow))));
    }
    else
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::local_time));
    }
}

// ============================================================================
// std::chrono::system_clock::time_point from toml::datetime variants

template<typename T, typename TC>
cxx::enable_if_t<std::is_same<std::chrono::system_clock::time_point, T>::value,
    result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    switch(v.type())
    {
        case value_t::local_date:
        {
            return ok(std::chrono::system_clock::time_point(v.as_local_date(std::nothrow)));
        }
        case value_t::local_datetime:
        {
            return ok(std::chrono::system_clock::time_point(v.as_local_datetime(std::nothrow)));
        }
        case value_t::offset_datetime:
        {
            return ok(std::chrono::system_clock::time_point(v.as_offset_datetime(std::nothrow)));
        }
        default:
        {
            const auto loc = v.location();
            return err(make_error_info(error_kind::type_error, "toml::try_get(): bad_cast to "
                "std::chrono::system_clock::time_point", loc,
                "the actual type is " + to_string(v.type())));
        }
    }
}

// ============================================================================
// array-like types; most likely STL container, like std::vector, etc.

template<typename T, typename TC>
cxx::enable_if_t<cxx::conjunction<
        detail::is_container<T>,                            // T is a container
        detail::has_push_back_method<T>,                    // .push_back() works
        detail::is_not_toml_type<T, basic_value<TC>>,       // but not toml::array
        cxx::negation<detail::has_from_toml_method<T, TC>>, // no T.from_toml()
        cxx::negation<detail::has_specialized_from<T>>,     // no toml::from<T>
        cxx::negation<std::is_constructible<T, const basic_value<TC>&>>
    >::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.type() != toml::value_t::array)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::array));
    }

    using value_type = typename T::value_type;
    const auto& a = v.as_array(std::nothrow);

    T container;
    detail::try_reserve(container, a.size()); // if T has .reserve(), call it

    for(const auto& elem : a)
    {
        auto converted = try_get<value_type>(elem);
        if(converted.is_err())
        {
            return err(converted.as_err());
        }
        container.push_back(std::move(converted.as_ok()));
    }
    return ok(container);
}

// ============================================================================
// std::array

template<typename T, typename TC>
cxx::enable_if_t<detail::is_std_array<T>::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.type() != toml::value_t::array)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::array));
    }

    using value_type = typename T::value_type;
    const auto& a = v.as_array(std::nothrow);

    T container;
    if(a.size() != container.size())
    {
        const auto loc = v.location();
        return err(make_error_info(error_kind::out_of_range, "toml::try_get: while converting to an array: "
            " array size is " + std::to_string(container.size()) +
            " but there are " + std::to_string(a.size()) + " elements in toml array.",
            loc, "here"));
    }
    for(std::size_t i=0; i<a.size(); ++i)
    {
        auto converted = try_get<value_type>(a[i]);
        if(converted.is_err())
        {
            return err(converted.as_err());
        }
        container[i] = std::move(converted.as_ok());
    }
    return ok(container);
}

// ============================================================================
// std::forward_list

template<typename T, typename TC>
cxx::enable_if_t<detail::is_std_forward_list<T>::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.type() != toml::value_t::array)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::array));
    }

    using value_type = typename T::value_type;

    T container;
    for(const auto& elem : v.as_array(std::nothrow))
    {
        auto converted = try_get<value_type>(elem);
        if(converted.is_err())
        {
            return err(converted.as_err());
        }
        container.push_front(std::move(converted.as_ok()));
    }
    container.reverse();
    return ok(container);
}

// ============================================================================
// std::pair

template<typename T, typename TC>
cxx::enable_if_t<detail::is_std_pair<T>::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.type() != toml::value_t::array)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::array));
    }

    using first_type  = typename T::first_type;
    using second_type = typename T::second_type;

    const auto& ar = v.as_array(std::nothrow);
    if(ar.size() != 2)
    {
        const auto loc = v.location();
        return err(make_error_info(error_kind::out_of_range, "toml::try_get: while converting std::pair: "
            " but there are " + std::to_string(ar.size()) +
            " > 2 elements in toml array.",
            loc, "here"));
    }

    auto first_result = try_get<first_type>(ar[0]);
    if(first_result.is_err())
    {
        return err(first_result.as_err());
    }
    auto second_result = try_get<second_type>(ar[1]);
    if(second_result.is_err())
    {
        return err(second_result.as_err());
    }
    return ok(std::make_pair(std::move(first_result.as_ok()),
                             std::move(second_result.as_ok())));
}

// ============================================================================
// std::tuple.

namespace detail
{
template<typename T, std::size_t I, std::size_t N>
struct try_get_tuple_impl
{
    template<typename Array, typename U>
    static result<T, error_info> invoke(const Array& a, U curr) noexcept
    {
        assert(I < a.size());
        using value_type = typename std::tuple_element<I, T>::type;

        auto converted = try_get<value_type>(a[I]);
        if(converted.is_err())
        {
            return err(converted.as_err());
        }
        return try_get_tuple_impl<T, I+1, N>::invoke(a, std::tuple_cat(
            std::move(curr), std::make_tuple(std::move(converted.as_ok()))));
    }
};
template<typename T, std::size_t I>
struct try_get_tuple_impl<T, I, I>
{
    template<typename Array>
    static result<T, error_info> invoke(const Array&, T x) noexcept
    {
        return ok(std::move(x));
    }
};
} // detail

template<typename T, typename TC>
cxx::enable_if_t<detail::is_std_tuple<T>::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    if(v.type() != toml::value_t::array)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::array));
    }

    const auto& ar = v.as_array(std::nothrow);
    if(ar.size() != std::tuple_size<T>::value)
    {
        const auto loc = v.location();
        return err(make_error_info(error_kind::out_of_range,
            "toml::try_get: while converting std::tuple: "
            " there are " + std::to_string(ar.size()) + " > " +
            std::to_string(std::tuple_size<T>::value) + " elements in toml array.",
            loc, "here"));
    }
    return detail::try_get_tuple_impl<T, 0, std::tuple_size<T>::value>::invoke(
            ar, std::make_tuple());
}

// ============================================================================
// map-like types; most likely STL map, like std::map or std::unordered_map.

template<typename T, typename TC>
cxx::enable_if_t<cxx::conjunction<
        detail::is_map<T>,                                  // T is map
        detail::is_not_toml_type<T, basic_value<TC>>,       // but not toml::table
        cxx::negation<detail::has_from_toml_method<T, TC>>, // no T.from_toml()
        cxx::negation<detail::has_specialized_from<T>>,     // no toml::from<T>
        cxx::negation<std::is_constructible<T, const basic_value<TC>&>>
    >::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    using key_type    = typename T::key_type;
    using mapped_type = typename T::mapped_type;
    static_assert(
        std::is_convertible<typename basic_value<TC>::key_type, key_type>::value,
        "toml::get only supports map type of which key_type is "
        "convertible from toml::basic_value::key_type.");

    if(v.type() != toml::value_t::table)
    {
        return err(detail::make_type_error(v, "try_get()", toml::value_t::table));
    }

    T m;
    for(const auto& kv : v.as_table(std::nothrow))
    {
        auto converted = try_get<mapped_type>(kv.second);
        if(converted.is_err())
        {
            return err(converted.as_err());
        }
        m.emplace(key_type(kv.first), std::move(converted.as_ok()));
    }
    return ok(m);
}

// ============================================================================
// user-defined type that defines `try_from<T>`.

template<typename T, typename TC>
cxx::enable_if_t<detail::has_specialized_try_from<T>::value, result<T, error_info>>
try_get(const basic_value<TC>& v) noexcept
{
    return ::toml::try_from<T>::try_from_toml(v);
}

} // toml
#endif // TOML11_TRY_GET_HPP

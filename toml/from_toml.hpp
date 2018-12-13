//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_FROM_TOML
#define TOML11_FROM_TOML
#include "get.hpp"

namespace toml
{

template<typename T>
void from_toml(T& x, const toml::value& v)
{
    x = toml::get<typename std::remove_reference<T>::type>(v);
    return;
}

namespace detail
{

template<typename T>
constexpr toml::value_t determine_castable_type()
{
    return check_type<T>() != toml::value_t::Unknown ? check_type<T>() :
           toml::detail::is_map<T>::value            ? toml::value_t::Table :
           toml::detail::is_container<T>::value      ? toml::value_t::Array :
           toml::value_t::Unknown;
}

template<std::size_t N, typename ... Ts>
struct from_toml_tie_impl
{
    constexpr static std::size_t   index = sizeof...(Ts) - N;
    constexpr static toml::value_t type_index =
        determine_castable_type<
            typename std::tuple_element<index, std::tuple<Ts...>>::type>();

    static void invoke(std::tuple<Ts& ...> tie, const toml::value& v)
    {
        // static_cast is needed because with intel c++ compiler, operator==
        // is only defined when the two types are strictly equal, and type_index
        // is const toml::value_t, while v.type() is toml::value_t.
        if(static_cast<toml::value_t>(type_index) == v.type())
        {
            from_toml(std::get<index>(tie), v);
            return;
        }
        return from_toml_tie_impl<N-1, Ts...>::invoke(tie, v);
    }
};

template<typename ... Ts>
struct from_toml_tie_impl<0, Ts...>
{
    static void invoke(std::tuple<Ts& ...> tie, const toml::value& v)
    {
        return;
    }
};

} // detail

template<typename ... Ts>
void from_toml(std::tuple<Ts& ...> tie, const toml::value& v)
{
    detail::from_toml_tie_impl<sizeof...(Ts), Ts...>::invoke(tie, v);
    return;
}


} // toml
#endif // TOML11_FROM_TOML

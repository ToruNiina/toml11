#ifndef TOML11_FROM_TOML
#define TOML11_FROM_TOML
#include "value.hpp"

namespace toml
{

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    if(v.type() != vT)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                std::string(" is not arguemnt type: ") + stringize(vT));
    x = v.cast<vT>();
    return;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    // TODO the case of x is not dynamic container case
    if(v.type() != value_t::Array)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Array"));
    const auto& ar = v.cast<value_t::Array>();
    try
    {
        toml::resize(x, ar.size());
    }
    catch(std::invalid_argument& iv)
    {
        throw toml::type_error("toml::from_toml: static array size is not enough");
    }
    auto iter = x.begin();
    for(const auto& val : ar)
    {
        typename T::value_type v;
        from_toml(v, val);
        *iter = std::move(v);
        ++iter;
    }
    return;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    if(v.type() != value_t::Table)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Table"));
    x.clear();
    const auto& tb = v.cast<value_t::Table>();
    for(const auto& kv : tb)
    {
        x.insert(kv);
    }
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

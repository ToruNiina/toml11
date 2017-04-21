#ifndef TOML11_GET
#define TOML11_GET
#include "value.hpp"
#include "from_toml.hpp"

namespace toml
{

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
inline T get(const toml::value& v)
{
    return static_cast<T>(v.cast<vT>());
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
T get(const toml::value& v)
{
    if(v.type() != value_t::Array)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Array"));
    T tmp;
    from_toml(tmp, v);
    return tmp;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
T get(const toml::value& v)
{
    if(v.type() != value_t::Table)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Table"));
    T tmp;
    from_toml(tmp, v);
    return tmp;
}

} // toml
#endif// TOML11_GET

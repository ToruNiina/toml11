//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_TO_TOML
#define TOML11_TO_TOML
#include "value.hpp"

namespace toml
{

template<typename T>
inline value to_toml(T&& x)
{
    return value(std::forward<T>(x));
}

template<typename T>
inline value to_toml(T&& x, string_t kind)
{
    return value(std::forward<T>(x), kind);
}

inline value to_toml(local_date d, local_time t)
{
    return value(local_datetime(d, t));
}
inline value to_toml(local_date d, local_time t, time_offset ofs)
{
    return value(offset_datetime(d, t, ofs));
}

template<typename ... Ts>
inline value to_toml(Ts&& ... xs)
{
    return value(toml::array{toml::value(std::forward<Ts>(xs)) ... });
}

inline value to_toml(std::initializer_list<std::pair<std::string, toml::value>> xs)
{
    return value(toml::table(xs.begin(), xs.end()));
}

} // toml
#endif // TOML11_TO_TOML

#ifndef TOML11_FROM_HPP
#define TOML11_FROM_HPP
#include "version.hpp"

namespace toml
{
inline namespace TOML11_INLINE_VERSION_NAMESPACE
{

template<typename T>
struct from;
// {
//     static T from_toml(const toml::value& v)
//     {
//         // User-defined conversions ...
//     }
// };

} // TOML11_INLINE_VERSION_NAMESPACE
} // toml
#endif // TOML11_FROM_HPP

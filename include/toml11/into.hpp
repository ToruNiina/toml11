#ifndef TOML11_INTO_HPP
#define TOML11_INTO_HPP
#include "version.hpp"

namespace toml
{
inline namespace TOML11_INLINE_VERSION_NAMESPACE
{

template<typename T>
struct into;
// {
//     static toml::value into_toml(const T& user_defined_type)
//     {
//         // User-defined conversions ...
//     }
// };

} // TOML11_INLINE_VERSION_NAMESPACE
} // toml
#endif // TOML11_INTO_HPP

#ifndef TOML11_FROM_HPP
#define TOML11_FROM_HPP

namespace toml
{

template<typename T>
struct from;
// {
//     template<typename TC>
//     static T from_toml(const toml::basic_value<TC>& v)
//     {
//         // User-defined conversions ...
//     }
// };

template<typename T>
struct try_from;
// {
//     template<typename TC>
//     static result<T, error_info> try_from_toml(const toml::basic_value<TC>& v) noexcept
//     {
//         // User-defined conversions ...
//     }
// };

} // toml
#endif // TOML11_FROM_HPP

#ifndef TOML11_LITERAL_FWD_HPP
#define TOML11_LITERAL_FWD_HPP

#include "../location.hpp"
#include "../types.hpp"
#include "../value.hpp"

namespace toml
{

namespace detail
{
// implementation
::toml::value literal_internal_impl(location loc);
} // detail

inline namespace literals
{
inline namespace toml_literals
{

::toml::value operator"" _toml(const char* str, std::size_t len);

#if defined(__cpp_char8_t)
#  if __cpp_char8_t >= 201811L
#    define TOML11_HAS_CHAR8_T 1
#  endif
#endif

#if defined(TOML11_HAS_CHAR8_T)
// value of u8"" literal has been changed from char to char8_t and char8_t is
// NOT compatible to char
::toml::value operator"" _toml(const char8_t* str, std::size_t len);
#endif

} // toml_literals
} // literals
} // toml
#endif // TOML11_LITERAL_FWD_HPP

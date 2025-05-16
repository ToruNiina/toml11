#ifndef TOML11_EXCEPTION_HPP
#define TOML11_EXCEPTION_HPP

#include <exception>

#include "version.hpp"

namespace toml
{
inline namespace TOML11_INLINE_VERSION_NAMESPACE
{

struct exception : public std::exception
{
  public:
    virtual ~exception() noexcept override = default;
    virtual const char* what() const noexcept override {return "";}
};

} // TOML11_INLINE_VERSION_NAMESPACE
} // toml
#endif // TOMl11_EXCEPTION_HPP

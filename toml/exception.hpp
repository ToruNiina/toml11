//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_EXCEPTION_HPP
#define TOML11_EXCEPTION_HPP
#include <stdexcept>
#include <string>

namespace toml
{

struct exception : public std::exception
{
  public:
    virtual ~exception() noexcept override = default;
    virtual const char* what() const noexcept override {return "";}
};

struct syntax_error : public toml::exception
{
  public:
    explicit syntax_error(const std::string& what_arg) : what_(what_arg){}
    explicit syntax_error(const char* what_arg)        : what_(what_arg){}
    virtual ~syntax_error() noexcept override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}

  protected:
    std::string what_;
};

struct type_error : public toml::exception
{
  public:
    explicit type_error(const std::string& what_arg) : what_(what_arg){}
    explicit type_error(const char* what_arg)        : what_(what_arg){}
    virtual ~type_error() noexcept override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}

  protected:
    std::string what_;
};

struct internal_error : public toml::exception
{
  public:
    explicit internal_error(const std::string& what_arg) : what_(what_arg){}
    explicit internal_error(const char* what_arg)        : what_(what_arg){}
    virtual ~internal_error() noexcept override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}
  protected:
    std::string what_;
};

} // toml
#endif // TOML_EXCEPTION

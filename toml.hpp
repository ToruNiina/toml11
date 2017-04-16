/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Toru Niina
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef TOML_FOR_MODERN_CPP
#define TOML_FOR_MODERN_CPP
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cassert>

namespace toml
{

class value;
using key = std::string;

using Boolean  = bool;
using Integer  = std::int64_t;
using Float    = double;
using String   = std::string;
using Datetime = std::chrono::system_clock::time_point;
using Array    = std::vector<value>;
using Table    = std::map<key, value>;

enum class type
{
    Boolean,
    Integer,
    Float,
    String,
    Datetime,
    Array,
    Table,
    Unknown,
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, type t)
{
    switch(t)
    {
        case toml::type::Boolean : os << "Boolean";  return os;
        case toml::type::Integer : os << "Integer";  return os;
        case toml::type::Float   : os << "Float";    return os;
        case toml::type::String  : os << "String";   return os;
        case toml::type::Datetime: os << "Datetime"; return os;
        case toml::type::Array   : os << "Array";    return os;
        case toml::type::Table   : os << "Table";    return os;
        case toml::type::Unknown : os << "Unknown";  return os;
        default                  : os << "Nothing";  return os;
    }
}

template<typename charT, typename traits = std::char_traits<charT>,
         typename alloc = std::allocator<charT>>
inline std::basic_string<charT, traits, alloc>
stringize(type t)
{
    switch(t)
    {
        case toml::type::Boolean : return "Boolean";
        case toml::type::Integer : return "Integer";
        case toml::type::Float   : return "Float";
        case toml::type::String  : return "String";
        case toml::type::Datetime: return "Datetime";
        case toml::type::Array   : return "Array";
        case toml::type::Table   : return "Table";
        case toml::type::Unknown : return "Unknown";
        default                  : return "Nothing";
    }
}

namespace detail
{

template<typename T>
using unwrap_t = typename std::decay<T>::type;

template<typename T>
struct is_std_array : std::false_type {};
template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type{};
template<typename T>
struct is_std_vector : std::false_type{};
template<typename T>
struct is_std_vector<std::vector<T>> : std::true_type{};
template<typename T>
struct is_array : std::integral_constant<bool, is_std_array<T>::value ||
    is_std_vector<T>::value || std::is_array<T>::value>{};

template<typename T>
struct is_table : std::false_type {};
template<typename cT, typename aT>
struct is_table<std::map<toml::key, toml::value, cT, aT>> : std::true_type{};
template<typename hT, typename pT, typename aT>
struct is_table<std::unordered_map<toml::key, toml::value, hT, pT, aT>> : std::true_type{};

template<typename T>
constexpr inline type check_type()
{
    return std::is_same<unwrap_t<T>, bool>::value        ? type::Boolean :
           std::is_integral<unwrap_t<T>>::value          ? type::Integer :
           std::is_floating_point<unwrap_t<T>>::value    ? type::Float   :
           std::is_same<unwrap_t<T>, std::string>::value ? type::String  :
           std::is_same<unwrap_t<T>, const char*>::value ? type::String  :
           toml::detail::is_array<unwrap_t<T>>::value    ? type::Array   :
           toml::detail::is_table<unwrap_t<T>>::value    ? type::Table   :
           type::Unknown;
}

template<type t>
struct toml_default_type{};
template<>
struct toml_default_type<type::Boolean>{typedef Boolean type;};
template<>
struct toml_default_type<type::Integer>{typedef Integer type;};
template<>
struct toml_default_type<type::Float>{typedef Float type;};
template<>
struct toml_default_type<type::String>{typedef String type;};
template<>
struct toml_default_type<type::Datetime>{typedef Datetime type;};
template<>
struct toml_default_type<type::Array>{typedef Array type;};
template<>
struct toml_default_type<type::Table>{typedef Table type;};
template<>
struct toml_default_type<type::Unknown>{typedef void type;};

} // detail

struct exception : public std::exception
{
  public:
    virtual ~exception() override = default;
    virtual const char* what() const noexcept override {return "";}
};

struct syntax_error : public toml::exception
{
  public:
    explicit syntax_error(const std::string& what_arg) : what_arg_(what_arg){}
    explicit syntax_error(const char* what_arg)        : what_arg_(what_arg){}
    virtual ~syntax_error() override = default;
    virtual const char* what() const noexcept override {return what_arg_.c_str();}

  protected:
    std::string what_arg_;
};

struct type_error : public toml::exception
{
  public:
    explicit type_error(const std::string& what_arg) : what_arg_(what_arg){}
    explicit type_error(const char* what_arg)        : what_arg_(what_arg){}
    virtual ~type_error() override = default;
    virtual const char* what() const noexcept override {return what_arg_.c_str();}

  protected:
    std::string what_arg_;
};

struct internal_error : public toml::exception
{
  public:
    explicit internal_error(const std::string& what_arg) : what_arg_(what_arg){}
    explicit internal_error(const char* what_arg)        : what_arg_(what_arg){}
    virtual ~internal_error() override = default;
    virtual const char* what() const noexcept override {return what_arg_.c_str();}
  protected:
    std::string what_arg_;
};

template<typename T>
struct value_traits
{
    constexpr static type type_index = detail::check_type<T>();
    constexpr static bool is_toml_value = (type_index != type::Unknown);
    typedef typename detail::toml_default_type<type_index>::type value_type;
};

class value
{
  public:

    template<typename T, typename std::enable_if<
        value_traits<T>::is_toml_value, std::nullptr_t>::type = nullptr>
    value(T&& v)
        : type_(toml::detail::check_type<T>())
    {
        switch_assign<toml::detail::check_type<T>()>::invoke(
                *this, std::forward<T>(v));
    }

    ~value()
    {
        switch_clean(this->type_);
    }

    type t() const {return type_;}

    template<type T>
    typename detail::toml_default_type<T>::type const& cast() const
    {
        if(T != type_)
            throw type_error(std::string("value type is ") +
                             stringize<char>(type_) + std::string(", not ") +
                             stringize<char>(T));
        return switch_cast<T>::invoke(*this);
    }

    template<type T>
    typename detail::toml_default_type<T>::type& cast()
    {
        if(T != type_)
            throw type_error(std::string("value type is ") +
                             stringize<char>(type_) + std::string(", not ") +
                             stringize<char>(T));
        return switch_cast<T>::invoke(*this);
    }

    template<typename T, typename std::enable_if<
        value_traits<T>::is_toml_value, std::nullptr_t>::type = nullptr>
    void reset(T&& v)
    {
        switch_clean(this->type_);
        type_ = toml::detail::check_type<T>();
        switch_assign<toml::detail::check_type<T>()>::invoke(*this, std::forward<T>(v));
    }

  private:

    void switch_clean(type t)
    {
        switch(t)
        {
            case type::Boolean : boolean_.~Boolean();   return;
            case type::Integer : integer_.~Integer();   return;
            case type::Float   : float_.~Float();       return;
            case type::String  : string_.~String();     return;
            case type::Datetime: datetime_.~Datetime(); return;
            case type::Array   : array_.~Array();       return;
            case type::Table   : table_.~Table();       return;
            case type::Unknown : return;
            default: assert(false);
        }
    }

    template<type t> struct switch_assign;
    template<type t> struct switch_cast;

  private:

    type type_;
    union
    {
        Boolean  boolean_;
        Integer  integer_;
        Float    float_;
        String   string_;
        Datetime datetime_;
        Array    array_;
        Table    table_;
    };
};

template<> struct value::switch_assign<type::Boolean>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.boolean_ = std::forward<Boolean>(val);
    }
};
template<> struct value::switch_assign<type::Integer>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.integer_ = std::forward<Integer>(val);
    }
};
template<> struct value::switch_assign<type::Float>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.float_ = std::forward<Float>(val);
    }
};
template<> struct value::switch_assign<type::String>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.string_) String(val);
    }
};
template<> struct value::switch_assign<type::Datetime>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.datetime_) Datetime(val);
    }
};
template<> struct value::switch_assign<type::Array>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.array_) Array(val);
    }
};
template<> struct value::switch_assign<type::Table>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.table_) Table(val);
    }
};

template<> struct value::switch_cast<type::Boolean>
{
    static Boolean&       invoke(value& v)       {return v.boolean_;}
    static Boolean const& invoke(value const& v) {return v.boolean_;}
};
template<> struct value::switch_cast<type::Integer>
{
    static Integer&       invoke(value& v)       {return v.integer_;}
    static Integer const& invoke(value const& v) {return v.integer_;}
};
template<> struct value::switch_cast<type::Float>
{
    static Float&       invoke(value& v)       {return v.float_;}
    static Float const& invoke(value const& v) {return v.float_;}
};
template<> struct value::switch_cast<type::String>
{
    static String&       invoke(value& v)       {return v.string_;}
    static String const& invoke(value const& v) {return v.string_;}
};
template<> struct value::switch_cast<type::Datetime>
{
    static Datetime&       invoke(value& v)       {return v.datetime_;}
    static Datetime const& invoke(value const& v) {return v.datetime_;}
};
template<> struct value::switch_cast<type::Array>
{
    static Array&       invoke(value& v)       {return v.array_;}
    static Array const& invoke(value const& v) {return v.array_;}
};
template<> struct value::switch_cast<type::Table>
{
    static Table&       invoke(value& v)       {return v.table_;}
    static Table const& invoke(value const& v) {return v.table_;}
};



}// toml
#endif// TOML_FOR_MODERN_CPP

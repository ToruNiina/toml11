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

enum class value_t
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
operator<<(std::basic_ostream<charT, traits>& os, value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean : os << "Boolean";  return os;
        case toml::value_t::Integer : os << "Integer";  return os;
        case toml::value_t::Float   : os << "Float";    return os;
        case toml::value_t::String  : os << "String";   return os;
        case toml::value_t::Datetime: os << "Datetime"; return os;
        case toml::value_t::Array   : os << "Array";    return os;
        case toml::value_t::Table   : os << "Table";    return os;
        case toml::value_t::Unknown : os << "Unknown";  return os;
        default                  : os << "Nothing";  return os;
    }
}

template<typename charT, typename traits = std::char_traits<charT>,
         typename alloc = std::allocator<charT>>
inline std::basic_string<charT, traits, alloc>
stringize(value_t t)
{
    switch(t)
    {
        case toml::value_t::Boolean : return "Boolean";
        case toml::value_t::Integer : return "Integer";
        case toml::value_t::Float   : return "Float";
        case toml::value_t::String  : return "String";
        case toml::value_t::Datetime: return "Datetime";
        case toml::value_t::Array   : return "Array";
        case toml::value_t::Table   : return "Table";
        case toml::value_t::Unknown : return "Unknown";
        default                     : return "Nothing";
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
constexpr inline value_t check_type()
{
    return std::is_same<unwrap_t<T>, bool>::value        ? value_t::Boolean :
           std::is_integral<unwrap_t<T>>::value          ? value_t::Integer :
           std::is_floating_point<unwrap_t<T>>::value    ? value_t::Float   :
           std::is_same<unwrap_t<T>, std::string>::value ? value_t::String  :
           std::is_same<unwrap_t<T>, const char*>::value ? value_t::String  :
           toml::detail::is_array<unwrap_t<T>>::value    ? value_t::Array   :
           toml::detail::is_table<unwrap_t<T>>::value    ? value_t::Table   :
           value_t::Unknown;
}

template<value_t t> struct toml_default_type{};
template<> struct toml_default_type<value_t::Boolean>{ typedef Boolean  type;};
template<> struct toml_default_type<value_t::Integer>{ typedef Integer  type;};
template<> struct toml_default_type<value_t::Float>{   typedef Float    type;};
template<> struct toml_default_type<value_t::String>{  typedef String   type;};
template<> struct toml_default_type<value_t::Datetime>{typedef Datetime type;};
template<> struct toml_default_type<value_t::Array>{   typedef Array    type;};
template<> struct toml_default_type<value_t::Table>{   typedef Table    type;};
template<> struct toml_default_type<value_t::Unknown>{ typedef void     type;};

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
    constexpr static value_t type_index = detail::check_type<T>();
    constexpr static bool is_toml_value = (type_index != value_t::Unknown);
    typedef typename detail::toml_default_type<type_index>::type type;
};

class value
{
  public:

    value() : type_(value_t::Unknown){}

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

    value(const value& v)
        : type_(v.type())
    {
        switch(v.type())
        {
            case value_t::Boolean : boolean_  = v.cast<value_t::Boolean>();  break;
            case value_t::Integer : integer_  = v.cast<value_t::Integer>();  break;
            case value_t::Float   : float_    = v.cast<value_t::Float>();    break;
            case value_t::String  : string_   = v.cast<value_t::String>();   break;
            case value_t::Datetime: datetime_ = v.cast<value_t::Datetime>(); break;
            case value_t::Array   : array_    = v.cast<value_t::Array>();    break;
            case value_t::Table   : table_    = v.cast<value_t::Table>();    break;
            case value_t::Unknown : break;
            default: break;
        }
    }

    value(value&& v)
        : type_(std::move(v.type_))
    {
        switch(v.type())
        {
            case value_t::Boolean : boolean_  = std::move(v.cast<value_t::Boolean>());  break;
            case value_t::Integer : integer_  = std::move(v.cast<value_t::Integer>());  break;
            case value_t::Float   : float_    = std::move(v.cast<value_t::Float>());    break;
            case value_t::String  : string_   = std::move(v.cast<value_t::String>());   break;
            case value_t::Datetime: datetime_ = std::move(v.cast<value_t::Datetime>()); break;
            case value_t::Array   : array_    = std::move(v.cast<value_t::Array>());    break;
            case value_t::Table   : table_    = std::move(v.cast<value_t::Table>());    break;
            case value_t::Unknown : break;
            default: break;
        }
    }

    value& operator=(const value& v)
    {
        if(this->type_ != value_t::Unknown) this->switch_clean(this->type_);

        this->type_ = v.type();
        switch(v.type())
        {
            case value_t::Boolean : boolean_  = v.cast<value_t::Boolean>();  break;
            case value_t::Integer : integer_  = v.cast<value_t::Integer>();  break;
            case value_t::Float   : float_    = v.cast<value_t::Float>();    break;
            case value_t::String  : string_   = v.cast<value_t::String>();   break;
            case value_t::Datetime: datetime_ = v.cast<value_t::Datetime>(); break;
            case value_t::Array   : array_    = v.cast<value_t::Array>();    break;
            case value_t::Table   : table_    = v.cast<value_t::Table>();    break;
            case value_t::Unknown : break;
            default: break;
        }
        return *this;
    }

    value& operator=(value&& v)
    {
        if(this->type_ != value_t::Unknown) this->switch_clean(this->type_);

        this->type_ = std::move(v.type_);
        switch(v.type())
        {
            case value_t::Boolean : boolean_  = std::move(v.cast<value_t::Boolean>());  break;
            case value_t::Integer : integer_  = std::move(v.cast<value_t::Integer>());  break;
            case value_t::Float   : float_    = std::move(v.cast<value_t::Float>());    break;
            case value_t::String  : string_   = std::move(v.cast<value_t::String>());   break;
            case value_t::Datetime: datetime_ = std::move(v.cast<value_t::Datetime>()); break;
            case value_t::Array   : array_    = std::move(v.cast<value_t::Array>());    break;
            case value_t::Table   : table_    = std::move(v.cast<value_t::Table>());    break;
            case value_t::Unknown : break;
            default: break;
        }
        return *this;
    }


    value_t type() const {return type_;}

    template<value_t T>
    typename detail::toml_default_type<T>::type const& cast() const
    {
        if(T != type_)
            throw type_error(std::string("value type is ") +
                             stringize<char>(type_) + std::string(", not ") +
                             stringize<char>(T));
        return switch_cast<T>::invoke(*this);
    }

    template<value_t T>
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

    void switch_clean(value_t t)
    {
        switch(t)
        {
            case value_t::Boolean : boolean_.~Boolean();   return;
            case value_t::Integer : integer_.~Integer();   return;
            case value_t::Float   : float_.~Float();       return;
            case value_t::String  : string_.~String();     return;
            case value_t::Datetime: datetime_.~Datetime(); return;
            case value_t::Array   : array_.~Array();       return;
            case value_t::Table   : table_.~Table();       return;
            case value_t::Unknown : return;
            default: assert(false);
        }
    }

    template<value_t t> struct switch_assign;
    template<value_t t> struct switch_cast;

  private:

    value_t type_;
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

template<> struct value::switch_assign<value_t::Boolean>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.boolean_ = std::forward<Boolean>(val);
    }
};
template<> struct value::switch_assign<value_t::Integer>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.integer_ = std::forward<Integer>(val);
    }
};
template<> struct value::switch_assign<value_t::Float>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.float_ = std::forward<Float>(val);
    }
};
template<> struct value::switch_assign<value_t::String>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.string_) String(val);
    }
};
template<> struct value::switch_assign<value_t::Datetime>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.datetime_) Datetime(val);
    }
};
template<> struct value::switch_assign<value_t::Array>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.array_) Array(val);
    }
};
template<> struct value::switch_assign<value_t::Table>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.table_) Table(val);
    }
};

template<> struct value::switch_cast<value_t::Boolean>
{
    static Boolean&       invoke(value& v)       {return v.boolean_;}
    static Boolean const& invoke(value const& v) {return v.boolean_;}
};
template<> struct value::switch_cast<value_t::Integer>
{
    static Integer&       invoke(value& v)       {return v.integer_;}
    static Integer const& invoke(value const& v) {return v.integer_;}
};
template<> struct value::switch_cast<value_t::Float>
{
    static Float&       invoke(value& v)       {return v.float_;}
    static Float const& invoke(value const& v) {return v.float_;}
};
template<> struct value::switch_cast<value_t::String>
{
    static String&       invoke(value& v)       {return v.string_;}
    static String const& invoke(value const& v) {return v.string_;}
};
template<> struct value::switch_cast<value_t::Datetime>
{
    static Datetime&       invoke(value& v)       {return v.datetime_;}
    static Datetime const& invoke(value const& v) {return v.datetime_;}
};
template<> struct value::switch_cast<value_t::Array>
{
    static Array&       invoke(value& v)       {return v.array_;}
    static Array const& invoke(value const& v) {return v.array_;}
};
template<> struct value::switch_cast<value_t::Table>
{
    static Table&       invoke(value& v)       {return v.table_;}
    static Table const& invoke(value const& v) {return v.table_;}
};



}// toml
#endif// TOML_FOR_MODERN_CPP

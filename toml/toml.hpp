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
#include "datetime.hpp"
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <cassert>
#include <cstdint>

namespace toml
{

class value;
using key = std::string;

using Boolean  = bool;
using Integer  = std::int64_t;
using Float    = double;
using String   = std::string;
using Datetime = basic_datetime<unsigned int, int>;
using Array    = std::vector<value>;
using Table    = std::unordered_map<key, value>;

enum class value_t : std::uint8_t
{
    Boolean  = 1,
    Integer  = 2,
    Float    = 3,
    String   = 4,
    Datetime = 5,
    Array    = 6,
    Table    = 7,
    Empty    = 0,
    Unknown  = 255,
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
        case toml::value_t::Empty   : os << "Empty";    return os;
        case toml::value_t::Unknown : os << "Unknown";  return os;
        default                     : os << "Nothing";  return os;
    }
}

template<typename charT = char, typename traits = std::char_traits<charT>,
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
        case toml::value_t::Empty   : return "Empty";
        case toml::value_t::Unknown : return "Unknown";
        default                     : return "Nothing";
    }
}

namespace detail
{

template<typename T>
using unwrap_t = typename std::decay<T>::type;

template<typename T>
constexpr inline value_t check_type()
{
    return std::is_same<unwrap_t<T>,        toml::Boolean >::value ? value_t::Boolean :
           std::is_integral<unwrap_t<T>>::value                    ? value_t::Integer :
           std::is_floating_point<unwrap_t<T>>::value              ? value_t::Float   :
           std::is_convertible<unwrap_t<T>, toml::String  >::value ? value_t::String  :
           std::is_convertible<unwrap_t<T>, toml::Datetime>::value ? value_t::Datetime:
           std::is_convertible<unwrap_t<T>, toml::Array   >::value ? value_t::Array   :
           std::is_convertible<unwrap_t<T>, toml::Table   >::value ? value_t::Table   :
           value_t::Unknown;
}

constexpr inline bool is_valid(value_t vt)
{
    return vt != value_t::Unknown;
}

template<typename T> struct is_toml_array              : std::false_type{};
template<>           struct is_toml_array<toml::Array> : std::true_type {};
template<typename T> struct is_toml_table              : std::false_type{};
template<>           struct is_toml_table<toml::Table> : std::true_type {};

template<value_t t> struct toml_default_type{};
template<> struct toml_default_type<value_t::Boolean >{typedef Boolean  type;};
template<> struct toml_default_type<value_t::Integer >{typedef Integer  type;};
template<> struct toml_default_type<value_t::Float   >{typedef Float    type;};
template<> struct toml_default_type<value_t::String  >{typedef String   type;};
template<> struct toml_default_type<value_t::Datetime>{typedef Datetime type;};
template<> struct toml_default_type<value_t::Array   >{typedef Array    type;};
template<> struct toml_default_type<value_t::Table   >{typedef Table    type;};
template<> struct toml_default_type<value_t::Empty   >{typedef void     type;};
template<> struct toml_default_type<value_t::Unknown >{typedef void     type;};

struct has_iterator_impl
{
    template<typename T> static std::true_type  check(typename T::iterator*);
    template<typename T> static std::false_type check(...);
};
struct has_value_type_impl
{
    template<typename T> static std::true_type  check(typename T::value_type*);
    template<typename T> static std::false_type check(...);
};
struct has_key_type_impl
{
    template<typename T> static std::true_type  check(typename T::key_type*);
    template<typename T> static std::false_type check(...);
};
struct has_mapped_type_impl
{
    template<typename T> static std::true_type  check(typename T::mapped_type*);
    template<typename T> static std::false_type check(...);
};

template<typename T>
struct has_iterator    : decltype(has_iterator_impl::check<T>(nullptr)){};
template<typename T>
struct has_value_type  : decltype(has_value_type_impl::check<T>(nullptr)){};
template<typename T>
struct has_key_type    : decltype(has_key_type_impl::check<T>(nullptr)){};
template<typename T>
struct has_mapped_type : decltype(has_mapped_type_impl::check<T>(nullptr)){};

template<typename T>
struct is_container : std::integral_constant<bool,
    has_iterator<T>::value && has_value_type<T>::value>{};

template<typename T>
struct is_map : std::integral_constant<bool,
    has_iterator<T>::value && has_key_type<T>::value &&
    has_mapped_type<T>::value>{};

struct is_key_convertible_impl
{
    template<typename T>
    static std::is_convertible<typename T::key_type, toml::key>
    check(typename T::key_type*);

    template<typename T> static std::false_type check(...);
};
template<typename T>
struct is_key_convertible : decltype(is_key_convertible_impl::check<T>(nullptr)){};

struct storage_base
{
    storage_base(): type(toml::value_t::Empty){}
    storage_base(toml::value_t t): type(t){}
    virtual ~storage_base() = default;
    toml::value_t type;
};

template<typename T>
struct storage : public storage_base
{
    static_assert(is_toml_array<T>::value || is_toml_table<T>::value,
                  "toml::detail::storage is for toml::Array or toml::Table!");
    typedef T value_type;

    storage() = default;
    ~storage() override = default;
    storage(storage const&) = default;
    storage(storage&&)      = default;
    storage& operator=(storage const&) = default;
    storage& operator=(storage&&)      = default;

    storage(value_type const& v) : value(v){}
    storage(value_type&& v) : value(std::move(v)){}

    value_type value;
};
} // detail

/* -------------------------------------------------------------------------- */
template<typename T, typename ... Ts>
inline std::unique_ptr<T> make_unique(Ts&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(args)...));
}

struct exception : public std::exception
{
  public:
    virtual ~exception() override = default;
    virtual const char* what() const noexcept override {return "";}
};

struct syntax_error : public toml::exception
{
  public:
    explicit syntax_error(const std::string& what_arg) : what_(what_arg){}
    explicit syntax_error(const char* what_arg)        : what_(what_arg){}
    virtual ~syntax_error() override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}

  protected:
    std::string what_;
};

struct type_error : public toml::exception
{
  public:
    explicit type_error(const std::string& what_arg) : what_(what_arg){}
    explicit type_error(const char* what_arg)        : what_(what_arg){}
    virtual ~type_error() override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}

  protected:
    std::string what_;
};

struct internal_error : public toml::exception
{
  public:
    explicit internal_error(const std::string& what_arg) : what_(what_arg){}
    explicit internal_error(const char* what_arg)        : what_(what_arg){}
    virtual ~internal_error() override = default;
    virtual const char* what() const noexcept override {return what_.c_str();}
  protected:
    std::string what_;
};
/* -------------------------------------------------------------------------- */

template<typename T>
struct value_traits
{
    constexpr static value_t type_index = detail::check_type<T>();
    constexpr static bool is_toml_type  = detail::is_valid(type_index);
    typedef typename detail::toml_default_type<type_index>::type type;
};

class value
{
    typedef std::unique_ptr<detail::storage_base> storage_ptr;

  public:

    value() : type_(value_t::Empty){}
    ~value();

    value(const value& v);
    value(value&& v);
    value& operator=(const value& v);
    value& operator=(value&& v);

    template<typename T, typename std::enable_if<
        value_traits<T>::is_toml_type, std::nullptr_t>::type = nullptr>
    value(T&& v);

    template<typename T, typename std::enable_if<
        value_traits<T>::is_toml_type, std::nullptr_t>::type = nullptr>
    value& operator=(T&& v);

    template<typename T, typename std::enable_if<
        value_traits<T>::is_toml_type, std::nullptr_t>::type = nullptr>
    value(std::initializer_list<T> init);

    value(std::initializer_list<std::pair<toml::key, toml::value>> init);

    value_t type() const {return type_;}

    template<value_t T>
    typename detail::toml_default_type<T>::type const& cast() const;
    template<value_t T>
    typename detail::toml_default_type<T>::type&       cast();

  private:

    void switch_clean(value_t t);
    template<value_t t> struct switch_assign;
    template<value_t t> struct switch_cast;

    static bool should_be_cleaned(value_t vt)
    {
        return (vt == value_t::String) || (vt == value_t::Array) ||
               (vt == value_t::Table)  || (vt == value_t::Datetime);
    }

  private:

    value_t type_;
    union
    {
        Boolean     boolean_;
        Integer     integer_;
        Float       float_;
        String      string_;
        Datetime    datetime_;
        storage_ptr storage_; //ptr to table or array
    };
};

template<> struct value::switch_assign<value_t::Boolean>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.boolean_ = static_cast<Boolean>(val);
    }
};
template<> struct value::switch_assign<value_t::Integer>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.integer_ = static_cast<Integer>(val);
    }
};
template<> struct value::switch_assign<value_t::Float>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        v.float_ = static_cast<Float>(val);
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
        new(&v.storage_) storage_ptr(
                toml::make_unique<detail::storage<Array>>(val));
    }
};
template<> struct value::switch_assign<value_t::Table>
{
    template<typename valT>
    static void invoke(value& v, valT&& val)
    {
        new(&v.storage_) storage_ptr(
                toml::make_unique<detail::storage<Table>>(val));
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
    // switch_cast assumes tmeplate argument is correct.
    // if not, the behaviour is undefined.
    static Array& invoke(value& v)
    {
        return static_cast<detail::storage<Array>*>(v.storage_.get())->value;
    }
    static Array const& invoke(value const& v)
    {
        return static_cast<detail::storage<Array>*>(v.storage_.get())->value;
    }
};
template<> struct value::switch_cast<value_t::Table>
{
    static Table&       invoke(value& v)
    {
        return static_cast<detail::storage<Table>*>(v.storage_.get())->value;
    }
    static Table const& invoke(value const& v)
    {
        return static_cast<detail::storage<Table>*>(v.storage_.get())->value;
    }
};

inline void value::switch_clean(value_t t)
{
    switch(t)
    {
        case value_t::Boolean  : {boolean_.~Boolean(); return;}
        case value_t::Integer  : {integer_.~Integer(); return;}
        case value_t::Float    : {float_.~Float(); return;}
        case value_t::String   : {string_.~String(); return;}
        case value_t::Datetime : {datetime_.~Datetime(); return;}
        case value_t::Array    : {storage_.~storage_ptr(); return;}
        case value_t::Table    : {storage_.~storage_ptr(); return;}
        case value_t::Empty    : return;
        case value_t::Unknown  : assert(false);
        default                : assert(false);
    }
}

inline value::~value()
{
    switch_clean(this->type_);
}

inline value::value(const value& v) : type_(v.type())
{
    switch(v.type())
    {
        case value_t::Boolean :
        {
            switch_assign<value_t::Boolean>::invoke(
                    *this, v.cast<value_t::Boolean>());
            break;
        }
        case value_t::Integer :
        {
            switch_assign<value_t::Integer>::invoke(
                    *this, v.cast<value_t::Integer>());
            break;
        }
        case value_t::Float   :
        {
            switch_assign<value_t::Float>::invoke(
                    *this, v.cast<value_t::Float>());
            break;
        }
        case value_t::String  :
        {
            switch_assign<value_t::String>::invoke(
                    *this, v.cast<value_t::String>());
            break;
        }
        case value_t::Datetime:
        {
            switch_assign<value_t::Datetime>::invoke(
                    *this, v.cast<value_t::Datetime>());
            break;
        }
        case value_t::Array   :
        {
            switch_assign<value_t::Array>::invoke(
                    *this, v.cast<value_t::Array>());
            break;
        }
        case value_t::Table   :
        {
            switch_assign<value_t::Table>::invoke(
                    *this, v.cast<value_t::Table>());
            break;
        }
        case value_t::Empty   : break;
        case value_t::Unknown : assert(false);
        default: assert(false);
    }
}

inline value::value(value&& v)
{
    this->type_ = v.type_;
    switch(this->type_)
    {
        case value_t::Boolean :
        {
            switch_assign<value_t::Boolean>::invoke(*this,
                    std::move(v.cast<value_t::Boolean>()));
            break;
        }
        case value_t::Integer :
        {
            switch_assign<value_t::Integer>::invoke(*this,
                    std::move(v.cast<value_t::Integer>()));
            break;
        }
        case value_t::Float   :
        {
            switch_assign<value_t::Float>::invoke(*this,
                    std::move(v.cast<value_t::Float>()));
            break;
        }
        case value_t::String  :
        {
            switch_assign<value_t::String>::invoke(*this,
                    std::move(v.cast<value_t::String>()));
            break;
        }
        case value_t::Datetime:
        {
            switch_assign<value_t::Datetime>::invoke(*this,
                    std::move(v.cast<value_t::Datetime>()));
            break;
        }
        case value_t::Array   :
        {
            new(&this->storage_) storage_ptr(std::move(v.storage_));
            break;
        }
        case value_t::Table   :
        {
            new(&this->storage_) storage_ptr(std::move(v.storage_));
            break;
        }
        case value_t::Empty : break;
        case value_t::Unknown : assert(false);
        default: assert(false);
    }
}

inline value& value::operator=(const value& v)
{
    if(should_be_cleaned(this->type_))
    {
        this->switch_clean(this->type_);
    }
    this->type_ = v.type();

    switch(this->type_)
    {
        case value_t::Boolean :
        {
            switch_assign<value_t::Boolean>::invoke(*this,
                    v.cast<value_t::Boolean>());
            break;
        }
        case value_t::Integer :
        {
            switch_assign<value_t::Integer>::invoke(*this,
                    v.cast<value_t::Integer>());
            break;
        }
        case value_t::Float   :
        {
            switch_assign<value_t::Float>::invoke(*this,
                    v.cast<value_t::Float>());
            break;
        }
        case value_t::String  :
        {
            switch_assign<value_t::String>::invoke(*this,
                    v.cast<value_t::String>());
            break;
        }
        case value_t::Datetime:
        {
            switch_assign<value_t::Datetime>::invoke(*this,
                    v.cast<value_t::Datetime>());
            break;
        }
        case value_t::Array   :
        {
            switch_assign<value_t::Array>::invoke(*this,
                    v.cast<value_t::Array>());
            break;
        }
        case value_t::Table   :
        {
            switch_assign<value_t::Table>::invoke(*this,
                    v.cast<value_t::Table>());
            break;
        }
        case value_t::Empty   : break;
        case value_t::Unknown : assert(false);
        default: assert(false);
    }
    return *this;
}

inline value& value::operator=(value&& v)
{
    if(should_be_cleaned(this->type_))
    {
        this->switch_clean(this->type_);
    }
    this->type_ = v.type_;

    switch(this->type_)
    {
        case value_t::Boolean :
        {
            switch_assign<value_t::Boolean>::invoke(*this,
                    std::move(v.cast<value_t::Boolean>()));
            break;
        }
        case value_t::Integer :
        {
            switch_assign<value_t::Integer>::invoke(*this,
                    std::move(v.cast<value_t::Integer>()));
            break;
        }
        case value_t::Float   :
        {
            switch_assign<value_t::Float>::invoke(*this,
                    std::move(v.cast<value_t::Float>()));
            break;
        }
        case value_t::String  :
        {
            switch_assign<value_t::String>::invoke(*this,
                    std::move(v.cast<value_t::String>()));
            break;
        }
        case value_t::Datetime:
        {
            switch_assign<value_t::Datetime>::invoke(*this,
                    std::move(v.cast<value_t::Datetime>()));
            break;
        }
        case value_t::Array   :
        {
            switch_assign<value_t::Array>::invoke(*this,
                    std::move(v.cast<value_t::Array>()));
            break;
        }
        case value_t::Table   :
        {
            switch_assign<value_t::Table>::invoke(*this,
                    std::move(v.cast<value_t::Table>()));
            break;
        }
        case value_t::Empty : break;
        case value_t::Unknown : assert(false);
        default: assert(false);
    }
    return *this;
}

template<typename T, typename std::enable_if<
    value_traits<T>::is_toml_type, std::nullptr_t>::type>
value::value(T&& v) : type_(toml::detail::check_type<T>())
{
    switch_assign<toml::detail::check_type<T>()>::invoke(
            *this, std::forward<T>(v));
}

template<typename T, typename std::enable_if<
    value_traits<T>::is_toml_type, std::nullptr_t>::type>
value& value::operator=(T&& v)
{
    if(should_be_cleaned(this->type_))
    {
        switch_clean(this->type_);
    }
    this->type_ = toml::detail::check_type<T>();
    switch_assign<toml::detail::check_type<T>()>::invoke(
            *this, std::forward<T>(v));
    return *this;
}

template<typename T, typename std::enable_if<
    value_traits<T>::is_toml_type, std::nullptr_t>::type>
value::value(std::initializer_list<T> init)
    : type_(toml::value_t::Array)
{
    toml::Array arr; arr.reserve(init.size());
    for(auto&& item : init)
        arr.emplace_back(std::move(item));
    switch_assign<toml::value_t::Array>::invoke(*this, std::move(arr));
}

inline value::value(
        std::initializer_list<std::pair<toml::key, toml::value>> init)
    : type_(toml::value_t::Table)
{
    toml::Table tmp;
    for(auto&& item : init)
        tmp.emplace(std::move(item.first), std::move(item.second));
    switch_assign<toml::value_t::Table>::invoke(*this, std::move(tmp));
}

template<value_t T>
inline typename detail::toml_default_type<T>::type const&
value::cast() const
{
    if(T != this->type_)
        throw type_error("current type: " + stringize(this->type_) +
                         std::string(" is not query type: ") + stringize(T));
    return switch_cast<T>::invoke(*this);
}
template<value_t T>
inline typename detail::toml_default_type<T>::type&
value::cast()
{
    if(T != this->type_)
        throw type_error("current type: " + stringize(this->type_) +
                         std::string(" is not query type: ") + stringize(T));
    return switch_cast<T>::invoke(*this);
}

inline bool operator==(const toml::value& lhs, const toml::value& rhs)
{
    if(lhs.type() != rhs.type()) return false;
    switch(lhs.type())
    {
        case value_t::Boolean :
            return lhs.cast<value_t::Boolean >() == rhs.cast<value_t::Boolean >();
        case value_t::Integer :
            return lhs.cast<value_t::Integer >() == rhs.cast<value_t::Integer >();
        case value_t::Float   :
            return lhs.cast<value_t::Float   >() == rhs.cast<value_t::Float   >();
        case value_t::String  :
            return lhs.cast<value_t::String  >() == rhs.cast<value_t::String  >();
        case value_t::Datetime:
            return lhs.cast<value_t::Datetime>() == rhs.cast<value_t::Datetime>();
        case value_t::Array   :
            return lhs.cast<value_t::Array   >() == rhs.cast<value_t::Array   >();
        case value_t::Table   :
            return lhs.cast<value_t::Table   >() == rhs.cast<value_t::Table   >();
        case value_t::Empty   : return true;
        case value_t::Unknown : return false;
        default: return false;
    }
}
inline bool operator<(const toml::value& lhs, const toml::value& rhs)
{
    if(lhs.type() != rhs.type()) return (lhs.type() < rhs.type());
    switch(lhs.type())
    {
        case value_t::Boolean :
            return lhs.cast<value_t::Boolean >() < rhs.cast<value_t::Boolean >();
        case value_t::Integer :
            return lhs.cast<value_t::Integer >() < rhs.cast<value_t::Integer >();
        case value_t::Float   :
            return lhs.cast<value_t::Float   >() < rhs.cast<value_t::Float   >();
        case value_t::String  :
            return lhs.cast<value_t::String  >() < rhs.cast<value_t::String  >();
        case value_t::Datetime:
            return lhs.cast<value_t::Datetime>() < rhs.cast<value_t::Datetime>();
        case value_t::Array   :
            return lhs.cast<value_t::Array   >() < rhs.cast<value_t::Array   >();
        case value_t::Table   :
            return lhs.cast<value_t::Table   >() < rhs.cast<value_t::Table   >();
        case value_t::Empty   : return false;
        case value_t::Unknown : return false;
        default: return false;
    }
}

inline bool operator!=(const toml::value& lhs, const toml::value& rhs)
{
    return !(lhs == rhs);
}
inline bool operator<=(const toml::value& lhs, const toml::value& rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}
inline bool operator>(const toml::value& lhs, const toml::value& rhs)
{
    return !(lhs <= rhs);
}
inline bool operator>=(const toml::value& lhs, const toml::value& rhs)
{
    return !(lhs < rhs);
}

/* ------------------------------- to_toml ---------------------------------- */

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
inline toml::value to_toml(T&& x)
{
    return toml::value(std::forward<T>(x));
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
toml::value to_toml(T&& x)
{
    toml::Array tmp; tmp.reserve(std::distance(std::begin(x), std::end(x)));
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
        tmp.emplace_back(*iter);
    return toml::value(std::move(tmp));
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
toml::value to_toml(T&& x)
{
    toml::Table tmp;
    for(auto iter = std::begin(x); iter != std::end(x); ++iter)
        tmp.emplace(iter->first, to_toml(iter->second));
    return toml::value(std::move(tmp));
}

template<typename T>
inline toml::value to_toml(std::initializer_list<T> init)
{
    return toml::value(std::move(init));
}

inline toml::value
to_toml(std::initializer_list<std::pair<std::string, toml::value>> init)
{
    return toml::value(std::move(init));
}

/* ------------------------------ from_toml --------------------------------- */


template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT != toml::value_t::Unknown &&
         vT != value_t::Empty), std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    if(v.type() != vT)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                std::string(" is not arguemnt type: ") + stringize(vT));
    x = v.cast<vT>();
    return;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         (!toml::detail::is_map<T>::value) &&
         toml::detail::is_container<T>::value, std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    // TODO the case of x is not dynamic container case
    if(v.type() != value_t::Array)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Array"));
    const auto& ar = v.cast<value_t::Array>();
    for(const auto& val : ar)
    {
        typename T::value_type v;
        from_toml(v, val);
        x.push_back(std::move(v));
    }
    return;
}

template<typename T, toml::value_t vT = toml::detail::check_type<T>(),
         typename std::enable_if<(vT == toml::value_t::Unknown) &&
         toml::detail::is_map<T>::value, std::nullptr_t>::type = nullptr>
void from_toml(T& x, const toml::value& v)
{
    if(v.type() != value_t::Table)
        throw type_error("from_toml: value type: " + stringize(v.type()) +
                         std::string(" is not argument type: Table"));
    x.clear();
    const auto& tb = v.cast<value_t::Table>();
    for(const auto& kv : tb)
    {
        x.insert(kv);
    }
    return;
}

namespace detail
{

template<typename T>
constexpr toml::value_t determine_castable_type()
{
    return check_type<T>() != toml::value_t::Unknown ? check_type<T>() :
           toml::detail::is_map<T>::value            ? toml::value_t::Table :
           toml::detail::is_container<T>::value      ? toml::value_t::Array :
           toml::value_t::Unknown;
}

template<std::size_t N, typename ... Ts>
struct from_toml_tie_impl
{
    constexpr static std::size_t   index = sizeof...(Ts) - N;
    constexpr static toml::value_t type_index =
        determine_castable_type<
            typename std::tuple_element<index, std::tuple<Ts...>>::type>();

    static void invoke(std::tuple<Ts& ...> tie, const toml::value& v)
    {
        if(type_index == v.type())
        {
            from_toml(std::get<index>(tie), v);
            return;
        }
        return from_toml_tie_impl<N-1, Ts...>::invoke(tie, v);
    }
};

template<typename ... Ts>
struct from_toml_tie_impl<0, Ts...>
{
    static void invoke(std::tuple<Ts& ...> tie, const toml::value& v)
    {
        throw type_error("from_toml(tie, value): no match");
    }
};

} // detail

template<typename ... Ts>
void from_toml(std::tuple<Ts& ...> tie, const toml::value& v)
{
    detail::from_toml_tie_impl<sizeof...(Ts), Ts...>::invoke(tie, v);
    return;
}

/* ------------------------------ get --------------------------------- */

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





}// toml
#endif// TOML_FOR_MODERN_CPP

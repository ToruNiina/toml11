#ifndef TOML11_VALUE
#define TOML11_VALUE
#include "datetime.hpp"
#include "traits.hpp"
#include "utility.hpp"
#include "exception.hpp"
#include <vector>
#include <tuple>
#include <unordered_map>
#include <cassert>
#include <cstdint>

namespace toml
{

using character = char;

class value;
using key = std::basic_string<character>;

using Boolean  = bool;
using Integer  = std::int64_t;
using Float    = double;
using String   = std::basic_string<character>;
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

template<typename charT = character, typename traits = std::char_traits<charT>>
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

template<typename charT = character, typename traits = std::char_traits<charT>,
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

template<typename T>
struct is_exact_toml_type : disjunction<
    std::is_same<T, Boolean >,
    std::is_same<T, Integer >,
    std::is_same<T, Float   >,
    std::is_same<T, String  >,
    std::is_same<T, Datetime>,
    std::is_same<T, Array   >,
    std::is_same<T, Table   >
    >{};

template<typename T>
struct is_map : conjunction<
    has_iterator<T>,
    has_value_type<T>,
    has_key_type<T>,
    has_mapped_type<T>
    >{};

template<typename T>
struct is_container : conjunction<
    negation<is_map<T>>,
    negation<std::is_same<T, String>>,
    has_iterator<T>,
    has_value_type<T>
    >{};

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
    static_assert(std::is_same<T, toml::Array>::value ||
                  std::is_same<T, toml::Table>::value,
                  "toml::detail::storage is for toml::Array or toml::Table!");
    typedef T value_type;

    storage() = default;
    ~storage() noexcept override = default;
    storage(storage const&) = default;
    storage(storage&&)      = default;
    storage& operator=(storage const&) = default;
    storage& operator=(storage&&)      = default;

    storage(value_type const& v) : value(v){}
    storage(value_type&& v) : value(std::move(v)){}

    value_type value;
};
} // detail

template<typename T>
struct value_traits
{
    constexpr static value_t type_index = detail::check_type<T>();
    constexpr static bool is_toml_type  = detail::is_valid(detail::check_type<T>());
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
    constexpr value_t kind = toml::detail::check_type<T>();
    switch_assign<kind>::invoke(*this, std::forward<T>(v));
}

template<typename T, typename std::enable_if<
    value_traits<T>::is_toml_type, std::nullptr_t>::type>
value& value::operator=(T&& v)
{
    constexpr value_t kind = toml::detail::check_type<T>();
    if(should_be_cleaned(this->type_))
    {
        switch_clean(this->type_);
    }
    this->type_ = kind;
    switch_assign<kind>::invoke(*this, std::forward<T>(v));
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

}// toml
#endif// TOML11_VALUE

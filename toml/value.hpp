//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_VALUE_HPP
#define TOML11_VALUE_HPP
#include "traits.hpp"
#include "into.hpp"
#include "utility.hpp"
#include "exception.hpp"
#include "storage.hpp"
#include "region.hpp"
#include "types.hpp"
#include <vector>
#include <tuple>
#include <unordered_map>
#include <cassert>
#include <cstdint>
#if __cplusplus >= 201703L
#include <string_view>
#endif

namespace toml
{

namespace detail
{
// to show error messages. not recommended for users.
region_base const& get_region(const value&);
template<typename Region>
void change_region(value&, Region&&);
}// detail

template<typename T>
struct value_traits
{
    constexpr static value_t type_index = detail::check_type<T>();
    constexpr static bool is_toml_type  = detail::is_valid(detail::check_type<T>());
    typedef typename detail::toml_default_type<type_index>::type type;
};
template<typename T>
constexpr value_t value_traits<T>::type_index;
template<typename T>
constexpr bool    value_traits<T>::is_toml_type;

class value
{
    template<typename T, typename U>
    static void assigner(T& dst, U&& v)
    {
        const auto tmp = ::new(std::addressof(dst)) T(std::forward<U>(v));
        assert(tmp == std::addressof(dst));
    }

    using region_base = detail::region_base;

  public:

    value() noexcept
        : type_(value_t::Empty),
          region_info_(std::make_shared<region_base>(region_base{}))
    {}

    ~value() noexcept {this->cleanup();}

    value(const value& v): type_(v.type()), region_info_(v.region_info_)
    {
        switch(v.type())
        {
            case value_t::Boolean       : assigner(boolean_        , v.boolean_        ); break;
            case value_t::Integer       : assigner(integer_        , v.integer_        ); break;
            case value_t::Float         : assigner(floating_       , v.floating_       ); break;
            case value_t::String        : assigner(string_         , v.string_         ); break;
            case value_t::OffsetDatetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::LocalDatetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::LocalDate     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::LocalTime     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::Array         : assigner(array_          , v.array_          ); break;
            case value_t::Table         : assigner(table_          , v.table_          ); break;
            default: break;
        }
    }
    value(value&& v): type_(v.type()), region_info_(std::move(v.region_info_))
    {
        switch(this->type_)
        {
            case value_t::Boolean       : assigner(boolean_        , std::move(v.boolean_        )); break;
            case value_t::Integer       : assigner(integer_        , std::move(v.integer_        )); break;
            case value_t::Float         : assigner(floating_       , std::move(v.floating_       )); break;
            case value_t::String        : assigner(string_         , std::move(v.string_         )); break;
            case value_t::OffsetDatetime: assigner(offset_datetime_, std::move(v.offset_datetime_)); break;
            case value_t::LocalDatetime : assigner(local_datetime_ , std::move(v.local_datetime_ )); break;
            case value_t::LocalDate     : assigner(local_date_     , std::move(v.local_date_     )); break;
            case value_t::LocalTime     : assigner(local_time_     , std::move(v.local_time_     )); break;
            case value_t::Array         : assigner(array_          , std::move(v.array_          )); break;
            case value_t::Table         : assigner(table_          , std::move(v.table_          )); break;
            default: break;
        }
    }
    value& operator=(const value& v)
    {
        this->cleanup();
        this->region_info_ = v.region_info_;
        this->type_ = v.type();
        switch(this->type_)
        {
            case value_t::Boolean       : assigner(boolean_        , v.boolean_        ); break;
            case value_t::Integer       : assigner(integer_        , v.integer_        ); break;
            case value_t::Float         : assigner(floating_       , v.floating_       ); break;
            case value_t::String        : assigner(string_         , v.string_         ); break;
            case value_t::OffsetDatetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::LocalDatetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::LocalDate     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::LocalTime     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::Array         : assigner(array_          , v.array_          ); break;
            case value_t::Table         : assigner(table_          , v.table_          ); break;
            default: break;
        }
        return *this;
    }
    value& operator=(value&& v)
    {
        this->cleanup();
        this->region_info_ = std::move(v.region_info_);
        this->type_ = v.type();
        switch(this->type_)
        {
            case value_t::Boolean       : assigner(boolean_        , std::move(v.boolean_        )); break;
            case value_t::Integer       : assigner(integer_        , std::move(v.integer_        )); break;
            case value_t::Float         : assigner(floating_       , std::move(v.floating_       )); break;
            case value_t::String        : assigner(string_         , std::move(v.string_         )); break;
            case value_t::OffsetDatetime: assigner(offset_datetime_, std::move(v.offset_datetime_)); break;
            case value_t::LocalDatetime : assigner(local_datetime_ , std::move(v.local_datetime_ )); break;
            case value_t::LocalDate     : assigner(local_date_     , std::move(v.local_date_     )); break;
            case value_t::LocalTime     : assigner(local_time_     , std::move(v.local_time_     )); break;
            case value_t::Array         : assigner(array_          , std::move(v.array_          )); break;
            case value_t::Table         : assigner(table_          , std::move(v.table_          )); break;
            default: break;
        }
        return *this;
    }

    // boolean ==============================================================

    value(boolean b)
        : type_(value_t::Boolean),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->boolean_, b);
    }

    value& operator=(boolean b)
    {
        this->cleanup();
        this->type_ = value_t::Boolean;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->boolean_, b);
        return *this;
    }

    template<typename Container>
    value(boolean b, detail::region<Container> reg)
        : type_(value_t::Boolean),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->boolean_, b);
    }

    // integer ==============================================================

    template<typename T, typename std::enable_if<detail::conjunction<
        std::is_integral<T>, detail::negation<std::is_same<T, boolean>>>::value,
        std::nullptr_t>::type = nullptr>
    value(T i)
        : type_(value_t::Integer),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->integer_, static_cast<integer>(i));
    }

    template<typename T, typename Container, typename std::enable_if<
        detail::conjunction<std::is_integral<T>,
        detail::negation<std::is_same<T, boolean>>
        >::value, std::nullptr_t>::type = nullptr>
    value(T i, detail::region<Container> reg)
        : type_(value_t::Integer),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->integer_, static_cast<integer>(i));
    }

    template<typename T, typename std::enable_if<detail::conjunction<
        std::is_integral<T>, detail::negation<std::is_same<T, boolean>>>::value,
        std::nullptr_t>::type = nullptr>
    value& operator=(T i)
    {
        this->cleanup();
        this->type_ = value_t::Integer;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->integer_, static_cast<integer>(i));
        return *this;
    }

    // floating =============================================================

    template<typename T, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    value(T f)
        : type_(value_t::Float),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->floating_, f);
    }

    template<typename T, typename Container, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    value(T f, detail::region<Container> reg)
        : type_(value_t::Float),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->floating_, f);
    }

    template<typename T, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    value& operator=(T f)
    {
        this->cleanup();
        this->type_ = value_t::Float;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->floating_, f);
        return *this;
    }

    // string ===============================================================

    value(toml::string s)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, std::move(s));
    }
    template<typename Container>
    value(toml::string s, detail::region<Container> reg)
        : type_(value_t::String),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->string_, std::move(s));
    }
    value& operator=(toml::string s)
    {
        this->cleanup();
        this->type_ = value_t::String;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, s);
        return *this;
    }

    value(std::string s)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::move(s)));
    }
    value& operator=(std::string s)
    {
        this->cleanup();
        this->type_ = value_t::String;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(std::move(s)));
        return *this;
    }
    value(std::string s, string_t kind)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::move(s), kind));
    }

    value(const char* s)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::string(s)));
    }
    value& operator=(const char* s)
    {
        this->cleanup();
        this->type_ = value_t::String;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(std::string(s)));
        return *this;
    }
    value(const char* s, string_t kind)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::string(s), kind));
    }

#if __cplusplus >= 201703L
    value(std::string_view s)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(s));
    }
    value& operator=(std::string_view s)
    {
        this->cleanup();
        this->type_ = value_t::String;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(s));
        return *this;
    }
    value(std::string_view s, string_t kind)
        : type_(value_t::String),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(s, kind));
    }
#endif

    // local date ===========================================================

    value(const local_date& ld)
        : type_(value_t::LocalDate),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_date_, ld);
    }
    template<typename Container>
    value(const local_date& ld, detail::region<Container> reg)
        : type_(value_t::LocalDate),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->local_date_, ld);
    }
    value& operator=(const local_date& ld)
    {
        this->cleanup();
        this->type_ = value_t::LocalDate;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_date_, ld);
        return *this;
    }

    // local time ===========================================================

    value(const local_time& lt)
        : type_(value_t::LocalTime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_time_, lt);
    }
    template<typename Container>
    value(const local_time& lt, detail::region<Container> reg)
        : type_(value_t::LocalTime),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->local_time_, lt);
    }
    value& operator=(const local_time& lt)
    {
        this->cleanup();
        this->type_ = value_t::LocalTime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_time_, lt);
        return *this;
    }
    template<typename Rep, typename Period>
    value(const std::chrono::duration<Rep, Period>& dur)
        : type_(value_t::LocalTime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_time_, local_time(dur));
    }
    template<typename Rep, typename Period>
    value& operator=(const std::chrono::duration<Rep, Period>& dur)
    {
        this->cleanup();
        this->type_ = value_t::LocalTime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_time_, local_time(dur));
        return *this;
    }

    // local datetime =======================================================

    value(const local_datetime& ldt)
        : type_(value_t::LocalDatetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_datetime_, ldt);
    }
    template<typename Container>
    value(const local_datetime& ldt, detail::region<Container> reg)
        : type_(value_t::LocalDatetime),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->local_datetime_, ldt);
    }
    value& operator=(const local_datetime& ldt)
    {
        this->cleanup();
        this->type_ = value_t::LocalDatetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_datetime_, ldt);
        return *this;
    }

    // offset datetime ======================================================

    value(const offset_datetime& odt)
        : type_(value_t::OffsetDatetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->offset_datetime_, odt);
    }
    template<typename Container>
    value(const offset_datetime& odt, detail::region<Container> reg)
        : type_(value_t::OffsetDatetime),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->offset_datetime_, odt);
    }
    value& operator=(const offset_datetime& odt)
    {
        this->cleanup();
        this->type_ = value_t::OffsetDatetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->offset_datetime_, odt);
        return *this;
    }
    value(const std::chrono::system_clock::time_point& tp)
        : type_(value_t::OffsetDatetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->offset_datetime_, offset_datetime(tp));
    }
    value& operator=(const std::chrono::system_clock::time_point& tp)
    {
        this->cleanup();
        this->type_ = value_t::OffsetDatetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->offset_datetime_, offset_datetime(tp));
        return *this;
    }

    // array ================================================================

    value(const array& ary)
        : type_(value_t::Array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->array_, ary);
    }
    template<typename Container>
    value(const array& ary, detail::region<Container> reg)
        : type_(value_t::Array),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->array_, ary);
    }
    value& operator=(const array& ary)
    {
        this->cleanup();
        this->type_ = value_t::Array;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->array_, ary);
        return *this;
    }

    template<typename T, typename std::enable_if<value_traits<T>::is_toml_type,
        std::nullptr_t>::type = nullptr>
    value(std::initializer_list<T> list)
        : type_(value_t::Array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        array ary; ary.reserve(list.size());
        for(auto& elem : list) {ary.emplace_back(std::move(elem));}
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<value_traits<T>::is_toml_type,
        std::nullptr_t>::type = nullptr>
    value& operator=(std::initializer_list<T> list)
    {
        this->cleanup();
        this->type_ = value_t::Array;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        array ary; ary.reserve(list.size());
        for(auto& elem : list) {ary.emplace_back(std::move(elem));}
        assigner(this->array_, std::move(ary));
        return *this;
    }

    template<typename T, typename std::enable_if<detail::is_container<T>::value,
        std::nullptr_t>::type = nullptr>
    value(T&& list)
        : type_(value_t::Array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        array ary; ary.reserve(list.size());
        for(const auto& elem : list) {ary.emplace_back(elem);}
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<detail::is_container<T>::value,
        std::nullptr_t>::type = nullptr>
    value& operator=(T&& list)
    {
        this->cleanup();
        this->type_ = value_t::Array;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        array ary; ary.reserve(list.size());
        for(const auto& elem : list) {ary.emplace_back(elem);}
        assigner(this->array_, std::move(ary));
        return *this;
    }

    // table ================================================================

    value(const table& tab)
        : type_(value_t::Table),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->table_, tab);
    }
    template<typename Container>
    value(const table& tab, detail::region<Container> reg)
        : type_(value_t::Table),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg)))
    {
        assigner(this->table_, tab);
    }
    value& operator=(const table& tab)
    {
        this->cleanup();
        this->type_ = value_t::Table;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->table_, tab);
        return *this;
    }
    value(std::initializer_list<std::pair<key, value>> list)
        : type_(value_t::Table),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        table tab;
        for(const auto& elem : list) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
    }
    value& operator=(std::initializer_list<std::pair<key, value>> list)
    {
        this->cleanup();
        this->type_ = value_t::Array;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        table tab;
        for(const auto& elem : list) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
        return *this;
    }

    // user-defined =========================================================

    // convert using into_toml() method -------------------------------------

    template<typename T, typename std::enable_if<detail::conjunction<
        detail::negation<detail::is_exact_toml_type<T>>, // not a toml::value
        detail::has_into_toml_method<T> // but has `into_toml` method
        >::value, std::nullptr_t>::type = nullptr>
    value(const T& ud): value(ud.into_toml()) {}

    template<typename T, typename std::enable_if<detail::conjunction<
        detail::negation<detail::is_exact_toml_type<T>>, // not a toml::value
        detail::has_into_toml_method<T> // but has `into_toml` method
        >::value, std::nullptr_t>::type = nullptr>
    value& operator=(const T& ud)
    {
        *this = ud.into_toml();
        return *this;
    }

    // convert using into<T> struct -----------------------------------------

    template<typename T, typename std::enable_if<
        detail::negation<detail::is_exact_toml_type<T>>::value,
        std::nullptr_t>::type = nullptr,
        std::size_t S = sizeof(::toml::into<T>)>
    value(const T& ud): value(::toml::into<T>::into_toml(ud)) {}

    template<typename T, typename std::enable_if<
        detail::negation<detail::is_exact_toml_type<T>>::value,
        std::nullptr_t>::type = nullptr,
        std::size_t S = sizeof(::toml::into<T>)>
    value& operator=(const T& ud)
    {
        *this = ::toml::into<T>::into_toml(ud);
        return *this;
    }

    // for internal use ------------------------------------------------------

    template<typename T, typename Container, typename std::enable_if<
        detail::is_exact_toml_type<T>::value, std::nullptr_t>::type = nullptr>
    value(std::pair<T, detail::region<Container>> parse_result)
        : value(std::move(parse_result.first), std::move(parse_result.second))
    {}

    // type checking and casting ============================================

    template<typename T>
    bool is() const noexcept {return value_traits<T>::type_index == this->type_;}
    bool is(value_t t) const noexcept {return t == this->type_;}

    bool is_uninitialized()   const noexcept {return this->is(value_t::Empty         );}
    bool is_boolean()         const noexcept {return this->is(value_t::Boolean       );}
    bool is_integer()         const noexcept {return this->is(value_t::Integer       );}
    bool is_float()           const noexcept {return this->is(value_t::Float         );}
    bool is_string()          const noexcept {return this->is(value_t::String        );}
    bool is_offset_datetime() const noexcept {return this->is(value_t::OffsetDatetime);}
    bool is_local_datetime()  const noexcept {return this->is(value_t::LocalDatetime );}
    bool is_local_date()      const noexcept {return this->is(value_t::LocalDate     );}
    bool is_local_time()      const noexcept {return this->is(value_t::LocalTime     );}
    bool is_array()           const noexcept {return this->is(value_t::Array         );}
    bool is_table()           const noexcept {return this->is(value_t::Table         );}

    value_t type() const {return type_;}

    template<value_t T>
    typename detail::toml_default_type<T>::type&       cast() &;
    template<value_t T>
    typename detail::toml_default_type<T>::type const& cast() const&;
    template<value_t T>
    typename detail::toml_default_type<T>::type&&      cast() &&;

    boolean         const& as_boolean()         const {return this->cast<value_t::Boolean       >();}
    integer         const& as_integer()         const {return this->cast<value_t::Integer       >();}
    floating        const& as_float()           const {return this->cast<value_t::Float         >();}
    string          const& as_string()          const {return this->cast<value_t::String        >();}
    offset_datetime const& as_offset_datetime() const {return this->cast<value_t::OffsetDatetime>();}
    local_datetime  const& as_local_datetime()  const {return this->cast<value_t::LocalDatetime >();}
    local_date      const& as_local_date()      const {return this->cast<value_t::LocalDate     >();}
    local_time      const& as_local_time()      const {return this->cast<value_t::LocalTime     >();}
    array           const& as_array()           const {return this->cast<value_t::Array         >();}
    table           const& as_table()           const {return this->cast<value_t::Table         >();}

    boolean&          as_boolean()         {return this->cast<value_t::Boolean       >();}
    integer&          as_integer()         {return this->cast<value_t::Integer       >();}
    floating&         as_float()           {return this->cast<value_t::Float         >();}
    string&           as_string()          {return this->cast<value_t::String        >();}
    offset_datetime&  as_offset_datetime() {return this->cast<value_t::OffsetDatetime>();}
    local_datetime&   as_local_datetime()  {return this->cast<value_t::LocalDatetime >();}
    local_date&       as_local_date()      {return this->cast<value_t::LocalDate     >();}
    local_time&       as_local_time()      {return this->cast<value_t::LocalTime     >();}
    array&            as_array()           {return this->cast<value_t::Array         >();}
    table&            as_table()           {return this->cast<value_t::Table         >();}

    std::string comment() const
    {
        return this->region_info_->comment();
    }
    std::string comment_before() const
    {
        return this->region_info_->comment_before();
    }
    std::string comment_inline() const
    {
        return this->region_info_->comment_inline();
    }

  private:

    void cleanup() noexcept
    {
        switch(this->type_)
        {
            case value_t::String : {string_.~string();       return;}
            case value_t::Array  : {array_.~array_storage(); return;}
            case value_t::Table  : {table_.~table_storage(); return;}
            default              : return;
        }
    }

    // for error messages
    friend region_base const& detail::get_region(const value&);

    template<typename Region>
    friend void detail::change_region(value&, Region&&);

    template<value_t T>
    struct switch_cast;

  private:

    using array_storage = detail::storage<array>;
    using table_storage = detail::storage<table>;

    value_t type_;

    // for error message information.
    std::shared_ptr<region_base> region_info_;

    union
    {
        boolean         boolean_;
        integer         integer_;
        floating        floating_;
        string          string_;
        offset_datetime offset_datetime_;
        local_datetime  local_datetime_;
        local_date      local_date_;
        local_time      local_time_;
        array_storage   array_;
        table_storage   table_;
    };
};

namespace detail
{
inline region_base const& get_region(const value& v)
{
    return *(v.region_info_);
}

template<typename Region>
void change_region(value& v, Region&& reg)
{
    using region_type = typename std::remove_reference<
        typename std::remove_cv<Region>::type
        >::type;

    std::shared_ptr<region_base> new_reg =
        std::make_shared<region_type>(std::forward<region_type>(reg));
    v.region_info_ = new_reg;
    return;
}

}// detail

template<> struct value::switch_cast<value_t::Boolean>
{
    static Boolean&       invoke(value&       v) {return v.boolean_;}
    static Boolean const& invoke(value const& v) {return v.boolean_;}
    static Boolean&&      invoke(value&&      v) {return std::move(v.boolean_);}
};
template<> struct value::switch_cast<value_t::Integer>
{
    static Integer&       invoke(value&       v) {return v.integer_;}
    static Integer const& invoke(value const& v) {return v.integer_;}
    static Integer&&      invoke(value&&      v) {return std::move(v.integer_);}
};
template<> struct value::switch_cast<value_t::Float>
{
    static Float&       invoke(value&       v) {return v.floating_;}
    static Float const& invoke(value const& v) {return v.floating_;}
    static Float&&      invoke(value&&      v) {return std::move(v.floating_);}
};
template<> struct value::switch_cast<value_t::String>
{
    static String&       invoke(value&       v) {return v.string_;}
    static String const& invoke(value const& v) {return v.string_;}
    static String&&      invoke(value&&      v) {return std::move(v.string_);}
};
template<> struct value::switch_cast<value_t::OffsetDatetime>
{
    static OffsetDatetime&       invoke(value&       v) {return v.offset_datetime_;}
    static OffsetDatetime const& invoke(value const& v) {return v.offset_datetime_;}
    static OffsetDatetime&&      invoke(value&&      v) {return std::move(v.offset_datetime_);}
};
template<> struct value::switch_cast<value_t::LocalDatetime>
{
    static LocalDatetime&       invoke(value&       v) {return v.local_datetime_;}
    static LocalDatetime const& invoke(value const& v) {return v.local_datetime_;}
    static LocalDatetime&&      invoke(value&&      v) {return std::move(v.local_datetime_);}
};
template<> struct value::switch_cast<value_t::LocalDate>
{
    static LocalDate&       invoke(value&       v) {return v.local_date_;}
    static LocalDate const& invoke(value const& v) {return v.local_date_;}
    static LocalDate&&      invoke(value&&      v) {return std::move(v.local_date_);}
};
template<> struct value::switch_cast<value_t::LocalTime>
{
    static LocalTime&       invoke(value&       v) {return v.local_time_;}
    static LocalTime const& invoke(value const& v) {return v.local_time_;}
    static LocalTime&&      invoke(value&&      v) {return std::move(v.local_time_);}
};
template<> struct value::switch_cast<value_t::Array>
{
    static Array&       invoke(value&       v) {return v.array_.value();}
    static Array const& invoke(value const& v) {return v.array_.value();}
    static Array&&      invoke(value&&      v) {return std::move(v.array_.value());}
};
template<> struct value::switch_cast<value_t::Table>
{
    static Table&       invoke(value&       v) {return v.table_.value();}
    static Table const& invoke(value const& v) {return v.table_.value();}
    static Table&&      invoke(value&&      v) {return std::move(v.table_.value());}
};

template<value_t T>
typename detail::toml_default_type<T>::type& value::cast() &
{
    if(T != this->type_)
    {
        throw type_error(detail::format_underline(concat_to_string(
            "[error] toml::value bad_cast to ", T), {
                {this->region_info_.get(),
                 concat_to_string("the actual type is ", this->type_)}
            }));
    }
    return switch_cast<T>::invoke(*this);
}
template<value_t T>
typename detail::toml_default_type<T>::type const& value::cast() const&
{
    if(T != this->type_)
    {
        throw type_error(detail::format_underline(concat_to_string(
            "[error] toml::value bad_cast to ", T), {
                {this->region_info_.get(),
                 concat_to_string("the actual type is ", this->type_)}
            }));
    }
    return switch_cast<T>::invoke(*this);
}
template<value_t T>
typename detail::toml_default_type<T>::type&&      value::cast() &&
{
    if(T != this->type_)
    {
        throw type_error(detail::format_underline(concat_to_string(
            "[error] toml::value bad_cast to ", T), {
                {this->region_info_.get(),
                 concat_to_string("the actual type is ", this->type_)}
             }));
    }
    return switch_cast<T>::invoke(std::move(*this));
}

inline bool operator==(const toml::value& lhs, const toml::value& rhs)
{
    if(lhs.type() != rhs.type()){return false;}
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
        case value_t::OffsetDatetime:
            return lhs.cast<value_t::OffsetDatetime>() == rhs.cast<value_t::OffsetDatetime>();
        case value_t::LocalDatetime:
            return lhs.cast<value_t::LocalDatetime>() == rhs.cast<value_t::LocalDatetime>();
        case value_t::LocalDate:
            return lhs.cast<value_t::LocalDate>() == rhs.cast<value_t::LocalDate>();
        case value_t::LocalTime:
            return lhs.cast<value_t::LocalTime>() == rhs.cast<value_t::LocalTime>();
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
    if(lhs.type() != rhs.type()){return (lhs.type() < rhs.type());}
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
        case value_t::OffsetDatetime:
            return lhs.cast<value_t::OffsetDatetime>() < rhs.cast<value_t::OffsetDatetime>();
        case value_t::LocalDatetime:
            return lhs.cast<value_t::LocalDatetime>() < rhs.cast<value_t::LocalDatetime>();
        case value_t::LocalDate:
            return lhs.cast<value_t::LocalDate>() < rhs.cast<value_t::LocalDate>();
        case value_t::LocalTime:
            return lhs.cast<value_t::LocalTime>() < rhs.cast<value_t::LocalTime>();
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

inline std::string format_error(const std::string& err_msg,
        const toml::value& v, const std::string& comment,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v)), comment}
        }, std::move(hints));
}

inline std::string format_error(const std::string& err_msg,
        const toml::value& v1, const std::string& comment1,
        const toml::value& v2, const std::string& comment2,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v1)), comment1},
            {std::addressof(detail::get_region(v2)), comment2}
        }, std::move(hints));
}

inline std::string format_error(const std::string& err_msg,
        const toml::value& v1, const std::string& comment1,
        const toml::value& v2, const std::string& comment2,
        const toml::value& v3, const std::string& comment3,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v1)), comment1},
            {std::addressof(detail::get_region(v2)), comment2},
            {std::addressof(detail::get_region(v3)), comment3}
        }, std::move(hints));
}

template<typename Visitor>
detail::return_type_of_t<Visitor, const toml::boolean&>
visit(Visitor&& visitor, const toml::value& v)
{
    switch(v.type())
    {
        case value_t::Boolean       : {return visitor(v.cast<value_t::Boolean       >());}
        case value_t::Integer       : {return visitor(v.cast<value_t::Integer       >());}
        case value_t::Float         : {return visitor(v.cast<value_t::Float         >());}
        case value_t::String        : {return visitor(v.cast<value_t::String        >());}
        case value_t::OffsetDatetime: {return visitor(v.cast<value_t::OffsetDatetime>());}
        case value_t::LocalDatetime : {return visitor(v.cast<value_t::LocalDatetime >());}
        case value_t::LocalDate     : {return visitor(v.cast<value_t::LocalDate     >());}
        case value_t::LocalTime     : {return visitor(v.cast<value_t::LocalTime     >());}
        case value_t::Array         : {return visitor(v.cast<value_t::Array         >());}
        case value_t::Table         : {return visitor(v.cast<value_t::Table         >());}
        case value_t::Empty         : break;
        case value_t::Unknown       : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::value "
            "does not have any valid value.", v, "here"));
}

template<typename Visitor>
detail::return_type_of_t<Visitor, toml::boolean&>
visit(Visitor&& visitor, toml::value& v)
{
    switch(v.type())
    {
        case value_t::Boolean       : {return visitor(v.cast<value_t::Boolean       >());}
        case value_t::Integer       : {return visitor(v.cast<value_t::Integer       >());}
        case value_t::Float         : {return visitor(v.cast<value_t::Float         >());}
        case value_t::String        : {return visitor(v.cast<value_t::String        >());}
        case value_t::OffsetDatetime: {return visitor(v.cast<value_t::OffsetDatetime>());}
        case value_t::LocalDatetime : {return visitor(v.cast<value_t::LocalDatetime >());}
        case value_t::LocalDate     : {return visitor(v.cast<value_t::LocalDate     >());}
        case value_t::LocalTime     : {return visitor(v.cast<value_t::LocalTime     >());}
        case value_t::Array         : {return visitor(v.cast<value_t::Array         >());}
        case value_t::Table         : {return visitor(v.cast<value_t::Table         >());}
        case value_t::Empty         : break;
        case value_t::Unknown       : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::value "
            "does not have any valid value.", v, "here"));
}

template<typename Visitor>
detail::return_type_of_t<Visitor, toml::boolean&&>
visit(Visitor&& visitor, toml::value&& v)
{
    switch(v.type())
    {
        case value_t::Boolean       : {return visitor(std::move(v.cast<value_t::Boolean       >()));}
        case value_t::Integer       : {return visitor(std::move(v.cast<value_t::Integer       >()));}
        case value_t::Float         : {return visitor(std::move(v.cast<value_t::Float         >()));}
        case value_t::String        : {return visitor(std::move(v.cast<value_t::String        >()));}
        case value_t::OffsetDatetime: {return visitor(std::move(v.cast<value_t::OffsetDatetime>()));}
        case value_t::LocalDatetime : {return visitor(std::move(v.cast<value_t::LocalDatetime >()));}
        case value_t::LocalDate     : {return visitor(std::move(v.cast<value_t::LocalDate     >()));}
        case value_t::LocalTime     : {return visitor(std::move(v.cast<value_t::LocalTime     >()));}
        case value_t::Array         : {return visitor(std::move(v.cast<value_t::Array         >()));}
        case value_t::Table         : {return visitor(std::move(v.cast<value_t::Table         >()));}
        case value_t::Empty         : break;
        case value_t::Unknown       : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::value "
            "does not have any valid value.", v, "here"));
}

}// toml
#endif// TOML11_VALUE

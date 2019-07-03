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
#include "source_location.hpp"
#include "comments.hpp"
#include <cassert>

namespace toml
{

namespace detail
{

// to show error messages. not recommended for users.
template<typename C, template<typename ...> class T, template<typename ...> class A>
region_base const& get_region(const basic_value<C, T, A>&);
template<typename Region,
         typename C, template<typename ...> class T, template<typename ...> class A>
void change_region(basic_value<C, T, A>&, Region&&);

template<value_t Expected,
         typename C, template<typename ...> class T, template<typename ...> class A>
[[noreturn]] inline void
throw_bad_cast(value_t actual, const ::toml::basic_value<C, T, A>& v)
{
    throw type_error(detail::format_underline(concat_to_string(
        "[error] toml::value bad_cast to ", Expected), {
            {std::addressof(get_region(v)),
             concat_to_string("the actual type is ", actual)}
        }));
}

// switch by `value_t` and call the corresponding `value::as_xxx()`. {{{
template<value_t T>
struct switch_cast {};
template<>
struct switch_cast<value_t::boolean>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::boolean& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_boolean();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::boolean const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_boolean();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::boolean&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_boolean();
    }
};
template<>
struct switch_cast<value_t::integer>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::integer& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_integer();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::integer const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_integer();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::integer&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_integer();
    }
};
template<>
struct switch_cast<value_t::floating>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::floating& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_floating();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::floating const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_floating();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::floating&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_floating();
    }
};
template<>
struct switch_cast<value_t::string>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::string& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_string();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::string const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_string();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::string&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_string();
    }
};
template<>
struct switch_cast<value_t::offset_datetime>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::offset_datetime& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_offset_datetime();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::offset_datetime const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_offset_datetime();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::offset_datetime&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_offset_datetime();
    }
};
template<>
struct switch_cast<value_t::local_datetime>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_datetime& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_local_datetime();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_datetime const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_local_datetime();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_datetime&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_local_datetime();
    }
};
template<>
struct switch_cast<value_t::local_date>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_date& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_local_date();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_date const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_local_date();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_date&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_local_date();
    }
};
template<>
struct switch_cast<value_t::local_time>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_time& invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_local_time();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_time const& invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_local_time();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static ::toml::local_time&& invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_local_time();
    }
};
template<>
struct switch_cast<value_t::array>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::array_type&
    invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_array();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::array_type const&
    invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_array();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::array_type &&
    invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_array();
    }
};
template<>
struct switch_cast<value_t::table>
{
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::table_type&
    invoke(basic_value<C, T, A>& v) noexcept
    {
        return v.as_table();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::table_type const&
    invoke(basic_value<C, T, A> const& v) noexcept
    {
        return v.as_table();
    }
    template<typename C, template<typename ...> class T, template<typename ...> class A>
    static typename basic_value<C, T, A>::table_type &&
    invoke(basic_value<C, T, A>&& v) noexcept
    {
        return std::move(v).as_table();
    }
}; // }}}

}// detail

template<typename Comment, // discard/preserve_comment
         template<typename ...> class Table = std::unordered_map,
         template<typename ...> class Array = std::vector>
class basic_value
{
    template<typename T, typename U>
    static void assigner(T& dst, U&& v)
    {
        const auto tmp = ::new(std::addressof(dst)) T(std::forward<U>(v));
        assert(tmp == std::addressof(dst));
        (void)tmp;
    }

    using region_base = detail::region_base;

    template<typename C, template<typename ...> class T,
             template<typename ...> class A>
    friend class basic_value;

  public:

    using comment_type         = Comment;
    using key_type             = ::toml::key;
    using value_type           = basic_value<comment_type, Table, Array>;
    using boolean_type         = ::toml::boolean;
    using integer_type         = ::toml::integer;
    using floating_type        = ::toml::floating;
    using string_type          = ::toml::string;
    using local_time_type      = ::toml::local_time;
    using local_date_type      = ::toml::local_date;
    using local_datetime_type  = ::toml::local_datetime;
    using offset_datetime_type = ::toml::offset_datetime;
    using array_type           = Array<value_type>;
    using table_type           = Table<key_type, value_type>;

  public:

    basic_value() noexcept
        : type_(value_t::empty),
          region_info_(std::make_shared<region_base>(region_base{}))
    {}
    ~basic_value() noexcept {this->cleanup();}

    basic_value(const basic_value& v)
        : type_(v.type()), region_info_(v.region_info_), comments_(v.comments_)
    {
        switch(v.type())
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          : assigner(array_          , v.array_          ); break;
            case value_t::table          : assigner(table_          , v.table_          ); break;
            default: break;
        }
    }
    basic_value(basic_value&& v)
        : type_(v.type()), region_info_(std::move(v.region_info_)),
          comments_(std::move(v.comments_))
    {
        switch(this->type_) // here this->type_ is already initialized
        {
            case value_t::boolean        : assigner(boolean_        , std::move(v.boolean_        )); break;
            case value_t::integer        : assigner(integer_        , std::move(v.integer_        )); break;
            case value_t::floating       : assigner(floating_       , std::move(v.floating_       )); break;
            case value_t::string         : assigner(string_         , std::move(v.string_         )); break;
            case value_t::offset_datetime: assigner(offset_datetime_, std::move(v.offset_datetime_)); break;
            case value_t::local_datetime : assigner(local_datetime_ , std::move(v.local_datetime_ )); break;
            case value_t::local_date     : assigner(local_date_     , std::move(v.local_date_     )); break;
            case value_t::local_time     : assigner(local_time_     , std::move(v.local_time_     )); break;
            case value_t::array          : assigner(array_          , std::move(v.array_          )); break;
            case value_t::table          : assigner(table_          , std::move(v.table_          )); break;
            default: break;
        }
    }
    basic_value& operator=(const basic_value& v)
    {
        this->cleanup();
        this->region_info_ = v.region_info_;
        this->comments_ = v.comments_;
        this->type_ = v.type();
        switch(this->type_)
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          : assigner(array_          , v.array_          ); break;
            case value_t::table          : assigner(table_          , v.table_          ); break;
            default: break;
        }
        return *this;
    }
    basic_value& operator=(basic_value&& v)
    {
        this->cleanup();
        this->region_info_ = std::move(v.region_info_);
        this->comments_ = std::move(v.comments_);
        this->type_ = v.type();
        switch(this->type_)
        {
            case value_t::boolean        : assigner(boolean_        , std::move(v.boolean_        )); break;
            case value_t::integer        : assigner(integer_        , std::move(v.integer_        )); break;
            case value_t::floating       : assigner(floating_       , std::move(v.floating_       )); break;
            case value_t::string         : assigner(string_         , std::move(v.string_         )); break;
            case value_t::offset_datetime: assigner(offset_datetime_, std::move(v.offset_datetime_)); break;
            case value_t::local_datetime : assigner(local_datetime_ , std::move(v.local_datetime_ )); break;
            case value_t::local_date     : assigner(local_date_     , std::move(v.local_date_     )); break;
            case value_t::local_time     : assigner(local_time_     , std::move(v.local_time_     )); break;
            case value_t::array          : assigner(array_          , std::move(v.array_          )); break;
            case value_t::table          : assigner(table_          , std::move(v.table_          )); break;
            default: break;
        }
        return *this;
    }

    // overwrite comments ----------------------------------------------------

    basic_value(const basic_value& v, std::vector<std::string> comments)
        : type_(v.type()), region_info_(v.region_info_),
          comments_(std::move(comments))
    {
        switch(v.type())
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          : assigner(array_          , v.array_          ); break;
            case value_t::table          : assigner(table_          , v.table_          ); break;
            default: break;
        }
    }

    basic_value(basic_value&& v, std::vector<std::string> comments)
        : type_(v.type()), region_info_(std::move(v.region_info_)),
          comments_(std::move(comments))
    {
        switch(this->type_) // here this->type_ is already initialized
        {
            case value_t::boolean        : assigner(boolean_        , std::move(v.boolean_        )); break;
            case value_t::integer        : assigner(integer_        , std::move(v.integer_        )); break;
            case value_t::floating       : assigner(floating_       , std::move(v.floating_       )); break;
            case value_t::string         : assigner(string_         , std::move(v.string_         )); break;
            case value_t::offset_datetime: assigner(offset_datetime_, std::move(v.offset_datetime_)); break;
            case value_t::local_datetime : assigner(local_datetime_ , std::move(v.local_datetime_ )); break;
            case value_t::local_date     : assigner(local_date_     , std::move(v.local_date_     )); break;
            case value_t::local_time     : assigner(local_time_     , std::move(v.local_time_     )); break;
            case value_t::array          : assigner(array_          , std::move(v.array_          )); break;
            case value_t::table          : assigner(table_          , std::move(v.table_          )); break;
            default: break;
        }
    }

    // -----------------------------------------------------------------------
    // conversion between different basic_values.
    template<typename C,
             template<typename ...> class T,
             template<typename ...> class A>
    basic_value(const basic_value<C, T, A>& v)
        : type_(v.type()), region_info_(v.region_info_), comments_(v.comments())
    {
        switch(v.type())
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          :
            {
                array_type tmp(v.as_array(std::nothrow).begin(),
                               v.as_array(std::nothrow).end());
                assigner(array_, std::move(tmp));
                break;
            }
            case value_t::table          :
            {
                table_type tmp(v.as_table(std::nothrow).begin(),
                               v.as_table(std::nothrow).end());
                assigner(table_, std::move(tmp));
                break;
            }
            default: break;
        }
    }
    template<typename C,
             template<typename ...> class T,
             template<typename ...> class A>
    basic_value(const basic_value<C, T, A>& v, std::vector<std::string> comments)
        : type_(v.type()), region_info_(v.region_info_),
          comments_(std::move(comments))
    {
        switch(v.type())
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          :
            {
                array_type tmp(v.as_array(std::nothrow).begin(),
                               v.as_array(std::nothrow).end());
                assigner(array_, std::move(tmp));
                break;
            }
            case value_t::table          :
            {
                table_type tmp(v.as_table(std::nothrow).begin(),
                               v.as_table(std::nothrow).end());
                assigner(table_, std::move(tmp));
                break;
            }
            default: break;
        }
    }
    template<typename C,
             template<typename ...> class T,
             template<typename ...> class A>
    basic_value& operator=(const basic_value<C, T, A>& v)
    {
        this->region_info_ = v.region_info_;
        this->comments_    = comment_type(v.comments());
        this->type_        = v.type();
        switch(v.type())
        {
            case value_t::boolean        : assigner(boolean_        , v.boolean_        ); break;
            case value_t::integer        : assigner(integer_        , v.integer_        ); break;
            case value_t::floating       : assigner(floating_       , v.floating_       ); break;
            case value_t::string         : assigner(string_         , v.string_         ); break;
            case value_t::offset_datetime: assigner(offset_datetime_, v.offset_datetime_); break;
            case value_t::local_datetime : assigner(local_datetime_ , v.local_datetime_ ); break;
            case value_t::local_date     : assigner(local_date_     , v.local_date_     ); break;
            case value_t::local_time     : assigner(local_time_     , v.local_time_     ); break;
            case value_t::array          :
            {
                array_type tmp(v.as_array(std::nothrow).begin(),
                               v.as_array(std::nothrow).end());
                assigner(array_, std::move(tmp));
                break;
            }
            case value_t::table          :
            {
                table_type tmp(v.as_table(std::nothrow).begin(),
                               v.as_table(std::nothrow).end());
                assigner(table_, std::move(tmp));
                break;
            }
            default: break;
        }
        return *this;
    }

    // boolean ==============================================================

    basic_value(boolean b)
        : type_(value_t::boolean),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->boolean_, b);
    }
    basic_value& operator=(boolean b)
    {
        this->cleanup();
        this->type_ = value_t::boolean;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->boolean_, b);
        return *this;
    }
    template<typename Container>
    basic_value(boolean b, detail::region<Container> reg)
        : type_(value_t::boolean),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->boolean_, b);
    }
    basic_value(boolean b, std::vector<std::string> comments)
        : type_(value_t::boolean),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->boolean_, b);
    }

    // integer ==============================================================

    template<typename T, typename std::enable_if<detail::conjunction<
        std::is_integral<T>, detail::negation<std::is_same<T, boolean>>>::value,
        std::nullptr_t>::type = nullptr>
    basic_value(T i)
        : type_(value_t::integer),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->integer_, static_cast<integer>(i));
    }

    template<typename T, typename Container, typename std::enable_if<
        detail::conjunction<
            std::is_integral<T>,
            detail::negation<std::is_same<T, boolean>>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value(T i, detail::region<Container> reg)
        : type_(value_t::integer),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->integer_, static_cast<integer>(i));
    }

    template<typename T, typename std::enable_if<detail::conjunction<
        std::is_integral<T>, detail::negation<std::is_same<T, boolean>>>::value,
        std::nullptr_t>::type = nullptr>
    basic_value& operator=(T i)
    {
        this->cleanup();
        this->type_ = value_t::integer;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->integer_, static_cast<integer>(i));
        return *this;
    }

    template<typename T, typename std::enable_if<detail::conjunction<
        std::is_integral<T>, detail::negation<std::is_same<T, boolean>>>::value,
        std::nullptr_t>::type = nullptr>
    basic_value(T i, std::vector<std::string> comments)
        : type_(value_t::integer),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->integer_, static_cast<integer>(i));
    }

    // floating =============================================================

    template<typename T, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    basic_value(T f)
        : type_(value_t::floating),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->floating_, static_cast<floating>(f));
    }

    template<typename T, typename Container, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    basic_value(T f, detail::region<Container> reg)
        : type_(value_t::floating),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->floating_, static_cast<floating>(f));
    }

    template<typename T, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    basic_value& operator=(T f)
    {
        this->cleanup();
        this->type_ = value_t::floating;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->floating_, static_cast<floating>(f));
        return *this;
    }

    template<typename T, typename std::enable_if<
        std::is_floating_point<T>::value, std::nullptr_t>::type = nullptr>
    basic_value(T f, std::vector<std::string> comments)
        : type_(value_t::floating),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->floating_, f);
    }

    // string ===============================================================

    basic_value(toml::string s)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, std::move(s));
    }
    template<typename Container>
    basic_value(toml::string s, detail::region<Container> reg)
        : type_(value_t::string),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->string_, std::move(s));
    }
    basic_value& operator=(toml::string s)
    {
        this->cleanup();
        this->type_ = value_t::string ;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, s);
        return *this;
    }
    basic_value(toml::string s, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, std::move(s));
    }

    basic_value(std::string s)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::move(s)));
    }
    basic_value& operator=(std::string s)
    {
        this->cleanup();
        this->type_ = value_t::string ;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(std::move(s)));
        return *this;
    }
    basic_value(std::string s, string_t kind)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::move(s), kind));
    }
    basic_value(std::string s, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(std::move(s)));
    }
    basic_value(std::string s, string_t kind, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(std::move(s), kind));
    }

    basic_value(const char* s)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::string(s)));
    }
    basic_value& operator=(const char* s)
    {
        this->cleanup();
        this->type_ = value_t::string ;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(std::string(s)));
        return *this;
    }
    basic_value(const char* s, string_t kind)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(std::string(s), kind));
    }
    basic_value(const char* s, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(std::string(s)));
    }
    basic_value(const char* s, string_t kind, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(std::string(s), kind));
    }

#if __cplusplus >= 201703L
    basic_value(std::string_view s)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(s));
    }
    basic_value& operator=(std::string_view s)
    {
        this->cleanup();
        this->type_ = value_t::string ;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->string_, toml::string(s));
        return *this;
    }
    basic_value(std::string_view s, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(s));
    }
    basic_value(std::string_view s, string_t kind)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->string_, toml::string(s, kind));
    }
    basic_value(std::string_view s, string_t kind, std::vector<std::string> comments)
        : type_(value_t::string),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->string_, toml::string(s, kind));
    }
#endif

    // local date ===========================================================

    basic_value(const local_date& ld)
        : type_(value_t::local_date),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_date_, ld);
    }
    template<typename Container>
    basic_value(const local_date& ld, detail::region<Container> reg)
        : type_(value_t::local_date),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->local_date_, ld);
    }
    basic_value& operator=(const local_date& ld)
    {
        this->cleanup();
        this->type_ = value_t::local_date;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_date_, ld);
        return *this;
    }
    basic_value(const local_date& ld, std::vector<std::string> comments)
        : type_(value_t::local_date),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->local_date_, ld);
    }

    // local time ===========================================================

    basic_value(const local_time& lt)
        : type_(value_t::local_time),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_time_, lt);
    }
    template<typename Container>
    basic_value(const local_time& lt, detail::region<Container> reg)
        : type_(value_t::local_time),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->local_time_, lt);
    }
    basic_value(const local_time& lt, std::vector<std::string> comments)
        : type_(value_t::local_time),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->local_time_, lt);
    }
    basic_value& operator=(const local_time& lt)
    {
        this->cleanup();
        this->type_ = value_t::local_time;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_time_, lt);
        return *this;
    }

    template<typename Rep, typename Period>
    basic_value(const std::chrono::duration<Rep, Period>& dur)
        : type_(value_t::local_time),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_time_, local_time(dur));
    }
    template<typename Rep, typename Period>
    basic_value(const std::chrono::duration<Rep, Period>& dur,
                std::vector<std::string> comments)
        : type_(value_t::local_time),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->local_time_, local_time(dur));
    }
    template<typename Rep, typename Period>
    basic_value& operator=(const std::chrono::duration<Rep, Period>& dur)
    {
        this->cleanup();
        this->type_ = value_t::local_time;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_time_, local_time(dur));
        return *this;
    }

    // local datetime =======================================================

    basic_value(const local_datetime& ldt)
        : type_(value_t::local_datetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->local_datetime_, ldt);
    }
    template<typename Container>
    basic_value(const local_datetime& ldt, detail::region<Container> reg)
        : type_(value_t::local_datetime),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->local_datetime_, ldt);
    }
    basic_value(const local_datetime& ldt, std::vector<std::string> comments)
        : type_(value_t::local_datetime),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->local_datetime_, ldt);
    }
    basic_value& operator=(const local_datetime& ldt)
    {
        this->cleanup();
        this->type_ = value_t::local_datetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->local_datetime_, ldt);
        return *this;
    }

    // offset datetime ======================================================

    basic_value(const offset_datetime& odt)
        : type_(value_t::offset_datetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->offset_datetime_, odt);
    }
    template<typename Container>
    basic_value(const offset_datetime& odt, detail::region<Container> reg)
        : type_(value_t::offset_datetime),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->offset_datetime_, odt);
    }
    basic_value(const offset_datetime& odt, std::vector<std::string> comments)
        : type_(value_t::offset_datetime),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->offset_datetime_, odt);
    }
    basic_value& operator=(const offset_datetime& odt)
    {
        this->cleanup();
        this->type_ = value_t::offset_datetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->offset_datetime_, odt);
        return *this;
    }
    basic_value(const std::chrono::system_clock::time_point& tp)
        : type_(value_t::offset_datetime),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->offset_datetime_, offset_datetime(tp));
    }
    basic_value(const std::chrono::system_clock::time_point& tp,
                std::vector<std::string> comments)
        : type_(value_t::offset_datetime),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->offset_datetime_, offset_datetime(tp));
    }
    basic_value& operator=(const std::chrono::system_clock::time_point& tp)
    {
        this->cleanup();
        this->type_ = value_t::offset_datetime;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->offset_datetime_, offset_datetime(tp));
        return *this;
    }

    // array ================================================================

    basic_value(const array_type& ary)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->array_, ary);
    }
    template<typename Container>
    basic_value(const array_type& ary, detail::region<Container> reg)
        : type_(value_t::array),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->array_, ary);
    }
    basic_value(const array_type& ary, std::vector<std::string> comments)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->array_, ary);
    }
    basic_value& operator=(const array_type& ary)
    {
        this->cleanup();
        this->type_ = value_t::array ;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->array_, ary);
        return *this;
    }

    // array (initializer_list) ----------------------------------------------

    template<typename T, typename std::enable_if<
            std::is_convertible<T, value_type>::value,
        std::nullptr_t>::type = nullptr>
    basic_value(std::initializer_list<T> list)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        array_type ary(list.begin(), list.end());
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<
            std::is_convertible<T, value_type>::value,
        std::nullptr_t>::type = nullptr>
    basic_value(std::initializer_list<T> list, std::vector<std::string> comments)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        array_type ary(list.begin(), list.end());
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<
            std::is_convertible<T, value_type>::value,
        std::nullptr_t>::type = nullptr>
    basic_value& operator=(std::initializer_list<T> list)
    {
        this->cleanup();
        this->type_ = value_t::array;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        array_type ary(list.begin(), list.end());
        assigner(this->array_, std::move(ary));
        return *this;
    }

    // array (STL Containers) ------------------------------------------------

    template<typename T, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<T, array_type>>,
            detail::is_container<T>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value(const T& list)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        static_assert(std::is_convertible<typename T::value_type, value_type>::value,
            "elements of a container should be convertible to toml::value");

        array_type ary(list.size());
        std::copy(list.begin(), list.end(), ary.begin());
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<T, array_type>>,
            detail::is_container<T>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value(const T& list, std::vector<std::string> comments)
        : type_(value_t::array),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        static_assert(std::is_convertible<typename T::value_type, value_type>::value,
            "elements of a container should be convertible to toml::value");

        array_type ary(list.size());
        std::copy(list.begin(), list.end(), ary.begin());
        assigner(this->array_, std::move(ary));
    }
    template<typename T, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<T, array_type>>,
            detail::is_container<T>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value& operator=(const T& list)
    {
        static_assert(std::is_convertible<typename T::value_type, value_type>::value,
            "elements of a container should be convertible to toml::value");

        this->cleanup();
        this->type_ = value_t::array;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        array_type ary(list.size());
        std::copy(list.begin(), list.end(), ary.begin());
        assigner(this->array_, std::move(ary));
        return *this;
    }

    // table ================================================================

    basic_value(const table_type& tab)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        assigner(this->table_, tab);
    }
    template<typename Container>
    basic_value(const table_type& tab, detail::region<Container> reg)
        : type_(value_t::table),
          region_info_(std::make_shared<detail::region<Container>>(std::move(reg))),
          comments_(region_info_->comments())
    {
        assigner(this->table_, tab);
    }
    basic_value(const table_type& tab, std::vector<std::string> comments)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        assigner(this->table_, tab);
    }
    basic_value& operator=(const table_type& tab)
    {
        this->cleanup();
        this->type_ = value_t::table;
        this->region_info_ = std::make_shared<region_base>(region_base{});
        assigner(this->table_, tab);
        return *this;
    }

    // initializer-list ------------------------------------------------------

    basic_value(std::initializer_list<std::pair<key, basic_value>> list)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        table_type tab;
        for(const auto& elem : list) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
    }

    basic_value(std::initializer_list<std::pair<key, basic_value>> list,
                std::vector<std::string> comments)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        table_type tab;
        for(const auto& elem : list) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
    }
    basic_value& operator=(std::initializer_list<std::pair<key, basic_value>> list)
    {
        this->cleanup();
        this->type_ = value_t::table;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        table_type tab;
        for(const auto& elem : list) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
        return *this;
    }

    // other table-like -----------------------------------------------------

    template<typename Map, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<Map, table_type>>,
            detail::is_map<Map>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value(const Map& mp)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{}))
    {
        table_type tab;
        for(const auto& elem : mp) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
    }
    template<typename Map, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<Map, table_type>>,
            detail::is_map<Map>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value(const Map& mp, std::vector<std::string> comments)
        : type_(value_t::table),
          region_info_(std::make_shared<region_base>(region_base{})),
          comments_(std::move(comments))
    {
        table_type tab;
        for(const auto& elem : mp) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
    }
    template<typename Map, typename std::enable_if<detail::conjunction<
            detail::negation<std::is_same<Map, table_type>>,
            detail::is_map<Map>
        >::value, std::nullptr_t>::type = nullptr>
    basic_value& operator=(const Map& mp)
    {
        this->cleanup();
        this->type_ = value_t::table;
        this->region_info_ = std::make_shared<region_base>(region_base{});

        table_type tab;
        for(const auto& elem : mp) {tab[elem.first] = elem.second;}
        assigner(this->table_, std::move(tab));
        return *this;
    }

    // user-defined =========================================================

    // convert using into_toml() method -------------------------------------

    template<typename T, typename std::enable_if<
        detail::has_into_toml_method<T>::value, std::nullptr_t>::type = nullptr>
    basic_value(const T& ud): basic_value(ud.into_toml()) {}

    template<typename T, typename std::enable_if<
        detail::has_into_toml_method<T>::value, std::nullptr_t>::type = nullptr>
    basic_value(const T& ud, std::vector<std::string> comments)
        : basic_value(ud.into_toml(), std::move(comments))
    {}
    template<typename T, typename std::enable_if<
        detail::has_into_toml_method<T>::value, std::nullptr_t>::type = nullptr>
    basic_value& operator=(const T& ud)
    {
        *this = ud.into_toml();
        return *this;
    }

    // convert using into<T> struct -----------------------------------------

    template<typename T, std::size_t S = sizeof(::toml::into<T>)>
    basic_value(const T& ud): basic_value(::toml::into<T>::into_toml(ud)) {}
    template<typename T, std::size_t S = sizeof(::toml::into<T>)>
    basic_value(const T& ud, std::vector<std::string> comments)
        : basic_value(::toml::into<T>::into_toml(ud), std::move(comments))
    {}
    template<typename T, std::size_t S = sizeof(::toml::into<T>)>
    basic_value& operator=(const T& ud)
    {
        *this = ::toml::into<T>::into_toml(ud);
        return *this;
    }

    // for internal use ------------------------------------------------------

    template<typename T, typename Container, typename std::enable_if<
        detail::is_exact_toml_type<T, value_type>::value,
        std::nullptr_t>::type = nullptr>
    basic_value(std::pair<T, detail::region<Container>> parse_result)
        : basic_value(std::move(parse_result.first), std::move(parse_result.second))
    {}

    // type checking and casting ============================================

    template<typename T, typename std::enable_if<
        detail::is_exact_toml_type<T, value_type>::value,
        std::nullptr_t>::type = nullptr>
    bool is() const noexcept
    {
        return detail::type_to_enum<T, value_type>::value == this->type_;
    }
    bool is(value_t t) const noexcept {return t == this->type_;}

    bool is_uninitialized()   const noexcept {return this->is(value_t::empty          );}
    bool is_boolean()         const noexcept {return this->is(value_t::boolean        );}
    bool is_integer()         const noexcept {return this->is(value_t::integer        );}
    bool is_floating()        const noexcept {return this->is(value_t::floating       );}
    bool is_string()          const noexcept {return this->is(value_t::string         );}
    bool is_offset_datetime() const noexcept {return this->is(value_t::offset_datetime);}
    bool is_local_datetime()  const noexcept {return this->is(value_t::local_datetime );}
    bool is_local_date()      const noexcept {return this->is(value_t::local_date     );}
    bool is_local_time()      const noexcept {return this->is(value_t::local_time     );}
    bool is_array()           const noexcept {return this->is(value_t::array          );}
    bool is_table()           const noexcept {return this->is(value_t::table          );}

    value_t type() const {return type_;}

    template<value_t T>
    typename detail::enum_to_type<T, value_type>::type&       cast() &
    {
        if(this->type_ != T)
        {
            detail::throw_bad_cast<T>(this->type_, *this);
        }
        return detail::switch_cast<T>::invoke(*this);
    }
    template<value_t T>
    typename detail::enum_to_type<T, value_type>::type const& cast() const&
    {
        if(this->type_ != T)
        {
            detail::throw_bad_cast<T>(this->type_, *this);
        }
        return detail::switch_cast<T>::invoke(*this);
    }
    template<value_t T>
    typename detail::enum_to_type<T, value_type>::type&&      cast() &&
    {
        if(this->type_ != T)
        {
            detail::throw_bad_cast<T>(this->type_, *this);
        }
        return detail::switch_cast<T>::invoke(std::move(*this));
    }

    // ------------------------------------------------------------------------
    // nothrow version

    boolean         const& as_boolean        (const std::nothrow_t&) const& noexcept {return this->boolean_;}
    integer         const& as_integer        (const std::nothrow_t&) const& noexcept {return this->integer_;}
    floating        const& as_floating       (const std::nothrow_t&) const& noexcept {return this->floating_;}
    string          const& as_string         (const std::nothrow_t&) const& noexcept {return this->string_;}
    offset_datetime const& as_offset_datetime(const std::nothrow_t&) const& noexcept {return this->offset_datetime_;}
    local_datetime  const& as_local_datetime (const std::nothrow_t&) const& noexcept {return this->local_datetime_;}
    local_date      const& as_local_date     (const std::nothrow_t&) const& noexcept {return this->local_date_;}
    local_time      const& as_local_time     (const std::nothrow_t&) const& noexcept {return this->local_time_;}
    array_type      const& as_array          (const std::nothrow_t&) const& noexcept {return this->array_.value();}
    table_type      const& as_table          (const std::nothrow_t&) const& noexcept {return this->table_.value();}

    boolean        & as_boolean        (const std::nothrow_t&) & noexcept {return this->boolean_;}
    integer        & as_integer        (const std::nothrow_t&) & noexcept {return this->integer_;}
    floating       & as_floating       (const std::nothrow_t&) & noexcept {return this->floating_;}
    string         & as_string         (const std::nothrow_t&) & noexcept {return this->string_;}
    offset_datetime& as_offset_datetime(const std::nothrow_t&) & noexcept {return this->offset_datetime_;}
    local_datetime & as_local_datetime (const std::nothrow_t&) & noexcept {return this->local_datetime_;}
    local_date     & as_local_date     (const std::nothrow_t&) & noexcept {return this->local_date_;}
    local_time     & as_local_time     (const std::nothrow_t&) & noexcept {return this->local_time_;}
    array_type     & as_array          (const std::nothrow_t&) & noexcept {return this->array_.value();}
    table_type     & as_table          (const std::nothrow_t&) & noexcept {return this->table_.value();}

    boolean        && as_boolean        (const std::nothrow_t&) && noexcept {return std::move(this->boolean_);}
    integer        && as_integer        (const std::nothrow_t&) && noexcept {return std::move(this->integer_);}
    floating       && as_floating       (const std::nothrow_t&) && noexcept {return std::move(this->floating_);}
    string         && as_string         (const std::nothrow_t&) && noexcept {return std::move(this->string_);}
    offset_datetime&& as_offset_datetime(const std::nothrow_t&) && noexcept {return std::move(this->offset_datetime_);}
    local_datetime && as_local_datetime (const std::nothrow_t&) && noexcept {return std::move(this->local_datetime_);}
    local_date     && as_local_date     (const std::nothrow_t&) && noexcept {return std::move(this->local_date_);}
    local_time     && as_local_time     (const std::nothrow_t&) && noexcept {return std::move(this->local_time_);}
    array_type     && as_array          (const std::nothrow_t&) && noexcept {return std::move(this->array_.value());}
    table_type     && as_table          (const std::nothrow_t&) && noexcept {return std::move(this->table_.value());}

    // ========================================================================
    // throw version
    // ------------------------------------------------------------------------
    // const reference

    boolean const& as_boolean() const&
    {
        if(this->type_ != value_t::boolean)
        {
            detail::throw_bad_cast<value_t::boolean>(this->type_, *this);
        }
        return this->boolean_;
    }
    integer const& as_integer() const&
    {
        if(this->type_ != value_t::integer)
        {
            detail::throw_bad_cast<value_t::integer>(this->type_, *this);
        }
        return this->integer_;
    }
    floating const& as_floating() const&
    {
        if(this->type_ != value_t::floating)
        {
            detail::throw_bad_cast<value_t::floating>(this->type_, *this);
        }
        return this->floating_;
    }
    string const& as_string() const&
    {
        if(this->type_ != value_t::string)
        {
            detail::throw_bad_cast<value_t::string>(this->type_, *this);
        }
        return this->string_;
    }
    offset_datetime const& as_offset_datetime() const&
    {
        if(this->type_ != value_t::offset_datetime)
        {
            detail::throw_bad_cast<value_t::offset_datetime>(this->type_, *this);
        }
        return this->offset_datetime_;
    }
    local_datetime const& as_local_datetime() const&
    {
        if(this->type_ != value_t::local_datetime)
        {
            detail::throw_bad_cast<value_t::local_datetime>(this->type_, *this);
        }
        return this->local_datetime_;
    }
    local_date const& as_local_date() const&
    {
        if(this->type_ != value_t::local_date)
        {
            detail::throw_bad_cast<value_t::local_date>(this->type_, *this);
        }
        return this->local_date_;
    }
    local_time const& as_local_time() const&
    {
        if(this->type_ != value_t::local_time)
        {
            detail::throw_bad_cast<value_t::local_time>(this->type_, *this);
        }
        return this->local_time_;
    }
    array_type const& as_array() const&
    {
        if(this->type_ != value_t::array)
        {
            detail::throw_bad_cast<value_t::array>(this->type_, *this);
        }
        return this->array_.value();
    }
    table_type const& as_table() const&
    {
        if(this->type_ != value_t::table)
        {
            detail::throw_bad_cast<value_t::table>(this->type_, *this);
        }
        return this->table_.value();
    }

    // ------------------------------------------------------------------------
    // nonconst reference

    boolean & as_boolean() &
    {
        if(this->type_ != value_t::boolean)
        {
            detail::throw_bad_cast<value_t::boolean>(this->type_, *this);
        }
        return this->boolean_;
    }
    integer & as_integer() &
    {
        if(this->type_ != value_t::integer)
        {
            detail::throw_bad_cast<value_t::integer>(this->type_, *this);
        }
        return this->integer_;
    }
    floating & as_floating() &
    {
        if(this->type_ != value_t::floating)
        {
            detail::throw_bad_cast<value_t::floating>(this->type_, *this);
        }
        return this->floating_;
    }
    string & as_string() &
    {
        if(this->type_ != value_t::string)
        {
            detail::throw_bad_cast<value_t::string>(this->type_, *this);
        }
        return this->string_;
    }
    offset_datetime & as_offset_datetime() &
    {
        if(this->type_ != value_t::offset_datetime)
        {
            detail::throw_bad_cast<value_t::offset_datetime>(this->type_, *this);
        }
        return this->offset_datetime_;
    }
    local_datetime & as_local_datetime() &
    {
        if(this->type_ != value_t::local_datetime)
        {
            detail::throw_bad_cast<value_t::local_datetime>(this->type_, *this);
        }
        return this->local_datetime_;
    }
    local_date & as_local_date() &
    {
        if(this->type_ != value_t::local_date)
        {
            detail::throw_bad_cast<value_t::local_date>(this->type_, *this);
        }
        return this->local_date_;
    }
    local_time & as_local_time() &
    {
        if(this->type_ != value_t::local_time)
        {
            detail::throw_bad_cast<value_t::local_time>(this->type_, *this);
        }
        return this->local_time_;
    }
    array_type & as_array() &
    {
        if(this->type_ != value_t::array)
        {
            detail::throw_bad_cast<value_t::array>(this->type_, *this);
        }
        return this->array_.value();
    }
    table_type & as_table() &
    {
        if(this->type_ != value_t::table)
        {
            detail::throw_bad_cast<value_t::table>(this->type_, *this);
        }
        return this->table_.value();
    }

    // ------------------------------------------------------------------------
    // rvalue reference

    boolean && as_boolean() &&
    {
        if(this->type_ != value_t::boolean)
        {
            detail::throw_bad_cast<value_t::boolean>(this->type_, *this);
        }
        return std::move(this->boolean_);
    }
    integer && as_integer() &&
    {
        if(this->type_ != value_t::integer)
        {
            detail::throw_bad_cast<value_t::integer>(this->type_, *this);
        }
        return std::move(this->integer_);
    }
    floating && as_floating() &&
    {
        if(this->type_ != value_t::floating)
        {
            detail::throw_bad_cast<value_t::floating>(this->type_, *this);
        }
        return std::move(this->floating_);
    }
    string && as_string() &&
    {
        if(this->type_ != value_t::string)
        {
            detail::throw_bad_cast<value_t::string>(this->type_, *this);
        }
        return std::move(this->string_);
    }
    offset_datetime && as_offset_datetime() &&
    {
        if(this->type_ != value_t::offset_datetime)
        {
            detail::throw_bad_cast<value_t::offset_datetime>(this->type_, *this);
        }
        return std::move(this->offset_datetime_);
    }
    local_datetime && as_local_datetime() &&
    {
        if(this->type_ != value_t::local_datetime)
        {
            detail::throw_bad_cast<value_t::local_datetime>(this->type_, *this);
        }
        return std::move(this->local_datetime_);
    }
    local_date && as_local_date() &&
    {
        if(this->type_ != value_t::local_date)
        {
            detail::throw_bad_cast<value_t::local_date>(this->type_, *this);
        }
        return std::move(this->local_date_);
    }
    local_time && as_local_time() &&
    {
        if(this->type_ != value_t::local_time)
        {
            detail::throw_bad_cast<value_t::local_time>(this->type_, *this);
        }
        return std::move(this->local_time_);
    }
    array_type && as_array() &&
    {
        if(this->type_ != value_t::array)
        {
            detail::throw_bad_cast<value_t::array>(this->type_, *this);
        }
        return std::move(this->array_.value());
    }
    table_type && as_table() &&
    {
        if(this->type_ != value_t::table)
        {
            detail::throw_bad_cast<value_t::table>(this->type_, *this);
        }
        return std::move(this->table_.value());
    }

    source_location location() const
    {
        return source_location(this->region_info_.get());
    }

    comment_type const& comments() const noexcept {return this->comments_;}
    comment_type&       comments()       noexcept {return this->comments_;}

  private:

    void cleanup() noexcept
    {
        switch(this->type_)
        {
            case value_t::string  : {string_.~string();       return;}
            case value_t::array   : {array_.~array_storage(); return;}
            case value_t::table   : {table_.~table_storage(); return;}
            default              : return;
        }
    }

    // for error messages
    template<typename C,
             template<typename ...> class T, template<typename ...> class A>
    friend region_base const& detail::get_region(const basic_value<C, T, A>&);

    template<typename Region, typename C,
             template<typename ...> class T, template<typename ...> class A>
    friend void detail::change_region(basic_value<C, T, A>&, Region&&);

  private:

    using array_storage = detail::storage<array_type>;
    using table_storage = detail::storage<table_type>;

    value_t type_;
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
    std::shared_ptr<region_base> region_info_;
    comment_type                 comments_;
};

// default toml::value and default array/table.
using value = basic_value<discard_comments, std::unordered_map, std::vector>;
using array = typename value::array_type;
using table = typename value::table_type;

namespace detail
{
template<typename C,
         template<typename ...> class T, template<typename ...> class A>
inline region_base const& get_region(const basic_value<C, T, A>& v)
{
    return *(v.region_info_);
}

template<typename Region, typename C,
         template<typename ...> class T, template<typename ...> class A>
void change_region(basic_value<C, T, A>& v, Region&& reg)
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

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline bool
operator==(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    if(lhs.type()     != rhs.type())     {return false;}
    if(lhs.comments() != rhs.comments()) {return false;}

    switch(lhs.type())
    {
        case value_t::boolean  :
        {
            return lhs.as_boolean() == rhs.as_boolean();
        }
        case value_t::integer  :
        {
            return lhs.as_integer() == rhs.as_integer();
        }
        case value_t::floating :
        {
            return lhs.as_floating() == rhs.as_floating();
        }
        case value_t::string   :
        {
            return lhs.as_string() == rhs.as_string();
        }
        case value_t::offset_datetime:
        {
            return lhs.as_offset_datetime() == rhs.as_offset_datetime();
        }
        case value_t::local_datetime:
        {
            return lhs.as_local_datetime() == rhs.as_local_datetime();
        }
        case value_t::local_date:
        {
            return lhs.as_local_date() == rhs.as_local_date();
        }
        case value_t::local_time:
        {
            return lhs.as_local_time() == rhs.as_local_time();
        }
        case value_t::array    :
        {
            return lhs.as_array() == rhs.as_array();
        }
        case value_t::table    :
        {
            return lhs.as_table() == rhs.as_table();
        }
        case value_t::empty    : {return true; }
        default:                 {return false;}
    }
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline bool operator!=(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    return !(lhs == rhs);
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
typename std::enable_if<detail::conjunction<
    detail::is_comparable<typename basic_value<C, T, A>::array_type>,
    detail::is_comparable<typename basic_value<C, T, A>::table_type>
    >::value, bool>::type
operator<(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    if(lhs.type() != rhs.type()){return (lhs.type() < rhs.type());}
    switch(lhs.type())
    {
        case value_t::boolean  :
        {
            return lhs.as_boolean() <  rhs.as_boolean() ||
                  (lhs.as_boolean() == rhs.as_boolean() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::integer  :
        {
            return lhs.as_integer() <  rhs.as_integer() ||
                  (lhs.as_integer() == rhs.as_integer() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::floating :
        {
            return lhs.as_floating() <  rhs.as_floating() ||
                  (lhs.as_floating() == rhs.as_floating() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::string   :
        {
            return lhs.as_string() <  rhs.as_string() ||
                  (lhs.as_string() == rhs.as_string() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::offset_datetime:
        {
            return lhs.as_offset_datetime() <  rhs.as_offset_datetime() ||
                  (lhs.as_offset_datetime() == rhs.as_offset_datetime() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::local_datetime:
        {
            return lhs.as_local_datetime() <  rhs.as_local_datetime() ||
                  (lhs.as_local_datetime() == rhs.as_local_datetime() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::local_date:
        {
            return lhs.as_local_date() <  rhs.as_local_date() ||
                  (lhs.as_local_date() == rhs.as_local_date() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::local_time:
        {
            return lhs.as_local_time() <  rhs.as_local_time() ||
                  (lhs.as_local_time() == rhs.as_local_time() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::array    :
        {
            return lhs.as_array() <  rhs.as_array() ||
                  (lhs.as_array() == rhs.as_array() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::table    :
        {
            return lhs.as_table() <  rhs.as_table() ||
                  (lhs.as_table() == rhs.as_table() &&
                   lhs.comments() < rhs.comments());
        }
        case value_t::empty    :
        {
            return lhs.comments() < rhs.comments();
        }
        default:
        {
            return lhs.comments() < rhs.comments();
        }
    }
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
typename std::enable_if<detail::conjunction<
    detail::is_comparable<typename basic_value<C, T, A>::array_type>,
    detail::is_comparable<typename basic_value<C, T, A>::table_type>
    >::value, bool>::type
operator<=(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}
template<typename C, template<typename ...> class T, template<typename ...> class A>
typename std::enable_if<detail::conjunction<
    detail::is_comparable<typename basic_value<C, T, A>::array_type>,
    detail::is_comparable<typename basic_value<C, T, A>::table_type>
    >::value, bool>::type
operator>(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    return !(lhs <= rhs);
}
template<typename C, template<typename ...> class T, template<typename ...> class A>
typename std::enable_if<detail::conjunction<
    detail::is_comparable<typename basic_value<C, T, A>::array_type>,
    detail::is_comparable<typename basic_value<C, T, A>::table_type>
    >::value, bool>::type
operator>=(const basic_value<C, T, A>& lhs, const basic_value<C, T, A>& rhs)
{
    return !(lhs < rhs);
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const basic_value<C, T, A>& v, const std::string& comment,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v)), comment}
        }, std::move(hints));
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v1)), comment1},
            {std::addressof(detail::get_region(v2)), comment2}
        }, std::move(hints));
}

template<typename C, template<typename ...> class T, template<typename ...> class A>
inline std::string format_error(const std::string& err_msg,
        const toml::basic_value<C, T, A>& v1, const std::string& comment1,
        const toml::basic_value<C, T, A>& v2, const std::string& comment2,
        const toml::basic_value<C, T, A>& v3, const std::string& comment3,
        std::vector<std::string> hints = {})
{
    return detail::format_underline(err_msg,
        std::vector<std::pair<detail::region_base const*, std::string>>{
            {std::addressof(detail::get_region(v1)), comment1},
            {std::addressof(detail::get_region(v2)), comment2},
            {std::addressof(detail::get_region(v3)), comment3}
        }, std::move(hints));
}

template<typename Visitor, typename C,
         template<typename ...> class T, template<typename ...> class A>
detail::return_type_of_t<Visitor, const toml::boolean&>
visit(Visitor&& visitor, const toml::basic_value<C, T, A>& v)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visitor(v.as_boolean        ());}
        case value_t::integer        : {return visitor(v.as_integer        ());}
        case value_t::floating       : {return visitor(v.as_floating       ());}
        case value_t::string         : {return visitor(v.as_string         ());}
        case value_t::offset_datetime: {return visitor(v.as_offset_datetime());}
        case value_t::local_datetime : {return visitor(v.as_local_datetime ());}
        case value_t::local_date     : {return visitor(v.as_local_date     ());}
        case value_t::local_time     : {return visitor(v.as_local_time     ());}
        case value_t::array          : {return visitor(v.as_array          ());}
        case value_t::table          : {return visitor(v.as_table          ());}
        case value_t::empty          : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid basic_value.", v, "here"));
}

template<typename Visitor, typename C,
         template<typename ...> class T, template<typename ...> class A>
detail::return_type_of_t<Visitor, toml::boolean&>
visit(Visitor&& visitor, toml::basic_value<C, T, A>& v)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visitor(v.as_boolean        ());}
        case value_t::integer        : {return visitor(v.as_integer        ());}
        case value_t::floating       : {return visitor(v.as_floating       ());}
        case value_t::string         : {return visitor(v.as_string         ());}
        case value_t::offset_datetime: {return visitor(v.as_offset_datetime());}
        case value_t::local_datetime : {return visitor(v.as_local_datetime ());}
        case value_t::local_date     : {return visitor(v.as_local_date     ());}
        case value_t::local_time     : {return visitor(v.as_local_time     ());}
        case value_t::array          : {return visitor(v.as_array          ());}
        case value_t::table          : {return visitor(v.as_table          ());}
        case value_t::empty          : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid basic_value.", v, "here"));
}

template<typename Visitor, typename C,
         template<typename ...> class T, template<typename ...> class A>
detail::return_type_of_t<Visitor, toml::boolean&&>
visit(Visitor&& visitor, toml::basic_value<C, T, A>&& v)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visitor(std::move(v.as_boolean        ()));}
        case value_t::integer        : {return visitor(std::move(v.as_integer        ()));}
        case value_t::floating       : {return visitor(std::move(v.as_floating       ()));}
        case value_t::string         : {return visitor(std::move(v.as_string         ()));}
        case value_t::offset_datetime: {return visitor(std::move(v.as_offset_datetime()));}
        case value_t::local_datetime : {return visitor(std::move(v.as_local_datetime ()));}
        case value_t::local_date     : {return visitor(std::move(v.as_local_date     ()));}
        case value_t::local_time     : {return visitor(std::move(v.as_local_time     ()));}
        case value_t::array          : {return visitor(std::move(v.as_array          ()));}
        case value_t::table          : {return visitor(std::move(v.as_table          ()));}
        case value_t::empty          : break;
        default: break;
    }
    throw std::runtime_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid basic_value.", v, "here"));
}

}// toml
#endif// TOML11_VALUE

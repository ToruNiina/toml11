//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_COMBINATOR_HPP
#define TOML11_COMBINATOR_HPP
#include "traits.hpp"
#include "result.hpp"
#include "utility.hpp"
#include "region.hpp"
#include <type_traits>
#include <iterator>
#include <limits>
#include <array>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <cctype>

// they scans characters and returns region if it matches to the condition.
// when they fail, it does not change the location.
// in lexer.hpp, these are used.

namespace toml
{
namespace detail
{

// to output character as an error message.
inline std::string show_char(const char c)
{
    // It supress an error that occurs only in Debug mode of MSVC++ on Windows.
    // I'm not completely sure but they check the value of char to be in the
    // range [0, 256) and some of the COMPLETELY VALID utf-8 character sometimes
    // has negative value (if char has sign). So here it re-interprets c as
    // unsigned char through pointer. In general, converting pointer to a
    // pointer that has different type cause UB, but `(signed|unsigned)?char`
    // are one of the exceptions. Converting pointer only to char and std::byte
    // (c++17) are valid.
    if(std::isgraph(*reinterpret_cast<unsigned char const*>(std::addressof(c))))
    {
        return std::string(1, c);
    }
    else
    {
        std::array<char, 5> buf;
        buf.fill('\0');
        const auto r = std::snprintf(
                buf.data(), buf.size(), "0x%02x", static_cast<int>(c) & 0xFF);
        (void) r; // Unused variable warning
        assert(r == static_cast<int>(buf.size()) - 1);
        return std::string(buf.data());
    }
}

template<char C>
struct character
{
    static constexpr char target = C;

    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        if(loc.iter() == loc.end()) {return none();}
        const auto first = loc.iter();

        const char c = *(loc.iter());
        if(c != target)
        {
            return none();
        }
        loc.advance(); // update location

        return ok(region<Cont>(loc, first, loc.iter()));
    }
};
template<char C>
constexpr char character<C>::target;

// closed interval [Low, Up]. both Low and Up are included.
template<char Low, char Up>
struct in_range
{
    // assuming ascii part of UTF-8...
    static_assert(Low <= Up, "lower bound should be less than upper bound.");

    static constexpr char upper = Up;
    static constexpr char lower = Low;

    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        if(loc.iter() == loc.end()) {return none();}
        const auto first = loc.iter();

        const char c = *(loc.iter());
        if(c < lower || upper < c)
        {
            return none();
        }

        loc.advance();
        return ok(region<Cont>(loc, first, loc.iter()));
    }
};
template<char L, char U> constexpr char in_range<L, U>::upper;
template<char L, char U> constexpr char in_range<L, U>::lower;

// keep iterator if `Combinator` matches. otherwise, increment `iter` by 1 char.
// for detecting invalid characters, like control sequences in toml string.
template<typename Combinator>
struct exclude
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        if(loc.iter() == loc.end()) {return none();}
        auto first = loc.iter();

        auto rslt = Combinator::invoke(loc);
        if(rslt.is_ok())
        {
            loc.reset(first);
            return none();
        }
        loc.reset(std::next(first)); // XXX maybe loc.advance() is okay but...
        return ok(region<Cont>(loc, first, loc.iter()));
    }
};

// increment `iter`, if matches. otherwise, just return empty string.
template<typename Combinator>
struct maybe
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        const auto rslt = Combinator::invoke(loc);
        if(rslt.is_ok())
        {
            return rslt;
        }
        return ok(region<Cont>(loc));
    }
};

template<typename ... Ts>
struct sequence;

template<typename Head, typename ... Tail>
struct sequence<Head, Tail...>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        const auto first = loc.iter();
        const auto rslt = Head::invoke(loc);
        if(rslt.is_err())
        {
            loc.reset(first);
            return none();
        }
        return sequence<Tail...>::invoke(loc, std::move(rslt.unwrap()), first);
    }

    // called from the above function only, recursively.
    template<typename Cont, typename Iterator>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc, region<Cont> reg, Iterator first)
    {
        const auto rslt = Head::invoke(loc);
        if(rslt.is_err())
        {
            loc.reset(first);
            return none();
        }
        reg += rslt.unwrap(); // concat regions
        return sequence<Tail...>::invoke(loc, std::move(reg), first);
    }
};

template<typename Head>
struct sequence<Head>
{
    // would be called from sequence<T ...>::invoke only.
    template<typename Cont, typename Iterator>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc, region<Cont> reg, Iterator first)
    {
        const auto rslt = Head::invoke(loc);
        if(rslt.is_err())
        {
            loc.reset(first);
            return none();
        }
        reg += rslt.unwrap(); // concat regions
        return ok(reg);
    }
};

template<typename ... Ts>
struct either;

template<typename Head, typename ... Tail>
struct either<Head, Tail...>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");

        const auto rslt = Head::invoke(loc);
        if(rslt.is_ok()) {return rslt;}
        return either<Tail...>::invoke(loc);
    }
};
template<typename Head>
struct either<Head>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        static_assert(std::is_same<char, typename Cont::value_type>::value,
                      "internal error: container::value_type should be `char`.");
        return Head::invoke(loc);
    }
};

template<typename T, typename N>
struct repeat;

template<std::size_t N> struct exactly{};
template<std::size_t N> struct at_least{};
struct unlimited{};

template<typename T, std::size_t N>
struct repeat<T, exactly<N>>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        region<Cont> retval(loc);
        const auto first = loc.iter();
        for(std::size_t i=0; i<N; ++i)
        {
            auto rslt = T::invoke(loc);
            if(rslt.is_err())
            {
                loc.reset(first);
                return none();
            }
            retval += rslt.unwrap();
        }
        return ok(std::move(retval));
    }
};

template<typename T, std::size_t N>
struct repeat<T, at_least<N>>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        region<Cont> retval(loc);

        const auto first = loc.iter();
        for(std::size_t i=0; i<N; ++i)
        {
            auto rslt = T::invoke(loc);
            if(rslt.is_err())
            {
                loc.reset(first);
                return none();
            }
            retval += rslt.unwrap();
        }
        while(true)
        {
            auto rslt = T::invoke(loc);
            if(rslt.is_err())
            {
                return ok(std::move(retval));
            }
            retval += rslt.unwrap();
        }
    }
};

template<typename T>
struct repeat<T, unlimited>
{
    template<typename Cont>
    static result<region<Cont>, none_t>
    invoke(location<Cont>& loc)
    {
        region<Cont> retval(loc);
        while(true)
        {
            auto rslt = T::invoke(loc);
            if(rslt.is_err())
            {
                return ok(std::move(retval));
            }
            retval += rslt.unwrap();
        }
    }
};

} // detail
} // toml
#endif// TOML11_COMBINATOR_HPP

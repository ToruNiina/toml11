//     Copyright Toru Niina 2019.
// Distributed under the MIT License.
#ifndef TOML11_FIND_HPP
#define TOML11_FIND_HPP
#include "get.hpp"
#include <numeric>

namespace toml
{

// ----------------------------------------------------------------------------
// these overloads do not require to set T. and returns value itself.
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V> const& find(const basic_value<C, M, V>& v, const key& ky)
{
    const auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>& find(basic_value<C, M, V>& v, const key& ky)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>&& find(basic_value<C, M, V>&& v, const key& ky)
{
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return std::move(tab.at(ky));
}

// ----------------------------------------------------------------------------
// find<T>(value, key);

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V> const&>()))
find(const basic_value<C, M, V>& v, const key& ky)
{
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&>()))
find(basic_value<C, M, V>& v, const key& ky)
{
    auto& tab = v.as_table();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&&>()))
find(basic_value<C, M, V>&& v, const key& ky)
{
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0)
    {
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(std::move(tab.at(ky)));
}

// --------------------------------------------------------------------------
// toml::find(toml::value, toml::key, Ts&& ... keys)

template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, const basic_value<C, M, V>&>
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, basic_value<C, M, V>&>
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, basic_value<C, M, V>&&>
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<const basic_value<C, M, V>&>()))>
find(const basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<basic_value<C, M, V>&>()))>
find(basic_value<C, M, V>& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(v, ky), std::forward<Ts>(keys)...);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename ... Ts>
detail::enable_if_t<detail::conjunction<std::is_convertible<Ts, std::string>...
    >::value, decltype(get<T>(std::declval<basic_value<C, M, V>&&>()))>
find(basic_value<C, M, V>&& v, const ::toml::key& ky, Ts&& ... keys)
{
    return ::toml::find<T>(::toml::find(std::move(v), ky), std::forward<Ts>(keys)...);
}

// ===========================================================================
// find_or(value, key, fallback)

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V> const&
find_or(const basic_value<C, M, V>& v, const key& ky,
        const basic_value<C, M, V>& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return tab.at(ky);
}

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, basic_value<C, M, V>& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return tab[ky];
}

template<typename C,
         template<typename ...> class M, template<typename ...> class V>
basic_value<C, M, V>
find_or(basic_value<C, M, V>&& v, const toml::key& ky, basic_value<C, M, V>&& opt)
{
    if(!v.is_table()) {return std::move(opt);}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return std::move(opt);}
    return std::move(tab[ky]);
}

// ---------------------------------------------------------------------------
// exact types (return type can be a reference)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T> const&
find_or(const basic_value<C, M, V>& v, const key& ky, const T& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, T& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab[ky], opt);
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_exact_toml_type<T, basic_value<C, M, V>>::value, T>&&
find_or(basic_value<C, M, V>&& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::forward<T>(opt);}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get_or(std::move(tab[ky]), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// std::string (return type can be a reference)

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string> const&
find_or(const basic_value<C, M, V>& v, const key& ky, const T& opt)
{
    if(!v.is_table()) {return opt;}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string>&
find_or(basic_value<C, M, V>& v, const toml::key& ky, T& opt)
{
    if(!v.is_table()) {return opt;}
    auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return opt;}
    return get_or(tab.at(ky), opt);
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<std::is_same<T, std::string>::value, std::string>
find_or(basic_value<C, M, V>&& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::forward<T>(opt);}
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get_or(std::move(tab.at(ky)), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// string literal (deduced as std::string)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<
    detail::is_string_literal<typename std::remove_reference<T>::type>::value,
    std::string>
find_or(const basic_value<C, M, V>& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::string(opt);}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return std::string(opt);}
    return get_or(tab.at(ky), std::forward<T>(opt));
}

// ---------------------------------------------------------------------------
// others (require type conversion and return type cannot be lvalue reference)
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
detail::enable_if_t<detail::conjunction<
    // T is not an exact toml type
    detail::negation<detail::is_exact_toml_type<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        basic_value<C, M, V>>>,
    // T is not std::string
    detail::negation<std::is_same<std::string,
        typename std::remove_cv<typename std::remove_reference<T>::type>::type>>,
    // T is not a string literal
    detail::negation<detail::is_string_literal<
        typename std::remove_reference<T>::type>>
    >::value, typename std::remove_cv<typename std::remove_reference<T>::type>::type>
find_or(const basic_value<C, M, V>& v, const toml::key& ky, T&& opt)
{
    if(!v.is_table()) {return std::forward<T>(opt);}
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0) {return std::forward<T>(opt);}
    return get_or(tab.at(ky), std::forward<T>(opt));
}

// ============================================================================
// expect

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
result<T, std::string> expect(const basic_value<C, M, V>& v) noexcept
{
    try
    {
        return ok(get<T>(v));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V>
result<T, std::string>
expect(const basic_value<C, M, V>& v, const toml::key& k) noexcept
{
    try
    {
        return ok(find<T>(v, k));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}
template<typename T, typename Table>
detail::enable_if_t<detail::conjunction<
    detail::is_map<Table>, detail::is_basic_value<typename Table::mapped_type>
    >::value, result<T, std::string>>
expect(const Table& t, const toml::key& k,
       std::string tablename = "unknown table") noexcept
{
    try
    {
        return ok(find<T>(t, k, std::move(tablename)));
    }
    catch(const std::exception& e)
    {
        return err(e.what());
    }
}

// ===========================================================================
// find_fuzzy

// ---------------------------------------------------------------------------
// default fuzzy matcher; levenstein distance (all cost is 1)

struct levenstein_matcher
{
    levenstein_matcher(): tolerance(1) {}
    levenstein_matcher(const std::uint32_t tol): tolerance(tol) {}
    ~levenstein_matcher() = default;
    levenstein_matcher(levenstein_matcher const&) = default;
    levenstein_matcher(levenstein_matcher &&)     = default;
    levenstein_matcher& operator=(levenstein_matcher const&) = default;
    levenstein_matcher& operator=(levenstein_matcher &&)     = default;

    template<typename charT, typename traitsT, typename Alloc1, typename Alloc2>
    bool operator()(const std::basic_string<charT, traitsT, Alloc1>& lhs,
                    const std::basic_string<charT, traitsT, Alloc2>& rhs) const
    {
        return this->distance(lhs, rhs) <= this->tolerance;
    }

    template<typename charT, typename traitsT, typename Alloc1, typename Alloc2>
    std::uint32_t distance(
            const std::basic_string<charT, traitsT, Alloc1>& lhs,
            const std::basic_string<charT, traitsT, Alloc2>& rhs) const
    {
        // force `lhs.size() <= rhs.size()`
        if(lhs.size() > rhs.size()) {return this->distance(rhs, lhs);}

        std::vector<std::uint32_t> matrix(lhs.size() + 1u);
        std::iota(matrix.begin(), matrix.end(), 0);

        for(const charT r : rhs)
        {
            std::uint32_t prev_diag = matrix.front();
            matrix.front() += 1;

            for(std::size_t i=0; i<lhs.size(); ++i)
            {
                const charT l = lhs[i];
                if(traitsT::eq(l, r))
                {
                    std::swap(matrix[i+1], prev_diag);
                }
                else
                {
                    const auto tmp = matrix[i+1];
                    matrix[i+1] = std::min(prev_diag, std::min(matrix[i], matrix[i+1])) + 1;
                    prev_diag = tmp;
                }
            }
        }
        return matrix.back();
    }

  private:
    std::uint32_t tolerance;
};

// ---------------------------------------------------------------------------
// toml::find_fuzzy<T>(v, "tablename", FuzzyMatcher);

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
auto find_fuzzy(const basic_value<C, M, V>& v, const key& ky,
                const FuzzyMatcher match = levenstein_matcher(1))
    -> decltype(find<T>(std::declval<const basic_value<C, M, V>&>(), ky))
{
    try
    {
        return find<T>(v, ky);
    }
    catch(const std::out_of_range& oor)
    {
        const auto& tab = v.as_table();
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return get<T>(kv.second);
            }
        }
        throw;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
auto find_fuzzy(basic_value<C, M, V>& v, const key& ky,
                const FuzzyMatcher match = levenstein_matcher(1))
    -> decltype(find<T>(std::declval<basic_value<C, M, V>&>(), ky))
{
    try
    {
        return find<T>(v, ky);
    }
    catch(const std::out_of_range& oor)
    {
        auto& tab = v.as_table();
        for(auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return get<T>(kv.second);
            }
        }
        throw;
    }
}
template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
auto find_fuzzy(basic_value<C, M, V>&& v_, const key& ky,
                const FuzzyMatcher match = levenstein_matcher(1))
    -> decltype(find<T>(std::declval<basic_value<C, M, V>&&>(), ky))
{
    basic_value<C, M, V> v = v_; // to re-use later, store it once
    try
    {
        return std::move(find<T>(v, ky)); // pass lref, move later
    }
    catch(const std::out_of_range& oor)
    {
        auto& tab = v.as_table(); // because v is used here
        for(auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return get<T>(std::move(kv.second));
            }
        }
        throw;
    }
}

// ---------------------------------------------------------------------------
// no-template-argument case (by default, return toml::value).
// toml::find_fuzzy(v, "tablename", FuzzyMatcher);

template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
basic_value<C, M, V> const&
find_fuzzy(const basic_value<C, M, V>& v, const key& ky,
           const FuzzyMatcher match = levenstein_matcher(1))
{
    try
    {
        return find(v, ky);
    }
    catch(const std::out_of_range& oor)
    {
        const auto& tab = v.as_table();
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return kv.second;
            }
        }
        throw;
    }
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
basic_value<C, M, V>&
find_fuzzy(basic_value<C, M, V>& v, const key& ky,
           const FuzzyMatcher match = levenstein_matcher(1))
{
    try
    {
        return find(v, ky);
    }
    catch(const std::out_of_range& oor)
    {
        auto& tab = v.as_table();
        for(auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return kv.second;
            }
        }
        throw;
    }
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher = levenstein_matcher>
basic_value<C, M, V>&&
find_fuzzy(basic_value<C, M, V>&& v_, const key& ky,
           const FuzzyMatcher match = levenstein_matcher(1))
{
    basic_value<C, M, V> v = v_; // to re-use later, store it once
    try
    {
        return std::move(find(v, ky));
    }
    catch(const std::out_of_range& oor)
    {
        auto& tab = v.as_table();
        for(auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                return std::move(kv.second);
            }
        }
        throw;
    }
}

// ===========================================================================
// find(v, k, matcher)
//
// when matcher is passed, check a key that matches exists or not. if it exists,
// suggest that in the error message

template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
basic_value<C, M, V> const&
find(const basic_value<C, M, V>& v, const key& ky, FuzzyMatcher match)
{
    const auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
basic_value<C, M, V>&
find(basic_value<C, M, V>& v, const key& ky, FuzzyMatcher match)
{
    auto& tab = v.template cast<value_t::table>();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return tab.at(ky);
}
template<typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
basic_value<C, M, V>&&
find(basic_value<C, M, V>&& v, const key& ky, FuzzyMatcher match)
{
    auto tab = std::move(v).as_table();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return std::move(tab.at(ky));
}

// ----------------------------------------------------------------------------
// find<T>(value, key, fuzzy_matcher);

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
detail::enable_if_t<
    detail::negation<std::is_convertible<FuzzyMatcher, std::string>>::value,
    decltype(::toml::get<T>(std::declval<basic_value<C, M, V> const&>()))>
find(const basic_value<C, M, V>& v, const key& ky, FuzzyMatcher match)
{
    const auto& tab = v.as_table();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this here?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
detail::enable_if_t<
    detail::negation<std::is_convertible<FuzzyMatcher, std::string>>::value,
    decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&>()))>
find(basic_value<C, M, V>& v, const key& ky, FuzzyMatcher match)
{
    auto& tab = v.as_table();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this here?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(tab.at(ky));
}

template<typename T, typename C,
         template<typename ...> class M, template<typename ...> class V,
         typename FuzzyMatcher>
detail::enable_if_t<
    detail::negation<std::is_convertible<FuzzyMatcher, std::string>>::value,
    decltype(::toml::get<T>(std::declval<basic_value<C, M, V>&&>()))>
find(basic_value<C, M, V>&& v, const key& ky, FuzzyMatcher match)
{
    auto tab = v.as_table();
    if(tab.count(ky) == 0)
    {
        for(const auto& kv : tab)
        {
            if(match(kv.first, ky))
            {
                throw std::out_of_range(detail::format_underline(concat_to_string(
                    "[error] key \"", ky, "\" not found."), {
                        {std::addressof(detail::get_region(v)), "in this table"},
                        {std::addressof(detail::get_region(kv.second)),
                         "did you mean this here?"}
                    }));
            }
        }
        throw std::out_of_range(detail::format_underline(concat_to_string(
            "[error] key \"", ky, "\" not found"), {
                {std::addressof(detail::get_region(v)), "in this table"}
            }));
    }
    return ::toml::get<T>(std::move(tab.at(ky)));
}

} // toml
#endif// TOML11_FIND_HPP

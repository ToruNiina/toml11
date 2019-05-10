//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_UTILITY_HPP
#define TOML11_UTILITY_HPP
#include "traits.hpp"
#include <utility>
#include <memory>
#include <sstream>

#if __cplusplus >= 201402L
#  define TOML11_MARK_AS_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__)
#  define TOML11_MARK_AS_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#  define TOML11_MARK_AS_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#  define TOML11_MARK_AS_DEPRECATED
#endif

namespace toml
{

template<typename T, typename ... Ts>
inline std::unique_ptr<T> make_unique(Ts&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(args)...));
}

namespace detail
{

template<typename T>
inline void resize_impl(T& container, std::size_t N, std::true_type)
{
    container.resize(N);
    return ;
}

template<typename T>
inline void resize_impl(T& container, std::size_t N, std::false_type)
{
    if(container.size() >= N) {return;}

    throw std::invalid_argument("not resizable type");
}

} // detail

template<typename T>
inline void resize(T& container, std::size_t N)
{
    if(container.size() == N) {return;}

    return detail::resize_impl(container, N, detail::has_resize_method<T>());
}

namespace detail
{
inline std::string concat_to_string_impl(std::ostringstream& oss)
{
    return oss.str();
}
template<typename T, typename ... Ts>
std::string concat_to_string_impl(std::ostringstream& oss, T&& head, Ts&& ... tail)
{
    oss << std::forward<T>(head);
    return concat_to_string_impl(oss, std::forward<Ts>(tail) ... );
}
} // detail

template<typename ... Ts>
std::string concat_to_string(Ts&& ... args)
{
    std::ostringstream oss;
    oss << std::boolalpha << std::fixed;
    return detail::concat_to_string_impl(oss, std::forward<Ts>(args) ...);
}

template<typename T, typename U>
T from_string(const std::string& str, U&& opt)
{
    T v(static_cast<T>(std::forward<U>(opt)));
    std::istringstream iss(str);
    iss >> v;
    return v;
}

}// toml
#endif // TOML11_UTILITY

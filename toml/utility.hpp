//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_UTILITY
#define TOML11_UTILITY
#include "traits.hpp"
#include <utility>
#include <memory>
#include <sstream>

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
    if(container.size() >= N) return;
    else throw std::invalid_argument("not resizable type");
}

} // detail

template<typename T>
inline void resize(T& container, std::size_t N)
{
    if(container.size() == N) return;
    else return detail::resize_impl(container, N, detail::has_resize_method<T>());
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
    T v(std::forward<U>(opt));
    std::istringstream iss(str);
    iss >> v;
    return v;
}



}// toml
#endif // TOML11_UTILITY

#ifndef TOML11_UTILITY
#define TOML11_UTILITY
#include "traits.hpp"
#include <utility>
#include <memory>

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

}

template<typename T>
inline void resize(T& container, std::size_t N)
{
    if(container.size() == N) return;
    else return detail::resize_impl(container, N, detail::has_resize_method<T>());
}

}// toml
#endif // TOML11_UTILITY

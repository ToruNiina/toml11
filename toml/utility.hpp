#ifndef TOML11_UTILITY
#define TOML11_UTILITY
#include <utility>
#include <memory>

namespace toml
{

template<typename T, typename ... Ts>
inline std::unique_ptr<T> make_unique(Ts&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(args)...));
}

}// toml
#endif // TOML11_UTILITY

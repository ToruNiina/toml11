#ifndef TOML11_TRAITS
#define TOML11_TRAITS
#include <type_traits>

namespace toml
{
namespace detail
{

template<typename T>
using unwrap_t = typename std::decay<T>::type;

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
struct has_resize_method_impl
{
    constexpr static std::size_t dummy=0;
    template<typename T> static std::true_type  check(decltype(std::declval<T>().resize(dummy))*);
    template<typename T> static std::false_type check(...);
};

/// Intel C++ compiler can not use decltype in parent class declaration, here
/// is a hack to work around it. https://stackoverflow.com/a/23953090/4692076
#ifdef __INTEL_COMPILER
#define decltype(...) std::enable_if<true, decltype(__VA_ARGS__)>::type
#endif

template<typename T>
struct has_iterator    : decltype(has_iterator_impl::check<T>(nullptr)){};
template<typename T>
struct has_value_type  : decltype(has_value_type_impl::check<T>(nullptr)){};
template<typename T>
struct has_key_type    : decltype(has_key_type_impl::check<T>(nullptr)){};
template<typename T>
struct has_mapped_type : decltype(has_mapped_type_impl::check<T>(nullptr)){};
template<typename T>
struct has_resize_method : decltype(has_resize_method_impl::check<T>(nullptr)){};

#ifdef __INTEL_COMPILER
#undef decltype(...)
#endif

template<typename T>
struct is_container : std::integral_constant<bool,
    has_iterator<T>::value && has_value_type<T>::value>{};

template<typename T>
struct is_map : std::integral_constant<bool,
    has_iterator<T>::value && has_key_type<T>::value &&
    has_mapped_type<T>::value>{};

}// detail
}//toml
#endif // TOML_TRAITS

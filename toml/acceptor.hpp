#ifndef TOML11_ACCEPTOR
#define TOML11_ACCEPTOR
#include <type_traits>
#include <iterator>

namespace toml
{

template<typename charT, charT c>
struct is_charactor
{
    typedef charT value_type;
    constexpr static value_type target = c;
 
    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    constexpr static Iterator invoke(Iterator iter)
    {
        return *iter == c ? std::next(iter) : iter;
    }
};

template<typename charT, charT head, charT ... tail>
struct is_none_of
{
    typedef charT value_type;
 
    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    constexpr static Iterator invoke(Iterator iter)
    {
        return *iter == head ? iter : is_not_a<tail...>::invoke(iter);
    }
};

template<typename charT, charT tail>
struct is_none_of<charT, tail>
{
    typedef charT value_type;
 
    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    constexpr static Iterator invoke(Iterator iter)
    {
        return *iter == tail ? iter : std::next(iter);
    }
};

template<typename charT, charT lw, charT up>
struct is_in_range
{
    typedef charT value_type;
    constexpr static value_type upper = up;
    constexpr static value_type lower = lw;
    static_assert(lower <= upper, "lower <= upper");
 
    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    constexpr static Iterator invoke(Iterator iter)
    {
        return (lower <= *iter && *iter <= upper) ? std::next(iter) : iter;
    }
};

template<typename ... condT>
struct is_one_of;
template<typename headT, typename ... condT>
struct is_one_of<headT, condT...>
{
    typedef typename headT::value_type value_type;
    static_assert(
        std::is_same<value_type, typename is_one_of<condT...>::value_type>::value,
        "different value_type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator invoke(Iterator iter)
    {
        const Iterator tmp = headT::invoke(iter);
        return (tmp != iter) ? tmp : is_one_of<condT...>::invoke(iter);
    }
};
template<typename tailT>
struct is_one_of<tailT>
{
    typedef typename tailT::value_type value_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator invoke(Iterator iter)
    {
        const Iterator tmp = tailT::invoke(iter);
        return (tmp != iter) ? tmp : iter;
    }
};

template<typename ...condT>
struct is_chain_of;
template<typename headT, typename ...condT>
struct is_chain_of<headT, condT ... >
{
    typedef typename headT::value_type value_type;
    static_assert(
        std::is_same<value_type, typename is_one_of<condT...>::value_type>::value,
        "different value_type");

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator invoke(Iterator iter)
    {
        const Iterator tmp = headT::invoke(iter);
        return (tmp != iter) ? is_chain_of<condT...>::invoke(tmp) : iter;
    }
};
template<typename tailT>
struct is_chain_of<tailT>
{
    typedef typename tailT::value_type value_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator invoke(Iterator iter)
    {
        const Iterator tmp = tailT::invoke(iter);
        return (tmp != iter) ? tmp : iter;
    }
};

template<typename condT>
struct is_repeat_of
{
    typedef typename condT::value_type value_type;

    template<typename Iterator, class = typename std::enable_if<
        std::is_same<typename std::iterator_traits<Iterator>::value_type,
                     value_type>::value>::type>
    static Iterator invoke(Iterator iter)
    {
        Iterator tmp = condT::invoke(iter);
        while(tmp != iter)
        {
            iter = tmp;
            tmp = condT::invoke(iter);
        }
        return iter;
    }
};

using is_space      = is_charactor<char, ' '>;
using is_tab        = is_charactor<char, '\t'>;
using is_number     = is_in_range<char, '0', '9'>;
using is_lowercase  = is_in_range<char, 'a', 'z'>;
using is_uppercase  = is_in_range<char, 'A', 'Z'>;
using is_alphabet   = is_one_of<is_lowercase, is_uppercase>;
using is_whitespace = is_one_of<is_space, is_tab>;
using is_newline    = is_one_of<is_charactor<char, '\n'>,
    is_chain_of<is_charactor<char, '\r'>, is_charactor<char, '\n'>>>;
using is_barekey_component = is_one_of<is_alphabet, is_number,
    is_charactor<char, '_'>, is_charactor<char, '-'>>;
using is_barekey = is_repeat_of<is_barekey_component>;


}//toml
#endif// TOML11_ACCEPTOR

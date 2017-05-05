#define BOOST_TEST_MODULE "test_acceptor"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/acceptor.hpp>
#include <iostream>

template<typename T>
struct wrapping_iterator
{
    typedef T value_type;
    typedef std::size_t difference_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef std::forward_iterator_tag iterator_category;

    wrapping_iterator(value_type v) : value_(v){}

    value_type operator*(){return value_;}

    wrapping_iterator operator++()   {advanced = true;return *this;}
    wrapping_iterator operator++(int){advanced = true;return *this;}

    operator bool() const {return advanced;}

  private:

    bool advanced = false;
    value_type value_;
};

template<typename T>
inline wrapping_iterator<T> wrap(T v) {return wrapping_iterator<T>(v);}

BOOST_AUTO_TEST_CASE(test_conditions)
{
    BOOST_CHECK( toml::is_space::invoke(wrap(' ')));
    BOOST_CHECK(!toml::is_space::invoke(wrap('a')));

    BOOST_CHECK( toml::is_tab::invoke(wrap('\t')));
    BOOST_CHECK(!toml::is_tab::invoke(wrap('a')));

    for(char c = '0'; c <= '9'; ++c)
        BOOST_CHECK(toml::is_number::invoke(wrap(c)));
    BOOST_CHECK(!toml::is_number::invoke(wrap('a')));

    for(char c = 'a'; c <= 'z'; ++c)
        BOOST_CHECK(toml::is_lowercase::invoke(wrap(c)));
    BOOST_CHECK(!toml::is_lowercase::invoke(wrap('A')));

    for(char c = 'A'; c <= 'Z'; ++c)
        BOOST_CHECK(toml::is_uppercase::invoke(wrap(c)));
    BOOST_CHECK(!toml::is_uppercase::invoke(wrap('a')));

    BOOST_CHECK(toml::is_whitespace::invoke(wrap(' ')));
    BOOST_CHECK(toml::is_whitespace::invoke(wrap('\t')));

    std::string barekey("hoge1-piyo2_fuga3");
    BOOST_CHECK(toml::is_barekey::invoke(barekey.cbegin()) == barekey.cend());
    std::string partial("hoge1.piyo2_fuga3");
    BOOST_CHECK(toml::is_barekey::invoke(partial.cbegin()) == partial.cbegin()+5);
}

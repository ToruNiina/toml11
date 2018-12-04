#include <iostream>
#include <iomanip>
#include <algorithm>
#include <toml/region.hpp>
#include <toml/result.hpp>

#define TOML11_TEST_LEX_ACCEPT(lxr, tkn, expct)                                \
do {                                                                           \
    const std::string token   (tkn);                                           \
    const std::string expected(expct);                                         \
    toml::detail::location<std::string> loc(token);                            \
    const auto result = lxr::invoke(loc);                                      \
    BOOST_CHECK(result.is_ok());                                               \
    if(result.is_ok()){                                                        \
    const auto matched = result.unwrap().str();                                \
    std::cerr << "matched -> ";                                                \
    std::for_each(matched.begin(), matched.end(), [](const char c) {           \
        std::cerr << std::setw(2) << std::hex << std::setfill('0');            \
        std::cerr << "0x" << static_cast<int>(c) << ", ";                      \
        return;                                                                \
    });                                                                        \
    std::cerr << std::endl;                                                    \
    const auto region = result.unwrap();                                       \
    BOOST_CHECK_EQUAL(region.str(), expected);                                 \
    BOOST_CHECK_EQUAL(region.str().size(), expected.size());                   \
    BOOST_CHECK_EQUAL(std::distance(loc.begin(), loc.iter()), region.size());  \
    } else {                                                                   \
    std::cerr << "lexer " << lxr::pattern() << " failed with input `";         \
    std::cerr << token << "`. expected `" << expected << "`\n";                \
    std::cerr << "reason: " << result.unwrap_err() << '\n';                    \
    }                                                                          \
} while(false);                                                                \
/**/

#define TOML11_TEST_LEX_REJECT(lxr, tkn)                                       \
do {                                                                           \
    const std::string token   (tkn);                                           \
    toml::detail::location<std::string> loc(token);                            \
    const auto result = lxr::invoke(loc);                                      \
    BOOST_CHECK(result.is_err());                                              \
    BOOST_CHECK(loc.begin() == loc.iter());                                    \
} while(false); /**/

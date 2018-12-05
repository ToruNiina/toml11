#define BOOST_TEST_MODULE "lex_key_comment_test"
#include <boost/test/unit_test.hpp>
#include <toml/lexer.hpp>
#include "test_lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_bare_key)
{
    TOML11_TEST_LEX_ACCEPT(lex_key, "barekey",  "barekey");
    TOML11_TEST_LEX_ACCEPT(lex_key, "bare-key", "bare-key");
    TOML11_TEST_LEX_ACCEPT(lex_key, "bare_key", "bare_key");
    TOML11_TEST_LEX_ACCEPT(lex_key, "1234",     "1234");
}

BOOST_AUTO_TEST_CASE(test_quoted_key)
{
    TOML11_TEST_LEX_ACCEPT(lex_key, "\"127.0.0.1\"", "\"127.0.0.1\"");
    TOML11_TEST_LEX_ACCEPT(lex_key, "\"character encoding\"", "\"character encoding\"");
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
    TOML11_TEST_LEX_ACCEPT(lex_key, "\"\xCA\x8E\xC7\x9D\xCA\x9E\"",
                                    "\"\xCA\x8E\xC7\x9D\xCA\x9E\"");
#else
    TOML11_TEST_LEX_ACCEPT(lex_key, u8"\"ʎǝʞ\"", u8"\"ʎǝʞ\"");
#endif
    TOML11_TEST_LEX_ACCEPT(lex_key, "'key2'", "'key2'");
    TOML11_TEST_LEX_ACCEPT(lex_key, "'quoted \"value\"'", "'quoted \"value\"'");
}

BOOST_AUTO_TEST_CASE(test_dotted_key)
{
    TOML11_TEST_LEX_ACCEPT(lex_key, "physical.color", "physical.color");
    TOML11_TEST_LEX_ACCEPT(lex_key, "physical.shape", "physical.shape");
    TOML11_TEST_LEX_ACCEPT(lex_key, "x.y.z.w", "x.y.z.w");
    TOML11_TEST_LEX_ACCEPT(lex_key, "site.\"google.com\"", "site.\"google.com\"");
}

BOOST_AUTO_TEST_CASE(test_comment)
{
    TOML11_TEST_LEX_ACCEPT(lex_comment, "#   hoge", "#   hoge");
    TOML11_TEST_LEX_ACCEPT(lex_comment, "#   \n",   "#   ");
    TOML11_TEST_LEX_ACCEPT(lex_comment, "#   \r\n", "#   ");
    TOML11_TEST_LEX_ACCEPT(lex_comment, "# # \n",   "# # ");
}
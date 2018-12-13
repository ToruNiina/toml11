#define BOOST_TEST_MODULE "test_lex_boolean"
#include <boost/test/unit_test.hpp>
#include <toml/lexer.hpp>
#include "test_lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_correct)
{
    TOML11_TEST_LEX_ACCEPT(lex_boolean, "true", "true");
    TOML11_TEST_LEX_ACCEPT(lex_boolean, "false", "false");
    TOML11_TEST_LEX_ACCEPT(lex_boolean, "true  # trailing", "true");
    TOML11_TEST_LEX_ACCEPT(lex_boolean, "false # trailing", "false");
}

BOOST_AUTO_TEST_CASE(test_invalid)
{
    TOML11_TEST_LEX_REJECT(lex_boolean, "TRUE");
    TOML11_TEST_LEX_REJECT(lex_boolean, "FALSE");
    TOML11_TEST_LEX_REJECT(lex_boolean, "True");
    TOML11_TEST_LEX_REJECT(lex_boolean, "False");
}

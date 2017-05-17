#define BOOST_TEST_MODULE "test_parse_unicode"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_CASE(test_hard_example_unicode)
{
    const auto data = toml::parse("toml/tests/hard_example_unicode.toml");

    const auto the = toml::get<toml::Table>(data.at("the"));
    BOOST_CHECK_EQUAL(toml::get<std::string>(the.at("test_string")),
                      u8"Ýôú'ℓℓ λáƭè ₥è áƒƭèř ƭλïƨ - #");

    const auto hard = toml::get<toml::Table>(the.at("hard"));
    const std::vector<std::string> expected_the_hard_test_array{"] ", " # "};
    BOOST_CHECK(toml::get<std::vector<std::string>>(hard.at("test_array")) ==
                expected_the_hard_test_array);
    const std::vector<std::string> expected_the_hard_test_array2{
        u8"Tèƨƭ #11 ]ƥřôƲèδ ƭλáƭ", u8"Éжƥèřï₥èñƭ #9 ωáƨ á ƨúççèƨƨ"};
    BOOST_CHECK(toml::get<std::vector<std::string>>(hard.at("test_array2")) ==
                expected_the_hard_test_array2);
    BOOST_CHECK_EQUAL(toml::get<std::string>(hard.at("another_test_string")),
                      u8"§á₥è ƭλïñϱ, βúƭ ωïƭλ á ƨƭřïñϱ #");
    BOOST_CHECK_EQUAL(toml::get<std::string>(hard.at("harder_test_string")),
                      u8" Âñδ ωλèñ \"'ƨ ářè ïñ ƭλè ƨƭřïñϱ, áℓôñϱ ωïƭλ # \"");

    const auto bit = toml::get<toml::Table>(hard.at(std::string(u8"βïƭ#")));
    BOOST_CHECK_EQUAL(toml::get<std::string>(bit.at(std::string(u8"ωλáƭ?"))),
                      u8"Ýôú δôñ'ƭ ƭλïñƙ ƨô₥è úƨèř ωôñ'ƭ δô ƭλáƭ?");
    const std::vector<std::string> expected_multi_line_array{"]"};
    BOOST_CHECK(toml::get<std::vector<std::string>>(bit.at("multi_line_array")) ==
                expected_multi_line_array);

}

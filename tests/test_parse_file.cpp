#define BOOST_TEST_MODULE "test_parse_file"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml.hpp>
#include <iostream>
#include <fstream>


BOOST_AUTO_TEST_CASE(test_example)
{
    const auto data = toml::parse("toml/tests/example.toml");

    BOOST_CHECK_EQUAL(toml::get<std::string>(data.at("title")), "TOML Example");
    toml::Table owner = toml::get<toml::Table>(data.at("owner"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("name")), "Tom Preston-Werner");
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("organization")), "GitHub");
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("bio")),
                          "GitHub Cofounder & CEO\nLikes tater tots and beer.");
        BOOST_CHECK_EQUAL(toml::get<toml::Datetime>(owner.at("dob")),
                          toml::Datetime(1979, 5, 27, 7, 32, 0, 0, 0, 0, 0));
    }

    toml::Table database = toml::get<toml::Table>(data.at("database"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(database.at("server")), "192.168.1.1");
        const std::vector<int> expected_ports{8001, 8001, 8002};
        BOOST_CHECK(toml::get<std::vector<int>>(database.at("ports")) == expected_ports);
        BOOST_CHECK_EQUAL(toml::get<int>(database.at("connection_max")), 5000);
        BOOST_CHECK_EQUAL(toml::get<bool>(database.at("enabled")), true);
    }

    toml::Table servers = toml::get<toml::Table>(data.at("servers"));
    {
        toml::Table alpha = toml::get<toml::Table>(servers.at("alpha"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("ip")), "10.0.0.1");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("dc")), "eqdc10");

        toml::Table beta = toml::get<toml::Table>(servers.at("beta"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("ip")), "10.0.0.2");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("dc")), "eqdc10");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("country")), "\xE4\xB8\xAD\xE5\x9B\xBD");
    }

    toml::Table clients = toml::get<toml::Table>(data.at("clients"));
    {
        toml::Array clients_data = toml::get<toml::Array>(clients.at("data"));
        std::vector<std::string> expected_name{"gamma", "delta"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients_data.at(0)) ==
                    expected_name);
        std::vector<int> expected_number{1, 2};
        BOOST_CHECK(toml::get<std::vector<int>>(clients_data.at(1)) ==
                    expected_number);
        std::vector<std::string> expected_hosts{"alpha", "omega"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients.at("hosts")) ==
                    expected_hosts);
    }

    std::vector<toml::Table> products =
        toml::get<std::vector<toml::Table>>(data.at("products"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(0).at("name")),
                          "Hammer");
        BOOST_CHECK_EQUAL(toml::get<std::int64_t>(products.at(0).at("sku")),
                          738594937);

        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(1).at("name")),
                          "Nail");
        BOOST_CHECK_EQUAL(toml::get<std::int64_t>(products.at(1).at("sku")),
                          284758393);
        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(1).at("color")),
                          "gray");
    }
}

BOOST_AUTO_TEST_CASE(test_example_stream)
{
    std::ifstream ifs("toml/tests/example.toml");
    const auto data = toml::parse(ifs);

    BOOST_CHECK_EQUAL(toml::get<std::string>(data.at("title")), "TOML Example");
    toml::Table owner = toml::get<toml::Table>(data.at("owner"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("name")), "Tom Preston-Werner");
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("organization")), "GitHub");
        BOOST_CHECK_EQUAL(toml::get<std::string>(owner.at("bio")),
                          "GitHub Cofounder & CEO\nLikes tater tots and beer.");
        BOOST_CHECK_EQUAL(toml::get<toml::Datetime>(owner.at("dob")),
                          toml::Datetime(1979, 5, 27, 7, 32, 0, 0, 0, 0, 0));
    }

    toml::Table database = toml::get<toml::Table>(data.at("database"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(database.at("server")), "192.168.1.1");
        const std::vector<int> expected_ports{8001, 8001, 8002};
        BOOST_CHECK(toml::get<std::vector<int>>(database.at("ports")) == expected_ports);
        BOOST_CHECK_EQUAL(toml::get<int>(database.at("connection_max")), 5000);
        BOOST_CHECK_EQUAL(toml::get<bool>(database.at("enabled")), true);
    }

    toml::Table servers = toml::get<toml::Table>(data.at("servers"));
    {
        toml::Table alpha = toml::get<toml::Table>(servers.at("alpha"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("ip")), "10.0.0.1");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("dc")), "eqdc10");

        toml::Table beta = toml::get<toml::Table>(servers.at("beta"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("ip")), "10.0.0.2");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("dc")), "eqdc10");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("country")), "\xE4\xB8\xAD\xE5\x9B\xBD");
    }

    toml::Table clients = toml::get<toml::Table>(data.at("clients"));
    {
        toml::Array clients_data = toml::get<toml::Array>(clients.at("data"));
        std::vector<std::string> expected_name{"gamma", "delta"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients_data.at(0)) ==
                    expected_name);
        std::vector<int> expected_number{1, 2};
        BOOST_CHECK(toml::get<std::vector<int>>(clients_data.at(1)) ==
                    expected_number);
        std::vector<std::string> expected_hosts{"alpha", "omega"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients.at("hosts")) ==
                    expected_hosts);
    }

    std::vector<toml::Table> products =
        toml::get<std::vector<toml::Table>>(data.at("products"));
    {
        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(0).at("name")),
                          "Hammer");
        BOOST_CHECK_EQUAL(toml::get<std::int64_t>(products.at(0).at("sku")),
                          738594937);

        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(1).at("name")),
                          "Nail");
        BOOST_CHECK_EQUAL(toml::get<std::int64_t>(products.at(1).at("sku")),
                          284758393);
        BOOST_CHECK_EQUAL(toml::get<std::string>(products.at(1).at("color")),
                          "gray");
    }
}


BOOST_AUTO_TEST_CASE(test_fruit)
{
    const auto data = toml::parse("toml/tests/fruit.toml");
    const auto blah = toml::get<std::vector<toml::Table>>(
                      toml::get<toml::Table>(data.at("fruit")).at("blah"));
    BOOST_CHECK_EQUAL(toml::get<std::string>(blah.at(0).at("name")), "apple");
    BOOST_CHECK_EQUAL(toml::get<std::string>(blah.at(1).at("name")), "banana");
    {
        const auto physical = toml::get<toml::Table>(blah.at(0).at("physical"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(physical.at("color")), "red");
        BOOST_CHECK_EQUAL(toml::get<std::string>(physical.at("shape")), "round");
    }
    {
        const auto physical = toml::get<toml::Table>(blah.at(1).at("physical"));
        BOOST_CHECK_EQUAL(toml::get<std::string>(physical.at("color")), "yellow");
        BOOST_CHECK_EQUAL(toml::get<std::string>(physical.at("shape")), "bent");
    }
}

BOOST_AUTO_TEST_CASE(test_hard_example)
{
    const auto data = toml::parse("toml/tests/hard_example.toml");
    const auto the = toml::get<toml::Table>(data.at("the"));
    BOOST_CHECK_EQUAL(toml::get<std::string>(the.at("test_string")),
                      "You'll hate me after this - #");

    const auto hard = toml::get<toml::Table>(the.at("hard"));
    const std::vector<std::string> expected_the_hard_test_array{"] ", " # "};
    BOOST_CHECK(toml::get<std::vector<std::string>>(hard.at("test_array")) ==
                expected_the_hard_test_array);
    const std::vector<std::string> expected_the_hard_test_array2{
        "Test #11 ]proved that", "Experiment #9 was a success"};
    BOOST_CHECK(toml::get<std::vector<std::string>>(hard.at("test_array2")) ==
                expected_the_hard_test_array2);
    BOOST_CHECK_EQUAL(toml::get<std::string>(hard.at("another_test_string")),
                      " Same thing, but with a string #");
    BOOST_CHECK_EQUAL(toml::get<std::string>(hard.at("harder_test_string")),
                      " And when \"'s are in the string, along with # \"");

    const auto bit = toml::get<toml::Table>(hard.at("bit#"));
    BOOST_CHECK_EQUAL(toml::get<std::string>(bit.at("what?")),
                      "You don't think some user won't do that?");
    const std::vector<std::string> expected_multi_line_array{"]"};
    BOOST_CHECK(toml::get<std::vector<std::string>>(bit.at("multi_line_array")) ==
                expected_multi_line_array);
}

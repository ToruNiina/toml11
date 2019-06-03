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

    BOOST_CHECK_EQUAL(toml::find<std::string>(data, "title"), "TOML Example");
    const auto& owner = toml::find(data, "owner");
    {
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "name"), "Tom Preston-Werner");
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "organization"), "GitHub");
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "bio"),
                          "GitHub Cofounder & CEO\nLikes tater tots and beer.");
        BOOST_CHECK_EQUAL(toml::find<toml::offset_datetime>(owner, "dob"),
                          toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                                toml::local_time(7, 32, 0), toml::time_offset(0, 0)));
    }

    const auto& database = toml::find(data, "database");
    {
        BOOST_CHECK_EQUAL(toml::find<std::string>(database, "server"), "192.168.1.1");
        const std::vector<int> expected_ports{8001, 8001, 8002};
        BOOST_CHECK(toml::find<std::vector<int>>(database, "ports") == expected_ports);
        BOOST_CHECK_EQUAL(toml::find<int >(database, "connection_max"), 5000);
        BOOST_CHECK_EQUAL(toml::find<bool>(database, "enabled"), true);
    }

    const auto& servers = toml::find(data, "servers");
    {
        toml::table alpha = toml::find<toml::table>(servers, "alpha");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("ip")), "10.0.0.1");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("dc")), "eqdc10");

        toml::table beta = toml::find<toml::table>(servers, "beta");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("ip")), "10.0.0.2");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("dc")), "eqdc10");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("country")), "\xE4\xB8\xAD\xE5\x9B\xBD");
    }

    const auto& clients = toml::find(data, "clients");
    {
        toml::array clients_data = toml::find<toml::array>(clients, "data");
        std::vector<std::string> expected_name{"gamma", "delta"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients_data.at(0)) ==
                    expected_name);
        std::vector<int> expected_number{1, 2};
        BOOST_CHECK(toml::get<std::vector<int>>(clients_data.at(1)) ==
                    expected_number);
        std::vector<std::string> expected_hosts{"alpha", "omega"};
        BOOST_CHECK(toml::find<std::vector<std::string>>(clients, "hosts") ==
                    expected_hosts);
    }

    std::vector<toml::table> products =
        toml::find<std::vector<toml::table>>(data, "products");
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

    BOOST_CHECK_EQUAL(toml::find<std::string>(data, "title"), "TOML Example");
    const auto& owner = toml::find(data, "owner");
    {
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "name"), "Tom Preston-Werner");
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "organization"), "GitHub");
        BOOST_CHECK_EQUAL(toml::find<std::string>(owner, "bio"),
                          "GitHub Cofounder & CEO\nLikes tater tots and beer.");
        BOOST_CHECK_EQUAL(toml::find<toml::offset_datetime>(owner, "dob"),
                          toml::offset_datetime(toml::local_date(1979, toml::month_t::May, 27),
                                                toml::local_time(7, 32, 0), toml::time_offset(0, 0)));
    }

    const auto& database = toml::find(data, "database");
    {
        BOOST_CHECK_EQUAL(toml::find<std::string>(database, "server"), "192.168.1.1");
        const std::vector<int> expected_ports{8001, 8001, 8002};
        BOOST_CHECK(toml::find<std::vector<int>>(database, "ports") == expected_ports);
        BOOST_CHECK_EQUAL(toml::find<int >(database, "connection_max"), 5000);
        BOOST_CHECK_EQUAL(toml::find<bool>(database, "enabled"), true);
    }

    const auto& servers = toml::find(data, "servers");
    {
        toml::table alpha = toml::find<toml::table>(servers, "alpha");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("ip")), "10.0.0.1");
        BOOST_CHECK_EQUAL(toml::get<std::string>(alpha.at("dc")), "eqdc10");

        toml::table beta = toml::find<toml::table>(servers, "beta");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("ip")), "10.0.0.2");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("dc")), "eqdc10");
        BOOST_CHECK_EQUAL(toml::get<std::string>(beta.at("country")), "\xE4\xB8\xAD\xE5\x9B\xBD");
    }

    const auto& clients = toml::find(data, "clients");
    {
        toml::array clients_data = toml::find<toml::array>(clients, "data");
        std::vector<std::string> expected_name{"gamma", "delta"};
        BOOST_CHECK(toml::get<std::vector<std::string>>(clients_data.at(0)) ==
                    expected_name);
        std::vector<int> expected_number{1, 2};
        BOOST_CHECK(toml::get<std::vector<int>>(clients_data.at(1)) ==
                    expected_number);
        std::vector<std::string> expected_hosts{"alpha", "omega"};
        BOOST_CHECK(toml::find<std::vector<std::string>>(clients, "hosts") ==
                    expected_hosts);
    }

    std::vector<toml::table> products =
        toml::find<std::vector<toml::table>>(data, "products");
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
    const auto blah = toml::find<toml::array>(toml::find(data, "fruit"), "blah");
    BOOST_CHECK_EQUAL(toml::find<std::string>(blah.at(0), "name"), "apple");
    BOOST_CHECK_EQUAL(toml::find<std::string>(blah.at(1), "name"), "banana");
    {
        const auto physical = toml::find(blah.at(0), "physical");
        BOOST_CHECK_EQUAL(toml::find<std::string>(physical, "color"), "red");
        BOOST_CHECK_EQUAL(toml::find<std::string>(physical, "shape"), "round");
    }
    {
        const auto physical = toml::find(blah.at(1), "physical");
        BOOST_CHECK_EQUAL(toml::find<std::string>(physical, "color"), "yellow");
        BOOST_CHECK_EQUAL(toml::find<std::string>(physical, "shape"), "bent");
    }
}

BOOST_AUTO_TEST_CASE(test_hard_example)
{
    const auto data = toml::parse("toml/tests/hard_example.toml");
    const auto the = toml::find(data, "the");
    BOOST_CHECK_EQUAL(toml::find<std::string>(the, "test_string"),
                      "You'll hate me after this - #");

    const auto hard = toml::find(the, "hard");
    const std::vector<std::string> expected_the_hard_test_array{"] ", " # "};
    BOOST_CHECK(toml::find<std::vector<std::string>>(hard, "test_array") ==
                expected_the_hard_test_array);
    const std::vector<std::string> expected_the_hard_test_array2{
        "Test #11 ]proved that", "Experiment #9 was a success"};
    BOOST_CHECK(toml::find<std::vector<std::string>>(hard, "test_array2") ==
                expected_the_hard_test_array2);
    BOOST_CHECK_EQUAL(toml::find<std::string>(hard, "another_test_string"),
                      " Same thing, but with a string #");
    BOOST_CHECK_EQUAL(toml::find<std::string>(hard, "harder_test_string"),
                      " And when \"'s are in the string, along with # \"");

    const auto bit = toml::find(hard, "bit#");
    BOOST_CHECK_EQUAL(toml::find<std::string>(bit, "what?"),
                      "You don't think some user won't do that?");
    const std::vector<std::string> expected_multi_line_array{"]"};
    BOOST_CHECK(toml::find<std::vector<std::string>>(bit, "multi_line_array") ==
                expected_multi_line_array);
}

// ---------------------------------------------------------------------------
// after here, the test codes generate the content of a file.

BOOST_AUTO_TEST_CASE(test_file_with_BOM)
{
    {
        const std::string table(
            "\xEF\xBB\xBF" // BOM
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss, "test_file_with_BOM.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "\xEF\xBB\xBF" // BOM
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            );
        {
            std::ofstream ofs("tmp.toml");
            ofs << table;
        }
        const auto data = toml::parse("tmp.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "\xEF\xBB\xBF" // BOM
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss, "test_file_with_BOM_CRLF.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "\xEF\xBB\xBF" // BOM
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            );
        {
            // with text-mode, "\n" is converted to "\r\n" and the resulting
            // value will be "\r\r\n". To avoid the additional "\r", use binary
            // mode.
            std::ofstream ofs("tmp.toml", std::ios_base::binary);
            ofs.write(table.data(), table.size());
        }
        const auto data = toml::parse("tmp.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
}

BOOST_AUTO_TEST_CASE(test_file_without_newline_at_the_end_of_file)
{
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\""
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\""
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file_CRLF.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\" # comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\" # comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\" \t"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file_ws.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\" \t"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_file_without_newline_at_the_end_of_file_ws.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
}


BOOST_AUTO_TEST_CASE(test_files_end_with_comment)
{
    // comment w/o newline
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "# comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "# comment\n"
            "# one more comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    // comment w/ newline

    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "# comment\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "# comment\n"
            "# one more comment\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    // CRLF version

    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "# comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "# comment\r\n"
            "# one more comment"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "# comment\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "# comment\r\n"
            "# one more comment\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_comment.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
}


BOOST_AUTO_TEST_CASE(test_files_end_with_empty_lines)
{
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "\n"
            "\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    // with whitespaces

    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "  \n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "  \n"
            "  \n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "\n"
            "  \n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "  \n"
            "\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    // with whitespaces but no newline
    {
        const std::string table(
            "key = \"value\"\n"
            "[table]\n"
            "key = \"value\"\n"
            "  "
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }


    // CRLF

    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "\r\n"
            "\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }

    // with whitespaces

    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "  \r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "\r\n"
            "  \r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "  \r\n"
            "\r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "  \r\n"
            "  \r\n"
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
    {
        const std::string table(
            "key = \"value\"\r\n"
            "[table]\r\n"
            "key = \"value\"\r\n"
            "  "
            );
        std::istringstream iss(table);
        const auto data = toml::parse(iss,
                "test_files_end_with_newline.toml");

        BOOST_CHECK_EQUAL(toml::find<std::string>(data, "key"), "value");
        BOOST_CHECK_EQUAL(toml::find<std::string>(toml::find(data, "table"), "key"), "value");
    }
}

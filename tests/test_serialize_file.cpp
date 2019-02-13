#define BOOST_TEST_MODULE "test_serialize_file"
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
    {
        std::ofstream ofs("tmp1.toml");
        ofs << data << std::endl;
    }
    std::cout << "\n==========================\n";
    std::cout << data << std::endl;
    std::cout << "\n--------------------------\n";
    {
        std::ifstream ifs("tmp1.toml", std::ios_base::binary);

        const auto beg = ifs.tellg();
        ifs.seekg(0, std::ios::end);
        const auto end = ifs.tellg();
        const auto fsize = end - beg;
        ifs.seekg(beg);

        // read whole file as a sequence of char
        std::vector<char> letters(fsize);
        ifs.read(letters.data(), fsize);

        for(const char c : letters)
        {
            if(c == '\n'){std::cout << std::endl;}
            else {std::cout << toml::detail::show_char(c);}
        }
    }
    const auto serialized = toml::parse("tmp1.toml");
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_fruit)
{
    const auto data = toml::parse("toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2.toml");
        ofs << data << std::endl;
    }
    std::cout << "\n==========================\n";
    std::cout << data << std::endl;
    std::cout << "\n--------------------------\n";
    {
        std::ifstream ifs("tmp2.toml", std::ios_base::binary);

        const auto beg = ifs.tellg();
        ifs.seekg(0, std::ios::end);
        const auto end = ifs.tellg();
        const auto fsize = end - beg;
        ifs.seekg(beg);

        // read whole file as a sequence of char
        std::vector<char> letters(fsize);
        ifs.read(letters.data(), fsize);

        for(const char c : letters)
        {
            if(c == '\n'){std::cout << std::endl;}
            else {std::cout << toml::detail::show_char(c);}
        }
    }

    const auto serialized = toml::parse("tmp2.toml");
    BOOST_CHECK(data == serialized);
}

BOOST_AUTO_TEST_CASE(test_hard_example)
{
    const auto data = toml::parse("toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3.toml");
        ofs << data << std::endl;
    }
    std::cout << "\n==========================\n";
    std::cout << data << std::endl;
    std::cout << "\n--------------------------\n";
    {
        std::ifstream ifs("tmp3.toml", std::ios_base::binary);

        const auto beg = ifs.tellg();
        ifs.seekg(0, std::ios::end);
        const auto end = ifs.tellg();
        const auto fsize = end - beg;
        ifs.seekg(beg);

        // read whole file as a sequence of char
        std::vector<char> letters(fsize);
        ifs.read(letters.data(), fsize);

        for(const char c : letters)
        {
            if(c == '\n'){std::cout << std::endl;}
            else {std::cout << toml::detail::show_char(c);}
        }
    }
    const auto serialized = toml::parse("tmp3.toml");
    BOOST_CHECK(data == serialized);
}

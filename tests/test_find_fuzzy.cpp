#define BOOST_TEST_MODULE "test_find_fuzzy"

#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif

#include <toml.hpp>

BOOST_AUTO_TEST_CASE(test_levenstein_distance)
{
    const toml::levenstein_matcher lev(1);

    // distance == 0
    {
        const std::string s1("foobar");
        const std::string s2 = s1;

        BOOST_TEST(lev.distance(s1, s2) == 0);
    }

    {
        const std::string s1("foobar");
        const std::string s2("foobaz");

        BOOST_TEST(lev.distance(s1, s2) == 1);
    }
    {
        const std::string s1("foobar"); // insertion (+x)
        const std::string s2("fooxbar");

        BOOST_TEST(lev.distance(s1, s2) == 1);
    }
    {
        const std::string s1("foobar");
        const std::string s2("fooar"); // insertion(+b)

        BOOST_TEST(lev.distance(s1, s2) == 1);
    }

    // distance > 1
    {
        const std::string s1("foobar");
        const std::string s2("fooquux");

        BOOST_TEST(lev.distance(s1, s2) == 4);
    }
    {
        const std::string s1("foobar");
        const std::string s2("fooqu");

        BOOST_TEST(s1 != s2);
        BOOST_TEST(lev.distance(s1, s2) == 3);
    }
}

BOOST_AUTO_TEST_CASE(test_find_fuzzy)
{
    {
        toml::value v{
            {"keu", "value"} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy(v, "key") == toml::value("value"));
        BOOST_CHECK_THROW(toml::find_fuzzy(v, "kiwi"), std::out_of_range);

        static_assert(std::is_same<
                toml::value&, decltype(toml::find_fuzzy(v, "key"))>::value, "");

        toml::find_fuzzy(v, "key") = "foobar";
        BOOST_TEST(toml::find(v, "keu") == toml::value("foobar"));
    }
    {
        const toml::value v{
            {"keu", "value"} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy(v, "key") == toml::value("value"));
        BOOST_CHECK_THROW(toml::find_fuzzy(v, "kiwi"), std::out_of_range);

        static_assert(std::is_same<
                toml::value const&, decltype(toml::find_fuzzy(v, "key"))>::value, "");
    }
    {
        toml::value v{
            {"keu", "value"} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy(std::move(v), "key") == toml::value("value"));

        static_assert(std::is_same<
                toml::value&&, decltype(toml::find_fuzzy(std::move(v), "key"))>::value, "");
    }
    {
        toml::value v{
            {"keu", "value"} // typo! key -> keu
        };

        BOOST_CHECK_THROW(toml::find_fuzzy(std::move(v), "kiwi"), std::out_of_range);

        static_assert(std::is_same<
                toml::value&&, decltype(toml::find_fuzzy(std::move(v), "key"))>::value, "");
    }

    // find with conversion

    {
        toml::value v{
            {"keu", 42} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy<int>(v, "key") == 42);
        BOOST_CHECK_THROW(toml::find_fuzzy<int>(v, "kiwi"), std::out_of_range);

        static_assert(std::is_same<int,
                decltype(toml::find_fuzzy<int>(v, "key"))>::value, "");
    }
    {
        const toml::value v{
            {"keu", 42} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy<int>(v, "key") == 42);
        BOOST_CHECK_THROW(toml::find_fuzzy<int>(v, "kiwi"), std::out_of_range);

        static_assert(std::is_same<int,
                decltype(toml::find_fuzzy<int>(v, "key"))>::value, "");
    }
    {
        toml::value v{
            {"keu", 42} // typo! key -> keu
        };

        BOOST_TEST(toml::find_fuzzy<int>(std::move(v), "key") == 42);

        static_assert(std::is_same<int,
                decltype(toml::find_fuzzy<int>(std::move(v), "key"))>::value, "");
    }
    {
        toml::value v{
            {"keu", 42} // typo! key -> keu
        };

        BOOST_CHECK_THROW(toml::find_fuzzy<int>(std::move(v), "kiwi"), std::out_of_range);
    }
}

BOOST_AUTO_TEST_CASE(test_find_fuzzy_throw)
{
    {
        toml::value v{
            {"keu", "value"}, // typo! key -> keu
            {"ky", "value"}   // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy(v, "key"), std::out_of_range);
    }
    {
        const toml::value v{
            {"keu", "value"}, // typo! key -> keu
            {"ky", "value"}   // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy(v, "key"), std::out_of_range);
    }
    {
        toml::value v{
            {"keu", "value"}, // typo! key -> keu
            {"ky", "value"}   // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy(std::move(v), "key"), std::out_of_range);
    }

    {
        toml::value v{
            {"keu", 42}, // typo! key -> keu
            {"ky",  42}  // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy<int>(v, "key"), std::out_of_range);
    }
    {
        const toml::value v{
            {"keu", 42}, // typo! key -> keu
            {"ky",  42}  // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy<int>(v, "key"), std::out_of_range);
    }
    {
        toml::value v{
            {"keu", 42}, // typo! key -> keu
            {"ky",  42}  // typo! key -> ky
        };

        BOOST_CHECK_THROW(toml::find_fuzzy<int>(std::move(v), "key"), std::out_of_range);
    }

}

BOOST_AUTO_TEST_CASE(test_find_throw_typo_aware_exception)
{
    using namespace toml::literals::toml_literals;
    const toml::levenstein_matcher lev(1);
    {
        toml::value v = u8R"(
            keu = "value"
        )"_toml;

        BOOST_CHECK_THROW(toml::find(v, "key", lev), std::out_of_range);
        try
        {
            const auto& ret = toml::find(v, "key", lev);
            (void)ret; // suppress unused variable
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);

//             std::cout << what << std::endl;
        }
        static_assert(std::is_same<
                toml::value&, decltype(toml::find(v, "key"))>::value, "");
    }
    {
        const toml::value v = u8R"(
            keu = "value"
        )"_toml;

        BOOST_CHECK_THROW(toml::find(v, "key", lev), std::out_of_range);
        try
        {
            const auto& ret = toml::find(v, "key", lev);
            (void)ret;
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);

//             std::cout << what << std::endl;
        }
        static_assert(std::is_same<
                toml::value const&, decltype(toml::find(v, "key"))>::value, "");
    }
    {
        toml::value v = u8R"(
            keu = "value"
        )"_toml;

        bool thrown = false; // since it moves, we need to check both once
        try
        {
            const auto& ret = toml::find(std::move(v), "key", lev);
            (void)ret;
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);
            thrown = true;

//             std::cout << what << std::endl;
        }
        BOOST_TEST(thrown);
        static_assert(std::is_same<
                toml::value&, decltype(toml::find(v, "key"))>::value, "");
    }
}

BOOST_AUTO_TEST_CASE(test_find_throw_conversion_typo_aware_exception)
{
    using namespace toml::literals::toml_literals;
    const toml::levenstein_matcher lev(1);
    {
        toml::value v = u8R"(
            keu = 42
        )"_toml;

        BOOST_CHECK_THROW(toml::find<int>(v, "key", lev), std::out_of_range);
        try
        {
            const auto& ret = toml::find<int>(v, "key", lev);
            (void)ret; // suppress unused variable
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);

//             std::cout << what << std::endl;
        }
        static_assert(std::is_same<int,
                decltype(toml::find<int>(v, "key"))>::value, "");
    }
    {
        const toml::value v = u8R"(
            keu = 42
        )"_toml;

        BOOST_CHECK_THROW(toml::find<int>(v, "key", lev), std::out_of_range);
        try
        {
            const auto& ret = toml::find<int>(v, "key", lev);
            (void)ret;
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);

//             std::cout << what << std::endl;
        }
        static_assert(std::is_same<int,
                decltype(toml::find<int>(v, "key"))>::value, "");
    }
    {
        toml::value v = u8R"(
            keu = 42
        )"_toml;

        bool thrown = false; // since it moves, we need to check both once
        try
        {
            const auto& ret = toml::find<int>(std::move(v), "key", lev);
            (void)ret;
        }
        catch(const std::out_of_range& oor)
        {
            // exception.what() should include the typo-ed key name
            const std::string what(oor.what());
            BOOST_TEST(what.find("keu") != std::string::npos);
            thrown = true;

//             std::cout << what << std::endl;
        }
        BOOST_TEST(thrown);
        static_assert(std::is_same<int,
                decltype(toml::find<int>(v, "key"))>::value, "");
    }
}

#define BOOST_TEST_MODULE "test_acceptor"
#ifdef UNITTEST_FRAMEWORK_LIBRARY_EXIST
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_NO_LIB
#include <boost/test/included/unit_test.hpp>
#endif
#include <toml/acceptor.hpp>
#include <locale>
#include <iostream>

BOOST_AUTO_TEST_CASE(test_conditions)
{
    {
        const std::string tmp(" ");
        const std::string dummy("dummy");
        BOOST_CHECK(toml::is_space<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
        BOOST_CHECK(toml::is_space<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
    }

    {
        const std::string tmp("\t");
        const std::string dummy("dummy");
        BOOST_CHECK(toml::is_tab<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
        BOOST_CHECK(toml::is_tab<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
    }

    {
        for(int i=0; i<10; ++i)
        {
            const std::string tmp = std::to_string(i);
            const std::string dummy("dummy");
            BOOST_CHECK(toml::is_number<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
            BOOST_CHECK(toml::is_number<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
        }
    }

    {
        for(char c='a'; c <= 'z'; ++c)
        {
            const std::string tmp(1, c);
            const std::string dummy(1, std::toupper(c));
            BOOST_CHECK(toml::is_lowercase<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
            BOOST_CHECK(toml::is_lowercase<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
        }
    }
    {
        for(char c='A'; c <= 'Z'; ++c)
        {
            const std::string tmp(1, c);
            const std::string dummy(1, std::tolower(c));
            BOOST_CHECK(toml::is_uppercase<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
            BOOST_CHECK(toml::is_uppercase<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
        }
    }
    {
        const std::string tmp(" ");
        const std::string dummy("a");
        BOOST_CHECK(toml::is_whitespace<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
        BOOST_CHECK(toml::is_whitespace<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
    }
    {
        const std::string tmp("\t");
        const std::string dummy("a");
        BOOST_CHECK(toml::is_whitespace<char>::invoke(tmp.begin(), tmp.end())     == tmp.end());
        BOOST_CHECK(toml::is_whitespace<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
    }
    {
        const std::string tmp("hoge1-piyo2_fuga3");
        const std::string dummy(" \t");
        BOOST_CHECK(toml::is_barekey<char>::invoke(tmp.begin(), tmp.end())     != tmp.begin());
        BOOST_CHECK(toml::is_barekey<char>::invoke(dummy.begin(), dummy.end()) == dummy.begin());
    }
}

BOOST_AUTO_TEST_CASE(test_basic_inline_string)
{
    using is_valid = toml::is_basic_inline_string<char>;
    {
        const std::string simple("\"hoge1-piyo2_fuga3\"");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
    }
    {
        const std::string quote("\"hoge1-\\\"piyo2\\\"_fuga3\"");
        BOOST_CHECK(is_valid::invoke(quote.cbegin(), quote.cend()) == quote.cend());
    }
    {
        const std::string escape("\"I'm a string. \\\"You can quote me\\\". Name\\tJos\\u00E9\\nLocation\\tSF.\"");
        BOOST_CHECK(is_valid::invoke(escape.cbegin(), escape.cend()) == escape.cend());
    }
    {
        const std::string empty("\"\"");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }

    {
        const std::string newline("\"newline\r\nhoge\"");
        BOOST_CHECK(is_valid::invoke(newline.cbegin(), newline.cend()) == newline.cbegin());
    }
    {
        const std::string invalid_escape("\"foo\\abar\"");
        BOOST_CHECK(is_valid::invoke(invalid_escape.cbegin(), invalid_escape.cend()) == invalid_escape.cbegin());
    }
    {
        const std::string invalid_character("\"foo\10bar\"");
        BOOST_CHECK(is_valid::invoke(invalid_character.cbegin(), invalid_character.cend()) == invalid_character.cbegin());
    }
    {
        const std::string multi("\"\"\"multiline\"\"\"");
        BOOST_CHECK(is_valid::invoke(multi.cbegin(), multi.cend()) == multi.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_basic_multiline_string)
{
    using is_valid = toml::is_basic_multiline_string<char>;
    {
        const std::string simple("\"\"\"foobar\"\"\"");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
    }
    {
        const std::string quote("\"\"\"hoge1-\"piyo2\"_fuga3\"\"\"");
        BOOST_CHECK(is_valid::invoke(quote.cbegin(), quote.cend()) == quote.cend());
    }
    {
        const std::string newline("\"\"\"hoge1-\npiyo2_\r\nfuga3\"\"\"");
        BOOST_CHECK(is_valid::invoke(newline.cbegin(), newline.cend()) == newline.cend());
    }
    {
        const std::string escape("\"\"\"I'm a string. \"You can quote me\". Name\\tJos\\u00E9\\nLocation\\tSF.\"\"\"");
        BOOST_CHECK(is_valid::invoke(escape.cbegin(), escape.cend()) == escape.cend());
    }
    {
        const std::string empty("\"\"\"\"\"\"");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }
    {
        const std::string ending_backslash("\"\"\"hoge\\\n   piyo\\\n\"\"\"");
        BOOST_CHECK(is_valid::invoke(ending_backslash.cbegin(), ending_backslash.cend()) == ending_backslash.cend());
    }

    {
        const std::string invalid_escape("\"\"\"foo\\abar\"\"\"");
        BOOST_CHECK(is_valid::invoke(invalid_escape.cbegin(), invalid_escape.cend()) == invalid_escape.cbegin());
    }
    {
        const std::string invalid_character("\"\"\"foo\10bar\"\"\"");
        BOOST_CHECK(is_valid::invoke(invalid_character.cbegin(), invalid_character.cend()) == invalid_character.cbegin());
    }
    {
        const std::string single("\"singleline\"");
        BOOST_CHECK(is_valid::invoke(single.cbegin(), single.cend()) == single.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_literal_inline_string)
{
    using is_valid = toml::is_literal_inline_string<char>;
    {
        const std::string simple("'foobar'");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
    }
    {
        const std::string nonescape("'C:\\Users\\nodejs\\templates'");
        BOOST_CHECK(is_valid::invoke(nonescape.cbegin(), nonescape.cend()) == nonescape.cend());
    }
    {
        const std::string empty("''");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }

    {
        const std::string quote("'hoge1-'piyo2'_fuga3'");
        BOOST_CHECK(is_valid::invoke(quote.cbegin(), quote.cend()) == quote.cbegin() + 8);
    }
    {
        const std::string newline("'hoge1-\npiyo2_\r\nfuga3'");
        BOOST_CHECK(is_valid::invoke(newline.cbegin(), newline.cend()) == newline.cbegin());
    }
    {
        const std::string invalid_character("'foo\10bar'");
        BOOST_CHECK(is_valid::invoke(invalid_character.cbegin(), invalid_character.cend()) == invalid_character.cbegin());
    }
    {
        const std::string multi("'''multiline'''");
        BOOST_CHECK(is_valid::invoke(multi.cbegin(), multi.cend()) == multi.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_literal_multiline_string)
{
    using is_valid = toml::is_literal_multiline_string<char>;
    {
        const std::string simple("'''foobar'''");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
    }
    {
        const std::string quote("'''hoge1-'piyo2'_fuga3'''");
        BOOST_CHECK(is_valid::invoke(quote.cbegin(), quote.cend()) == quote.cend());
    }
    {
        const std::string nonescape("'''C:\\Users\\nodejs\\templates'''");
        BOOST_CHECK(is_valid::invoke(nonescape.cbegin(), nonescape.cend()) == nonescape.cend());
    }
    {
        const std::string newline("'''hoge1-\npiyo2_\r\nfuga3'''");
        BOOST_CHECK(is_valid::invoke(newline.cbegin(), newline.cend()) == newline.cend());
    }
    {
        const std::string empty("''''''");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }

    {
        const std::string invalid_character("'''foo\10bar'''");
        BOOST_CHECK(is_valid::invoke(invalid_character.cbegin(), invalid_character.cend()) == invalid_character.cbegin());
    }
    {
        const std::string single("'singleline'");
        BOOST_CHECK(is_valid::invoke(single.cbegin(), single.cend()) == single.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_integer)
{
    using is_valid = toml::is_integer<char>;
    {
        const std::string simple("1");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
    }
    {
        const std::string psign("+1234");
        BOOST_CHECK(is_valid::invoke(psign.cbegin(), psign.cend()) == psign.cend());
        const std::string nsign("-1234");
        BOOST_CHECK(is_valid::invoke(nsign.cbegin(), nsign.cend()) == nsign.cend());
    }
    {
        const std::string zero("0");
        BOOST_CHECK(is_valid::invoke(zero.cbegin(), zero.cend()) == zero.cend());
    }
    {
        const std::string us("1_2_3_4_5");
        BOOST_CHECK(is_valid::invoke(us.cbegin(), us.cend()) == us.cend());
    }

    {
        const std::string f("12.34");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cbegin()+2);
    }
    {
        const std::string f("12e34");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cbegin()+2);
    }
    {
        const std::string ascii("1234a");
        BOOST_CHECK(is_valid::invoke(ascii.cbegin(), ascii.cend()) == ascii.cbegin()+4);
    }
}

BOOST_AUTO_TEST_CASE(test_float)
{
    using is_valid = toml::is_float<char>;
    {
        const std::string simplef("1.0");
        BOOST_CHECK(is_valid::invoke(simplef.cbegin(), simplef.cend()) == simplef.cend());
        const std::string simplee("1e0");
        BOOST_CHECK(is_valid::invoke(simplee.cbegin(), simplee.cend()) == simplee.cend());
        const std::string both("6.626e-34");
        BOOST_CHECK(is_valid::invoke(both.cbegin(), both.cend()) == both.cend());
    }
    {
        const std::string psign("+1.0");
        BOOST_CHECK(is_valid::invoke(psign.cbegin(), psign.cend()) == psign.cend());
        const std::string nsign("-1.0");
        BOOST_CHECK(is_valid::invoke(nsign.cbegin(), nsign.cend()) == nsign.cend());
    }
    {
        const std::string psmall("+0.001");
        BOOST_CHECK(is_valid::invoke(psmall.cbegin(), psmall.cend()) == psmall.cend());
        const std::string nsmall("-0.001");
        BOOST_CHECK(is_valid::invoke(nsmall.cbegin(), nsmall.cend()) == nsmall.cend());
    }
    {
        const std::string zero("0.0");
        BOOST_CHECK(is_valid::invoke(zero.cbegin(), zero.cend()) == zero.cend());
    }
    {
        const std::string us("9_224_617.445_991_228_313");
        BOOST_CHECK(is_valid::invoke(us.cbegin(), us.cend()) == us.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_boolean)
{
    using is_valid = toml::is_boolean<char>;
    {
        const std::string t("true");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cend());
        const std::string f("false");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cend());
    }
    {
        const std::string t("True");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cbegin());
        const std::string f("False");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_localtime)
{
    using is_valid = toml::is_local_time<char>;
    {
        const std::string t("07:32:00");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cend());
        const std::string tf("07:32:00.0000");
        BOOST_CHECK(is_valid::invoke(tf.cbegin(), tf.cend()) == tf.cend());
    }
    {
        const std::string d("1907-32-00");
        BOOST_CHECK(is_valid::invoke(d.cbegin(), d.cend()) == d.cbegin());
        const std::string f("1907:32:00");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_localdate)
{
    using is_valid = toml::is_local_date<char>;
    {
        const std::string d("1907-32-00");
        BOOST_CHECK(is_valid::invoke(d.cbegin(), d.cend()) == d.cend());
    }
    {
        const std::string t("07:32:00");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cbegin());
        const std::string f("1907:32:00");
        BOOST_CHECK(is_valid::invoke(f.cbegin(),f.cend()) == f.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_localdatetime)
{
    using is_valid = toml::is_local_date_time<char>;
    {
        const std::string dt("1907-32-00T07:32:00");
        BOOST_CHECK(is_valid::invoke(dt.cbegin(), dt.cend()) == dt.cend());
        const std::string dtf("1907-32-00T07:32:00.0000");
        BOOST_CHECK(is_valid::invoke(dtf.cbegin(), dtf.cend()) == dtf.cend());
    }
    {
        const std::string d("1907-32-00");
        BOOST_CHECK(is_valid::invoke(d.cbegin(), d.cend()) == d.cbegin());
        const std::string t("07:32:00");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cbegin());
        const std::string f("1907-32-00 07:32:00");
        BOOST_CHECK(is_valid::invoke(f.cbegin(), f.cend()) == f.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_offsetdatetime)
{
    using is_valid = toml::is_offset_date_time<char>;
    {
        const std::string dtZ("1907-32-00T07:32:00Z");
        BOOST_CHECK(is_valid::invoke(dtZ.cbegin(), dtZ.cend()) == dtZ.cend());
        const std::string dtfZ("1907-32-00T07:32:00.0000Z");
        BOOST_CHECK(is_valid::invoke(dtfZ.cbegin(), dtfZ.cend()) == dtfZ.cend());
        const std::string dtp("1907-32-00T07:32:00+12:34");
        BOOST_CHECK(is_valid::invoke(dtp.cbegin(), dtp.cend()) == dtp.cend());
        const std::string dtfp("1907-32-00T07:32:00.0000+12:34");
        BOOST_CHECK(is_valid::invoke(dtfp.cbegin(), dtfp.cend()) == dtfp.cend());
        const std::string dtn("1907-32-00T07:32:00-12:34");
        BOOST_CHECK(is_valid::invoke(dtn.cbegin(), dtn.cend()) == dtn.cend());
        const std::string dtfn("1907-32-00T07:32:00.0000-12:34");
        BOOST_CHECK(is_valid::invoke(dtfn.cbegin(), dtfn.cend()) == dtfn.cend());
    }
    {
        const std::string d("1907-32-00");
        BOOST_CHECK(is_valid::invoke(d.cbegin(), d.cend()) == d.cbegin());
        const std::string t("07:32:00");
        BOOST_CHECK(is_valid::invoke(t.cbegin(), t.cend()) == t.cbegin());
        const std::string l("1907-32-00T07:32:00");
        BOOST_CHECK(is_valid::invoke(l.cbegin(), l.cend()) == l.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_array)
{
    using is_valid = toml::is_array<char>;
    {
        const std::string arr0("[]");
        BOOST_CHECK(is_valid::invoke(arr0.cbegin(), arr0.cend()) == arr0.cend());
        const std::string arr1("[1,2,3]");
        BOOST_CHECK(is_valid::invoke(arr1.cbegin(), arr1.cend()) == arr1.cend());
        const std::string arr2("[ 1,2,3 ]");
        BOOST_CHECK(is_valid::invoke(arr2.cbegin(), arr2.cend()) == arr2.cend());
        const std::string arr3("[ 1, 2, 3 ]");
        BOOST_CHECK(is_valid::invoke(arr3.cbegin(), arr3.cend()) == arr3.cend());
        const std::string arr4("[ 1, 2, 3, ]");
        BOOST_CHECK(is_valid::invoke(arr4.cbegin(), arr4.cend()) == arr4.cend());
        const std::string arr5("[ 1, 2, 3,]");
        BOOST_CHECK(is_valid::invoke(arr5.cbegin(), arr5.cend()) == arr5.cend());
        const std::string arr6("[ 1 , 2 , 3 ,]");
        BOOST_CHECK(is_valid::invoke(arr6.cbegin(), arr6.cend()) == arr6.cend());
    }
    {
        const std::string arr1("[\"red\", \"yellow\", \"green\"]");
        BOOST_CHECK(is_valid::invoke(arr1.cbegin(), arr1.cend()) == arr1.cend());
        const std::string arr2("[\"]\", \"#\", \"  \"]");
        BOOST_CHECK(is_valid::invoke(arr2.cbegin(), arr2.cend()) == arr2.cend());
        const std::string arr3("[[1, 2, 3], ['a', 'b', 'c']]");
        BOOST_CHECK(is_valid::invoke(arr3.cbegin(), arr3.cend()) == arr3.cend());
        const std::string arr4("[{hoge = 1}, {piyo = 'a'}, {fuga = [1,2,3]}]");
        BOOST_CHECK(is_valid::invoke(arr4.cbegin(), arr4.cend()) == arr4.cend());
    }
    {
        const std::string arr1("[1,\n2,#comment\n3]");
        BOOST_CHECK(is_valid::invoke(arr1.cbegin(), arr1.cend()) == arr1.cend());
        const std::string arr2("[#c\n1,\n2,#comment\r\n3]");
        BOOST_CHECK(is_valid::invoke(arr2.cbegin(), arr2.cend()) == arr2.cend());
    }

    {
        const std::string invalid("[1, 3.14, 'string']");
        BOOST_CHECK(is_valid::invoke(invalid.cbegin(), invalid.cend()) == invalid.cbegin());
        const std::string valid("[[1,2,3], [3.14, 2.71, 1.414], ['foo', 'bar']]");
        BOOST_CHECK(is_valid::invoke(valid.cbegin(), valid.cend()) == valid.cend());
    }

}

BOOST_AUTO_TEST_CASE(test_inline_table)
{
    using is_valid = toml::is_inline_table<char>;
    {
        const std::string tab0("{}");
        BOOST_CHECK(is_valid::invoke(tab0.cbegin(), tab0.cend()) == tab0.cend());
        const std::string tab1("{hoge=1,piyo=2,fuga=3}");
        BOOST_CHECK(is_valid::invoke(tab1.cbegin(), tab1.cend()) == tab1.cend());
        const std::string tab2("{hoge=1, piyo=2, fuga=3}");
        BOOST_CHECK(is_valid::invoke(tab2.cbegin(), tab2.cend()) == tab2.cend());
        const std::string tab3("{ hoge=1, piyo=2, fuga=3 }");
        BOOST_CHECK(is_valid::invoke(tab3.cbegin(), tab3.cend()) == tab3.cend());
        const std::string tab4("{ hoge = 1, piyo = 2, fuga = 3 }");
        BOOST_CHECK(is_valid::invoke(tab4.cbegin(), tab4.cend()) == tab4.cend());
        const std::string tab5("{hoge = 1, piyo = 2, fuga = 3}");
        BOOST_CHECK(is_valid::invoke(tab5.cbegin(), tab5.cend()) == tab5.cend());
        const std::string tab6("{hoge = 1, piyo = 2, fuga = 3,}");
        BOOST_CHECK(is_valid::invoke(tab6.cbegin(), tab6.cend()) == tab6.cend());
        const std::string tab7("{hoge = 1, piyo = 2, fuga = 3, }");
        BOOST_CHECK(is_valid::invoke(tab7.cbegin(), tab7.cend()) == tab7.cend());
    }
    {
        const std::string tab0("{hoge = 1, piyo = 2.0}");
        BOOST_CHECK(is_valid::invoke(tab0.cbegin(), tab0.cend()) == tab0.cend());
        const std::string tab1("{hoge = [1,2,3], piyo = {fuga = {}}}");
        BOOST_CHECK(is_valid::invoke(tab1.cbegin(), tab1.cend()) == tab1.cend());
        const std::string tab2("{hoge = \"}\", piyo = \"#\"}");
        BOOST_CHECK(is_valid::invoke(tab2.cbegin(), tab2.cend()) == tab2.cend());
        const std::string tab3("{b=true, i=1, f=2.0, d=1907-03-02T07:32:00, s='str', a=[1,2,3], t={foo=1}}");
        BOOST_CHECK(is_valid::invoke(tab3.cbegin(), tab3.cend()) == tab3.cend());
    }
    {
        const std::string tab0("{hoge = \"}\",\n piyo = \"#\"}");
        BOOST_CHECK(is_valid::invoke(tab0.cbegin(), tab0.cend()) == tab0.cbegin());
    }
}

BOOST_AUTO_TEST_CASE(test_table_definition)
{
    using is_valid = toml::is_table_definition<char>;
    {
        const std::string simple("[hoge]");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
        const std::string dotted("[hoge.piyo.fuga]");
        BOOST_CHECK(is_valid::invoke(dotted.cbegin(), dotted.cend()) == dotted.cend());
        const std::string spaced_dotted("[hoge . piyo .fuga. foo]");
        BOOST_CHECK(is_valid::invoke(spaced_dotted.cbegin(), spaced_dotted.cend()) == spaced_dotted.cend());
        const std::string quoted("[\"hoge\"]");
        BOOST_CHECK(is_valid::invoke(quoted.cbegin(), quoted.cend()) == quoted.cend());
        const std::string quoted_dot("[\"hoge\".'piyo'.fuga]");
        BOOST_CHECK(is_valid::invoke(quoted_dot.cbegin(), quoted_dot.cend()) == quoted_dot.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_array_of_table_definition)
{
    using is_valid = toml::is_array_of_table_definition<char>;
    {
        const std::string simple("[[hoge]]");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
        const std::string dotted("[[hoge.piyo.fuga]]");
        BOOST_CHECK(is_valid::invoke(dotted.cbegin(), dotted.cend()) == dotted.cend());
        const std::string spaced_dotted("[[hoge . piyo .fuga. foo]]");
        BOOST_CHECK(is_valid::invoke(spaced_dotted.cbegin(), spaced_dotted.cend()) == spaced_dotted.cend());
        const std::string quoted("[[\"hoge\"]]");
        BOOST_CHECK(is_valid::invoke(quoted.cbegin(), quoted.cend()) == quoted.cend());
        const std::string quoted_dot("[[\"hoge\".'piyo'.fuga]]");
        BOOST_CHECK(is_valid::invoke(quoted_dot.cbegin(), quoted_dot.cend()) == quoted_dot.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_key)
{
    using is_valid = toml::is_key<char>;
    {
        const std::string simple("foobar");
        BOOST_CHECK(is_valid::invoke(simple.cbegin(), simple.cend()) == simple.cend());
        const std::string quoted("\"foo#bar.baz\\n\"");
        BOOST_CHECK(is_valid::invoke(quoted.cbegin(), quoted.cend()) == quoted.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_value)
{
    using is_valid = toml::is_value<char>;
    {
        const std::string boolean("true");
        BOOST_CHECK(is_valid::invoke(boolean.cbegin(), boolean.cend()) == boolean.cend());
        const std::string integer("-42");
        BOOST_CHECK(is_valid::invoke(integer.cbegin(), integer.cend()) == integer.cend());
        const std::string floating("-42e0");
        BOOST_CHECK(is_valid::invoke(floating.cbegin(), floating.cend()) == floating.cend());
        const std::string string("\"string\"");
        BOOST_CHECK(is_valid::invoke(string.cbegin(), string.cend()) == string.cend());
        const std::string datetime("1901-01-01T00:00:00");
        BOOST_CHECK(is_valid::invoke(datetime.cbegin(), datetime.cend()) == datetime.cend());
        const std::string array("[1,2,3]");
        BOOST_CHECK(is_valid::invoke(array.cbegin(), array.cend()) == array.cend());
        const std::string table("{foo=1,bar=2.0,baz='3'}");
        BOOST_CHECK(is_valid::invoke(table.cbegin(), table.cend()) == table.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_key_value_pair)
{
    using is_valid = toml::is_key_value_pair<char>;
    {
        const std::string kv("key=1");
        BOOST_CHECK(is_valid::invoke(kv.cbegin(), kv.cend()) == kv.cend());
    }
    {
        const std::string kv("key = 1");
        BOOST_CHECK(is_valid::invoke(kv.cbegin(), kv.cend()) == kv.cend());
    }
    {
        const std::string kv("    key = 1");
        BOOST_CHECK(is_valid::invoke(kv.cbegin(), kv.cend()) == kv.cend());
    }
    {
        const std::string kv("    key = 1   ");
        BOOST_CHECK(is_valid::invoke(kv.cbegin(), kv.cend()) == kv.cend());
    }
    {
        const std::string boolean("key = true");
        BOOST_CHECK(is_valid::invoke(boolean.cbegin(), boolean.cend()) == boolean.cend());
        const std::string integer("key = -42");
        BOOST_CHECK(is_valid::invoke(integer.cbegin(), integer.cend()) == integer.cend());
        const std::string floating("key = -42.0");
        BOOST_CHECK(is_valid::invoke(floating.cbegin(), floating.cend()) == floating.cend());
        const std::string string("key = \"string\"");
        BOOST_CHECK(is_valid::invoke(string.cbegin(), string.cend()) == string.cend());
        const std::string datetime("key = 1901-01-01T00:00:00");
        BOOST_CHECK(is_valid::invoke(datetime.cbegin(), datetime.cend()) == datetime.cend());
        const std::string array("key = [1,2,3]");
        BOOST_CHECK(is_valid::invoke(array.cbegin(), array.cend()) == array.cend());
        const std::string table("key = {foo=1,bar=2.0,baz='3'}");
        BOOST_CHECK(is_valid::invoke(table.cbegin(), table.cend()) == table.cend());
    }
}

BOOST_AUTO_TEST_CASE(test_empty_line)
{
    using is_valid = toml::is_empty_line<char>;
    {
        const std::string empty("\n");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }
    {
        const std::string empty("    \n");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }
    {
        const std::string empty("#comment\n");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }
    {
        const std::string empty("    #comment\n");
        BOOST_CHECK(is_valid::invoke(empty.cbegin(), empty.cend()) == empty.cend());
    }
}

toml11
======

[![Build Status on TravisCI](https://travis-ci.org/ToruNiina/toml11.svg?branch=master)](https://travis-ci.org/ToruNiina/toml11)
[![Build status on Appveyor](https://ci.appveyor.com/api/projects/status/m2n08a926asvg5mg/branch/master?svg=true)](https://ci.appveyor.com/project/ToruNiina/toml11/branch/master)
[![Build status on CircleCI](https://circleci.com/gh/ToruNiina/toml11/tree/master.svg?style=svg)](https://circleci.com/gh/ToruNiina/toml11/tree/master)
[![Version](https://img.shields.io/github/release/ToruNiina/toml11.svg?style=flat)](https://github.com/ToruNiina/toml11/releases)
[![License](https://img.shields.io/github/license/ToruNiina/toml11.svg?style=flat)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1209136.svg)](https://doi.org/10.5281/zenodo.1209136)

toml11 is a C++11 header-only toml parser/encoder depending only on C++ standard library.

compatible to the latest version of
[TOML v0.5.0](https://github.com/toml-lang/toml/blob/master/versions/en/toml-v0.5.0.md)
after version 2.0.0.

It passes [the language agnostic test suite for TOML parsers by BurntSushi](https://github.com/BurntSushi/toml-test).
Not only the test suite itself, a TOML reader/encoder also runs on [CircleCI](https://circleci.com/gh/ToruNiina/toml11).
You can see the error messages about invalid files and serialization results of valid files at
[CircleCI](https://circleci.com/gh/ToruNiina/toml11).

## Example

```cpp
#include <toml11/toml.hpp>
#include <iostream>

int main()
{
    const auto data  = toml::parse("example.toml");

    // title = "an example toml file"
    std::string title = toml::get<std::string>(data.at("title"));
    std::cout << "the title is " << title << std::endl;

    // nums = [1, 2, 3, 4, 5]
    std::vector<int> nums  = toml::get<std::vector<int>>(data.at("nums"));
    std::cout << "the length of `nums` is" << nums.size() << std::endl;

    return 0;
}
```

## Table of Contents

- [Integration](#integration)
- [Decoding a toml file](#decoding-a-toml-file)
  - [In the case of syntax error](#in-the-case-of-syntax-error)
- [Getting a toml value](#getting-a-toml-value)
  - [In the case of type error](#in-the-case-of-type-error)
- [Getting an array](#getting-an-array)
- [Getting a table](#getting-a-table)
  - [Dotted keys](#dotted-keys)
- [Getting an array of tables](#getting-an-array-of-tables)
- [Cost of conversion](#cost-of-conversion)
- [Getting datetime and its variants](#getting-datetime-and-its-variants)
- [Getting with a fallback](#getting-with-a-fallback)
- [Expecting conversion](#expecting-conversion)
- [Finding a value from a table](#finding-a-value-from-a-table)
- [Checking value type](#checking-value-type)
- [Visiting a toml::value](#visiting-a-tomlvalue)
- [TOML literal](#toml-literal)
- [Conversion between toml value and arbitrary types](#conversion-between-toml-value-and-arbitrary-types)
- [Invalid UTF-8 Codepoints](#invalid-utf-8-codepoints)
- [Formatting user-defined error messages](#formatting-user-defined-error-messages)
- [Getting comments related to a value](#getting-comments)
- [Serializing TOML data](#serializing-toml-data)
- [Underlying types](#underlying-types)
- [Running Tests](#running-tests)
- [Contributors](#contributors)
- [Licensing Terms](#licensing-terms)

## Integration

Just include the file after adding it to the include path.

```cpp
#include <toml11/toml.hpp> // that's all! now you can use it.
#include <iostream>

int main()
{
    const auto data  = toml::parse("example.toml");
    const auto title = toml::get<std::string>(data.at("title"));
    std::cout << "the title is " << title << std::endl;
    return 0;
}
```

## Decoding a toml file

To parse a toml file, the only thing you have to do is
to pass a filename to the `toml::parse` function.

```cpp
const std::string fname("sample.toml");
const toml::table data = toml::parse(fname);
```

If it encounters a file open error, it will throw `std::runtime_error`.

You can also pass a `std::istream` to the  `toml::parse` function.
To show a filename in an error message, it is recommended to pass the filename
with the stream.

```cpp
std::ifstream ifs("sample.toml", std::ios_base::binary);
assert(ifs.good());
const auto data = toml::parse(ifs, /*optional*/ "sample.toml");
```

Note that on Windows, if a file is opened in text-mode, CRLF ("\r\n") will
automatically be converted to LF ("\n") and this causes inconsistency between
file size and the contents that would be read. This causes weird error.
To use a file stream with `toml::parse` on Windows, don't forget to open it
in binary mode.

### In the case of syntax error

If there is a syntax error in a toml file, `toml::parse` will throw `toml::syntax_error`.

toml11 has clean and informative error messages inspired by Rust and
it looks like the following.

```console
terminate called after throwing an instance of 'toml::syntax_error'
  what():  [error] toml::parse_table: invalid line format # error description
 --> example.toml                                         # file name
 3 | a = 42 = true                                        # line num and content
   |        ^------ expected newline, but got '='.        # error reason
```

If you (mistakenly) duplicate tables and got an error, it is helpful to see
where they are. toml11 shows both at the same time like the following.

```console
terminate called after throwing an instance of 'toml::syntax_error'
  what():  [error] toml::insert_value: table ("table") already exists.
 --> duplicate-table.toml
 1 | [table]
   | ~~~~~~~ table already exists here
 ...
 3 | [table]
   | ~~~~~~~ table defined twice
```

Since the error message generation is generally a difficult task, the current
status is not ideal. If you encounter a weird error message, please let us know
and contribute to improve the quality!

## Getting a toml value

After parsing successfully, you can obtain the values from the result of
`toml::parse` using `toml::get` function.

```toml
# sample.toml
answer  = 42
pi      = 3.14
numbers = [1,2,3]
time    = 1979-05-27T07:32:00Z
[tab]
key = "value"
```

``` cpp
const auto data      = toml::parse("sample.toml");
const auto answer    = toml::get<std::int64_t    >(data.at("answer"));
const auto pi        = toml::get<double          >(data.at("pi"));
const auto numbers   = toml::get<std::vector<int>>(data.at("numbers"));
const auto timepoint = toml::get<std::chrono::system_clock::time_point>(data.at("time"));
const auto tab       = toml::get<toml::Table>(data.at("tab"));
const auto key       = toml::get<std::string>( tab.at("key"));
```

When you pass an exact TOML type that does not require type conversion,
`toml::get` returns a reference through which you can modify the content.

```cpp
auto  data   = toml::parse("sample.toml");
auto& answer = toml::get<toml::integer>(data["answer"]);     // get reference
answer = 6 * 9;                                              // write to data.answer
std::cout << toml::get<int>(data.at("answer")) << std::endl; // 54
```

If the specified type requires conversion, you can't take a reference to the value.
See also [underlying types](#underlying-types).

NOTE: To enable to get a reference, conversions between Float and Integer are not supported.

After C++17, you can use `std::string_view` to get a string from a `toml::value`.

```cpp
const auto sv = toml::get<std::string_view>(tab.at("key"));
```

### In the case of type error

If you pass an invalid type to `toml::get`, `toml::type_error` will be thrown.
Similar to the case of syntax error, toml11 also displays clean error messages.
The error message when you choose `int` to get `string` value would be like this.

```console
terminate called after throwing an instance of 'toml::type_error'
  what():  [error] toml::value bad_cast to integer
 --> example.toml
 3 | title = "TOML Example"
   |         ~~~~~~~~~~~~~~ the actual type is string
```

NOTE: In order to show this kind of error message, all the toml values have
pointers to represent its range in a file. The entire contents of a file is
shared by `toml::value`s and remains on the heap memory. It is recommended to
destruct all the `toml::value` classes after configuring your application
if you have a large TOML file compared to the memory resource.

## Getting an array

You can get any kind of `container` class from a `toml::array`
except for `map`-like classes.

``` cpp
// # sample.toml
// numbers = [1,2,3]

const auto vc  = toml::get<std::vector<int>  >(data.at("numbers"));
const auto ls  = toml::get<std::list<int>    >(data.at("numbers"));
const auto dq  = toml::get<std::deque<int>   >(data.at("numbers"));
const auto ar  = toml::get<std::array<int, 3>>(data.at("numbers"));
// if the size of data.at("numbers") is larger than that of std::array,
// it will throw toml::type_error because std::array is not resizable.
```

Surprisingly, you can also get `std::pair`s and `std::tuple`s from `toml::array`.

```cpp
const auto tp = toml::get<std::tuple<short, int, unsigned int>>(data.at("numbers"));
```

This functionality is helpful when you have the following toml file.

```toml
array_of_arrays = [[1, 2, 3], ["foo", "bar", "baz"]] # toml allows this
```

What is the corresponding C++ type? Obviously, it is a `std::pair` of `std::vector`s.

```cpp
const auto aofa = toml::get<
    std::pair<std::vector<int>, std::vector<std::string>>
    >(data.at("array_of_arrays"));
```

If you don't know the type of the elements, you can use `toml::array`,
which is a `std::vector` of `toml::value`, instead.

```cpp
const auto aofa  = toml::get<toml::array>(data.at("array_of_arrays"));
const auto first = toml::get<std::vector<int>>(aofa.at(0));
```

See also [expecting conversion](#expecting-conversion)
and [checking-value-type](#checking-value-type).

## Getting a table

`toml::table` is a key component of this library, which is an alias of
a `std::unordered_map` from `toml::key (a.k.a. std::string)` to `toml::value`.
`toml::parse` returns this.

Since it is just an alias of `std::unordered_map`, it has all the functionalities
that `std::unordered_map` has, e.g. `operator[]`, `count`, and `find`.

```cpp
toml::table data = toml::parse("example.toml");
if(data.count("title") != 0)
{
    data["title"] = std::string("TOML example");
}
```

When all the values of the table have the same type, toml11 allows you to
convert a `toml::table` to a `map` that contains the convertible type.

```toml
[tab]
key1 = "foo" # all the values are
key2 = "bar" # toml String
```

```cpp
const auto data = toml::parse("sample.toml");
const auto tab = toml::get<std::map<std::string, std::string>>(data.at("tab"));
std::cout << tab["key1"] << std::endl; // foo
std::cout << tab["key2"] << std::endl; // bar
```

## Dotted keys

TOML v0.5.0 has a new feature named "dotted keys".
You can chain keys to represent the structure of the data.

```toml
physical.color = "orange"
physical.shape = "round"
```

This is equivalent to the following.

```toml
[physical]
color = "orange"
shape = "round"
```

You can get both of the above formats with the same c++ code.

```cpp
const auto physical = toml::get<toml::table>(data.at("physical"));
const auto color    = toml::get<std::string>(physical.at("color"));
```

## Getting an array of tables

An array of tables is just an array of tables.
You can get it completely in the same way as the other arrays and tables.

```toml
# sample.toml
array_of_inline_tables = [{key = "value1"}, {key = "value2"}, {key = "value3"}]

[[array_of_tables]]
key = "value4"
[[array_of_tables]]
key = "value5"
[[array_of_tables]]
key = "value6"
```

```cpp
const auto data = toml::parse("sample.toml");
const auto aot1 = toml::get<std::vector<toml::table>>(data.at("array_of_inline_tables"));
const auto aot2 = toml::get<std::vector<toml::table>>(data.at("array_of_tables"));
```

## Cost of conversion

Although `toml::get` is convenient, it has additional copy-cost because
it copies data contained in `toml::value` to the user-specified type.
Of course in some cases this overhead is not ignorable.

```cpp
// the following code constructs a std::vector.
// it requires heap allocation for vector and element conversion.
const auto array = toml::get<std::vector<int>>(data.at("foo"));
```

By passing the exact types, `toml::get` returns reference that has no overhead.

``` cpp
const auto& tab     = toml::get<toml::array>(data.at("tab"));
const auto& numbers = toml::get<toml::table>(data.at("numbers"));
```

In this case you need to call `toml::get` each time you access to
the element of `toml::array` because `toml::array` is an array of `toml::value`.

```cpp
const auto& num0 = toml::get<toml::integer>(numbers.at(0));
const auto& num1 = toml::get<toml::integer>(numbers.at(1));
const auto& num2 = toml::get<toml::integer>(numbers.at(2));
```

## Getting datetime and its variants

TOML v0.5.0 has 4 different datetime objects, `local_date`, `local_time`,
`local_datetime`, and `offset_datetime`.

Since `local_date`, `local_datetime`, and `offset_datetime` represent a time
point, you can convert them to `std::chrono::system_clock::time_point`.

Contrary, `local_time` does not represents a time point because they lack a
date information, but it can be converted to `std::chrono::duration` that
represents a duration from the beginning of the day, `00:00:00.000`.

```toml
date = 2018-12-23
time = 12:30:00
l_dt = 2018-12-23T12:30:00
o_dt = 2018-12-23T12:30:00+09:30
```

```cpp
const auto data = toml::parse("sample.toml");

const auto date = toml::get<std::chrono::system_clock::time_point>(data.at("date"));
const auto l_dt = toml::get<std::chrono::system_clock::time_point>(data.at("l_dt"));
const auto o_dt = toml::get<std::chrono::system_clock::time_point>(data.at("o_dt"));

const auto time = toml::get<std::chrono::minutes>(data.at("time")); // 12 * 60 + 30 min
```

toml11 contains datetime as its own struct.
You can see the definitions in [toml/datetime.hpp](toml/datetime.hpp).

## Getting with a fallback

`toml::get_or` returns a default value if `toml::get<T>` failed.

```cpp
toml::value v("foo"); // v contains String
const int value = toml::get_or(v, 42); // conversion fails. it returns 42.
```

`toml::get_or` automatically deduces what type you want to get from
the default value you passed.

To get a reference through this function, take care about the default value.

```cpp
toml::value v("foo"); // v contains String
toml::integer& i = toml::get_or(v, 42); // does not work because binding `42`
                                        // to `integer&` is invalid
toml::integer opt = 42;
toml::integer& i = toml::get_or(v, opt); // this works.
```

## Expecting conversion

By using `toml::expect`, you will get your expected value or an error message
without throwing `toml::type_error`.

```cpp
const auto value = toml::expect<std::string>(data.at("title"));
if(value.is_ok()) {
    std::cout << value.unwrap() << std::endl;
} else {
    std::cout << value.unwrap_err() << std::endl;
}
```

Also, you can pass a function object to modify the expected value.

```cpp
const auto value = toml::expect<int>(data.at("number"))
    .map(// function that receives expected type (here, int)
    [](const int number) -> double {
        return number * 1.5 + 1.0;
    }).unwrap_or(/*default value =*/ 3.14);
```

## Finding a value from a table

toml11 provides utility function to find a value from `toml::table`.
Of course, you can do this in your own way with `toml::get` because
it just searches an  `unordered_map` and returns a value if it exists.

```cpp
const auto data = toml::parse("example.toml");
const auto num  = toml::find<int>(data, "num", /*for err msg*/"example.toml");
```

If the value does not exist, it throws `std::out_of_range` with an error message.

```console
terminate called after throwing an instance of 'std::out_of_range'
  what():  [error] key "num" not found in example.toml
```

You can use this with a `toml::value` that is expected to be a `toml::table`.
It automatically casts the value to table.

```cpp
const auto data = toml::parse("example.toml");
const auto num  = toml::find<int>(data.at("table"), "num");
// expecting the following example.toml
// [table]
// num = 42
```

In this case, because the value `data.at("table")` knows the locatoin of itself,
you don't need to pass where you find the value.
`toml::find` will show you an error message including table location.

```console
terminate called after throwing an instance of 'std::out_of_range'
  what():  [error] key "num" not found
 --> example.toml
 3 | [table]
   | ~~~~~~~ in this table
```

If it's not a `toml::table`, the same error as "invalid type" would be thrown.

There is another utility function, `toml::find_or`.
It is almost same as `toml::find`, but returns a default value if the value is
not found or has a different type, like `toml::get_or`.

```cpp
const auto data = toml::parse("example.toml");
const auto num  = toml::find_or(data.at("table"), "num", 42);
```

## Checking value type

You can check what type of value does `toml::value` contains by `is_*` function.

```cpp
toml::value v = /* ... */;
if(v.is_integer())
{
    std::cout << "value is an integer" << std::endl;
}
```

The complete list of the functions is below.

```cpp
const toml::value v(/*...*/);
v.is_boolean();
v.is_integer();
v.is_float();
v.is_string();
v.is_offset_datetime();
v.is_local_datetime();
v.is_local_date();
v.is_local_time();
v.is_array();
v.is_table();
v.is_uninitialized();
```

Also, you can get `enum class` value from `toml::value`.

```cpp
switch(data.at("something").type())
{
    case toml::value_t::Integer: /*do some stuff*/ ; break;
    case toml::value_t::Float  : /*do some stuff*/ ; break;
    case toml::value_t::String : /*do some stuff*/ ; break;
    default : throw std::runtime_error(
        "unexpected type : " + toml::stringize(data.at("something").type()));
}
```

The complete list of the `enum`s can be found in the section
[underlying types](#underlying-types).

The `enum`s can be used as a parameter of `toml::value::is` function like the following.

```cpp
toml::value v = /* ... */;
if(v.is(toml::value_t::Boolean)) // ...
```

## Casting value

So far, `toml::get` is introduced, but if you don't need any type conversion,
`as_*` is simpler to use.

```cpp
toml::value v = /* ... */;
if(v.is_integer() && v.as_integer() == 42)
{
    std::cout << "value is 42" << std::endl;
}
```

The complete list of the functions is below.

```cpp
const toml::value v(/*...*/);
v.as_boolean();
v.as_integer();
v.as_float();
v.as_string();
v.as_offset_datetime();
v.as_local_datetime();
v.as_local_date();
v.as_local_time();
v.as_array();
v.as_table();
v.as_uninitialized();
```

## Visiting a toml::value

toml11 provides `toml::visit` to apply a function to `toml::value` in the
same way as `std::variant`.

```cpp
const toml::value v(3.14);
toml::visit([](const auto& val) -> void {
        std::cout << val << std::endl;
    }, v);
```

The function object that would be passed to `toml::visit` must be able to
recieve all the possible TOML types. Also, the result types should be the same
each other.

## TOML literal

toml11 supports `"..."_toml` literal.
It accept both a bare value and a file content.

```cpp
using namespace toml::literals::toml_literals;

// `_toml` can convert a bare value without key
const toml::value v = u8"0xDEADBEEF"_toml;
// v is an Integer value containing 0xDEADBEEF.

// raw string literal (`R"(...)"` is useful for this purpose)
const toml::value t = u8R"(
    title = "this is TOML literal"
    [table]
    key = "value"
)"_toml;
// the literal will be parsed and the result will be contained in t
```

The literal function is defined in the same way as the standard library literals
such as `std::literals::string_literals::operator""s`.

```cpp
namespace toml
{
inline namespace literals
{
inline namespace toml_literals
{
toml::value operator""_toml(const char* str, std::size_t len);

} // toml_literals
} // literals
} // toml
```

Access to the operator can be gained with `using namespace toml::literals;`,
`using namespace toml::toml_literals`, and `using namespace toml::literals::toml_literals`.

Note that a key that is composed only of digits is allowed in TOML.
And, unlike the file parser, toml-literal allows a bare value without a key.
Thus it is difficult to distinguish arrays having integers and definitions of
tables that are named as digits.
Currently, literal `[1]` becomes a table named "1".
To ensure a literal to be considered as an array with one element, you need to
add a comma after the first element (like `[1,]`).

```cpp
"[1,2,3]"_toml;   // This is an array
"[table]"_toml;   // This is a table that has an empty table named "table" inside.
"[[1,2,3]]"_toml; // This is an array of arrays
"[[table]]"_toml; // This is a table that has an array of tables inside.

"[[1]]"_toml;     // This literal is ambiguous.
                  // Currently, it becomes a table that has array of table "1".
"1 = [{}]"_toml;  // This is a table that has an array of table named 1.
"[[1,]]"_toml;    // This is an array of arrays.
"[[1],]"_toml;    // ditto.
```

## Conversion between toml value and arbitrary types

You can also use `toml::get` and other related functions with the types you defined
after you implement some stuff.

```cpp
namespace ext
{
struct foo
{
    int         a;
    double      b;
    std::string c;
};
} // ext

const auto data = toml::parse("example.toml");

const foo f = toml::get<ext::foo>(data.at("foo"));
```

There are 2 ways to use `toml::get` with the types that you defined.

The first one is to implement `from_toml(const toml::value&)` member function.

```cpp
namespace ext
{
struct foo
{
    int         a;
    double      b;
    std::string c;

    void from_toml(const toml::value& v)
    {
        this->a = toml::find<int        >(v, "a");
        this->b = toml::find<double     >(v, "b");
        this->c = toml::find<std::string>(v, "c");
        return;
    }
};
} // ext
```

In this way, because `toml::get` first constructs `foo` without arguments,
the type should be default-constructible.

The second is to implement specialization of `toml::from` for your type.

```cpp
namespace ext
{
struct foo
{
    int         a;
    double      b;
    std::string c;
};
} // ext

namespace toml
{
template<>
struct from<ext::foo>
{
    ext::foo from_toml(const toml::value& v)
    {
        ext::foo f;
        f.a = toml::find<int        >(v, "a");
        f.b = toml::find<double     >(v, "b");
        f.c = toml::find<std::string>(v, "c");
        return f;
    }
};
} // toml
```

In this way, since the conversion function is defined outside of the class,
you can add conversion between `toml::value` and classes defined in another library.

Note that you cannot implement both of the functions because the overload
resolution of `toml::get` will be ambiguous.

----

The opposite direction is also supported in a similar way. You can directly
pass your type to `toml::value`'s constructor by introducing `into_toml` or
`toml::into<T>`.

```cpp
namespace ext
{
struct foo
{
    int         a;
    double      b;
    std::string c;

    toml::table into_toml() const // you need to mark it const.
    {
        return toml::table{{"a", this->a}, {"b", this->b}, {"c", this->c}};
    }
};
} // ext

ext::foo    f{42, 3.14, "foobar"};
toml::value v(f);
```

The definition of `toml::into<T>` is similar to `toml::from<T>`.

```cpp
namespace ext
{
struct foo
{
    int         a;
    double      b;
    std::string c;
};
} // ext

namespace toml
{
template<>
struct into<ext::foo>
{
    toml::table into_toml(const ext::foo& f)
    {
        return toml::table{{"a", f.a}, {"b", f.b}, {"c", f.c}};
    }
};
} // toml

ext::foo    f{42, 3.14, "foobar"};
toml::value v(f);
```

Any type that can be converted to `toml::value`, e.g. `toml::table`, `toml::array`,
is okay to return from `into_toml`.

## Invalid UTF-8 codepoints

toml11 throws `syntax_error` if a value of an escape sequence
representing unicode character is not a valid UTF-8 codepoint.

```console
  what():  [error] toml::read_utf8_codepoint: input codepoint is too large.
 --> utf8.toml
 1 | exceeds_unicode = "\U0011FFFF example"
   |                              ^--------- should be in [0x00..0x10FFFF]
```

## Formatting user-defined error messages

When you encounter an error after you read the toml value, you may want to
show the error with the value.

toml11 provides you a function that formats user-defined error message with
related values. With a code like the following,

```cpp
const auto value = toml::find<int>(data, "num");
if(value < 0)
{
    std::cerr << toml::format_error("[error] value should be positive",
                                    data.at("num"), "positive number required")
              << std::endl;
}
```

you will get an error message like this.

```console
[error] value should be positive
 --> example.toml
 3 | num = -42
   |       ~~~ positive number required
```

When you pass two values to `toml::format_error`,

```cpp
const auto min = toml::find<int>(range, "min");
const auto max = toml::find<int>(range, "max");
if(max < min)
{
    std::cerr << toml::format_error("[error] max should be larger than min",
                                    data.at("min"), "minimum number here",
                                    data.at("max"), "maximum number here");
              << std::endl;
}
```

you will get an error message like this.

```console
[error] max should be larger than min
 --> example.toml
 3 | min = 54
   |       ~~ minimum number here
 ...
 4 | max = 42
   |       ~~ maximum number here
```

## Getting comments

Since toml11 keeps a file data until all the values are destructed, you can
also extract a comment related to a value by calling `toml::value::comment()`.

If there is a comment just after a value (within the same line), you can get
the specific comment by `toml::value::comment_inline()`.

If there are comments just before a value (without any newline between them),
you can get the comments by `toml::value::comment_before()`.

`toml::value::comment()` returns the results of both functions after
concatenating them.

```toml
a = 42 # comment for a.

# comment for b.
# this is also a comment for b.
b = "foo"

c = [ # comment for c.
    3.14, # this is not a comment for c, but for 3.14.
] # this is also a comment for c.
```

```cpp
// "# comment for a."
const std::string com1 = toml::find(data, "a").comment();

// "# comment for b."
const std::string com2 = toml::find(data, "b").comment();

// "# comment for c.\n# this is also a comment for c."
const std::string com3 = toml::find(data, "c").comment();

// "# this is not a comment for c, but for 3.14."
const std::string com3 = toml::find<toml::array>(data, "c").front().comment();
```

Note that once a data in a value is modified, the related file region
information would be deleted. Thus after modifying a data, you cannot find any
comments.

Also note that currently it does not support any way to set a comment to a value.
And currently, serializer does not take comments into account.

## Serializing TOML data

toml11 (after v2.1.0) enables you to serialize data into toml format.

```cpp
const auto data = toml::table{{"foo", 42}, {"bar", "baz"}};
std::cout << data << std::endl;
// bar = "baz"
// foo = 42
```

toml11 automatically makes a small table and small array inline.
You can specify the width to make them inline by `std::setw` for streams.

```cpp
const auto data = toml::table{
    {"qux",    toml::table{{"foo", 42}, {"bar", "baz"}}},
    {"quux",   toml::array{"small", "array", "of", "strings"}},
    {"foobar", toml::array{"this", "array", "of", "strings", "is", "too", "long",
                           "to", "print", "into", "single", "line", "isn't", "it?"}},
};

// the threshold becomes 80.
std::cout << std::setw(80) << data << std::endl;
// foobar = [
// "this","array","of","strings","is","too","long","to","print","into",
// "single","line","isn't","it?",
// ]
// quux = ["small","array","of","strings"]
// qux = {bar="baz",foo=42}


// the width is 0. nothing become inline.
std::cout << std::setw(0) << data << std::endl;
// foobar = [
// "this",
// ... (snip)
// "it?",
// ]
// quux = [
// "small",
// "array",
// "of",
// "strings",
// ]
// [qux]
// bar = "baz"
// foo = 42
```

It is recommended to set width before printing data. Some I/O functions changes
width to 0, and it makes all the stuff (including `toml::array`) multiline.
The resulting files becomes too long.

To control the precision of floating point numbers, you need to pass
`std::setprecision` to stream.

```cpp
const auto data = toml::table{
    {"pi", 3.141592653589793},
    {"e",  2.718281828459045}
};
std::cout << std::setprecision(17) << data << std::endl;
// e = 2.7182818284590451
// pi = 3.1415926535897931
std::cout << std::setprecision( 7) << data << std::endl;
// e = 2.718282
// pi = 3.141593
```

There is another way to format toml values, `toml::format()`.
It returns `std::string` that represents a value.

```cpp
const toml::value v{{"a", 42}};
const std::string fmt = toml::format(v);
// a = 42
```

Note that since `toml::format` formats a value, the resulting string may lack
the key value.

```cpp
const toml::value v{3.14};
const std::string fmt = toml::format(v);
// 3.14
```

To control the width and precision, `toml::format` receives optional second and
third arguments to set them. By default, the witdh is 80 and the precision is
`std::numeric_limits<double>::max_digit10`.

```cpp
const auto serial = toml::format(data, /*width = */ 0, /*prec = */ 17);
```

## Underlying types

The toml types (can be used as `toml::*` in this library) and corresponding `enum` names are listed in the table below.

| toml::type     | underlying c++ type                          | enum                            |
| -------------- | -------------------------------------------- | ------------------------------- |
| Boolean        | `bool`                                       | `toml::value_t::Boolean`        |
| Integer        | `std::int64_t`                               | `toml::value_t::Integer`        |
| Float          | `double`                                     | `toml::value_t::Float`          |
| String         | `toml::string`                               | `toml::value_t::String`         |
| LocalDate      | `toml::local_date`                           | `toml::value_t::LocalDate`      |
| LocalTime      | `toml::local_time`                           | `toml::value_t::LocalTime`      |
| LocalDatetime  | `toml::local_datetime`                       | `toml::value_t::LocalDatetime`  |
| OffsetDatetime | `toml::offset_datetime`                      | `toml::value_t::OffsetDatetime` |
| Array          | `std::vector<toml::value>`                   | `toml::value_t::Array`          |
| Table          | `std::unordered_map<toml::key, toml::value>` | `toml::value_t::Table`          |

`toml::string` is effectively the same as `std::string` but has an additional
flag that represents a kind of a string, `string_t::basic` and `string_t::literal`.
Although `std::string` is not an exact toml type, still you can get a reference
that points to internal `std::string` by using `toml::get<std::string>()` for convenience.

`Datetime` variants are `struct` that are defined in this library.
Because `std::chrono::system_clock::time_point` is a __time point__,
not capable of representing a Local Time independent from a specific day.

It is recommended to get `Datetime`s as `std::chrono` classes through `toml::get`.

## Running Tests

To run test codes, you need to clone toml-lang/toml repository under `build/` directory
because some of the test codes read a file in the repository.

```sh
$ mkdir build
$ cd build
$ git clone https://github.com/toml-lang/toml.git
$ cmake ..
$ make
$ make test
```

To run the language agnostic test suite, you need to compile
`tests/check_toml_test.cpp` and pass it to the tester.

## Contributors

I appreciate the help of the contributors who introduced the great feature to this library.

- Guillaume Fraux (@Luthaf)
  - Windows support and CI on Appvayor
  - Intel Compiler support
- Quentin Khan (@xaxousis)
  - Found & Fixed a bug around ODR
  - Improved error messages for invaild keys to show the location where the parser fails
- Petr Beneš (@wbenny)
  - Fixed warnings on MSVC

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2017-2019 Toru Niina

All rights reserved.

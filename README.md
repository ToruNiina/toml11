toml11
======

[![Build Status](https://travis-ci.org/ToruNiina/toml11.svg?branch=master)](https://travis-ci.org/ToruNiina/toml11)
[![Build status](https://ci.appveyor.com/api/projects/status/m2n08a926asvg5mg?svg=true)](https://ci.appveyor.com/project/ToruNiina/toml11)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1209136.svg)](https://doi.org/10.5281/zenodo.1209136)

c++11 header-only toml parser depending only on c++ standard library.

compatible to the latest version of TOML v0.5.0 after version 2.0.0.

Are you looking for pre-C++11 compatible toml parser? Try [Boost.toml](https://github.com/ToruNiina/Boost.toml)! It has almost the same functionality as this library and works with C++98 & Boost.

## How to use

## Installation

Just include the file after adding it to the include path.

```cpp
#include <toml11/toml.hpp> // that's all! now you can use it.

int main()
{
    const auto data  = toml::parse("example.toml");
    const auto title = toml::get<std::string>(data.at("title"));
    std::cout << "the title is " << title << std::endl;
}
```

### Decoding toml file

The only thing you have to do is to pass a filename to the `toml::parse` function.

```cpp
const std::string fname("sample.toml");
const toml::table data = toml::parse(fname);
```

In the case of file open error, it will throw `std::runtime_error`.

You can also pass a `stream` to the  `toml::parse` function after checking the status.

```cpp
std::ifstream ifs("sample.toml");
assert(ifs.good());
const auto data = toml::parse(ifs /*, "filename" (optional)*/);
```

To show a better error message, it is recommended to pass a filename with `istream`. See also [in the case of syntax error](#in-the-case-of-syntax-error) and [passing invalid type to toml::get](#passing-invalid-type-to-tomlget).

### In the case of syntax error

If there is a syntax error in a toml file, `toml::parse` will throw `toml::syntax_error`.

toml11 now has clean and informative error messages inspired by Rust and it looks like the following (comment after hash sign are actually not shown).

```console
terminate called after throwing an instance of 'toml::syntax_error'
  what():  [error] toml::parse_table: invalid line format # error description
 --> example.toml                                         # file name
 3 | a = 42 = true                                        # line num and content
   |        ^------ expected newline, but got '='.        # error reason
```

Since the error message generation is generally a difficult task, the current status is not ideal. toml11 needs your help. If you encounter a weird error message, please let us know and contribute to improve the quality!

### Getting a toml value

After parsing successfully, you can obtain the values from the result of `toml::parse` (here, `data`) using `toml::get` function.

```toml
answer  = 42
pi      = 3.14
numbers = [1,2,3]
time    = 1979-05-27T07:32:00Z
[tab]
key = "value"
```

``` cpp
const auto answer    = toml::get<std::int64_t    >(data.at("answer"));
const auto pi        = toml::get<double          >(data.at("pi"));
const auto numbers   = toml::get<std::vector<int>>(data.at("numbers"));
const auto timepoint = toml::get<std::chrono::system_clock::time_point>(data.at("time"));
const auto tab       = toml::get<toml::Table>(data.at("tab"));
const auto key       = toml::get<std::string>( tab.at("key"));
```

When you pass an exact TOML type that does not require type conversion, `toml::get` returns also a reference through which you can modify the content.

```cpp
toml::get<toml::integer>(data["answer"]) = 6 * 9;
std::cout << toml::get<int>(data.at("answer")) << std::endl; // 54
```

If the specified type requires conversion, you can't take a reference to the value. See also [underlying types](#underlying-types).

#### Passing invalid type to toml::get

If you choose the invalid type, `toml::type_error` will be thrown. Similar to the `syntax_error`, toml11 also displays informative error messages. The error message when you choose `int` to get `string` value would be like this.

```console
terminate called after throwing an instance of 'toml::type_error'
  what():  [error] toml::value bad_cast to integer
 --> example.toml
 3 | title = "TOML Example"
   |         ~~~~~~~~~~~~~~ the actual type is string
```

NOTE: In order to show this kind of error message, all the toml values have 1 shared_ptr that points the corresponding byte sequence and 2 iterators that point the range. It is recommended to destruct all the `toml::value` classes after configuring your application to save memory resources.

### Getting arrays

You can set any kind of `container` class to obtain a `toml::array` except for `map`-like classes.

``` cpp
const auto vc  = toml::get<std::vector<int>  >(data.at("numbers"));
const auto ls  = toml::get<std::list<int>    >(data.at("numbers"));
const auto dq  = toml::get<std::deque<int>   >(data.at("numbers"));
const auto ar  = toml::get<std::array<int, 3>>(data.at("numbers"));
// if the size of data.at("numbers") is larger than that of std::array,
// it will throw toml::type_error because std::array is not resizable.
```

Surprisingly, you can also get a `toml::array` as `std::pair` and `std::tuple.`

```cpp
const auto tp = toml::get<std::tuple<short, int, unsigned int>>(data.at("numbers"));
```

The case when you need this functionality is to get an array of arrays.

```toml
aofa = [[1,2,3], ["foo", "bar", "baz"]] # toml allows this
```

What is the corresponding C++ type? Obviously, it is a `std::pair` of `std::vector`s.

```cpp
const auto aofa = toml::get<
    std::pair<std::vector<int>, std::vector<std::string>>
    >(data.at("aofa"));
```

If you don't know what the type is inside the array, you can use `toml::array`, which is a `std::vector` of `toml::value`, instead.

```cpp
const auto aofa  = toml::get<toml::array>(data.at("aofa"));
const auto first = toml::get<toml::array>(aofa.at(0));
```

See also [expecting conversion](#expecting-conversion) and [checking-value-type](#checking-value-type).

### Getting tables

`toml::table` is a key component of this library, which is an alias of a `std::unordered_map` from `toml::key (a.k.a. std::string)` to `toml::value`. `toml::parse` returns this as a result.

Since it is just an alias of `std::unordered_map`, it has all the functionalities that `std::unordered_map` has, e.g. `operator[]`, `count`, and `find`.

```cpp
toml::table data = toml::parse("example.toml");
if(data.count("title") != 0)
{
    data["title"] = std::string("TOML example");
}
```

When all the values of the table have the same type, toml11 allows you to convert a `toml::table` to a `map` that contains the convertible type.

```toml
[tab]
key1 = "foo" # all the values are
key2 = "bar" # toml String
```

```cpp
const auto tab = toml::get<std::map<std::string, std::string>>(data.at("tab"));
std::cout << tab["key1"] << std::endl; // foo
std::cout << tab["key2"] << std::endl; // bar
```

### Dotted keys

TOML v0.5.0 has a new feature named "dotted keys". You can chain keys to represent the structure of the data.

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

### An array of tables

An array of tables is just an array of tables. You can get it completely in the same way as the other arrays and tables.

```toml
array_of_inline_table = [{key = "value1"}, {key = "value2"}, {key = "value3"}]

[[array_of_table]]
key = "value4"
[[array_of_table]]
key = "value5"
[[array_of_table]]
key = "value6"
```

```cpp
const auto aot1 = toml::get<std::vector<toml::table>>(data.at("array_of_inline_table"));
const auto aot2 = toml::get<std::vector<toml::table>>(data.at("array_of_table"));
```

### Cost of conversion

Although `toml::get` is convenient, it has additional copy-cost because it copies data contained in `toml::value` to the user-specified type. Of course in some case this overhead is not ignorable.

By passing the exact types, `toml::get` returns reference that has nealy zero overhead.

``` cpp
const auto& tab     = toml::get<toml::array>(data.at("tab"));
const auto& numbers = toml::get<toml::table>(data.at("numbers"));
```

Unfortunately, in this case you need to call `toml::get` each time you access to the element of `toml::array` because `toml::array` is an array of `toml::value`.

```cpp
const auto& num0 = toml::get<toml::integer>(numbers.at(0));
const auto& num1 = toml::get<toml::integer>(numbers.at(1));
const auto& num2 = toml::get<toml::integer>(numbers.at(2));
```

### Datetime and its variants

TOML v0.5.0 has 4 different datetime objects, `local_date`, `local_time`, `local_datetime`, and `offset_datetime`. With toml11, you can convert `local_time` to your favorite `std::chrono::duration` and others to `std::chrono::system_clock::time_point`.

```toml
time = 12:30:00
date = 2018-12-23
```

```cpp
const auto dur = toml::get<std::chrono::minutes>(data.at("time")); // 12 * 60 + 30 min
const auto tp  = toml::get<std::chrono::system_clock::time_point>(data.at("date"));
```

### Getting with a fallback

 `toml::get_or` returns a default value if `toml::get<T>` failed.

```cpp
toml::table data; // empty table!
const auto value = toml::get_or(data, "key", 42); // value => int 42.
```

`toml::get_or` automatically deduces what type you want to get from the default value you passed.

### Expecting conversion

By using `toml::expect`, you will get your expected value or an error message without throwing `toml::type_error`.

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

### Finding value from table

toml11 provides utility function to find a value from `toml::table`. Of course, you can do this in your own way with `toml::get` because it just searches an  `unordered_map` and returns a value if it exists.

```cpp
const auto data = toml::parse("example.toml");
const auto num  = toml::find<int>(data, "num", /*for err msg*/"example.toml");
```

If the value does not exist, it throws `std::out_of_range` with informative error message.

```console
terminate called after throwing an instance of 'std::out_of_range'
  what():  [error] key "num" not found in example.toml
```

You can use this with a `toml::value` that is expected to be a `toml::table`. It automatically casts the value to table.

```cpp
const auto data = toml::parse("example.toml");
const auto num  = toml::find<int>(data.at("table"), "num");
// expecting the following example.toml
// [table]
// num = 42
```

In this case, because the value `data.at("table")` knows the locatoin of itself, you don't need to pass where you find the value. `toml::find` will show you a great error message.

```console
terminate called after throwing an instance of 'std::out_of_range'
  what():  [error] key "num" not found
 --> example.toml
 3 | [table]
   | ~~~~~~~ in this table
```

If it's not a `toml::table`, the same error as "invalid type" would be thrown.

### Checking value type

When you don't know the exact type of toml-value, you can get `enum` type from `toml::value`.

```cpp
switch(data.at("something").type())
{
    case toml::value_t::Integer: /* do some stuff */; break;
    case toml::value_t::Float  : /* do some stuff */; break;
    case toml::value_t::String : /* do some stuff */; break;
    default : throw std::runtime_error(
        "unexpected type : " + toml::stringize(data.at("something").type()));
}
```

### Fill only the matched value

The more sophisticated way is using `toml::from_toml` and `std::tie`.

```cpp
toml::table data{{"something", toml::value("foo")}};
int         i = 0;
double      d = 0.;
std::string s;
toml::from_toml(std::tie(i, d, s), data.at("something"));
std::cout << i << ", " << d << ", " << s << std::endl; // 0, 0, foo
```

Here, only matched value will be filled. The others are left intact after calling `from_toml`.
It should be noted that `toml::from_toml` returns as usual even if there are no matched type.

`from_toml` can be used also for single type. 

```cpp
int i = 0;
toml::from_toml(i, data.at("something"));
```

### Sanitizing UTF-8 codepoints

toml11 shows warning if a value of an escape sequence used to represent unicode character exceeds the unicode range.

```console
[warning] input codepoint (0011FFFF) is too large to decode as a unicode character. The result may not be able to render to your screen.
 --> example.toml
 3 | exceeds_unicode = "\U0011FFFF example"
   |                     ~~~~~~~~~ should be in [0x00..0x10FFFF]
```

Also, toml11 throws `std::domain_error` if the code point exceeds the range that can be represented by utf-8.

```console
terminate called after throwing an instance of 'std::range_error'
  what():  [error] input codepoint (0020FFFF) is too large to encode as utf-8.
 --> example.toml
 3 | exceeds_utf8    = "\U0020FFFF example"
   |                     ~~~~~~~~~ should be in [0x00..0x10FFFF]
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
| OffsetDatetime | `toml::offset_datetime`                      | `toml::value_t::offsetDatetime` |
| Array          | `std::vector<toml::value>`                   | `toml::value_t::Array`          |
| Table          | `std::unordered_map<std::string, toml::key>` | `toml::value_t::Table`          |

`toml::string` is effectively the same as `std::string` but has an additional flag that represents a kind of a string, `string_t::basic` and `string_t::literal`. Although `std::string` is not an exact toml type, still you can get a reference that points to internal `std::string` by using `toml::get<std::string>()` for convenience.

`Datetime` variants are `struct` that are defined in this library. Because `std::chrono::system_clock::time_point` is a __time point__, not capable of representing a Local Time independent from a specific day.

It is recommended to get `Datetime`s as `std::chrono` classes through `toml::get`.

## Contributors

I thank the contributor for providing great feature to this repository.

- Guillaume Fraux (@Luthaf)
  - Windows support and CI on Appvayor
  - Intel Compiler support

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2017 Toru Niina

All rights reserved.

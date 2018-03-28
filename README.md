toml11
======

[![Build Status](https://travis-ci.org/ToruNiina/toml11.svg?branch=master)](https://travis-ci.org/ToruNiina/toml11)
[![Build status](https://ci.appveyor.com/api/projects/status/m2n08a926asvg5mg?svg=true)](https://ci.appveyor.com/project/ToruNiina/toml11)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1209136.svg)](https://doi.org/10.5281/zenodo.1209136)

c++11 header-only toml parser depending only on c++11 standard library.

compatible to TOML v0.4.0.

## How to use

Just include the file after adding correct include path.

```cpp
#include <toml11/toml.hpp>

int main()
{
    /* do something ... */
}
```

### decoding toml file

The only thing you have to do is passing filename to `toml::parse` function.

```cpp
const std::string fname("sample.toml");
const auto data = toml::parse(fname);
```

In the case of file open error, it will throw `std::runtime_error`.

You can pass also `stream` to `toml::parse` function.

```cpp
std::ifstream ifs("sample.toml");
assert(ifs.good());
const auto data = toml::parse(ifs);
```

If there are syntax error in the toml file,
`toml::parse` will throw `toml::syntax_error`.

#### toml::get()

Then you can obtain the various value from the `data` using `toml::get` function.

```toml
answer = 42
pi = 3.14
numbers = [1,2,3]
time = 1979-05-27T07:32:00Z
[tab]
key = "value"
```

``` cpp
const auto answer    = toml::get<std::int64_t>(data.at("answer"));
const auto pi        = toml::get<double>(data.at("pi"));
const auto numbers   = toml::get<std::vector<int>>(data.at("numbers"));
const auto timepoint = toml::get<std::chrono::system_clock::time_point>(data.at("time"));
const auto tab       = toml::get<toml::Table>(data.at("tab"));
const auto key       = toml::get<std::string>(tab.at("key"));
```

You can set any kind of `container` class to obtain `toml::Array` except for
`map`-like class.

``` cpp
const auto vc  = toml::get<std::vector<int>>(data.at("numbers"));
const auto ls  = toml::get<std::list<int>>(data.at("numbers"));
const auto dq  = toml::get<std::deque<int>>(data.at("numbers"));
const auto ar  = toml::get<std::array<int, 3>>(data.at("numbers"));
// if size of data.at("numbers") is larger than 3, it will throw toml::type_error
// because std::array is not resizable.
```

If the type you passed as a template parameter is incorrect,
it will throw `toml::type_error`.

``` cpp
const auto wrong1 = toml::get<bool>(data.at("integer")); // exception thrown!
const auto wrong2 = toml::get<float>(data.at("integer"));        // ditto
const auto wrong3 = toml::get<toml::Datetime>(data.at("array")); // ditto
```

Although `toml::get` is convenient, it has additional copy-cost because it
copies data contained in `toml::value` to user-specified type.
Of course in some case this overhead is not ignorable.

You can get reference pointing to contained value using `toml::value::cast()` like this.

``` cpp
const auto& pi      = data.at("pi").cast<toml::value_t::Float>();
const auto& tab     = data.at("tab").cast<toml::value_t::Table>();
const auto& numbers = data.at("numbers").cast<toml::value_t::Array>();
```

Unfortunately, if you use `toml::value::cast` to get an array, you would need to
`cast` each element in `toml::Array` because `toml::Array` is represented as
an array of `toml::value`.

```cpp
const auto& num0 = numbers.at(0).cast<toml::value_t::Integer>();
const auto& num1 = numbers.at(1).cast<toml::value_t::Integer>();
const auto& num2 = numbers.at(2).cast<toml::value_t::Integer>();
```

#### toml::get\_or

You can also set default value for `toml::get`.

```cpp
toml::Table data; // empty table!

const auto value1 = toml::get_or(data, "key1", 42); // value1 => int 42.

toml::Integer i(123);
const auto value2 = toml::get_or(data, "key1", i);  // value2 => toml::Integer 42.
```

#### toml::value\_t

When you don't know the exact type of toml-value, you can get `enum` type from
`toml::value`.

```cpp
int i;
double d;
std::string s;
std::vector<int> a;
const auto t = data.at("something").type();
switch(t)
{
    case toml::value_t::Integer: i = toml::get<int>(data.at("something")); break;
    case toml::value_t::Float  : d = toml::get<double>(data.at("something")); break;
    case toml::value_t::String : s = toml::get<std::string>(data.at("something")); break;
    case toml::value_t::Array  : a = toml::get<std::vector<int>>(data.at("something")); break;
    default : throw std::runtime_error("unexpected type : " + toml::stringize(t));
}
```

Okey, but it is painful to write `switch-case` for many time.

#### toml::from\_toml()

The more sophisticated way is using `toml::from_toml` and `std::tie`.

```cpp
int i = 0;
double d = 0.;
std::string s;
std::vector<int> a;

toml::from_toml(std::tie(i, d, s, a), data.at("something"));
```

Here, only matched value will be filled.
The others are left intact after calling `from_toml`.
It should be noted that `toml::from_toml` returns as usual even if there are no
matched type.


`from_toml` can be used also for single type. 

```cpp
int i;
toml::from_toml(i, data.at("something"));
```

Unlike `toml::get`, `toml::from_toml` does not require to specify the type
through the template argument because the type can be deduced from argument.

#### toml::value

In toml, `Array` is capable of having `Array` s and each of them possibly have
different types like this.

```toml
array_of_array = [[1,2,3,4,5], ["foo", "bar", "baz"]]
```

In this case, you can use `toml::value` directly.

```cpp
// use toml::value in a container 
const auto a  = toml::get<std::vector<toml::value>>(data.at("array_of_array"));
// or you can use default toml::Array.
const auto a_ = toml::get<toml::Array>(data.at("array_of_array"));
// you can obtain values from toml::value in the same way as described above.
const auto ns = toml::get<std::vector<std::int64_t>>(a.at(0));
const auto ss = toml::get<std::vector<std::string>>(a.at(1));
```

#### Array of Table

Of course, you can obtain `array of table` in the same way.

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
const auto aot1 = toml::get<std::vector<toml::Table>>(data.at("array_of_inline_table"))
const auto aot2 = toml::get<std::vector<toml::Table>>(data.at("array_of_table"))
```

## Documentation

The toml types and corresponding `enum` name are listed in the table below.
`value_t` is a scoped-enum defined in the namespace toml.

| toml-type | c++ type                                       | enum                      |
| --------- | ---------------------------------------------- | ------------------------- |
| Boolean   | `bool`                                         | `toml::value_t::Boolean`  |
| Integer   | `std::int64_t`                                 | `toml::value_t::Integer`  |
| Float     | `double`                                       | `toml::value_t::Float`    |
| String    | `std::string`                                  | `toml::value_t::String`   |
| Datetime  | `toml::Datetime`                               | `toml::value_t::Datetime` |
| Array     | `std::vector<toml::value>`                     | `toml::value_t::Array`    |
| Table     | `std::unordered_map<std::string, toml::value>` | `toml::value_t::Table`    |

`Datetime` is the `struct` that is defined in this library.
Because `std::chrono::system_clock::time_point` is a __time point__, not capable
of representing a Local Time independent from a specific day.

For user-convenience, `toml::Datetime` is _implicitly_ convertible to
`std::chrono::system_clock::time_point`. If `toml::Datetime` does not have any
Date information, the information will be generated from
`std::chrono::system_clock::now()` when cast is performed.

The definition of Datetime struct is below.

```cpp
namespace toml
{
template<typename uintT, typename intT>
struct basic_datetime
{
    uintT  year;          // since 0.
    uintT  month;         // [1-12]
    uintT  day;           // [1-31]
    uintT  hour;          // [0-23]
    uintT  minite;        // [0-59]
    uintT  second;        // [0-59]
    uintT  millisecond    // [0-999]
    uintT  microsecond    // [0-999]
    intT   offset_hour;   // [-12 - +12]
    intT   offset_minute; // [-59 - +59]
};

typedef basic_datetime<unsigned, int> Datetime;
}
```

It must be noted that the range of some values in `basic_datetime` is different
from `std::tm`. For example, month is in the range of `[1,12]` and year starts
from 0 (not 1900).

## Contributors

I thank the contributor for providing great feature to this repository.

- Guillaume Fraux (@Luthaf)
  - Windows support and CI on Appvayor
  - Intel Compiler support

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2017 Toru Niina

All rights reserved.

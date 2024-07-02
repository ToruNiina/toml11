# toml11 v4

[![Build Status on GitHub Actions](https://github.com/ToruNiina/toml11/workflows/build/badge.svg)](https://github.com/ToruNiina/toml11/actions)
[![Version](https://img.shields.io/github/release/ToruNiina/toml11.svg?style=flat)](https://github.com/ToruNiina/toml11/releases)
[![License](https://img.shields.io/github/license/ToruNiina/toml11.svg?style=flat)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1209136.svg)](https://doi.org/10.5281/zenodo.1209136)

toml11は、C++11,14,17,20のための豊富な機能を持つTOML言語ライブラリです。

- [TOML言語の最新規格](https://toml.io/ja/v1.0.0)に準拠しています。
- TOML言語標準のテストケースすべてにパスしています。
- TOML言語の次期バージョン (v1.1.0) にマージされた新機能のそれぞれを試すことができます。
- エラーが起きた位置を含めたわかりやすいエラーメッセージを出力します。
- コメントもパースし、対応する値に保存します。
- 16進整数やクオートの種類などのフォーマット情報を保持し、シリアライズ時に考慮します。
- 例外を投げないバージョンをサポートします。
- TOML値からの複雑な型変換をサポートします。
- 整数、浮動小数点数、コンテナ等の型を変更可能です。
- TOML言語にない一部の拡張機能を試すことができます。

## Example

```cpp
#include <toml.hpp>
#include <iostream>

// ```toml
// title = "an example toml file"
// nums  = [3, 1, 4, 1, 5] # pi!
// ```

int main()
{
    // select TOML version at runtime (optional)
    auto data = toml::parse("example.toml", toml::spec::v(1,1,0));

    // find a value with the specified type from a table
    std::string title = toml::find<std::string>(data, "title");

    // convert the whole array into STL-like container automatically
    std::vector<int> nums = toml::find<std::vector<int>>(data, "nums");

    // access with STL-like manner
    if( ! data.contains("foo"))
    {
        data["foo"] = "bar";
    }
    if(data.at("nums").is_array())
    {
        data.push_back(9);
    }

    // check comments
    assert(data.at("nums").comments().at(0) == "# pi!");

    // pass a fallback
    std::string name = toml::find_or<std::string>(data, "name", "not found");

    // serialization considering format info
    data.at("nums").as_array_fmt().fmt = toml::array_format::multiline;
    data.at("nums").as_array_fmt().indent_type = toml::indent_char::space;
    data.at("nums").as_array_fmt().body_indent = 2;

    std::cout << toml::format(data) << std::endl;

    return 0;
}
```

詳細な機能とリファレンスに関しては、[ドキュメント](https://toruniina.github.io/toml11/ja/)を参照してください。

## Table of Contents

- [Integration](#integration)
- [Features](#features)
  - [parsing a file](#parsing-a-file)
  - [finding a value](#finding-a-value)
  - [comments](#comments)
  - [error messages](#error-messages)
  - [serialization](#serialization)
- [Breaking Changes from v3](#changes-from-v3)
- [Contributors](#contributors)
- [Licensing Terms](#licensing-terms)

## Integration

toml11を使うには複数の方法があります。

ここではそれぞれを短く紹介します。詳細は、[ドキュメント](https://toruniina.github.io/toml11/ja/docs/installation/)を参照してください。

### Single include file

`single_include/toml.hpp`を好きなところにコピーして、インクルードパスを通してください。

### git submodule

git submoduleなどでサブディレクトリにすれば、`toml11/include`にインクルードパスを通すか、
`add_subdirectory(toml11)` とすることで使用できます。

### Install using CMake

以下の手順で、CMakeを使ってインストールすることができます。

```console
$ git clone https://github.com/ToruNiina/toml11
$ cd toml11
$ git submodule update --init --recursive
$ cmake -B ./build/
$ cmake --build ./build/
$ cmake --install ./build --prefix /path/to/toml11
```

### Precompile library

`-DTOML11_PRECOMPILE=ON` とすることで、ライブラリの一部の関数をコンパイルできます。

この場合、C++バージョンによって使用できる標準ライブラリ機能が変化し、
インターフェースの一部が変わるため、`CMAKE_CXX_STANDARD`を指定する必要があります。

```console
$ cmake -B ./build/ -DTOML11_PRECOMPILE=ON -DCMAKE_CXX_STANDARD=11/14/17/20
$ cmake --build ./build/
```

ライブラリをリンクする場合は、CMakeで

```cmake
target_link_libraries(your_target PUBLIC toml11::toml11)
```

とするか、コンパイラに`-DTOML11_COMPILE_SOURCES`を渡してください。

### Building example

`-DTOML11_BUILD_EXAMPLES=ON`とすることで、`examples/`をコンパイルできます。

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

### Building example

`-DTOML11_BUILD_TESTS=ON`とすることで、ユニットテストをコンパイルできます。

また、`-DTOML11_BUILD_TOML_TESTS=ON`とすることで、toml-test用の`encoder`と`decoder`をコンパイルできます。

```console
$ cmake -B ./build/ -DTOML11_BUILD_EXAMPLES=ON
$ cmake --build ./build/
```

## Features

ここでは、toml11の持つ機能を短く紹介します。

詳細については[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/)を参照してください。

### parsing a file

ファイルをパースする場合は、`toml::parse`を使います。

ファイル全体の型は常にテーブルですが、 `toml::value` はコメントや
フォーマット情報などのメタデータを持つので、`toml::parse`からは
`toml::table` ではなく `toml::value` が返ります。

```cpp
const toml::value input = toml::parse("input.toml");
```

文字列そのものをパースする場合は、`toml::parse_str`を使います。

```cpp
const toml::value input = toml::parse_str("a = 42");
```

`toml::parse`は文法エラーの際に `toml::syntax_error` 例外を投げます。
これを避けるには、 `toml::result` を返す `toml::try_parse` を使います。

```cpp
const auto input = toml::try_parse("input.toml");
if(input.is_ok())
{
    std::cout << input.unwrap().at("a").as_integer() << std::endl;
}
```

また、使用するTOML言語のバージョンを簡単に、かつ細かく変更できるようになりました。

TOML v1.0.0に加えて、v1.1.0の新機能を一部だけ使用するというような制御が可能です。

```cpp
toml::spec s = toml::spec::v(1, 0, 0);
s.v1_1_0_allow_trailing_comma_in_inline_tables = true;

const toml::value input = toml::parse("input.toml");
```

また、TOML言語自体に含まれない言語拡張もいくつか用意しています。

```cpp
toml::spec s = toml::spec::v(1, 0, 0);
s.ext_hex_float  = true; // 16進数浮動小数点数を許可
s.ext_null_value = true; // 空の値 `null` を許可
s.ext_num_suffix = true; // `100_msec`などのsuffixを許可
```

各機能の紹介とリファレンスは、[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/)を参照してください。

### finding a value

`toml::value` はアクセス用のメンバ関数、`at()` や `is_xxx()`, `as_xxx()` を持ちます。

```cpp
const toml::value input = toml::parse("input.toml");
if(input.contains("a") && input.at("a").is_integer())
{
    std::cout << input.at("a").as_integer() << std::endl;
}
```

`toml::find` を使うことで、型変換と検索が同時に行えます。

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << toml::find<int>(input, "a") << std::endl;
```

また、複雑なTOML値に対して、高度な型変換を行うことができます。

```toml
mixed_array = [
    42,
    3.14,
    {a = "foo", b = "bar"}
]
```

```cpp
const toml::value input = toml::parse("input.toml");

const auto mixed = toml::find<
        std::tuple<int, double, std::map<std::string, std::string>>
    >(input, "mixed_array") << std::endl;
```

詳細については[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/value/)を参照してください。

### comments

toml11は、値に関するコメントを値に保存します。

値に関するコメントとは、ある値の直前に書かれた一連のコメントと、値の直後に改行を挟まず書かれたコメントです。

```toml
# これは a のコメントです。
# これも a のコメントです。
a = 42 # これも a のコメントです。

# これは改行で分かれているので、 b のコメントではありません。

# これは b のコメントです。
b = "foo"
```

これは `std::vector<std::string>` と同じインターフェースで `toml::value` に格納されます。

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << input.at("a").comments().size() << std::endl;
std::cout << input.at("a").comments().at(0) << std::endl;
```

詳細については[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/value/#%E3%82%B3%E3%83%A1%E3%83%B3%E3%83%88%E3%81%AB%E3%82%A2%E3%82%AF%E3%82%BB%E3%82%B9%E3%81%99%E3%82%8B)を参照してください。

### error messages

toml11の目標の一つは、わかりやすいエラーメッセージを出力することです。

パースエラーに対しては以下のようなエラーメッセージが、

```
[error] bad integer: `_` must be surrounded by digits
 --> internal string at line 64 in file main.cpp
   |
 1 | a = 123__456
   |        ^-- invalid underscore
Hint: valid  : -42, 1_000, 1_2_3_4_5, 0xC0FFEE, 0b0010, 0o755
Hint: invalid: _42, 1__000, 0123
```

実際に格納されている型と異なる型を要求した場合には以下のようなエラーメッセージが表示されます。

```
[error] toml::value::as_string(): bad_cast to string
 --> input.toml
   |
 1 | a = 123_456
   |     ^^^^^^^-- the actual type is integer
```

このようなエラーメッセージを、TOMLの文法とは関係のないユーザー特有の処理に対しても出力することが可能です。

```cpp
const toml::value& a = input.at("a");
if(a.as_integer() < 0)
{
    const toml::error_info err = toml::make_error_info(
            "positive integer is required",
            a, "but got negative value"
        );
    std::cerr << toml::format_error(err) << std::endl;
}
```

詳細は[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/error_message/)を参照してください。

### serialization

`toml::format` を使うことで、`toml::value` を `std::string` にフォーマットできます。

```cpp
const toml::value input = toml::parse("input.toml");
std::cout << toml::format(input) << std::endl;
```

`toml::format` はフォーマット情報を参照します。
なので、フォーマット方法を変更することが可能です。

```cpp
toml::value output(toml::table{ {"a", 0xDEADBEEF} });
output.at("a").as_integer_fmt().fmt = toml::integer_format::hex;
output.at("a").as_integer_fmt().spacer = 4;

std::cout << toml::format(input) << std::endl;
```

どのような指定が可能かなどの詳細は[ドキュメント](https://toruniina.github.io/toml11/ja/docs/features/serialize/)を参照してください。

### configuring types

[`examples`ディレクトリ](https://github.com/ToruNiina/toml11/tree/main/examples)には、多倍長整数を使用する場合やユニコードを正規化する場合、
外部のリフレクションライブラリと連携する場合などの複雑な使用例を用意しています。

そのような状況での実装例として参照してください。

## Changes from v3

toml11 v3からは複数の破壊的変更が追加されています。

シンプルな使い方をしている場合にはほとんど変更せずに済むように努力はしていますが、
高度な機能を利用していた場合は多少の変更が必要になります。

しかし、追加された機能や整理された機能は、その分の利便性を提供できると考えています。

### 破壊的な変更

- ブランチを `master` から `main` に変更
- `toml::basic_value` の `template` 引数を変更
- フォーマット情報を陽に格納するため`toml::string` を廃止
- フォーマット情報を使用するため`toml::format`の引数を変更
- TOMLバージョン情報を格納する`toml::spec`を追加するため`toml::parse`のデフォルト引数を変更
- `toml::source_location` のインターフェースを複数行を前提とした形に変更
- `toml::format_error` の引数を変更
- `toml::format_underline` の名称を `toml::format_location` に変更
- `toml::color` の制御方法を `toml::color::enable()`に統一

### 破壊的でない変更

- `toml::parse_str`の追加
- `toml::try_parse`の追加
- バイト列のパースをサポート
- `toml::value` にフォーマット情報を追加
- コメントをデフォルトで保存するよう変更
- `single_include/toml.hpp`の追加
- コンパイル済みライブラリとしての使用を可能に

## Contributors

このライブラリに新機能を追加、あるいはバグを修正してくださったコントリビュータの方々に感謝します。

- Guillaume Fraux (@Luthaf)
  - Windows support and CI on Appvayor
  - Intel Compiler support
- Quentin Khan (@xaxousis)
  - Found & Fixed a bug around ODR
  - Improved error messages for invalid keys to show the location where the parser fails
- Petr Beneš (@wbenny)
  - Fixed warnings on MSVC
- Ivan Shynkarenka (@chronoxor)
  - Fixed Visual Studio 2019 warnings
  - Fix compilation error in `<filesystem>` with MinGW
- Khoi Dinh Trinh (@khoitd1997)
  - Fixed warnings while type conversion
- @KerstinKeller
  - Added installation script to CMake
- J.C. Moyer (@jcmoyer)
  - Fixed an example code in the documentation
- Jt Freeman (@blockparty-sh)
  - Fixed feature test macro around `localtime_s`
  - Suppress warnings in Debug mode
- OGAWA Kenichi (@kenichiice)
  - Suppress warnings on intel compiler
  - Fix include path in README
- Jordan Williams (@jwillikers)
  - Fixed clang range-loop-analysis warnings
  - Fixed feature test macro to suppress -Wundef
  - Use cache variables in CMakeLists.txt
  - Automate test set fetching, update and refactor CMakeLists.txt
- Scott McCaskill
  - Parse 9 digits (nanoseconds) of fractional seconds in a `local_time`
- Shu Wang (@halfelf)
  - fix "Finding a value in an array" example in README
- @maass-tv and @SeverinLeonhardt
  - Fix MSVC warning C4866
- Mohammed Alyousef (@MoAlyousef)
  - Made testing optional in CMake
- Alex Merry (@amerry)
  - Add missing include files
- sneakypete81 (@sneakypete81)
  - Fix typo in error message
- Oliver Kahrmann (@founderio)
  - Fix missing filename in error message if parsed file is empty
- Karl Nilsson (@karl-nilsson)
  - Fix many spelling errors
- ohdarling88 (@ohdarling)
  - Fix a bug in a constructor of serializer
- estshorter (@estshorter)
  - Fix MSVC warning C26478
- Philip Top (@phlptp)
  - Improve checking standard library feature availability check
- Louis Marascio (@marascio)
  - Fix free-nonheap-object warning
- Axel Huebl (@ax3l)
  - Make installation optional if the library is embedded
- Ken Matsui (@ken-matsui)
  - Support user-defined error message prefix
  - Support dynamic color mode
- Giel van Schijndel (@muggenhor)
  - Remove needless copy in `parse` function
- Lukáš Hrázký (@lukash)
  - Add a `parse(FILE *)` interface and improve file-related error messages
- spiderman idog (@spiderman-idog)
  - Fix typo in README
- Jajauma's GitHub (@Jajauma)
  - Avoid possible lexer truncation warnings
- Moritz Klammler (@ctcmkl)
  - Many patches in (#200) including:
  - Improve CMake scripts, build process, and test file handling
  - Detect error when `discard_comments` is accessed
  - And more.
- Chris White (@cxw42)
  - Fix address-sanitizer error when parsing literal strings having invalid UTF-8 characters
  - Fix function name in error messages
- offa (@offa)
  - Update checkout action to v3
  - Update Required CMake version
  - Cleanup old CI settings
- Sergey Vidyuk (@VestniK)
  - Fix for case when vector iterator is raw pointer
- Kfir Gollan (@kfirgollan)
  - Add installation example with checkinstall and cmake
- Martin Tournoij (@arp242)
  - Escape control characters in keys
- @DavidKorczynski
  - Add fuzzing test based on ClusterFuzzLite
- Esonhugh Skyworship (@Esonhugh)
  - Fix function signature of `strerror_r` on macos

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2017-2024 Toru Niina

All rights reserved.

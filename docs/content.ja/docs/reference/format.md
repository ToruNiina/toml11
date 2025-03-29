+++
title = "format.hpp"
type  = "docs"
+++

# format.hpp

`toml::value`のフォーマット情報を持つ構造体と列挙型を定義します。

# `indent_char`

インデント情報を表す列挙体です。

```cpp
enum class indent_char : std::uint8_t
{
    space, // use space
    tab,   // use tab
    none   // no indent
};
std::ostream& operator<<(std::ostream& os, const indent_char& c);
std::string to_string(const indent_char);
```

`none`を選んだ場合、super tableでの値によらず、インデントは使用されません。

シリアライズ対象の値のなかに`space`と`tab`を指定する値が同時に存在していた場合、その動作は未規定で、指定していない方の文字が出現する可能性があります。

# `boolean_format_info`

`boolean`のフォーマット情報です。

```cpp
struct boolean_format_info {};

bool operator==(const boolean_format_info&, const boolean_format_info&) noexcept;
bool operator!=(const boolean_format_info&, const boolean_format_info&) noexcept;
```

`boolean`のフォーマット方法は一通りしかないため、設定できる値を持ちません。

# `integer_format`

```cpp
enum class integer_format : std::uint8_t
{
    dec = 0,
    bin = 1,
    oct = 2,
    hex = 3,
};
std::ostream& operator<<(std::ostream& os, const integer_format f);
std::string to_string(const integer_format);
```

`integer`の基数を指定します。

# `integer_format_info`

```cpp
struct integer_format_info
{
    integer_format fmt    = integer_format::dec;
    bool        uppercase = true; // use uppercase letters
    std::size_t width     = 0;       // minimal width (may exceed)
    std::size_t spacer    = 0;       // position of `_` (if 0, no spacer)
    std::string suffix    = "";      // _suffix (library extension)
};

bool operator==(const integer_format_info&, const integer_format_info&) noexcept;
bool operator!=(const integer_format_info&, const integer_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value v(0xDEADBEEF);
    std::cout << v << std::endl; // 3735928559

    v.as_integer_fmt().uppercase = true;
    v.as_integer_fmt().fmt = toml::integer_format::hex;
    std::cout << v << std::endl; // 0xDEADBEEF

    v.as_integer_fmt().spacer = 4;
    std::cout << v << std::endl; // 0xDEAD_BEEF

    v.as_integer_fmt().spacer = 8;
    v.as_integer_fmt().width = 16;
    std::cout << v << std::endl; // 0x00000000_DEADBEEF
}
```

## メンバ変数

### `integer_format fmt`

基数を指定します。

### `bool uppercase`

16進数表記で大文字を使用します。

### `std::size_t width`

最小の幅を指定します。値によってはこの幅を超えることがあります。

フォーマットした値がこの幅よりも小さい場合、`integer_format::dec`の場合は効果はありませんが、それ以外の場合は先頭にリーディング`0`が追加されます。

### `std::size_t spacer`

アンダースコア`_`を追加する幅を指定します。

`3`の場合`1_234_567`のように、`4`の場合`0xdead_beef`のようにフォーマットされます。

`0`の場合、アンダースコアは挿入されません。

不規則な幅を指定することはできません。

### `std::string suffix`

toml11拡張の`spec::ext_num_suffix`を`true`にしている場合、その`suffix`がここに保存されます。

参考：[spec.hpp]({{<ref "spec.md#ext_num_suffix">}})

# `floating_format`

```cpp
enum class floating_format : std::uint8_t
{
    defaultfloat = 0,
    fixed        = 1, // does not include exponential part
    scientific   = 2, // always include exponential part
    hex          = 3  // hexfloat extension
};
std::ostream& operator<<(std::ostream& os, const floating_format f);
std::string to_string(const floating_format);
```

`floating`のフォーマット形式を指定します。
それぞれ、`std::defaultfloat`, `std::fixed`, `std::scientific`, `std::hexfloat`に対応します。

`hexfloat`は、`toml::spec::ext_hex_float`が`true`の場合のみ使用可能です。

参考：[spec.hpp]({{<ref "spec.md">}})

# `floating_format_info`

```cpp
struct floating_format_info
{
    floating_format fmt = floating_format::defaultfloat;
    std::size_t prec  = 0;        // precision (if 0, use the default)
    std::string suffix = "";      // 1.0e+2_suffix (library extension)
};

bool operator==(const floating_format_info&, const floating_format_info&) noexcept;
bool operator!=(const floating_format_info&, const floating_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value pi(3.141592653589793);
    std::cout << pi << std::endl; // 3.14159

    pi.as_floating_fmt().fmt = toml::floating_format::fixed;
    std::cout << pi << std::endl; // 3.141593

    pi.as_floating_fmt().prec = 16;
    std::cout << pi << std::endl; // 3.1415926535897931

    toml::value na(6.02214076e+23);
    std::cout << na << std::endl; // 6.022e+23

    na.as_floating_fmt().fmt = toml::floating_format::fixed;
    std::cout << na << std::endl; // 602214075999999987023872.000000

    na.as_floating_fmt().fmt = toml::floating_format::scientific;
    std::cout << na << std::endl; // 6.022141e+23

    na.as_floating_fmt().prec = 16;
    std::cout << na << std::endl; // 6.0221407599999999e+23

    return 0;
}
```

## メンバ変数

### `floating_format fmt`

フォーマット形式を指定します。

### `std::size_t prec`

小数点以下の精度を指定します。

### `std::string suffix`

toml11拡張の`spec::ext_num_suffix`を`true`にしている場合、その`suffix`がここに保存されます。

参考：[spec.hpp]({{<ref "spec.md#ext_num_suffix">}})

# `string_format`

```cpp
enum class string_format : std::uint8_t
{
    basic             = 0,
    literal           = 1,
    multiline_basic   = 2,
    multiline_literal = 3
};
std::ostream& operator<<(std::ostream& os, const string_format f);
std::string to_string(const string_format);
```

文字列のフォーマット形式を指定します。

# `string_format_info`

```cpp
struct string_format_info
{
    string_format fmt = string_format::basic;
    bool start_with_newline    = false;
};

bool operator==(const string_format_info&, const string_format_info&) noexcept;
bool operator!=(const string_format_info&, const string_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value s("foo");

    std::cout << s << std::endl; // "foo"

    s.as_string_fmt().fmt = toml::string_format::literal;
    std::cout << s << std::endl; // 'foo'

    s.as_string_fmt().fmt = toml::string_format::multiline_basic;
    std::cout << s << std::endl; // """foo"""

    s.as_string_fmt().fmt = toml::string_format::multiline_literal;
    std::cout << s << std::endl; // '''foo'''

    toml::value multiline("foo\nbar");

    std::cout << multiline << std::endl; // "foo\nbar"

    multiline.as_string_fmt().fmt = toml::string_format::multiline_basic;
    std::cout << multiline << std::endl; // """foo
                                         // bar"""

    multiline.as_string_fmt().start_with_newline = true;
    std::cout << multiline << std::endl; // """
                                         // foo
                                         // bar"""

    multiline.as_string_fmt().fmt = toml::string_format::multiline_literal;
    std::cout << multiline << std::endl; // '''
                                         // foo
                                         // bar'''

    return 0;
}
```

## メンバ変数

### `string_format fmt`

文字列のフォーマット情報を指定します。

### `bool start_with_newline`

`multiline_basic`または`multiline_literal`の場合、最初の`"""`や`'''`の後に改行を入れるかどうかを指定します。

# `datetime_delimiter_kind`

```cpp
enum class datetime_delimiter_kind : std::uint8_t
{
    upper_T = 0,
    lower_t = 1,
    space   = 2,
};
std::ostream& operator<<(std::ostream& os, const datetime_delimiter_kind d);
std::string to_string(const datetime_delimiter_kind);
```

`datetime`で日付と時刻の間のデリミタにどの文字を使うかを指定します。

`T`, `t`, ` `が使用可能です。

# `offset_datetime_format_info`

```cpp
struct offset_datetime_format_info
{
    datetime_delimiter_kind delimiter = datetime_delimiter_kind::upper_T;
    bool has_seconds = true;
    std::size_t subsecond_precision = 6; // [us]
};

bool operator==(const offset_datetime_format_info&, const offset_datetime_format_info&) noexcept;
bool operator!=(const offset_datetime_format_info&, const offset_datetime_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value v(toml::offset_datetime(
            toml::local_date(2025, toml::month_t::Mar, 29),
            toml::local_time(1, 23, 45, /*ms=*/678, /*us=*/901, /*ns=*/234),
            toml::time_offset(9, 0)
        ));
    std::cout << v << std::endl; // 2025-03-29T01:23:45.678901+09:00

    v.as_offset_datetime_fmt().subsecond_precision = 9;
    std::cout << v << std::endl; // 2025-03-29T01:23:45.678901234+09:00

    v.as_offset_datetime_fmt().has_seconds = false;
    std::cout << v << std::endl; // 2025-03-29T01:23+09:00

    v.as_offset_datetime_fmt().delimiter = toml::datetime_delimiter_kind::space;
    std::cout << v << std::endl; // 2025-03-29 01:23+09:00

    return 0;
}
```

## メンバ変数

### `datetime_delimiter_kind delimiter`

使用するデリミタを指定します。

### `bool has_seconds`

秒数を省略するかどうかを指定します。

### `std::size_t subsecond_precision`

秒以下の精度を何桁出力するかを指定します。

# `local_datetime_format_info`

```cpp
struct local_datetime_format_info
{
    datetime_delimiter_kind delimiter = datetime_delimiter_kind::upper_T;
    bool has_seconds = true;
    std::size_t subsecond_precision = 6; // [us]
};

bool operator==(const local_datetime_format_info&, const local_datetime_format_info&) noexcept;
bool operator!=(const local_datetime_format_info&, const local_datetime_format_info&) noexcept;
```

## メンバ変数

### `datetime_delimiter_kind delimiter`

使用するデリミタを指定します。

### `bool has_seconds`

秒数を省略するかどうかを指定します。

### `std::size_t subsecond_precision`

秒以下の精度を何桁出力するかを指定します。

# `local_date_format_info`

```cpp
struct local_date_format_info
{
    // nothing, for now
};

bool operator==(const local_date_format_info&, const local_date_format_info&) noexcept;
bool operator!=(const local_date_format_info&, const local_date_format_info&) noexcept;
```

`local_datetime`にはフォーマット指定するパラメータはありません。

# `local_time_format_info`

```cpp
struct local_time_format_info
{
    bool has_seconds = true;
    std::size_t subsecond_precision = 6; // [us]
};

bool operator==(const local_time_format_info&, const local_time_format_info&) noexcept;
bool operator!=(const local_time_format_info&, const local_time_format_info&) noexcept;
```

## メンバ変数

### `bool has_seconds`

秒数を省略するかどうかを指定します。

### `std::size_t subsecond_precision`

秒以下の精度を何桁出力するかを指定します。

# `array_format`

```cpp
enum class array_format : std::uint8_t
{
    default_format  = 0,
    oneline         = 1,
    multiline       = 2,
    array_of_tables = 3 // [[format.in.this.way]]
};

std::ostream& operator<<(std::ostream& os, const array_format f);
std::string to_string(const array_format);
```

- `default_format`
  - 適したフォーマットを自動的に選択します。ある程度長い配列は複数行になります。
- `oneline`
  - 全ての要素を一行でフォーマットします。
- `multiline`
  - 一行ごとに一つの要素を出力します。
- `array_of_tables`
  - `[[array.of.tables]]`の形式でフォーマットします。`table`以外の要素を含むことはできません。

# `array_format_info`

```cpp
struct array_format_info
{
    array_format fmt            = array_format::default_format;
    indent_char  indent_type    = indent_char::space;
    std::int32_t body_indent    = 4; // indent in case of multiline
    std::int32_t closing_indent = 0; // indent of `]`
};

bool operator==(const array_format_info&, const array_format_info&) noexcept;
bool operator!=(const array_format_info&, const array_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value a(toml::array{ 3, 1, 4 });

    a.as_array_fmt().fmt = toml::array_format::oneline;
    std::cout << "a = " << a << std::endl; // a = [3, 1, 4]

    a.as_array_fmt().fmt = toml::array_format::multiline;
    std::cout << "a = " << a << std::endl;
    // a = [
    //     3,
    //     1,
    //     4,
    // ]

    a.as_array_fmt().body_indent = 4;
    a.as_array_fmt().closing_indent = 2;
    std::cout << "a = " << a << std::endl;
    // a = [
    //     3,
    //     1,
    //     4,
    //   ]

    return 0;
}
```

## メンバ変数

### `array_format fmt`

フォーマット形式を指定します。

### `indent_char indent_type`

インデントに使用する文字の種類を選択します。

### `std::int32_t body_indent`

`array_format::multiline`の場合、要素の前に出力するインデントの文字数を指定します。

### `std::int32_t closing_indent`

`array_format::multiline`の場合、閉じ括弧`]`の前に出力するインデントの文字数を指定します。

# `table_format`

```cpp
enum class table_format : std::uint8_t
{
    multiline         = 0, // [foo] \n bar = "baz"
    oneline           = 1, // foo = {bar = "baz"}
    dotted            = 2, // foo.bar = "baz"
    multiline_oneline = 3, // foo = { \n bar = "baz" \n }
    implicit          = 4  // [x] defined by [x.y.z]. skip in serializer.
};

std::ostream& operator<<(std::ostream& os, const table_format f);
std::string to_string(const table_format);
```

- `multiline`
  - 複数行の通常のテーブルとしてフォーマットします。
- `oneline`
  - インラインテーブルとしてフォーマットします。
- `dotted`
  - `a.b.c = "d"`の形式でフォーマットします。
- `multiline_oneline`
  - 改行を含むインラインテーブルとしてフォーマットします。TOML v1.1.0以降で使用可能です。
  - 参考：[spec.hpp]({{<ref "spec.md">}})
- `implicit`
  - `[x.y.z.w]`だけが定義されている際の`[x]`, `[x.y]`, `[x.y.z]`のように、暗黙定義としてスキップします。

{{< hint warning >}}

TOMLの文法上、`dotted`はサブテーブルを持つことができます。

```toml
[fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]        # INVALID
# [fruit.apple.taste]  # INVALID
[fruit.apple.texture]  # you can add sub-tables
smooth = true
```

toml11は現時点ではこのフォーマットに対応していません。
`dotted`テーブルの下にあるテーブルは全て`dotted`になり、テーブルは強制的にインラインテーブルになります。

{{< /hint >}}


# `table_format_info`

```cpp
struct table_format_info
{
    table_format fmt = table_format::multiline;
    indent_char  indent_type    = indent_char::space;
    std::int32_t body_indent    = 0; // indent of values
    std::int32_t name_indent    = 0; // indent of [table]
    std::int32_t closing_indent = 0; // in case of {inline-table}
};

bool operator==(const table_format_info&, const table_format_info&) noexcept;
bool operator!=(const table_format_info&, const table_format_info&) noexcept;
```

## 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    toml::value t(toml::table{
        {"a",   42},
        {"pi",  3.14},
        {"foo", "bar"},
        {
            "table", toml::table{
                {"a",   42},
                {"pi",  3.14},
                {"foo", "bar"}
            }
        },
    });

    std::cout << t << std::endl;
    // pi = 3.14
    // foo = "bar"
    // a = 42
    //
    // [table]
    // pi = 3.14
    // foo = "bar"
    // a = 42

    // root table cannot be dotted.
    // t.as_table_fmt().fmt = toml::table_format::dotted;
    t.at("table").as_table_fmt().fmt = toml::table_format::dotted;
    std::cout << t << std::endl;
    // table.pi = 3.14
    // table.foo = "bar"
    // table.a = 42
    // pi = 3.14
    // foo = "bar"
    // a = 42

    t.as_table_fmt().fmt = toml::table_format::oneline;
    std::cout << t << std::endl;
    // {table = {pi = 3.14, foo = "bar", a = 42}, pi = 3.14, foo = "bar", a = 42}

    return 0;
}
```

## メンバ変数

### `table_format fmt`

フォーマット方法を指定します。

### `indent_char indent_type`

インデントに使用する文字を指定します。

### `std::int32_t body_indent`

キーの前に出力するインデントの幅を指定します。

スーパーテーブルのインデント幅は加算されません。

### `std::int32_t name_indent`

`[table]`形式のキーのインデントを指定します。

スーパーテーブルのインデント幅は加算されません。

### `std::int32_t closing_indent`

`multiline_oneline`の場合に、閉じ括弧`}`の前のインデント幅を指定します。


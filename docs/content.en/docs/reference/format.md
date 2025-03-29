+++
title = "format.hpp"
type  = "docs"
+++

# format.hpp

Defines structures and enumerations related to formatting information for `toml::value`.

# `indent_char`

An enumeration representing the indentation character choice.

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

Choosing `none` means no indentation is used, regardless of the value in super tables.

If both `space` and `tab` are specified within the serializable value, the behavior is unspecified; typically, the unspecified indentation character appears.

# `boolean_format_info`

Formatting information for `boolean`.

```cpp
struct boolean_format_info {};

bool operator==(const boolean_format_info&, const boolean_format_info&) noexcept;
bool operator!=(const boolean_format_info&, const boolean_format_info&) noexcept;
```

There is only one way to format `boolean`, so no configurable values are provided.

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

Specifies the radix of an `integer`.

# `integer_format_info`

```cpp
struct integer_format_info
{
    integer_format fmt    = integer_format::dec;
    bool        uppercase = true; // use uppercase letters
    std::size_t width     = 0;  // minimal width (may exceed)
    std::size_t spacer    = 0;  // position of `_` (if 0, no spacer)
    std::string suffix    = ""; // _suffix (library extension)
};

bool operator==(const integer_format_info&, const integer_format_info&) noexcept;
bool operator!=(const integer_format_info&, const integer_format_info&) noexcept;
```

## example

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

## Member Variables

### `integer_format fmt`

Specifies the radix.

### `bool uppercase`

Uses uppercase letters when formatted as a hexadecimal integer.

### `std::size_t width`

Specifies the minimum width. The formatted value may exceed this width.

For values smaller than this width, if `integer_format::dec`, no effect. Otherwise, leading zeros are added.

### `std::size_t spacer`

Specifies the width at which underscores `_` are inserted.

- If `3`, formatted as `1_234_567`.
- If `4`, formatted as `0xdead_beef`.
- If `0`, no underscores are inserted.

Irregular widths are not allowed.

### `std::string suffix`

Stores the suffix when `spec::ext_num_suffix` of toml11 extension is `true`.

cf. [spec.hpp]({{< ref "spec.md" >}})

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

Specifies the formatting style for `floating` numbers.
Corresponds to `std::defaultfloat`, `std::fixed`, `std::scientific`, `std::hexfloat`.

`hexfloat` is available only if `toml::spec::ext_hex_float` is `true`.

cf. [spec.hpp]({{< ref "spec.md#ext_num_suffix" >}})

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

## example

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

## Member Variables

### `floating_format fmt`

Specifies the formatting style.

### `std::size_t prec`

Specifies the precision after the decimal point.

### `std::string suffix`

Stores the suffix when `spec::ext_num_suffix` of toml11 extension is `true`.

cf. [spec.hpp]({{< ref "spec.md#ext_num_suffix" >}})

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

Specifies the formatting style for strings.

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

## example

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

## Member Variables

### `string_format fmt`

Specifies the formatting information for strings.

### `bool start_with_newline`

For `multiline_basic` or `multiline_literal`, specifies whether to include a newline after the initial `"""` or `'''`.

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

Specifies the delimiter used between date and time in `datetime`.

Possible options include `T`, `t`, and a space ` `.

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

## example

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

## Member Variables

### `datetime_delimiter_kind delimiter`

Specifies the delimiter to use.

### `bool has_seconds`

Specifies whether to omit seconds.

### `std::size_t subsecond_precision`

Specifies how many digits to output for subseconds.

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

## Member Variables

### `datetime_delimiter_kind delimiter`

Specifies the delimiter to use.

### `bool has_seconds`

Specifies whether to omit seconds.

### `std::size_t subsecond_precision`

Specifies how many digits to output for subseconds.

# `local_date_format_info`

```cpp
struct local_date_format_info
{
    // nothing, for now
};

bool operator==(const local_date_format_info&, const local_date_format_info&) noexcept;
bool operator!=(const local_date_format_info&, const local_date_format_info&) noexcept;
```

No formatting parameters are specified for `local_date`.

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

## Member Variables

### `bool has_seconds`

Specifies whether to omit seconds.

### `std::size_t subsecond_precision`

Specifies how many digits to output for subseconds.

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
  - Automatically selects the appropriate format. Longer arrays may span multiple lines.
- `oneline`
  - Formats all elements in a single line.
- `multiline`
  - Outputs each element on its own line.
- `array_of_tables`
  - Formats in the `[[array.of.tables]]` style. Cannot contain elements other than `table`.

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

## example

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

## Member Variables

### `array_format fmt`

Specifies the format style.

### `indent_char indent_type`

Selects the type of character used for indentation.

### `std::int32_t body_indent`

Specifies the number of characters to indent before each element in `array_format::multiline`.

### `std::int32_t closing_indent`

Specifies the number of characters to indent before the closing bracket `]` in `array_format::multiline`.

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
  - Formats as a multiline normal table.
- `oneline`
  - Formats as an inline table.
- `dotted`
  - Formats in the form of `a.b.c = "d"`.
- `multiline_oneline`
  - Formats as a multiline inline table with line breaks. Available from TOML v1.1.0 onwards.
  - cf. [spec.hpp]({{< ref "spec.md" >}})
- `implicit`
  - Skips implicit definitions like `[x.y.z.w]`, leaving `[x]`, `[x.y]`, `[x.y.z]` as implicit.

{{< hint warning >}}

According to TOML syntax, `dotted` table can have sub-tables:

```toml
[fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]        # INVALID
# [fruit.apple.taste]  # INVALID
[fruit.apple.texture]  # you can add sub-tables
smooth = true
```

toml11 currently does not support this format.
Sub-tables under a `dotted` table would all be `dotted`, and tables are forced into inline table format.

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

## example

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


## Member Variables

### `table_format fmt`

Specifies the formatting method.

### `indent_char indent_type`

Specifies the character used for indentation.

### `std::int32_t body_indent`

Specifies the width of indentation before keys.

The indentation width of the super table is not added.

### `std::int32_t name_indent`

Specifies the indentation of keys in `[table]` format.

The indentation width of the super table is not added.

### `std::int32_t closing_indent`

Specifies the indentation width before the closing brace `}` in the case of `multiline_oneline`.

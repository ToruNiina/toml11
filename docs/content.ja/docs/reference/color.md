+++
title = "color.hpp"
type  = "docs"
+++

# color.hpp

`color.hpp`では、エラーメッセージの色付けに関する関数が定義されます。

色はANSIエスケープシーケンスによって指定されます。
ANSIエスケープシーケンスをサポートしていないターミナルやその他の出力先では、読みにくくなる可能性があります。

## マクロ

```cpp
TOML11_COLORIZE_ERROR_MESSAGE
```

コンパイル時にこのマクロが定義されていた場合（`-DTOML11_COLORIZE_ERROR_MESASGE`）、
デフォルトでエラーメッセージに色が付きます。

定義されていなかった場合、デフォルトでは色は付きません。以下の `toml::color::enable()` を
使用して指定する必要があります。


## 関数

### `enable()`

```cpp
namespace toml {
namespace color {
void enable();
} // color
} // toml
```

ANSIエスケープシーケンスによる色付けを行うよう設定します。

#### 例

```cpp
#include <toml.hpp>

int main()
{
    toml::color::enable(); // この後の全てのエラーがカラーになります。
    const auto input = toml::parse("input.toml");
    return 0;
}
```

### `disable()`

```cpp
namespace toml {
namespace color {
void disable();
} // color
} // toml
```

ANSIエスケープシーケンスによる色付けを行わないよう設定します。

#### 例

```cpp
#include <toml.hpp>

int main()
{
    toml::color::enable(); // この後の全てのエラーがカラーになります。
    const auto input = toml::parse("input.toml");
    return 0;
}
```

### `should_color()`

```cpp
namespace toml {
namespace color {
bool should_color();
} // color
} // toml
```

色付けを行う設定になっている場合`true`が、そうでない場合`false`が返されます。

#### 例

```cpp
#include <toml.hpp>
#include <iomanip>
#include <iostream>

int main()
{
    std::cout << "colorized? : " << std::boolalpha << toml::color::should_color() << std::endl;
    return 0;
}
```

## マニピュレータ

```cpp
namespace toml {
namespace color {
std::ostream& reset  (std::ostream&);
std::ostream& bold   (std::ostream&);
std::ostream& grey   (std::ostream&);
std::ostream& gray   (std::ostream&);
std::ostream& red    (std::ostream&);
std::ostream& green  (std::ostream&);
std::ostream& yellow (std::ostream&);
std::ostream& blue   (std::ostream&);
std::ostream& magenta(std::ostream&);
std::ostream& cyan   (std::ostream&);
std::ostream& white  (std::ostream&);
} // color
} // toml
```

ANSIエスケープシーケンスによって、`fg`を色付けします。

#### 例

```cpp
#include <toml.hpp>
#include <iostream>

int main()
{
    std::cout << toml::color::red << "red!" << std::endl;
    return 0;
}
```

# 関連項目

- [error_info.hpp]({{<ref "error_info.md">}})


### encoding user's data

You can encode your data to toml format.

```cpp
const toml::value integer(1);
const toml::value array{3.1, 3.14, 3.141, 3.1415};
const toml::value table{{"answer", 42}, {"pi", 3.14}, {"string", "foobar"}};

std::cout << toml::format("integer", integer) << std::endl;
std::cout << toml::format("array",   array)   << std::endl;
std::cout << toml::format("table",   table)   << std::endl;
```

this program will output as below.

```toml
integer = 1
array = [3.1, 3.14, 3.141, 3.1415]
[table]
answer = 42
pi = 3.14
string = "foobar"
```

Without key name, you can make string formatted as toml.

```cpp
const std::string integer_ = toml::format(integer); // "1"
const std::string array_   = toml::format(array);   // "[3.1, 3.14, 3.141, 3.1415]"
const std::string table_   = toml::format(table);   // "answer = 42\npi=3.14\nstring=foobar"
```

### inlinize

You can make `toml::Table` inline.

```cpp
const toml::value table{{"answer", 42}, {"pi", 3.14}, {"string", "foobar"}};
// if the inline-table format length is less than 80, the table will be inlined
std::cout << toml::format("table", table, toml::make_inline(80)) << std::endl;
// In any case, the table will be inlined.
std::cout << toml::format("table", table, toml::forceinline) << std::endl;
```

```toml
table = {answer = 42, pi = 3.14, string = "foobar"}
```

And there are some stream manipulators for toml format.

```cpp
const toml::value table{{"answer", 42}, {"pi", 3.14}, {"string", "foobar"}};
// if the inline-table format length is less than 80, the table will be inlined
std::cout << toml::make_inline(80) << table << std::endl;
// In any case, the table will be inlined.
std::cout << toml::forceinline << table << std::endl;
```

#include "toml.hpp"

int main()
{
    bool b = true;
    int i=42;
    double f = 3.14;
    toml::value v1(i);
    toml::value v2(b);
    toml::value v3(f);
    toml::value v4("hoge");

    std::cout << "v1: " << v1.type() << " = " << v1.cast<toml::value_t::Integer>() << std::endl;
    std::cout << "v2: " << v2.type() << " = " << v2.cast<toml::value_t::Boolean>() << std::endl;
    std::cout << "v3: " << v3.type() << " = " << v3.cast<toml::value_t::Float>()   << std::endl;
    std::cout << "v4: " << v4.type() << " = " << v4.cast<toml::value_t::String>()  << std::endl;
    v1.reset(f);
    std::cout << "v1: " << v1.type() << " = " << v1.cast<toml::value_t::Float>() << std::endl;

    toml::Table tab;
    tab["v1"] = v1;
    tab["v2"] = v2;
    tab["v3"] = v3;
    tab["v4"] = v4;

    toml::Array arr;
    arr.emplace_back(3);
    arr.emplace_back(1);
    arr.emplace_back(4);
    arr.emplace_back(1);
    arr.emplace_back(5);

    toml::value v5(tab);
    std::cout << "v5: " << v5.type() << std::endl;
    toml::value v6(arr);
    std::cout << "v6: " << v6.type() << std::endl;

    return 0;
}

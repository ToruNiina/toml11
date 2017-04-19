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
    v1.cast<toml::value_t::Integer>() += 1;
    std::cout << "v1: " << v1.type() << " = " << v1.cast<toml::value_t::Integer>() << std::endl;
    v4.cast<toml::value_t::String>() += " piyo";
    std::cout << "v4: " << v4.type() << " = " << v4.cast<toml::value_t::String>()  << std::endl;

    v1 = f;
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
    std::cout << "v5: " << v5.type() << " = {" << std::endl;
    const auto& v5t = v5.cast<toml::value_t::Table>();
    std::cout << "v5t[v1] = " << v5t.at("v1").cast<toml::value_t::Float>() << std::endl;
    std::cout << "v5t[v2] = " << v5t.at("v2").cast<toml::value_t::Boolean>() << std::endl;
    std::cout << "v5t[v3] = " << v5t.at("v3").cast<toml::value_t::Float>() << std::endl;
    std::cout << "v5t[v4] = " << v5t.at("v4").cast<toml::value_t::String>() << std::endl;
    std::cout << "}" << std::endl;

    toml::value v6(arr);
    std::cout << "v6: " << v6.type() << " = ";
    for(auto&& item : v6.cast<toml::value_t::Array>())
    {
        std::cout << item.cast<toml::value_t::Integer>() << ", ";
    }
    std::cout << std::endl;

    std::string piyo("piyo");
    std::swap(v4.cast<toml::value_t::String>(), piyo);
    std::cout << "v4: " << v4.type() << " = " << v4.cast<toml::value_t::String>()  << std::endl;

    toml::value v7(v4);
    std::cout << "v7: " << v7.type() << " = " << v7.cast<toml::value_t::String>() << std::endl;

    toml::value v8(std::move(v4));
    std::cout << "v8: " << v8.type() << " = " << v8.cast<toml::value_t::String>() << std::endl;

    std::cout << toml::is_castable<int,              toml::value_t::Integer>::value << std::endl;
    std::cout << toml::is_castable<std::vector<int>, toml::value_t::Array>::value << std::endl;
    std::cout << toml::is_castable<int[5],           toml::value_t::Array>::value << std::endl;
    std::cout << toml::is_castable<std::map<std::string, int>, toml::value_t::Table>::value << std::endl;
    std::cout << toml::is_castable<int, toml::value_t::Table>::value << std::endl;
    std::cout << toml::is_castable<std::map<int, std::string>, toml::value_t::Table>::value << std::endl;

    return 0;
}

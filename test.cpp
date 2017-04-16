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

    std::cout << "v1: " << v1.t() << " = " << v1.cast<toml::type::Integer>() << std::endl;
    std::cout << "v2: " << v2.t() << " = " << v2.cast<toml::type::Boolean>() << std::endl;
    std::cout << "v3: " << v3.t() << " = " << v3.cast<toml::type::Float>()   << std::endl;
    std::cout << "v4: " << v4.t() << " = " << v4.cast<toml::type::String>()  << std::endl;
    v1.reset(f);
    std::cout << "v1: " << v1.t() << " = " << v1.cast<toml::type::Float>() << std::endl;


    return 0;
}

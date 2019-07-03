#include "toml.hpp"
#include <iostream>
#include <iomanip>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cerr << "usage: ./check [filename]" << std::endl;
        return 1;
    }

    const std::string filename(argv[1]);

    {
        const auto data = toml::parse(filename);
        {
            std::ofstream ofs("tmp.toml");
            ofs << std::setprecision(16) << std::setw(80) << data;
        }
        const auto serialized = toml::parse("tmp.toml");

        if(data != serialized)
        {
            std::cerr << "============================================================\n";
            std::cerr << "result (w/o comment) different: " << filename << std::endl;
            std::cerr << "------------------------------------------------------------\n";
            std::cerr << "# serialized\n";
            std::cerr << serialized;
            std::cerr << "------------------------------------------------------------\n";
            std::cerr << "# data\n";
            std::cerr << data;
            return 1;
        }
    }
    {
        const auto data = toml::parse<toml::preserve_comments>(filename);
        {
            std::ofstream ofs("tmp.toml");
            ofs << std::setprecision(16) << std::setw(80) << data;
        }
        const auto serialized = toml::parse<toml::preserve_comments>("tmp.toml");
        if(data != serialized)
        {
            std::cerr << "============================================================\n";
            std::cerr << "result (w/  comment) different: " << filename << std::endl;
            std::cerr << "------------------------------------------------------------\n";
            std::cerr << "# serialized\n";
            std::cerr << serialized;
            std::cerr << "------------------------------------------------------------\n";
            std::cerr << "# data\n";
            std::cerr << data;
            return 1;
        }
    }
    return 0;
}

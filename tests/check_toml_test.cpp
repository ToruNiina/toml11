#include "toml.hpp"
#include <iostream>
#include <iomanip>

struct json_serializer
{
    void operator()(toml::boolean v)
    {
        std::cout << "{\"type\":\"bool\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(toml::integer v)
    {
        std::cout << "{\"type\":\"integer\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(toml::floating v)
    {
        std::cout << "{\"type\":\"float\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(const toml::string& v)
    {
        // since toml11 automatically convert string to multiline string that is
        // valid only in TOML, we need to format the string to make it valid in
        // JSON.
        std::cout << "{\"type\":\"string\",\"value\":\""
                  << this->escape_string(v.str) << "\"}";
        return ;
    }
    void operator()(const toml::local_time& v)
    {
        std::cout << "{\"type\":\"local_time\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(const toml::local_date& v)
    {
        std::cout << "{\"type\":\"local_date\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(const toml::local_datetime& v)
    {
        std::cout << "{\"type\":\"local_datetime\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(const toml::offset_datetime& v)
    {
        std::cout << "{\"type\":\"datetime\",\"value\":\"" << toml::value(v) << "\"}";
        return ;
    }
    void operator()(const toml::array& v)
    {
        if(!v.empty() && v.front().is_table())
        {
            std::cout << '[';
            bool is_first = true;
            for(const auto& elem : v)
            {
                if(!is_first) {std::cout << ", ";}
                is_first = false;
                toml::visit(*this, elem);
            }
            std::cout << ']';
        }
        else
        {
            std::cout << "{\"type\":\"array\",\"value\":[";
            bool is_first = true;
            for(const auto& elem : v)
            {
                if(!is_first) {std::cout << ", ";}
                is_first = false;
                toml::visit(*this, elem);
            }
            std::cout << "]}";
        }
        return ;
    }
    void operator()(const toml::table& v)
    {
        std::cout << '{';
        bool is_first = true;
        for(const auto& elem : v)
        {
            if(!is_first) {std::cout << ", ";}
            is_first = false;
            std::cout << this->format_key(elem.first) << ':';
            toml::visit(*this, elem.second);
        }
        std::cout << '}';
        return ;
    }

    std::string escape_string(const std::string& s) const
    {
        std::string retval;
        for(const char c : s)
        {
            switch(c)
            {
                case '\\': {retval += "\\\\"; break;}
                case '\"': {retval += "\\\""; break;}
                case '\b': {retval += "\\b";  break;}
                case '\t': {retval += "\\t";  break;}
                case '\f': {retval += "\\f";  break;}
                case '\n': {retval += "\\n";  break;}
                case '\r': {retval += "\\r";  break;}
                default  : {retval += c;      break;}
            }
        }
        return retval;
    }

    std::string format_key(const std::string& s) const
    {
        const auto quote("\"");
        return quote + escape_string(s) + quote;
    }
};

int main()
{
    try
    {
        std::vector<char> buf;
        std::cin.peek();
        while(!std::cin.eof())
        {
            buf.push_back(std::cin.get());
            std::cin.peek();
        }
        std::string bufstr(buf.begin(), buf.end());

        std::istringstream ss(bufstr);

        const auto data = toml::parse(ss);
        std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
        toml::visit(json_serializer(), data);
        return 0;
    }
    catch(const toml::syntax_error& err)
    {
        std::cout << "what(): " << err.what() << std::endl;
        return 1;
    }
}

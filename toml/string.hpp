//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_STRING_H
#define TOML11_STRING_H
#include <string>
#include <cstdint>

namespace toml
{

enum class string_t : std::uint8_t
{
    basic   = 0,
    literal = 1,
};

struct string
{
    string()  = default;
    ~string() = default;
    string(const string& s) = default;
    string(string&& s)      = default;
    string& operator=(const string& s) = default;
    string& operator=(string&& s)      = default;

    string(const std::string& s): kind(string_t::basic), str(s){}
    string(const std::string& s, string_t k):   kind(k), str(s){}
    string(const char* s):        kind(string_t::basic), str(s){}
    string(const char* s,        string_t k):   kind(k), str(s){}

    string(std::string&& s): kind(string_t::basic), str(std::move(s)){}
    string(std::string&& s, string_t k):   kind(k), str(std::move(s)){}

    string& operator=(const std::string& s)
    {kind = string_t::basic; str = s; return *this;}
    string& operator=(std::string&& s)
    {kind = string_t::basic; str = std::move(s); return *this;}

    operator std::string&       () &      noexcept {return str;}
    operator std::string const& () const& noexcept {return str;}
    operator std::string&&      () &&     noexcept {return std::move(str);}

    string_t    kind;
    std::string str;
};

inline bool operator==(const string& lhs, const string& rhs)
{
    return lhs.kind == rhs.kind && lhs.str == rhs.str;
}
inline bool operator!=(const string& lhs, const string& rhs)
{
    return !(lhs == rhs);
}
inline bool operator<(const string& lhs, const string& rhs)
{
    return (lhs.kind == rhs.kind) ? (lhs.str < rhs.str) : (lhs.kind < rhs.kind);
}
inline bool operator>(const string& lhs, const string& rhs)
{
    return rhs < lhs;
}
inline bool operator<=(const string& lhs, const string& rhs)
{
    return !(rhs < lhs);
}
inline bool operator>=(const string& lhs, const string& rhs)
{
    return !(lhs < rhs);
}

inline bool
operator==(const string& lhs, const std::string& rhs) {return lhs.str == rhs;}
inline bool
operator!=(const string& lhs, const std::string& rhs) {return lhs.str != rhs;}
inline bool
operator< (const string& lhs, const std::string& rhs) {return lhs.str <  rhs;}
inline bool
operator> (const string& lhs, const std::string& rhs) {return lhs.str >  rhs;}
inline bool
operator<=(const string& lhs, const std::string& rhs) {return lhs.str <= rhs;}
inline bool
operator>=(const string& lhs, const std::string& rhs) {return lhs.str >= rhs;}

inline bool
operator==(const std::string& lhs, const string& rhs) {return lhs == rhs.str;}
inline bool
operator!=(const std::string& lhs, const string& rhs) {return lhs != rhs.str;}
inline bool
operator< (const std::string& lhs, const string& rhs) {return lhs <  rhs.str;}
inline bool
operator> (const std::string& lhs, const string& rhs) {return lhs >  rhs.str;}
inline bool
operator<=(const std::string& lhs, const string& rhs) {return lhs <= rhs.str;}
inline bool
operator>=(const std::string& lhs, const string& rhs) {return lhs >= rhs.str;}

inline bool
operator==(const string& lhs, const char* rhs) {return lhs.str == std::string(rhs);}
inline bool
operator!=(const string& lhs, const char* rhs) {return lhs.str != std::string(rhs);}
inline bool
operator< (const string& lhs, const char* rhs) {return lhs.str <  std::string(rhs);}
inline bool
operator> (const string& lhs, const char* rhs) {return lhs.str >  std::string(rhs);}
inline bool
operator<=(const string& lhs, const char* rhs) {return lhs.str <= std::string(rhs);}
inline bool
operator>=(const string& lhs, const char* rhs) {return lhs.str >= std::string(rhs);}

inline bool
operator==(const char* lhs, const string& rhs) {return std::string(lhs) == rhs.str;}
inline bool
operator!=(const char* lhs, const string& rhs) {return std::string(lhs) != rhs.str;}
inline bool
operator< (const char* lhs, const string& rhs) {return std::string(lhs) <  rhs.str;}
inline bool
operator> (const char* lhs, const string& rhs) {return std::string(lhs) >  rhs.str;}
inline bool
operator<=(const char* lhs, const string& rhs) {return std::string(lhs) <= rhs.str;}
inline bool
operator>=(const char* lhs, const string& rhs) {return std::string(lhs) >= rhs.str;}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const string& str)
{
    os << str.str;
    return os;
}

} // toml
#endif// TOML11_STRING_H

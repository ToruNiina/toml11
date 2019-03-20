//     Copyright Toru Niina 2019.
// Distributed under the MIT License.
#ifndef TOML11_SERIALIZER_HPP
#define TOML11_SERIALIZER_HPP
#include "value.hpp"
#include "lexer.hpp"
#include <limits>
#include <cstdio>

namespace toml
{

struct serializer
{
    serializer(const std::size_t w              = 80,
               const int         float_prec     = std::numeric_limits<toml::floating>::max_digits10,
               const bool        can_be_inlined = false,
               std::vector<toml::key> ks        = {})
        : can_be_inlined_(can_be_inlined), float_prec_(float_prec), width_(w),
          keys_(std::move(ks))
    {}
    ~serializer() = default;

    std::string operator()(const toml::boolean& b) const
    {
        return b ? "true" : "false";
    }
    std::string operator()(const integer i) const
    {
        return std::to_string(i);
    }
    std::string operator()(const toml::floating f) const
    {
        const auto fmt = "%.*g";
        const auto bsz = std::snprintf(nullptr, 0, fmt, this->float_prec_, f);
        std::vector<char> buf(bsz + 1, '\0'); // +1 for null character(\0)
        std::snprintf(buf.data(), buf.size(), fmt, this->float_prec_, f);

        std::string token(buf.begin(), std::prev(buf.end()));
        if(token.back() == '.') // 1. => 1.0
        {
            token += '0';
        }
        const auto e = std::find_if(token.cbegin(), token.cend(),
            [](const char c) -> bool {
                return c == 'E' || c == 'e';
            });
        if(e == token.cend())
        {
            return token; // there is no exponent part. just return it.
        }

        // zero-prefix in an exponent is not allowed in TOML.
        // remove it if it exists.
        bool        sign_exists = false;
        std::size_t zero_prefix = 0;
        for(auto iter = std::next(e), iend = token.cend(); iter != iend; ++iter)
        {
            if(*iter == '+' || *iter == '-'){sign_exists = true; continue;}
            if(*iter == '0'){zero_prefix += 1;}
            else {break;}
        }
        if(zero_prefix != 0)
        {
            const auto offset = std::distance(token.cbegin(), e) +
                               (sign_exists ? 2 : 1);
            token.erase(offset, zero_prefix);
        }
        return token;
    }
    std::string operator()(const string& s) const
    {
        if(s.kind == string_t::basic)
        {
            if(std::find(s.str.cbegin(), s.str.cend(), '\n') != s.str.cend())
            {
                // if linefeed is contained, make it multiline-string.
                const std::string open("\"\"\"\n");
                const std::string close("\\\n\"\"\"");
                return open + this->escape_ml_basic_string(s.str) + close;
            }

            // no linefeed. try to make it oneline-string.
            std::string oneline = this->escape_basic_string(s.str);
            if(oneline.size() + 2 < width_ || width_ < 2)
            {
                const std::string quote("\"");
                return quote + oneline + quote;
            }

            // the line is too long compared to the specified width.
            // split it into multiple lines.
            std::string token("\"\"\"\n");
            while(!oneline.empty())
            {
                if(oneline.size() < width_)
                {
                    token += oneline;
                    oneline.clear();
                }
                else if(oneline.at(width_-2) == '\\')
                {
                    token += oneline.substr(0, width_-2);
                    token += "\\\n";
                    oneline.erase(0, width_-2);
                }
                else
                {
                    token += oneline.substr(0, width_-1);
                    token += "\\\n";
                    oneline.erase(0, width_-1);
                }
            }
            return token + std::string("\\\n\"\"\"");
        }
        else // the string `s` is literal-string.
        {
            if(std::find(s.str.cbegin(), s.str.cend(), '\n') != s.str.cend() ||
               std::find(s.str.cbegin(), s.str.cend(), '\'') != s.str.cend() )
            {
                const std::string open("'''\n");
                const std::string close("'''");
                return open + s.str + close;
            }
            else
            {
                const std::string quote("'");
                return quote + s.str + quote;
            }
        }
    }

    std::string operator()(const local_date& d) const
    {
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    std::string operator()(const local_time& t) const
    {
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }
    std::string operator()(const local_datetime& dt) const
    {
        std::ostringstream oss;
        oss << dt;
        return oss.str();
    }
    std::string operator()(const offset_datetime& odt) const
    {
        std::ostringstream oss;
        oss << odt;
        return oss.str();
    }

    std::string operator()(const array& v) const
    {
        if(!v.empty() && v.front().is(value_t::Table))// v is an array of tables
        {
            // if it's not inlined, we need to add `[[table.key]]`.
            // but if it can be inlined, we need `table.key = [...]`.
            if(this->can_be_inlined_)
            {
                std::string token;
                if(!keys_.empty())
                {
                    token += this->serialize_key(keys_.back());
                    token += " = ";
                }
                bool width_exceeds = false;
                token += "[\n";
                for(const auto& item : v)
                {
                    const auto t =
                        this->make_inline_table(item.cast<value_t::Table>());

                    if(t.size() + 1 > width_ || // +1 for the last comma {...},
                       std::find(t.cbegin(), t.cend(), '\n') != t.cend())
                    {
                        width_exceeds = true;
                        break;
                    }
                    token += t;
                    token += ",\n";
                }
                if(!width_exceeds)
                {
                    token += "]\n";
                    return token;
                }
                // if width_exceeds, serialize it as [[array.of.tables]].
            }

            std::string token;
            for(const auto& item : v)
            {
                token += "[[";
                token += this->serialize_dotted_key(keys_);
                token += "]]\n";
                token += this->make_multiline_table(item.cast<value_t::Table>());
            }
            return token;
        }
        if(v.empty())
        {
            return std::string("[]");
        }

        // not an array of tables. normal array. first, try to make it inline.
        {
            const auto inl = this->make_inline_array(v);
            if(inl.size() < this->width_ &&
               std::find(inl.cbegin(), inl.cend(), '\n') == inl.cend())
            {
                return inl;
            }
        }

        // if the length exceeds this->width_, print multiline array
        std::string token;
        std::string current_line;
        token += "[\n";
        for(const auto& item : v)
        {
            auto next_elem = toml::visit(*this, item);
            // newline between array-value and comma is not allowed
            if(next_elem.back() == '\n'){next_elem.pop_back();}

            if(current_line.size() + next_elem.size() + 1 < this->width_)
            {
                current_line += next_elem;
                current_line += ',';
            }
            else if(current_line.empty())
            {
                // the next elem cannot be within the width.
                token += next_elem;
                token += ",\n";
                // keep current line empty
            }
            else // current_line has some tokens and it exceeds width
            {
                assert(current_line.back() == ',');
                token += current_line;
                token += '\n';
                current_line = next_elem;
                current_line += ',';
            }
        }
        if(!current_line.empty())
        {
            if(current_line.back() != '\n') {current_line += '\n';}
            token += current_line;
        }
        token += "]\n";
        return token;
    }

    std::string operator()(const table& v) const
    {
        if(this->can_be_inlined_)
        {
            std::string token;
            if(!this->keys_.empty())
            {
                token += this->serialize_key(this->keys_.back());
                token += " = ";
            }
            token += this->make_inline_table(v);
            if(token.size() < this->width_)
            {
                return token;
            }
        }

        std::string token;
        if(!keys_.empty())
        {
            token += '[';
            token += this->serialize_dotted_key(keys_);
            token += "]\n";
        }
        token += this->make_multiline_table(v);
        return token;
    }

  private:

    std::string serialize_key(const toml::key& key) const
    {
        detail::location<toml::key> loc(key, key);
        detail::lex_unquoted_key::invoke(loc);
        if(loc.iter() == loc.end())
        {
            return key; // all the tokens are consumed. the key is unquoted-key.
        }
        std::string token("\"");
        token += this->escape_basic_string(key);
        token += "\"";
        return token;
    }

    std::string serialize_dotted_key(const std::vector<toml::key>& keys) const
    {
        std::string token;
        if(keys.empty()){return token;}

        for(const auto& k : keys)
        {
            token += this->serialize_key(k);
            token += '.';
        }
        token.erase(token.size() - 1, 1); // remove trailing `.`
        return token;
    }

    std::string escape_basic_string(const std::string& s) const
    {
        //XXX assuming `s` is a valid utf-8 sequence.
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

    std::string escape_ml_basic_string(const std::string& s) const
    {
        std::string retval;
        for(auto i=s.cbegin(), e=s.cend(); i!=e; ++i)
        {
            switch(*i)
            {
                case '\\': {retval += "\\\\"; break;}
                case '\"': {retval += "\\\""; break;}
                case '\b': {retval += "\\b";  break;}
                case '\t': {retval += "\\t";  break;}
                case '\f': {retval += "\\f";  break;}
                case '\n': {retval += "\n";   break;}
                case '\r':
                {
                    if(std::next(i) != e && *std::next(i) == '\n')
                    {
                        retval += "\r\n";
                        ++i;
                    }
                    else
                    {
                        retval += "\\r";
                    }
                    break;
                }
                default: {retval += *i; break;}
            }
        }
        return retval;
    }

    std::string make_inline_array(const array& v) const
    {
        std::string token;
        token += '[';
        bool is_first = true;
        for(const auto& item : v)
        {
            if(is_first) {is_first = false;} else {token += ',';}
            token += visit(serializer(std::numeric_limits<std::size_t>::max(),
                                      this->float_prec_, true), item);
        }
        token += ']';
        return token;
    }

    std::string make_inline_table(const table& v) const
    {
        assert(this->can_be_inlined_);
        std::string token;
        token += '{';
        bool is_first = true;
        for(const auto& kv : v)
        {
            // in inline tables, trailing comma is not allowed (toml-lang #569).
            if(is_first) {is_first = false;} else {token += ',';}
            token += this->serialize_key(kv.first);
            token += '=';
            token += visit(serializer(std::numeric_limits<std::size_t>::max(),
                                      this->float_prec_, true), kv.second);
        }
        token += '}';
        return token;
    }

    std::string make_multiline_table(const table& v) const
    {
        std::string token;

        // print non-table stuff first. because after printing [foo.bar], the
        // remaining non-table values will be assigned into [foo.bar], not [foo]
        for(const auto kv : v)
        {
            if(kv.second.is(value_t::Table) || is_array_of_tables(kv.second))
            {
                continue;
            }

            const auto key_and_sep    = this->serialize_key(kv.first) + " = ";
            const auto residual_width = (this->width_ > key_and_sep.size()) ?
                                        this->width_ - key_and_sep.size() : 0;
            token += key_and_sep;
            token += visit(serializer(residual_width, this->float_prec_, true),
                           kv.second);
            if(token.back() != '\n')
            {
                token += '\n';
            }
        }

        // normal tables / array of tables

        // after multiline table appeared, the other tables cannot be inline
        // because the table would be assigned into the table.
        // [foo]
        // ...
        // bar = {...} # <- bar will be a member of [foo].
        bool multiline_table_printed = false;
        for(const auto& kv : v)
        {
            if(!kv.second.is(value_t::Table) && !is_array_of_tables(kv.second))
            {
                continue; // other stuff are already serialized. skip them.
            }

            std::vector<toml::key> ks(this->keys_);
            ks.push_back(kv.first);

            auto tmp = visit(serializer(
                this->width_, this->float_prec_, !multiline_table_printed, ks),
                kv.second);

            if((!multiline_table_printed) &&
               std::find(tmp.cbegin(), tmp.cend(), '\n') != tmp.cend())
            {
                multiline_table_printed = true;
            }
            else
            {
                // still inline tables only.
                tmp += '\n';
            }
            token += tmp;
        }
        return token;
    }

    bool is_array_of_tables(const value& v) const
    {
        if(!v.is(value_t::Array)) {return false;}

        const auto& a = v.cast<value_t::Array>();
        return !a.empty() && a.front().is(value_t::Table);
    }

  private:

    bool        can_be_inlined_;
    int         float_prec_;
    std::size_t width_;
    std::vector<toml::key> keys_;
};

inline std::string
format(const value& v, std::size_t w = 80,
       int fprec = std::numeric_limits<toml::floating>::max_digits10,
       bool force_inline = false)
{
    // if value is a table, it is considered to be a root object.
    // the root object can't be an inline table. so pass false. otherwise, true.
    return visit(serializer(w, fprec, (!v.is_table()) || force_inline), v);
}
inline std::string
format(const table& t, std::size_t w = 80,
       int fprec = std::numeric_limits<toml::floating>::max_digits10,
       bool force_inline = false)
{
    return serializer(w, fprec, force_inline)(t);
}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const value& v)
{
    // get status of std::setw().
    const std::size_t w     = os.width();
    const int         fprec = os.precision();
    os.width(0);
    // the root object can't be an inline table. so pass `false`.
    os << visit(serializer(w, fprec, false), v);
    return os;
}
template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const table& v)
{
    // get status of std::setw().
    const std::size_t w     = os.width();
    const int         fprec = os.precision();
    os.width(0);
    // the root object can't be an inline table. so pass `false`.
    os << serializer(w, fprec, false)(v);
    return os;
}

} // toml
#endif// TOML11_SERIALIZER_HPP

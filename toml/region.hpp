//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_REGION_H
#define TOML11_REGION_H
#include "exception.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <iomanip>

namespace toml
{
namespace detail
{

// helper function to avoid std::string(0, 'c')
template<typename Iterator>
std::string make_string(Iterator first, Iterator last)
{
    if(first == last) {return "";}
    return std::string(first, last);
}
inline std::string make_string(std::size_t len, char c)
{
    if(len == 0) {return "";}
    return std::string(len, c);
}

// location in a container, normally in a file content.
// shared_ptr points the resource that the iter points.
// it can be used not only for resource handling, but also error message.
template<typename Container>
struct location
{
    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    using const_iterator = typename Container::const_iterator;
    using source_ptr     = std::shared_ptr<const Container>;

    location(std::string name, Container cont)
      : source_(std::make_shared<Container>(std::move(cont))),
        source_name_(std::move(name)), iter_(source_->cbegin())
    {}
    location(const location&) = default;
    location(location&&)      = default;
    location& operator=(const location&) = default;
    location& operator=(location&&)      = default;
    ~location() = default;

    const_iterator& iter()       noexcept {return iter_;}
    const_iterator  iter() const noexcept {return iter_;}

    const_iterator  begin() const noexcept {return source_->cbegin();}
    const_iterator  end()   const noexcept {return source_->cend();}

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

    std::string const& name() const noexcept {return source_name_;}

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator iter_;
};

// region in a container, normally in a file content.
// shared_ptr points the resource that the iter points.
// combinators returns this.
// it will be used to generate better error messages.
struct region_base
{
    region_base() = default;
    virtual ~region_base() = default;
    region_base(const region_base&) = default;
    region_base(region_base&&     ) = default;
    region_base& operator=(const region_base&) = default;
    region_base& operator=(region_base&&     ) = default;

    virtual bool is_ok()           const noexcept {return false;}

    virtual std::string str()      const {return std::string("unknown region");}
    virtual std::string name()     const {return std::string("unknown file");}
    virtual std::string line()     const {return std::string("unknown line");}
    virtual std::string line_num() const {return std::string("?");}


    virtual std::size_t before()   const noexcept {return 0;}
    virtual std::size_t size()     const noexcept {return 0;}
    virtual std::size_t after()    const noexcept {return 0;}
};

template<typename Container>
struct region final : public region_base
{
    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    using const_iterator = typename Container::const_iterator;
    using source_ptr     = std::shared_ptr<const Container>;

    // delete default constructor. source_ never be null.
    region() = delete;

    region(const location<Container>& loc)
      : source_(loc.source()), source_name_(loc.name()),
        first_(loc.iter()), last_(loc.iter())
    {}
    region(location<Container>&& loc)
      : source_(loc.source()), source_name_(loc.name()),
        first_(loc.iter()), last_(loc.iter())
    {}

    region(const location<Container>& loc, const_iterator f, const_iterator l)
      : source_(loc.source()), source_name_(loc.name()), first_(f), last_(l)
    {}
    region(location<Container>&& loc, const_iterator f, const_iterator l)
      : source_(loc.source()), source_name_(loc.name()), first_(f), last_(l)
    {}

    region(const region&) = default;
    region(region&&)      = default;
    region& operator=(const region&) = default;
    region& operator=(region&&)      = default;
    ~region() = default;

    region& operator+=(const region& other)
    {
        if(this->begin() != other.begin() || this->end() != other.end() ||
           this->last_  != other.first_)
        {
            throw internal_error("invalid region concatenation");
        }
        this->last_ = other.last_;
        return *this;
    }

    bool is_ok() const noexcept override {return static_cast<bool>(source_);}

    std::string str()  const override {return make_string(first_, last_);}
    std::string line() const override
    {
        if(this->contain_newline())
        {
            return make_string(this->line_begin(),
                    std::find(this->line_begin(), this->last(), '\n'));
        }
        return make_string(this->line_begin(), this->line_end());
    }
    std::string line_num() const override
    {
        return std::to_string(1 + std::count(this->begin(), this->first(), '\n'));
    }

    std::size_t size() const noexcept override
    {
        return std::distance(first_, last_);
    }
    std::size_t before() const noexcept override
    {
        return std::distance(this->line_begin(), this->first());
    }
    std::size_t after() const noexcept override
    {
        return std::distance(this->last(), this->line_end());
    }

    bool contain_newline() const noexcept
    {
        return std::find(this->first(), this->last(), '\n') != this->last();
    }

    const_iterator line_begin() const noexcept
    {
        using reverse_iterator = std::reverse_iterator<const_iterator>;
        return std::find(reverse_iterator(this->first()),
                         reverse_iterator(this->begin()), '\n').base();
    }
    const_iterator line_end() const noexcept
    {
        return std::find(this->last(), this->end(), '\n');
    }

    const_iterator begin() const noexcept {return source_->cbegin();}
    const_iterator end()   const noexcept {return source_->cend();}
    const_iterator first() const noexcept {return first_;}
    const_iterator last()  const noexcept {return last_;}

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

    std::string name() const override {return source_name_;}

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator first_, last_;
};

// to show a better error message.
inline std::string format_underline(const std::string& message,
        const region_base& reg, const std::string& comment_for_underline,
        std::vector<std::string> helps = {})
{
#ifdef _WIN32
    const auto newline = "\r\n";
#else
    const char newline = '\n';
#endif
    const auto line        = reg.line();
    const auto line_number = reg.line_num();

    std::string retval;
    retval += message;
    retval += newline;
    retval += " --> ";
    retval += reg.name();
    retval += newline;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += line;
    retval += newline;
    retval += make_string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += make_string(reg.before(), ' ');
    retval += make_string(reg.size(),   '~');
    retval += ' ';
    retval += comment_for_underline;
    if(helps.size() != 0)
    {
        retval += newline;
        retval += make_string(line_number.size() + 1, ' ');
        retval += " | ";
        for(const auto help : helps)
        {
            retval += newline;
            retval += "Hint: ";
            retval += help;
        }
    }
    return retval;
}

// to show a better error message.
inline std::string format_underline(const std::string& message,
        const region_base& reg1, const std::string& comment_for_underline1,
        const region_base& reg2, const std::string& comment_for_underline2,
        std::vector<std::string> helps = {})
{
#ifdef _WIN32
    const auto newline = "\r\n";
#else
    const char newline = '\n';
#endif
    const auto line1        = reg1.line();
    const auto line_number1 = reg1.line_num();
    const auto line2        = reg2.line();
    const auto line_number2 = reg2.line_num();
    const auto line_num_width =
        std::max(line_number1.size(), line_number2.size());

    std::ostringstream retval;
    retval << message << newline;
    retval << " --> " << reg1.name() << newline;
//  ---------------------------------------
    retval << ' ' << std::setw(line_num_width) << line_number1;
    retval << " | " << line1 << newline;
    retval << make_string(line_num_width + 1, ' ');
    retval << " | ";
    retval << make_string(reg1.before(), ' ');
    retval << make_string(reg1.size(),   '~');
    retval << ' ';
    retval << comment_for_underline1 << newline;
//  ---------------------------------------
    if(reg2.name() != reg1.name())
    {
        retval << " --> " << reg2.name() << newline;
    }
    else
    {
        retval << " ..." << newline;
    }
    retval << ' ' << std::setw(line_num_width) << line_number2;
    retval << " | " << line2 << newline;
    retval << make_string(line_num_width + 1, ' ');
    retval << " | ";
    retval << make_string(reg2.before(), ' ');
    retval << make_string(reg2.size(),   '~');
    retval << ' ';
    retval << comment_for_underline2;
    if(helps.size() != 0)
    {
        retval << newline;
        retval << make_string(line_num_width + 1, ' ');
        retval << " | ";
        for(const auto help : helps)
        {
            retval << newline;
            retval << "Hint: ";
            retval << help;
        }
    }
    return retval.str();
}


// to show a better error message.
template<typename Container>
std::string
format_underline(const std::string& message, const location<Container>& loc,
                 const std::string& comment_for_underline,
                 std::vector<std::string> helps = {})
{
#ifdef _WIN32
    const auto newline = "\r\n";
#else
    const char newline = '\n';
#endif
    using const_iterator   = typename location<Container>::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    const auto line_begin  = std::find(reverse_iterator(loc.iter()),
                                       reverse_iterator(loc.begin()),
                                       '\n').base();
    const auto line_end    = std::find(loc.iter(), loc.end(), '\n');

    const auto line_number = std::to_string(
            1 + std::count(loc.begin(), loc.iter(), '\n'));

    std::string retval;
    retval += message;
    retval += newline;
    retval += " --> ";
    retval += loc.name();
    retval += newline;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += make_string(line_begin, line_end);
    retval += newline;
    retval += make_string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += make_string(std::distance(line_begin, loc.iter()),' ');
    retval += '^';
    retval += make_string(std::distance(loc.iter(), line_end), '-');
    retval += ' ';
    retval += comment_for_underline;
    if(helps.size() != 0)
    {
        retval += newline;
        retval += make_string(line_number.size() + 1, ' ');
        retval += " | ";
        for(const auto help : helps)
        {
            retval += newline;
            retval += "Hint: ";
            retval += help;
        }
    }
    return retval;
}

} // detail
} // toml
#endif// TOML11_REGION_H

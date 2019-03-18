//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_REGION_HPP
#define TOML11_REGION_HPP
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

// helper function to avoid std::string(0, 'c') or std::string(iter, iter)
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

// region_base is a base class of location and region that are defined below.
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

    // length of the region
    virtual std::size_t size()     const noexcept {return 0;}
    // number of characters in the line before the region
    virtual std::size_t before()   const noexcept {return 0;}
    // number of characters in the line after the region
    virtual std::size_t after()    const noexcept {return 0;}
};

// location represents a position in a container, which contains a file content.
// it can be considered as a region that contains only one character.
//
// it contains pointer to the file content and iterator that points the current
// location.
template<typename Container>
struct location final : public region_base
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

    bool is_ok() const noexcept override {return static_cast<bool>(source_);}

    const_iterator& iter()       noexcept {return iter_;}
    const_iterator  iter() const noexcept {return iter_;}

    const_iterator  begin() const noexcept {return source_->cbegin();}
    const_iterator  end()   const noexcept {return source_->cend();}

    std::string str()  const override {return make_string(1, *this->iter());}
    std::string name() const override {return source_name_;}

    std::string line_num() const override
    {
        return std::to_string(1+std::count(this->begin(), this->iter(), '\n'));
    }

    std::string line() const override
    {
        return make_string(this->line_begin(), this->line_end());
    }

    const_iterator line_begin() const noexcept
    {
        using reverse_iterator = std::reverse_iterator<const_iterator>;
        return std::find(reverse_iterator(this->iter()),
                         reverse_iterator(this->begin()), '\n').base();
    }
    const_iterator line_end() const noexcept
    {
        return std::find(this->iter(), this->end(), '\n');
    }

    // location is always points a character. so the size is 1.
    std::size_t size() const noexcept override
    {
        return 1u;
    }
    std::size_t before() const noexcept override
    {
        return std::distance(this->line_begin(), this->iter());
    }
    std::size_t after() const noexcept override
    {
        return std::distance(this->iter(), this->line_end());
    }

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator iter_;
};

// region represents a range in a container, which contains a file content.
//
// it contains pointer to the file content and iterator that points the first
// and last location.
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
        std::vector<std::pair<region_base const*, std::string>> reg_com,
        std::vector<std::string> helps = {})
{
    assert(!reg_com.empty());

#ifdef _WIN32
    const auto newline = "\r\n";
#else
    const char newline = '\n';
#endif

    const auto line_num_width = std::max_element(reg_com.begin(), reg_com.end(),
        [](std::pair<region_base const*, std::string> const& lhs,
           std::pair<region_base const*, std::string> const& rhs)
        {
            return lhs.first->line_num().size() < rhs.first->line_num().size();
        }
    )->first->line_num().size();

    std::ostringstream retval;
    retval << message << newline;

    for(std::size_t i=0; i<reg_com.size(); ++i)
    {
        if(i!=0 && reg_com.at(i-1).first->name() == reg_com.at(i).first->name())
        {
            retval << newline << " ..." << newline;
        }
        else
        {
            if(i != 0) {retval << newline;}
            retval << " --> " << reg_com.at(i).first->name() << newline;
        }

        const region_base* const reg = reg_com.at(i).first;
        const std::string&   comment = reg_com.at(i).second;


        retval << ' ' << std::setw(line_num_width) << reg->line_num();
        retval << " | " << reg->line() << newline;
        retval << make_string(line_num_width + 1, ' ');
        retval << " | " << make_string(reg->before(), ' ');

        if(reg->size() == 1)
        {
            // invalid
            // ^------
            retval << '^';
            retval << make_string(reg->after(), '-');
        }
        else
        {
            // invalid
            // ~~~~~~~
            retval << make_string(reg->size(), '~');
        }

        retval << ' ';
        retval << comment;
    }

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

} // detail
} // toml
#endif// TOML11_REGION_H

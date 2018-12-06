#ifndef TOML11_REGION_H
#define TOML11_REGION_H
#include "exception.hpp"
#include <memory>
#include <algorithm>
#include <iterator>
#include <iostream>

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
        source_name_(std::move(name)), begin_(source_->cbegin()),
        iter_(source_->cbegin()), end_(source_->cend())
    {}
    location(const location&) = default;
    location(location&&)      = default;
    location& operator=(const location&) = default;
    location& operator=(location&&)      = default;
    ~location() = default;

    const_iterator& iter()       noexcept {return iter_;}
    const_iterator  iter() const noexcept {return iter_;}

    const_iterator  begin() const noexcept {return begin_;}
    const_iterator  end()   const noexcept {return end_;}

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

    std::string const& name() const noexcept {return source_name_;}

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator begin_, iter_, end_;
};

// region in a container, normally in a file content.
// shared_ptr points the resource that the iter points.
// combinators returns this.
// it can be used not only for resource handling, but also error message.
template<typename Container>
struct region
{
    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    using const_iterator = typename Container::const_iterator;
    using source_ptr     = std::shared_ptr<const Container>;

    region(const location<Container>& loc)
      : source_(loc.source()), source_name_(loc.name()),
        begin_(loc.begin()), first_(loc.iter()),
        last_(loc.iter()), end_(loc.end())
    {}
    region(location<Container>&& loc)
      : source_(loc.source()), source_name_(loc.name()),
        begin_(loc.begin()), first_(loc.iter()),
        last_(loc.iter()), end_(loc.end())
    {}

    region(const location<Container>& loc, const_iterator f, const_iterator l)
      : source_(loc.source()), source_name_(loc.name()),
        begin_(loc.begin()), first_(f), last_(l), end_(loc.end())
    {}
    region(location<Container>&& loc, const_iterator f, const_iterator l)
      : source_(loc.source()), source_name_(loc.name()),
        begin_(loc.begin()), first_(f), last_(l), end_(loc.end())
    {}

    region(const region&) = default;
    region(region&&)      = default;
    region& operator=(const region&) = default;
    region& operator=(region&&)      = default;
    ~region() = default;

    region& operator+=(const region& other)
    {
        if(this->begin_ != other.begin_ || this->end_ != other.end_ ||
           this->last_  != other.first_)
        {
            throw internal_error("invalid region concatenation");
        }
        this->last_ = other.last_;
        return *this;
    }

    std::string str()  const {return make_string(first_, last_);}
    std::size_t size() const {return std::distance(first_, last_);}

    const_iterator  begin() const noexcept {return begin_;}
    const_iterator  end()   const noexcept {return end_;}
    const_iterator  first() const noexcept {return first_;}
    const_iterator  last()  const noexcept {return last_;}

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

    std::string const& name() const noexcept {return source_name_;}

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator begin_, first_, last_, end_;
};

// to show a better error message.
template<typename Container>
std::string
format_underline(const std::string& message, const region<Container>& reg,
                 const std::string& comment_for_underline)
{
    using const_iterator = typename region<Container>::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    const auto line_begin = std::find(reverse_iterator(reg.first()),
                                      reverse_iterator(reg.begin()),
                                      '\n').base();
    const auto line_end   = std::find(reg.last(), reg.end(), '\n');

    const auto line_number = std::to_string(
            1 + std::count(reg.begin(), reg.first(), '\n'));

    std::string retval;
    retval += message;
    retval += "\n --> ";
    retval += reg.name();
    retval += "\n ";
    retval += line_number;
    retval += " | ";
    retval += make_string(line_begin, line_end);
    retval += '\n';
    retval += make_string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += make_string(std::distance(line_begin, reg.first()), ' ');
    retval += make_string(std::distance(reg.first(), reg.last()), '~');
    retval += ' ';
    retval += comment_for_underline;
    return retval;
}

// to show a better error message.
template<typename Container>
std::string
format_underline(const std::string& message, const location<Container>& loc,
                 const std::string& comment_for_underline)
{
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
    retval += "\n --> ";
    retval += loc.name();
    retval += "\n ";
    retval += line_number;
    retval += " | ";
    retval += make_string(line_begin, line_end);
    retval += '\n';
    retval += make_string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += make_string(std::distance(line_begin, loc.iter()),' ');
    retval += '^';
    retval += make_string(std::distance(loc.iter(), line_end), '-');
    retval += ' ';
    retval += comment_for_underline;

    return retval;
}

} // detail
} // toml
#endif// TOML11_REGION_H

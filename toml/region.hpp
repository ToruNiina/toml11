#ifndef TOML11_REGION_H
#define TOML11_REGION_H
#include "exception.hpp"
#include <memory>
#include <algorithm>
#include <iterator>

namespace toml
{
namespace detail
{

// location in a container, normally in a file content.
// shared_ptr points the resource that the iter points.
// it can be used not only for resource handling, but also error message.
template<typename Container>
struct location
{
    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    using const_iterator = typename Container::const_iterator;
    using source_ptr     = std::shared_ptr<const Container>;

    location(Container cont)
      : source_(std::make_shared<Container>(std::move(cont))),
        begin_(source_->cbegin()), iter_(source_->cbegin()),
        end_(source_->cend())
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

  private:

    source_ptr source_;
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
      : begin_(loc.begin()), first_(loc.iter()),
        last_(loc.iter()), end_(loc.end()), source_(loc.source())
    {}
    region(location<Container>&& loc)
      : begin_(loc.begin()), first_(loc.iter()),
        last_(loc.iter()), end_(loc.end()), source_(std::move(loc.source()))
    {}

    region(const location<Container>& loc, const_iterator f, const_iterator l)
      : begin_(loc.begin()), first_(f), last_(l), end_(loc.end()),
        source_(loc.source())
    {}
    region(location<Container>&& loc, const_iterator f, const_iterator l)
      : begin_(loc.begin()), first_(f), last_(l), end_(loc.end()),
        source_(std::move(loc.source()))
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

    std::string str()  const {return std::string(first_, last_);}
    std::size_t size() const {return std::distance(first_, last_);}

    const_iterator  begin() const noexcept {return begin_;}
    const_iterator  end()   const noexcept {return end_;}
    const_iterator  first() const noexcept {return first_;}
    const_iterator  last()  const noexcept {return last_;}

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

  private:

    const_iterator begin_, first_, last_, end_;
    source_ptr source_;
};

// to show a better error message.
template<typename Container>
std::string
format_underline(const region<Container>& reg, const std::string& msg)
{
    using const_iterator = typename region<Container>::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    const auto line_begin = std::find(
            reverse_iterator(reg.first()),
            reverse_iterator(reg.begin()),
            '\n').base();
    const auto line_end = std::find(reg.last(), reg.end(), '\n');

    const auto line_number = std::to_string(
            1 + std::count(reg.begin(), reg.first(), '\n'));

    std::string retval;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += std::string(line_begin, line_end);
    retval += '\n';
    retval += std::string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += std::string(std::distance(line_begin, reg.first()), ' ');
    retval += std::string(std::distance(reg.first(), reg.last()), '~');
    retval += ' ';
    retval += msg;
    return retval;
}

// to show a better error message.
template<typename Container>
std::string format_underline(const region<Container>& reg,
                             typename Container::const_iterator pos,
                             const std::string& msg)
{
    using const_iterator = typename region<Container>::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    const auto line_begin = std::find(
            reverse_iterator(reg.first()),
            reverse_iterator(reg.begin()),
            '\n').base();
    const auto line_end = std::find(reg.last(), reg.end(), '\n');

    const auto line_number = std::to_string(
            1 + std::count(reg.begin(), reg.first(), '\n'));

    std::string retval;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += std::string(line_begin, line_end);
    retval += '\n';
    retval += std::string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += std::string(std::distance(line_begin, reg.first()), ' ');

    if(std::distance(reg.first(), std::prev(pos)) > 0)
    {
        retval += std::string(std::distance(reg.first(), std::prev(pos)), '-');
    }
    retval += '^';
    if(std::distance(pos, reg.last()) > 0)
    {
        retval += std::string(std::distance(std::next(pos), reg.last()), '-');
    }

    retval += ' ';
    retval += msg;
    return retval;
}

} // detail
} // toml
#endif// TOML11_REGION_H

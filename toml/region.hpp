#ifndef TOML11_REGION_H
#define TOML11_REGION_H
#include <memory>
#include <algorithm>
#include <iterator>
#include <string>

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

    location(Container cont)
      : source(std::make_shared<Container>(std::move(cont))),
        begin(source->cbegin()), iter(source->cbegin()), end(source->cend())
    {}
    location(const location&) = default;
    location(location&&)      = default;
    location& operator=(const location&) = default;
    location& operator=(location&&)      = default;
   ~location() = default;

    std::shared_ptr<const Container> source;
    const_iterator begin, iter, end;
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

    region(const location<Container>& loc)
      : begin(loc.begin), first(loc.iter), last(loc.iter), end(loc.end),
        source(loc.source)
    {}
    region(location<Container>&& loc)
      : begin(loc.begin), first(loc.iter), last(loc.iter), end(loc.end),
        source(std::move(loc.source))
    {}

    region(const location<Container>& loc, const_iterator f, const_iterator l)
      : begin(loc.begin), first(f), last(l), end(loc.end),
        source(loc.source)
    {}
    region(location<Container>&& loc, const_iterator f, const_iterator l)
      : begin(loc.begin), first(f), last(l), end(loc.end),
        source(std::move(loc.source))
    {}

    region(const region&) = default;
    region(region&&)      = default;
    region& operator=(const region&) = default;
    region& operator=(region&&)      = default;
    ~region() = default;

    std::string str()  const {return std::string(first, last);}
    std::size_t size() const {return std::distance(first, last);}

    const_iterator begin, first, last, end;
    std::shared_ptr<const Container> source;
};

// to show a better error message.
template<typename Container>
std::string
format_underline(const region<Container>& reg, const std::string& msg)
{
    using const_iterator = typename region<Container>::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    const auto line_begin = std::find(
            reverse_iterator(reg.first),
            reverse_iterator(reg.begin),
            '\n').base();
    const auto line_end = std::find(reg.last, reg.end, '\n');

    const auto line_number = std::to_string(
            1 + std::count(reg.begin, reg.first, '\n'));

    std::string retval;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += std::string(line_begin, line_end);
    retval += '\n';
    retval += std::string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += std::string(std::distance(line_begin, reg.first), ' ');
    retval += std::string(std::distance(reg.first, reg.last), '~');
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
            reverse_iterator(reg.first),
            reverse_iterator(reg.begin),
            '\n').base();
    const auto line_end = std::find(reg.last, reg.end, '\n');

    const auto line_number = std::to_string(
            1 + std::count(reg.begin, reg.first, '\n'));

    std::string retval;
    retval += ' ';
    retval += line_number;
    retval += " | ";
    retval += std::string(line_begin, line_end);
    retval += '\n';
    retval += std::string(line_number.size() + 1, ' ');
    retval += " | ";
    retval += std::string(std::distance(line_begin, reg.first), ' ');

    if(std::distance(reg.first, std::prev(pos)) > 0)
    {
        retval += std::string(std::distance(reg.first, std::prev(pos)), '-');
    }
    retval += '^';
    if(std::distance(pos, reg.last) > 0)
    {
        retval += std::string(std::distance(std::next(pos), reg.last), '-');
    }

    retval += ' ';
    retval += msg;
    return retval;
}

} // detail
} // toml
#endif// TOML11_REGION_H

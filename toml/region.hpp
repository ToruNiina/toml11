//     Copyright Toru Niina 2017.
// Distributed under the MIT License.
#ifndef TOML11_REGION_HPP
#define TOML11_REGION_HPP
#include <memory>
#include <vector>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <iomanip>
#include <cassert>
#include "color.hpp"

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
    virtual char front()           const noexcept {return '\0';}

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

    virtual std::vector<std::string> comments() const {return {};}
    // ```toml
    // # comment_before
    // key = "value" # comment_inline
    // ```
};

// location represents a position in a container, which contains a file content.
// it can be considered as a region that contains only one character.
//
// it contains pointer to the file content and iterator that points the current
// location.
template<typename Container>
struct location final : public region_base
{
    using const_iterator  = typename Container::const_iterator;
    using difference_type = typename const_iterator::difference_type;
    using source_ptr      = std::shared_ptr<const Container>;

    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    static_assert(std::is_same<std::random_access_iterator_tag,
        typename std::iterator_traits<const_iterator>::iterator_category>::value,
        "container should be randomly accessible");

    location(std::string name, Container cont)
      : source_(std::make_shared<Container>(std::move(cont))), line_number_(1),
        source_name_(std::move(name)), iter_(source_->cbegin())
    {}
    location(const location&) = default;
    location(location&&)      = default;
    location& operator=(const location&) = default;
    location& operator=(location&&)      = default;
    ~location() = default;

    bool is_ok() const noexcept override {return static_cast<bool>(source_);}
    char front() const noexcept override {return *iter_;}

    // this const prohibits codes like `++(loc.iter())`.
    const const_iterator iter()  const noexcept {return iter_;}

    const_iterator begin() const noexcept {return source_->cbegin();}
    const_iterator end()   const noexcept {return source_->cend();}

    // XXX `location::line_num()` used to be implemented using `std::count` to
    // count a number of '\n'. But with a long toml file (typically, 10k lines),
    // it becomes intolerably slow because each time it generates error messages,
    // it counts '\n' from thousands of characters. To workaround it, I decided
    // to introduce `location::line_number_` member variable and synchronize it
    // to the location changes the point to look. So an overload of `iter()`
    // which returns mutable reference is removed and `advance()`, `retrace()`
    // and `reset()` is added.
    void advance(difference_type n = 1) noexcept
    {
        this->line_number_ += static_cast<std::size_t>(
                std::count(this->iter_, std::next(this->iter_, n), '\n'));
        this->iter_ += n;
        return;
    }
    void retrace(difference_type n = 1) noexcept
    {
        this->line_number_ -= static_cast<std::size_t>(
                std::count(std::prev(this->iter_, n), this->iter_, '\n'));
        this->iter_ -= n;
        return;
    }
    void reset(const_iterator rollback) noexcept
    {
        // since c++11, std::distance works in both ways for random-access
        // iterators and returns a negative value if `first > last`.
        if(0 <= std::distance(rollback, this->iter_)) // rollback < iter
        {
            this->line_number_ -= static_cast<std::size_t>(
                    std::count(rollback, this->iter_, '\n'));
        }
        else // iter < rollback [[unlikely]]
        {
            this->line_number_ += static_cast<std::size_t>(
                    std::count(this->iter_, rollback, '\n'));
        }
        this->iter_ = rollback;
        return;
    }

    std::string str()  const override {return make_string(1, *this->iter());}
    std::string name() const override {return source_name_;}

    std::string line_num() const override
    {
        return std::to_string(this->line_number_);
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
        const auto sz = std::distance(this->line_begin(), this->iter());
        assert(sz >= 0);
        return static_cast<std::size_t>(sz);
    }
    std::size_t after() const noexcept override
    {
        const auto sz = std::distance(this->iter(), this->line_end());
        assert(sz >= 0);
        return static_cast<std::size_t>(sz);
    }

    source_ptr const& source() const& noexcept {return source_;}
    source_ptr&&      source() &&     noexcept {return std::move(source_);}

  private:

    source_ptr     source_;
    std::size_t    line_number_;
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
    using const_iterator = typename Container::const_iterator;
    using source_ptr     = std::shared_ptr<const Container>;

    static_assert(std::is_same<char, typename Container::value_type>::value,"");
    static_assert(std::is_same<std::random_access_iterator_tag,
        typename std::iterator_traits<const_iterator>::iterator_category>::value,
        "container should be randomly accessible");

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
        // different regions cannot be concatenated
        assert(this->begin() == other.begin() && this->end() == other.end() &&
               this->last_   == other.first_);

        this->last_ = other.last_;
        return *this;
    }

    bool is_ok() const noexcept override {return static_cast<bool>(source_);}
    char front() const noexcept override {return *first_;}

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
        const auto sz = std::distance(first_, last_);
        assert(sz >= 0);
        return static_cast<std::size_t>(sz);
    }
    std::size_t before() const noexcept override
    {
        const auto sz = std::distance(this->line_begin(), this->first());
        assert(sz >= 0);
        return static_cast<std::size_t>(sz);
    }
    std::size_t after() const noexcept override
    {
        const auto sz = std::distance(this->last(), this->line_end());
        assert(sz >= 0);
        return static_cast<std::size_t>(sz);
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

    std::vector<std::string> comments() const override
    {
        // assuming the current region (`*this`) points a value.
        // ```toml
        // a = "value"
        //     ^^^^^^^- this region
        // ```
        using rev_iter = std::reverse_iterator<const_iterator>;

        std::vector<std::string> com{};
        {
            // find comments just before the current region.
            // ```toml
            // # this should be collected.
            // # this also.
            // a = value # not this.
            // ```

            // # this is a comment for `a`, not array elements.
            // a = [1, 2, 3, 4, 5]
            if(this->first() == std::find_if(this->line_begin(), this->first(),
                [](const char c) noexcept -> bool {return c == '[' || c == '{';}))
            {
                auto iter = this->line_begin(); // points the first character
                while(iter != this->begin())
                {
                    iter = std::prev(iter);

                    // range [line_start, iter) represents the previous line
                    const auto line_start   = std::find(
                            rev_iter(iter), rev_iter(this->begin()), '\n').base();
                    const auto comment_found = std::find(line_start, iter, '#');
                    if(comment_found == iter)
                    {
                        break; // comment not found.
                    }

                    // exclude the following case.
                    // > a = "foo" # comment // <-- this is not a comment for b but a.
                    // > b = "current value"
                    if(std::all_of(line_start, comment_found,
                            [](const char c) noexcept -> bool {
                                return c == ' ' || c == '\t';
                            }))
                    {
                        // unwrap the first '#' by std::next.
                        auto str = make_string(std::next(comment_found), iter);
                        if(str.back() == '\r') {str.pop_back();}
                        com.push_back(std::move(str));
                    }
                    else
                    {
                        break;
                    }
                    iter = line_start;
                }
            }
        }

        if(com.size() > 1)
        {
            std::reverse(com.begin(), com.end());
        }

        {
            // find comments just after the current region.
            // ```toml
            // # not this.
            // a = value # this one.
            // a = [ # not this (technically difficult)
            //
            // ] # and this.
            // ```
            // The reason why it's difficult is that it requires parsing in the
            // following case.
            // ```toml
            // a = [ 10 # this comment is for `10`. not for `a` but `a[0]`.
            // # ...
            // ] # this is apparently a comment for a.
            //
            // b = [
            // 3.14 ] # there is no way to add a comment to `3.14` currently.
            //
            // c = [
            //   3.14 # do this if you need a comment here.
            // ]
            // ```
            const auto comment_found =
                std::find(this->last(), this->line_end(), '#');
            if(comment_found != this->line_end()) // '#' found
            {
                // table = {key = "value"} # what is this for?
                // the above comment is not for "value", but {key="value"}.
                if(comment_found == std::find_if(this->last(), comment_found,
                    [](const char c) noexcept -> bool {
                        return !(c == ' ' || c == '\t' || c == ',');
                    }))
                {
                    // unwrap the first '#' by std::next.
                    auto str = make_string(std::next(comment_found), this->line_end());
                    if(str.back() == '\r') {str.pop_back();}
                    com.push_back(std::move(str));
                }
            }
        }
        return com;
    }

  private:

    source_ptr     source_;
    std::string    source_name_;
    const_iterator first_, last_;
};

// to show a better error message.
inline std::string format_underline(const std::string& message,
        const std::vector<std::pair<region_base const*, std::string>>& reg_com,
        const std::vector<std::string>& helps = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED)
{
    assert(!reg_com.empty());

    const auto line_num_width = static_cast<int>(std::max_element(
        reg_com.begin(), reg_com.end(),
        [](std::pair<region_base const*, std::string> const& lhs,
           std::pair<region_base const*, std::string> const& rhs)
        {
            return lhs.first->line_num().size() < rhs.first->line_num().size();
        }
    )->first->line_num().size());

    std::ostringstream retval;

    if(colorize)
    {
        retval << color::colorize; // turn on ANSI color
    }

    // XXX
    // Here, before `colorize` support, it does not output `[error]` prefix
    // automatically. So some user may output it manually and this change may
    // duplicate the prefix. To avoid it, check the first 7 characters and
    // if it is "[error]", it removes that part from the message shown.
    if(message.size() > 7 && message.substr(0, 7) == "[error]")
    {
        retval << color::bold << color::red << "[error]" << color::reset
               << color::bold << message.substr(7) << color::reset << '\n';
    }
    else
    {
        retval << color::bold << color::red << "[error] " << color::reset
               << color::bold << message << color::reset << '\n';
    }

    for(auto iter = reg_com.begin(); iter != reg_com.end(); ++iter)
    {
        // if the filenames are the same, print "..."
        if(iter != reg_com.begin() &&
           std::prev(iter)->first->name() == iter->first->name())
        {
            retval << color::bold << color::blue << "\n ...\n" << color::reset;
        }
        else // if filename differs, print " --> filename.toml"
        {
            if(iter != reg_com.begin()) {retval << '\n';}
            retval << color::bold << color::blue << " --> " << color::reset
                   << iter->first->name() << '\n';
            // add one almost-empty line for readability
            retval << make_string(static_cast<std::size_t>(line_num_width + 1), ' ')
                   << color::bold << color::blue << " | "  << color::reset << '\n';
        }
        const region_base* const reg = iter->first;
        const std::string&   comment = iter->second;

        retval << ' ' << color::bold << color::blue << std::setw(line_num_width)
               << std::right << reg->line_num() << " | "  << color::reset
               << reg->line() << '\n';

        retval << make_string(static_cast<std::size_t>(line_num_width + 1), ' ')
               << color::bold << color::blue << " | " << color::reset
               << make_string(reg->before(), ' ');

        if(reg->size() == 1)
        {
            // invalid
            // ^------
            retval << color::bold << color::red
                   << '^' << make_string(reg->after(), '-') << color::reset;
        }
        else
        {
            // invalid
            // ~~~~~~~
            const auto underline_len = (std::min)(reg->size(), reg->line().size());
            retval << color::bold << color::red
                   << make_string(underline_len, '~') << color::reset;
        }
        retval << ' ';
        retval << comment;
    }

    if(!helps.empty())
    {
        retval << '\n';
        retval << make_string(static_cast<std::size_t>(line_num_width + 1), ' ');
        retval << color::bold << color::blue << " | " << color::reset;
        for(const auto& help : helps)
        {
            retval << color::bold << "\nHint: " << color::reset;
            retval << help;
        }
    }
    return retval.str();
}

} // detail
} // toml
#endif// TOML11_REGION_H

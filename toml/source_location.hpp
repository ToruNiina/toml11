//     Copyright Toru Niina 2019.
// Distributed under the MIT License.
#ifndef TOML11_SOURCE_LOCATION_HPP
#define TOML11_SOURCE_LOCATION_HPP
#include <cstdint>

#include "region.hpp"

namespace toml
{

// A struct to contain location in a toml file.
// The interface imitates std::experimental::source_location,
// but not completely the same.
//
// It would be constructed by toml::value. It can be used to generate
// user-defined error messages.
//
// - std::uint_least32_t line() const noexcept
//   - returns the line number where the region is on.
// - std::uint_least32_t column() const noexcept
//   - returns the column number where the region starts.
// - std::uint_least32_t region() const noexcept
//   - returns the size of the region.
//
// +-- line()       +-- region of interest (region() == 9)
// v            .---+---.
// 12 | value = "foo bar"
//              ^
//              +-- column()
//
// - std::string const& file_name() const noexcept;
//   - name of the file.
// - std::string const& line_str() const noexcept;
//   - the whole line that contains the region of interest.
//
struct source_location
{
  public:

    source_location()
        : line_num_(0), column_num_(0), region_size_(0),
          file_name_("unknown file"), line_str_("")
    {}

    explicit source_location(const detail::region_base* reg)
        : line_num_(0), column_num_(0), region_size_(0),
          file_name_("unknown file"), line_str_("")
    {
        if(reg)
        {
            if(reg->line_num() != detail::region_base().line_num())
            {
                line_num_ = static_cast<std::uint_least32_t>(
                        std::stoul(reg->line_num()));
            }
            column_num_  = static_cast<std::uint_least32_t>(reg->before() + 1);
            region_size_ = static_cast<std::uint_least32_t>(reg->size());
            file_name_   = reg->name();
            line_str_    = reg->line();
        }
    }

    explicit source_location(const detail::region& reg)
        : line_num_(static_cast<std::uint_least32_t>(std::stoul(reg.line_num()))),
          column_num_(static_cast<std::uint_least32_t>(reg.before() + 1)),
          region_size_(static_cast<std::uint_least32_t>(reg.size())),
          file_name_(reg.name()),
          line_str_ (reg.line())
    {}
    explicit source_location(const detail::location& loc)
        : line_num_(static_cast<std::uint_least32_t>(std::stoul(loc.line_num()))),
          column_num_(static_cast<std::uint_least32_t>(loc.before() + 1)),
          region_size_(static_cast<std::uint_least32_t>(loc.size())),
          file_name_(loc.name()),
          line_str_ (loc.line())
    {}

    ~source_location() = default;
    source_location(source_location const&) = default;
    source_location(source_location &&)     = default;
    source_location& operator=(source_location const&) = default;
    source_location& operator=(source_location &&)     = default;

    std::uint_least32_t line()      const noexcept {return line_num_;}
    std::uint_least32_t column()    const noexcept {return column_num_;}
    std::uint_least32_t region()    const noexcept {return region_size_;}

    std::string const&  file_name() const noexcept {return file_name_;}
    std::string const&  line_str()  const noexcept {return line_str_;}

  private:

    std::uint_least32_t line_num_;
    std::uint_least32_t column_num_;
    std::uint_least32_t region_size_;
    std::string         file_name_;
    std::string         line_str_;
};

namespace detail
{

// internal error message generation.
inline std::string format_underline(const std::string& message,
        const std::vector<std::pair<source_location, std::string>>& loc_com,
        const std::vector<std::string>& helps = {},
        const bool colorize = TOML11_ERROR_MESSAGE_COLORIZED)
{
    assert(!loc_com.empty());

    const auto line_num_width = static_cast<int>(std::to_string(std::max_element(
        loc_com.begin(), loc_com.end(),
        [](std::pair<source_location, std::string> const& lhs,
           std::pair<source_location, std::string> const& rhs)
        {
            return std::to_string(lhs.first.line()).size() <
                   std::to_string(rhs.first.line()).size();
        }
    )->first.line()).size());

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

    for(auto iter = loc_com.begin(); iter != loc_com.end(); ++iter)
    {
        // if the filenames are the same, print "..."
        if(iter != loc_com.begin() &&
           std::prev(iter)->first.file_name() == iter->first.file_name())
        {
            retval << color::bold << color::blue << "\n ...\n" << color::reset;
        }
        else // if filename differs, print " --> filename.toml"
        {
            if(iter != loc_com.begin()) {retval << '\n';}

            retval << color::bold << color::blue << " --> " << color::reset
                   << iter->first.file_name() << '\n';
            // add one almost-empty line for readability
            retval << make_string(static_cast<std::size_t>(line_num_width + 1), ' ')
                   << color::bold << color::blue << " | "  << color::reset << '\n';
        }
        const source_location& loc = iter->first;
        const std::string& comment = iter->second;

        retval << ' ' << color::bold << color::blue << std::setw(line_num_width)
               << std::right << loc.line() << " | "  << color::reset
               << loc.line_str() << '\n';

        retval << make_string(static_cast<std::size_t>(line_num_width + 1), ' ')
               << color::bold << color::blue << " | " << color::reset
               << make_string(loc.column()-1 /*1-origin*/, ' ');

        if(loc.region() == 1)
        {
            // invalid
            // ^------
            retval << color::bold << color::red << "^---" << color::reset;
        }
        else
        {
            // invalid
            // ~~~~~~~
            const auto underline_len = (std::min)(
                static_cast<std::size_t>(loc.region()), loc.line_str().size());
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
#endif// TOML11_SOURCE_LOCATION_HPP

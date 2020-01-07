//     Copyright Toru Niina 2019.
// Distributed under the MIT License.
#ifndef TOML11_SOURCE_LOCATION_HPP
#define TOML11_SOURCE_LOCATION_HPP
#include "region.hpp"
#include <cstdint>

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

} // toml
#endif// TOML11_SOURCE_LOCATION_HPP

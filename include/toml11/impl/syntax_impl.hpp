#ifndef TOML11_SYNTAX_IMPL_HPP
#define TOML11_SYNTAX_IMPL_HPP

#include "../fwd/syntax_fwd.hpp"
#include "../scanner.hpp"
#include "../spec.hpp"
#include "../version.hpp"

namespace toml
{
inline namespace TOML11_INLINE_VERSION_NAMESPACE
{
namespace detail
{
namespace syntax
{

using char_type = location::char_type;

template<typename F>
struct syntax_cache
{
    using value_type = cxx::return_type_of_t<F, const spec&>;
    static_assert(std::is_base_of<scanner_base, value_type>::value, "");

    explicit syntax_cache(F f)
        : func_(std::move(f)), cache_(cxx::make_nullopt())
    {}

    value_type const& at(const spec& s)
    {
        if( ! this->cache_.has_value() || this->cache_.value().first != s)
        {
            this->cache_ = std::make_pair(s, func_(s));
        }
        return this->cache_.value().second;
    }

  private:
    F func_;
    cxx::optional<std::pair<spec, value_type>> cache_;
};

template<typename F>
syntax_cache<cxx::remove_cvref_t<F>> make_cache(F&& f)
{
    return syntax_cache<cxx::remove_cvref_t<F>>(std::forward<F>(f));
}

// ===========================================================================
// UTF-8

// avoid redundant representation and out-of-unicode sequence

TOML11_INLINE character_in_range const& utf8_1byte(const spec&)
{
    static thread_local character_in_range cache(0x00, 0x7F);
    return cache;
}

TOML11_INLINE sequence const& utf8_2bytes(const spec&)
{
    static thread_local sequence cache(
            character_in_range(0xC2, 0xDF),
            character_in_range(0x80, 0xBF));
    return cache;
}

TOML11_INLINE sequence const& utf8_3bytes(const spec&)
{
    static thread_local sequence cache(/*1~2 bytes = */either(
        sequence(character         (0xE0),       character_in_range(0xA0, 0xBF)),
        sequence(character_in_range(0xE1, 0xEC), character_in_range(0x80, 0xBF)),
        sequence(character         (0xED),       character_in_range(0x80, 0x9F)),
        sequence(character_in_range(0xEE, 0xEF), character_in_range(0x80, 0xBF))
    ), /*3rd byte = */ character_in_range(0x80, 0xBF));

    return cache;
}

TOML11_INLINE sequence const& utf8_4bytes(const spec&)
{
    static thread_local sequence cache(/*1~2 bytes = */either(
        sequence(character         (0xF0),       character_in_range(0x90, 0xBF)),
        sequence(character_in_range(0xF1, 0xF3), character_in_range(0x80, 0xBF)),
        sequence(character         (0xF4),       character_in_range(0x80, 0x8F))
    ), character_in_range(0x80, 0xBF), character_in_range(0x80, 0xBF));

    return cache;
}

// ===========================================================================
// Whitespace

TOML11_INLINE character_either const& wschar(const spec&)
{
    static thread_local character_either cache(" \t");
    return cache;
}

TOML11_INLINE repeat_at_least const& ws(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s){
        return repeat_at_least(0, wschar(s));
    });
    return cache.at(sp);
}

// ===========================================================================
// Newline

TOML11_INLINE either const& newline(const spec&)
{
    static thread_local either cache(character(char_type('\n')), literal("\r\n"));
    return cache;
}

// ===========================================================================
// Comments

TOML11_INLINE either const& allowed_comment_char(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s){
            if(s.ext_allow_control_characters_in_comments)
            {
                return either(
                    character_in_range(0x01, 0x09),
                    character_in_range(0x0E, 0x7F),
                    non_ascii(s)
                );
            }
            else
            {
                return either(
                    character(0x09),
                    character_in_range(0x20, 0x7E),
                    non_ascii(s)
                );
            }
        });
    return cache.at(sp);
}

// XXX Note that it does not take newline
TOML11_INLINE sequence const& comment(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s){
        return sequence(character(char_type('#')),
                    repeat_at_least(0, allowed_comment_char(s)));
    });
    return cache.at(sp);
}

// optional('#') + allowed comment chars + optional('\n' or '\r\n')
TOML11_INLINE comment_line_scanner const& comment_line(const spec &sp, const bool include_comment_symbol, const bool include_newline)
{
    if (include_comment_symbol)
    {
        if (include_newline)
        {
            static thread_local auto cache = make_cache([](const spec& s) {
                return comment_line_scanner(s, true, true);
            });
            return cache.at(sp);
        }
        else
        {
            static thread_local auto cache = make_cache([](const spec& s) {
                return comment_line_scanner(s, true, false);
            });
            return cache.at(sp);
        }
    }
    else
    {
        if (include_newline)
        {
            static thread_local auto cache = make_cache([](const spec& s) {
                return comment_line_scanner(s, false, true);
            });
            return cache.at(sp);
        }
        else
        {
            static thread_local auto cache = make_cache([](const spec& s) {
                return comment_line_scanner(s, false, false);
            });
            return cache.at(sp);
        }
    }
}

TOML11_INLINE region comment_line_scanner::scan(location &loc) const
{
    if (loc.eof())
    {
        return region{};
    }
    if (include_comment_symbol_)
    {
        const auto c_comment_symbol = loc.current();
        if (c_comment_symbol != '#')
        {
            return region {};
        }
    }

    const auto first = loc;
    loc.advance(1);

    while (!loc.eof())
    {
        const auto c = loc.current();

        // in ascii range
        if (c < 0x80)
        {
            if (ext_allow_control_characters_in_comments_)
            {
                // toml11 ext allow 0x01-0x09, 0x0E-0x7F
                if ((c >= 0x01 && c <= 0x09) || (c >= 0x0E && c <= 0x7F))
                {
                    loc.advance(1);
                    continue;
                }
            }
            else
            {
                // v1.0.0 allow 0x09, 0x20-0x7E
                if (c == 0x09 || (c >= 0x20 && c <= 0x7E))
                {
                    loc.advance(1);
                    continue;
                }
            }

            if (!include_newline_) { break; }
            if (c == '\n')
            {
                loc.advance(1);
            }
            else if (c == '\r')
            {
                loc.advance(1); if (loc.eof()|| loc.current() != '\n') { loc = first; break; }
                loc.advance(1);
            }
            break;
        }

        // utf-8 multi bytes
        if (!try_advance_multi_utf8(loc, c))
            break;
    }

    if (loc.get_location() == first.get_location())
    {
        return region{};
    }
    return region(first, loc);
}

TOML11_INLINE const bool comment_line_scanner::try_advance_multi_utf8(location &loc, location::char_type c1) const
{
    if (c1 < 0xC2) { return false; }

    const auto first = loc;
    loc.advance(1);
    if (loc.eof()) { loc = first; return false; }

    const auto c2 = loc.current();

    if (c1 <= 0xDF)
    {
        // 2 bytes: [0xC2, 0xDF][0x80, 0xBF]
        if (c2 < 0x80 || c2 > 0xBF) { loc = first; return false; }
        loc.advance(1);
        return true;
    }

    if (c1 <= 0xEF)
    {
        // 3 bytes: [0xE0][0xA0, 0xBF][0x80, 0xBF] | [0xE1, 0xEC][0x80, 0xBF][0x80, 0xBF] | [0xED][0x80, 0x9F][0x80, 0xBF] | [0xEE, 0xEF][0x80, 0xBF][0x80, 0xBF]
        bool ok = false;
        if (c1 == 0xE0)
            ok = (c2 >= 0xA0 && c2 <= 0xBF);
        else if (c1 == 0xED)
            ok = (c2 >= 0x80 && c2 <= 0x9F);
        else
            ok = (c2 >= 0x80 && c2 <= 0xBF);

        if (!ok) { loc = first; return false; }

        loc.advance(1);
        if (loc.eof()) { loc = first; return false; }

        const auto c3 = loc.current();
        if (c3 < 0x80 || c3 > 0xBF) { loc = first; return false; }
        loc.advance(1);
        return true;
    }

    if (c1 <= 0xF4)
    {
        // 4 bytes: [0xF0][0x90, 0xBF][0x80, 0xBF][0x80, 0xBF] | [0xF1, 0xF3][0x80, 0xBF][0x80, 0xBF][0x80, 0xBF] | [0xF4][0x80, 0x8F][0x80, 0xBF][0x80, 0xBF]
        bool ok = false;
        if (c1 == 0xF0)
            ok = (c2 >= 0x90 && c2 <= 0xBF);
        else if (c1 == 0xF4)
            ok = (c2 >= 0x80 && c2 <= 0x8F);
        else
            ok = (c2 >= 0x80 && c2 <= 0xBF);

        if (!ok) { loc = first; return false; }

        loc.advance(1); if (loc.eof()) { loc = first; return false; }

        const auto c3 = loc.current();
        if (c3 < 0x80 || c3 > 0xBF) { loc = first; return false; }
        loc.advance(1); if (loc.eof()) { loc = first; return false; }

        const auto c4 = loc.current();
        if (c4 < 0x80 || c4 > 0xBF) { loc = first; return false; }
        loc.advance(1);
        return true;
    }

    // only c1 > 0xF4 reaches here.
    // kept out of the early check to keep the common path to one compare.
    return false;
}

// ===========================================================================
// Boolean

TOML11_INLINE either const& boolean(const spec&)
{
    static thread_local either cache(literal("true"), literal("false"));
    return cache;
}

// ===========================================================================
// Integer

// non-digit-graph = ([a-zA-Z]|unicode mb char)
// graph           = ([a-zA-Z0-9]|unicode mb char)
// suffix          = _ non-digit-graph (graph | _graph)
TOML11_INLINE sequence const& num_suffix(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto non_digit_graph = [&s]() {
            return either(
                alpha(s),
                non_ascii(s)
            );
        };
        const auto graph = [&s]() {
            return either(
                alpha(s),
                digit(s),
                non_ascii(s)
            );
        };

        return sequence(
                character(char_type('_')),
                non_digit_graph(),
                repeat_at_least(0,
                    either(
                        sequence(character(char_type('_')), graph()),
                        graph()
                    )
                )
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& dec_int(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto digit19 = []() {
            return character_in_range(char_type('1'), char_type('9'));
        };
        return sequence(
                maybe(character_either("+-")),
                either(
                    sequence(
                        digit19(),
                        repeat_at_least(1,
                            either(
                                digit(s),
                                sequence(character(char_type('_')), digit(s))
                            )
                        )
                    ),
                    digit(s)
                )
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& hex_int(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                literal("0x"),
                hexdig(s),
                repeat_at_least(0,
                    either(
                        hexdig(s),
                        sequence(character(char_type('_')), hexdig(s))
                    )
                )
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& oct_int(const spec& s)
{
    static thread_local auto cache = make_cache([](const spec&) {
        const auto digit07 = []() {
            return character_in_range(char_type('0'), char_type('7'));
        };
        return sequence(
                literal("0o"),
                digit07(),
                repeat_at_least(0,
                    either(
                        digit07(),
                        sequence(character(char_type('_')), digit07())
                    )
                )
            );
        });
    return cache.at(s);
}

TOML11_INLINE sequence const& bin_int(const spec& s)
{
    static thread_local auto cache = make_cache([](const spec&) {
        const auto digit01 = []() {
            return character_either("01");
        };
        return sequence(
                literal("0b"),
                digit01(),
                repeat_at_least(0,
                    either(
                        digit01(),
                        sequence(character(char_type('_')), digit01())
                    )
                )
            );
        });
    return cache.at(s);
}

TOML11_INLINE either const& integer(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(
                hex_int(s),
                oct_int(s),
                bin_int(s),
                dec_int(s)
            );
        });
    return cache.at(sp);
}


// ===========================================================================
// Floating

TOML11_INLINE sequence const& zero_prefixable_int(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                digit(s),
                repeat_at_least(0,
                    either(
                        digit(s),
                        sequence(character('_'), digit(s))
                    )
                )
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& fractional_part(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                character('.'),
                zero_prefixable_int(s)
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& exponent_part(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                character_either("eE"),
                maybe(character_either("+-")),
                zero_prefixable_int(s)
            );
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& hex_floating(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        // C99 hexfloat (%a)
        // [+-]? 0x ( [0-9a-fA-F]*\.[0-9a-fA-F]+ | [0-9a-fA-F]+\.? ) [pP] [+-]? [0-9]+

        // - 0x(int).(frac)p[+-](int)
        // - 0x(int).p[+-](int)
        // - 0x.(frac)p[+-](int)
        // - 0x(int)p[+-](int)

        return sequence(
                maybe(character_either("+-")),
                character('0'),
                character_either("xX"),
                either(
                    sequence(
                        repeat_at_least(0, hexdig(s)),
                        character('.'),
                        repeat_at_least(1, hexdig(s))
                    ),
                    sequence(
                        repeat_at_least(1, hexdig(s)),
                        maybe(character('.'))
                    )
                ),
                character_either("pP"),
                maybe(character_either("+-")),
                repeat_at_least(1, character_in_range('0', '9'))
            );
        });
    return cache.at(sp);
}

TOML11_INLINE either const& floating(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(
                sequence(
                    dec_int(s),
                    either(
                        exponent_part(s),
                        sequence(fractional_part(s), maybe(exponent_part(s)))
                    )
                ),
                sequence(
                    maybe(character_either("+-")),
                    either(literal("inf"), literal("nan"))
                )
            );
        });
    return cache.at(sp);
}

// ===========================================================================
// Datetime

TOML11_INLINE sequence const& local_date(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                repeat_exact(4, digit(s)),
                character('-'),
                repeat_exact(2, digit(s)),
                character('-'),
                repeat_exact(2, digit(s))
            );
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& local_time(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        if(s.v1_1_0_make_seconds_optional)
        {
            return sequence(
                repeat_exact(2, digit(s)),
                character(':'),
                repeat_exact(2, digit(s)),
                maybe(sequence(
                    character(':'),
                    repeat_exact(2, digit(s)),
                    maybe(sequence(character('.'), repeat_at_least(1, digit(s))))
                )));
        }
        else
        {
            return sequence(
                repeat_exact(2, digit(s)),
                character(':'),
                repeat_exact(2, digit(s)),
                character(':'),
                repeat_exact(2, digit(s)),
                maybe(sequence(character('.'), repeat_at_least(1, digit(s))))
            );
        }
        });
    return cache.at(sp);
}
TOML11_INLINE either const& time_offset(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(
                character_either("zZ"),
                sequence(character_either("+-"),
                         repeat_exact(2, digit(s)),
                         character(':'),
                         repeat_exact(2, digit(s))
                 )
            );
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& full_time(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(local_time(s), time_offset(s));
        });
    return cache.at(sp);
}
TOML11_INLINE character_either const& time_delim(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec&) {
            return character_either("Tt ");
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& local_datetime(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(local_date(s), time_delim(s), local_time(s));
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& offset_datetime(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(local_date(s), time_delim(s), full_time(s));
        });
    return cache.at(sp);
}

// ===========================================================================
// String

TOML11_INLINE sequence const& escaped_x2(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(character('x'), repeat_exact(2, hexdig(s)));
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& escaped_u4(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(character('u'), repeat_exact(4, hexdig(s)));
        });
    return cache.at(sp);
}
TOML11_INLINE sequence const& escaped_U8(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
            return sequence(character('U'), repeat_exact(8, hexdig(s)));
        });
    return cache.at(sp);
}

TOML11_INLINE sequence const& escaped(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto escape_char = [&s] {
            if(s.v1_1_0_add_escape_sequence_e)
            {
                return character_either("\"\\bfnrte");
            }
            else
            {
                return character_either("\"\\bfnrt");
            }
        };

        const auto escape_seq = [&s, &escape_char] {
            if(s.v1_1_0_add_escape_sequence_x)
            {
                return either(
                    escape_char(),
                    escaped_u4(s),
                    escaped_U8(s),
                    escaped_x2(s)
                );
            }
            else
            {
                return either(
                    escape_char(),
                    escaped_u4(s),
                    escaped_U8(s)
                );
            }
        };

        return sequence(character('\\'), escape_seq());
    });
    return cache.at(sp);
}

TOML11_INLINE either const& basic_char(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto basic_unescaped = [&s]() {
            return either(
                    wschar(s),
                    character(0x21),                // 22 is "
                    character_in_range(0x23, 0x5B), // 5C is backslash
                    character_in_range(0x5D, 0x7E), // 7F is DEL
                    non_ascii(s)
                );
        };
        return either(basic_unescaped(), escaped(s));
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& basic_string(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                character('"'),
                repeat_at_least(0, basic_char(s)),
                character('"')
            );
    });
    return cache.at(sp);
}

// ---------------------------------------------------------------------------
// multiline string

TOML11_INLINE sequence const& escaped_newline(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                character('\\'), ws(s), newline(s),
                repeat_at_least(0, either(wschar(s), newline(s)))
            );
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& ml_basic_string(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto mlb_content = [&s]() {
            return either(basic_char(s), newline(s), escaped_newline(s));
        };
        const auto mlb_quotes = []() {
            return either(literal("\"\""), character('\"'));
        };

        return sequence(
                literal("\"\"\""),
                maybe(newline(s)),
                repeat_at_least(0, mlb_content()),
                repeat_at_least(0,
                    sequence(
                        mlb_quotes(),
                        repeat_at_least(1, mlb_content())
                    )
                ),
                // XXX """ and mlb_quotes are intentionally reordered to avoid
                //     unexpected match of mlb_quotes
                literal("\"\"\""),
                maybe(mlb_quotes())
            );
    });
    return cache.at(sp);
}

// ---------------------------------------------------------------------------
// literal string

TOML11_INLINE either const& literal_char(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(
                character         (0x09),
                character_in_range(0x20, 0x26),
                character_in_range(0x28, 0x7E),
                non_ascii(s)
            );
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& literal_string(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
                character('\''),
                repeat_at_least(0, literal_char(s)),
                character('\'')
            );
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& ml_literal_string(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto mll_quotes = []() {
            return either(literal("''"), character('\''));
        };
        const auto mll_content = [&s]() {
            return either(literal_char(s), newline(s));
        };

        return sequence(
                literal("'''"),
                maybe(newline(s)),
                repeat_at_least(0, mll_content()),
                repeat_at_least(0, sequence(
                        mll_quotes(),
                        repeat_at_least(1, mll_content())
                    )
                ),
                literal("'''"),
                maybe(mll_quotes())
                // XXX ''' and mll_quotes are intentionally reordered to avoid
                //     unexpected match of mll_quotes
            );
    });
    return cache.at(sp);
}

TOML11_INLINE either const& string(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(
                ml_basic_string(s),
                ml_literal_string(s),
                basic_string(s),
                literal_string(s)
            );
    });
    return cache.at(sp);
}

// ===========================================================================
// Keys

// to keep `expected_chars` simple
TOML11_INLINE non_ascii_key_char::non_ascii_key_char(const spec& s) noexcept
{
    assert(s.ext_allow_non_english_in_bare_keys);
    (void)s; // for NDEBUG
}

TOML11_INLINE std::uint32_t non_ascii_key_char::read_utf8(location& loc) const
{
    // U+0000   ... U+0079  ; 0xxx_xxxx
    // U+0080   ... U+07FF  ; 110y_yyyx 10xx_xxxx;
    // U+0800   ... U+FFFF  ; 1110_yyyy 10yx_xxxx 10xx_xxxx
    // U+010000 ... U+10FFFF; 1111_0yyy 10yy_xxxx 10xx_xxxx 10xx_xxxx

    const unsigned char b1 = loc.current(); loc.advance(1);
    if(b1 < 0x80)
    {
        return static_cast<std::uint32_t>(b1);
    }
    else if((b1 >> 5) == 6) // 0b110 == 6
    {
        const auto b2 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 5) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t codep = (c1 << 6) + c2;

        if(codep < 0x80)
        {
            return 0xFFFFFFFF;
        }
        return codep;
    }
    else if((b1 >> 4) == 14) // 0b1110 == 14
    {
        const auto b2 = loc.current(); loc.advance(1); if(loc.eof()) {return 0xFFFFFFFF;}
        const auto b3 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 4) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t c3 = b3 & ((1 << 6) - 1);

        const std::uint32_t codep = (c1 << 12) + (c2 << 6) + c3;
        if(codep < 0x800)
        {
            return 0xFFFFFFFF;
        }
        return codep;
    }
    else if((b1 >> 3) == 30) // 0b11110 == 30
    {
        const auto b2 = loc.current(); loc.advance(1); if(loc.eof()) {return 0xFFFFFFFF;}
        const auto b3 = loc.current(); loc.advance(1); if(loc.eof()) {return 0xFFFFFFFF;}
        const auto b4 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 3) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t c3 = b3 & ((1 << 6) - 1);
        const std::uint32_t c4 = b4 & ((1 << 6) - 1);
        const std::uint32_t codep = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4;

        if(codep < 0x10000)
        {
            return 0xFFFFFFFF;
        }
        return codep;
    }
    else // not a Unicode codepoint in UTF-8
    {
        return 0xFFFFFFFF;
    }
}

TOML11_INLINE region non_ascii_key_char::scan(location& loc) const
{
    if(loc.eof()) {return region{};}

    const auto first = loc;

    const auto cp = read_utf8(loc);

    if(cp == 0xFFFFFFFF)
    {
        loc = first;
        return region{};
    }

    // ALPHA / DIGIT / %x2D / %x5F    ; a-z A-Z 0-9 - _
    // / %xB2 / %xB3 / %xB9 / %xBC-BE ; superscript digits, fractions
    // / %xC0-D6 / %xD8-F6 / %xF8-37D ; non-symbol chars in Latin block
    // / %x37F-1FFF                   ; exclude GREEK QUESTION MARK, which is basically a semi-colon
    // / %x200C-200D / %x203F-2040    ; from General Punctuation Block, include the two tie symbols and ZWNJ, ZWJ
    // / %x2070-218F / %x2460-24FF    ; include super-/subscripts, letterlike/numberlike forms, enclosed alphanumerics
    // / %x2C00-2FEF / %x3001-D7FF    ; skip arrows, math, box drawing etc, skip 2FF0-3000 ideographic up/down markers and spaces
    // / %xF900-FDCF / %xFDF0-FFFD    ; skip D800-DFFF surrogate block, E000-F8FF Private Use area, FDD0-FDEF intended for process-internal use (unicode)
    // / %x10000-EFFFF                ; all chars outside BMP range, excluding Private Use planes (F0000-10FFFF)

    if(cp == 0xB2 || cp == 0xB3 || cp == 0xB9 || (0xBC <= cp && cp <= 0xBE) ||
       (0xC0    <= cp && cp <= 0xD6  ) || (0xD8 <= cp && cp <= 0xF6) || (0xF8 <= cp && cp <= 0x37D) ||
       (0x37F   <= cp && cp <= 0x1FFF) ||
       (0x200C  <= cp && cp <= 0x200D) || (0x203F <= cp && cp <= 0x2040) ||
       (0x2070  <= cp && cp <= 0x218F) || (0x2460 <= cp && cp <= 0x24FF) ||
       (0x2C00  <= cp && cp <= 0x2FEF) || (0x3001 <= cp && cp <= 0xD7FF) ||
       (0xF900  <= cp && cp <= 0xFDCF) || (0xFDF0 <= cp && cp <= 0xFFFD) ||
       (0x10000 <= cp && cp <= 0xEFFFF) )
    {
        return region(first, loc);
    }
    loc = first;
    return region{};
}

// when error: no rollback
// when success: advance
// don't check if first byte is EOF
// code from non_ascii_key_char::read_utf8
TOML11_INLINE const std::uint32_t bare_key_scanner::advance_multi_utf8_bytes(location& loc) const {
    // U+0000   ... U+0079  ; 0xxx_xxxx
    // U+0080   ... U+07FF  ; 110y_yyyx 10xx_xxxx;
    // U+0800   ... U+FFFF  ; 1110_yyyy 10yx_xxxx 10xx_xxxx
    // U+010000 ... U+10FFFF; 1111_0yyy 10yy_xxxx 10xx_xxxx 10xx_xxxx

    const char_type b1 = loc.current();
    assert(b1 >= 0x80);

    loc.advance(1);
    if (loc.eof()) { return 0xFFFFFFFF; }

    if ((b1 >> 5) == 6) // 0b110 == 6
    {
        const auto b2 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 5) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t codep = (c1 << 6) + c2;

        if (codep < 0x80) { return 0xFFFFFFFF; }
        return codep;
    } else if ((b1 >> 4) == 14) // 0b1110 == 14
    {
        const auto b2 = loc.current(); loc.advance(1); if (loc.eof()) { return 0xFFFFFFFF; }
        const auto b3 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 4) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t c3 = b3 & ((1 << 6) - 1);

        const std::uint32_t codep = (c1 << 12) + (c2 << 6) + c3;
        if (codep < 0x80) { return 0xFFFFFFFF; }

        return codep;
    } else if ((b1 >> 3) == 30) // 0b11110 == 30
    {
        const auto b2 = loc.current(); loc.advance(1); if (loc.eof()) { return 0xFFFFFFFF; }
        const auto b3 = loc.current(); loc.advance(1); if (loc.eof()) { return 0xFFFFFFFF; }
        const auto b4 = loc.current(); loc.advance(1);

        const std::uint32_t c1 = b1 & ((1 << 3) - 1);
        const std::uint32_t c2 = b2 & ((1 << 6) - 1);
        const std::uint32_t c3 = b3 & ((1 << 6) - 1);
        const std::uint32_t c4 = b4 & ((1 << 6) - 1);
        const std::uint32_t codep = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4;
        if (codep < 0x80) { return 0xFFFFFFFF; }

        return codep;
    } else // not a Unicode codepoint in UTF-8
    {
        return 0xFFFFFFFF;
    }
}

// when error: rollback
// when success: advance
// don't check if first byte is EOF
TOML11_INLINE const bool bare_key_scanner::try_advance(location& loc) const
{
    const char_type c = loc.current();
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == 0x2D) || (c == 0x5F))
    {
        loc.advance(1);
        return true;
    }

    if (this->ext_allow_non_english_in_bare_keys_ && c >= 0x80)
    {
        const auto first = loc;
        const std::uint32_t cp = advance_multi_utf8_bytes(loc);
        // see non_ascii_key_char::scan
        if (cp == 0xB2 || cp == 0xB3 || cp == 0xB9 || (0xBC <= cp && cp <= 0xBE) ||
            (0xC0 <= cp && cp <= 0xD6) || (0xD8 <= cp && cp <= 0xF6) ||
            (0xF8 <= cp && cp <= 0x37D) || (0x37F <= cp && cp <= 0x1FFF) ||
            (0x200C <= cp && cp <= 0x200D) || (0x203F <= cp && cp <= 0x2040) ||
            (0x2070 <= cp && cp <= 0x218F) || (0x2460 <= cp && cp <= 0x24FF) ||
            (0x2C00 <= cp && cp <= 0x2FEF) || (0x3001 <= cp && cp <= 0xD7FF) ||
            (0xF900 <= cp && cp <= 0xFDCF) || (0xFDF0 <= cp && cp <= 0xFFFD) ||
            (0x10000 <= cp && cp <= 0xEFFFF))
        {
            return true;
        }

        loc = first;
        return false;
    }

    return false;
}

TOML11_INLINE repeat_at_least const& unquoted_key(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        const auto keychar = [&s] {
            if(s.ext_allow_non_english_in_bare_keys)
            {
                return either(alpha(s), digit(s), character{0x2D}, character{0x5F},
                              non_ascii_key_char(s));
            }
            else
            {
                return either(alpha(s), digit(s), character{0x2D}, character{0x5F});
            }
        };
        return repeat_at_least(1, keychar());
    });
    return cache.at(sp);
}

TOML11_INLINE bare_key_scanner const& unquoted_key_v2(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return bare_key_scanner(s);
    });
    return cache.at(sp);
}

TOML11_INLINE either const& quoted_key(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
    return either(basic_string(s), literal_string(s));
    });
    return cache.at(sp);
}

TOML11_INLINE either const& simple_key(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return either(unquoted_key_v2(s), quoted_key(s));
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& dot_sep(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(ws(s), character('.'), ws(s));
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& dotted_key(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(
            simple_key(s),
            repeat_at_least(1, sequence(dot_sep(s), simple_key(s)))
        );
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& keyval_sep(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(ws(s), character('='), ws(s));
    });
    return cache.at(sp);
}

// ===========================================================================
// Table key

TOML11_INLINE sequence const& std_table(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(character('['), ws(s), key(s), ws(s), character(']'));
    });
    return cache.at(sp);
}

TOML11_INLINE sequence const& array_table(const spec& sp)
{
    static thread_local auto cache = make_cache([](const spec& s) {
        return sequence(literal("[["), ws(s), key(s), ws(s), literal("]]"));
    });
    return cache.at(sp);
}

// ===========================================================================
// extension: null

TOML11_INLINE literal const& null_value(const spec&)
{
    static thread_local literal cache("null");
    return cache;
}

} // namespace syntax
} // namespace detail
} // TOML11_INLINE_VERSION_NAMESPACE
} // namespace toml
#endif // TOML11_SYNTAX_IMPL_HPP

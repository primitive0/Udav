module;

#include <cassert>
#include <utility>

#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.eval.sema:eval_literals_pass;

import udav.runtime;

import :common;

namespace udav {

class StringLiteralParser final
{
public:
    explicit StringLiteralParser(StrView literal)
        : current_{literal.begin()}
        , end_{literal.end()}
    {
    }

    auto parse() -> UdavString
    {
        assert(
            chars_left() >= 2 &&
            *current_ == '"' &&
            *(end_ - 1) == '"' &&
            "Input string in StringLiteralParser must be quoted.");

        ++current_;
        --end_;

        return parse_literal_content();
    }

private:
    auto parse_literal_content() -> UdavString
    {
        auto result = String{};

        // Given that literal content length >= actual string length, and
        // string literals rarely contain many escape sequences, this
        // String::reserve() call can be a useful heuristic.
        result.reserve(chars_left());

        while (current_ != end_) {
            if (*current_ == '\\') {
                ++current_;
                assert(current_ != end_ && "Unexpected EOF in string literal.");

                auto ch = decode_escape(*current_);
                if (!ch) {
                    throw PassException{};
                }
                result.push_back(*ch);
            } else {
                result.push_back(*current_);
            }

            ++current_;
        }

        return UdavString{std::move(result)};
    }

    auto chars_left() const -> size_t
    {
        return end_ - current_;
    }

    static auto decode_escape(char escaped) -> Option<char>
    {
        switch (escaped) {
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case '0':
            return '\0';
        case '\\':
            return '\\';
        case '"':
            return '"';
        default:
            return std::nullopt;
        }
    }

    const char* current_;
    const char* end_;
};

auto parse_string_literal(StrView literal) -> UdavString
{
    return StringLiteralParser{literal}.parse();
}

TEST_CASE("parse_string_literal parses correct literals", "[sema]")
{
    using namespace std::string_view_literals;

    struct TC
    {
        StrView input;
        StrView expected;
    };

    auto [input, expected] = GENERATE(
        TC{R"("")", ""},
        TC{R"("foo")", "foo"},
        TC{R"("Юникод тоже.")", "Юникод тоже."},

        // Escape sequences
        TC{R"("\"")", "\""},
        TC{R"("\\")", "\\"},
        TC{R"("\n")", "\n"},
        TC{R"("\r")", "\r"},
        TC{R"("\t")", "\t"},
        TC{R"("\0")", "\0"sv},
        TC{R"("\\\\\\")", R"(\\\)"},
        TC{R"("Mix \n \t \\\\ \0 ed")", "Mix \n \t \\\\ \0 ed"sv},
        TC{R"("Hello\nПривет!\n漢 🤗")", "Hello\nПривет!\n漢 🤗"});

    CHECK(parse_string_literal(input) == expected);
}

TEST_CASE("parse_string_literal throws on invalid escape sequence", "[sema]")
{
    auto input = GENERATE(
        as<StrView>{},
        R"("\x")",
        R"("\5")",
        R"("\q")",
        R"("\foo")",
        R"("\t\0 \2")");

    CHECK_THROWS_AS(parse_string_literal(input), PassException);
}

export class EvalLiteralsPass final
{
};

} // namespace udav

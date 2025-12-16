module;

#include <boost/multiprecision/cpp_int.hpp>

#include "support/option.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.sema.literal_parsing;

import udav.runtime;

namespace udav {

export class StringLiteralParser final
{
public:
    explicit StringLiteralParser() = default;

    auto parse(StrView literal) -> Option<UdavString>
    {
        assert(
            literal.size() >= 2 &&
            literal.front() == '"' &&
            literal.back() == '"' &&
            "Input string in StringLiteralParser must be quoted.");

        return parse_literal_content(literal.substr(1, literal.size() - 2));
    }

private:
    auto parse_literal_content(StrView content) -> Option<UdavString>
    {
        auto result = String{};

        // Given that literal content length >= actual string length, and
        // string literals rarely contain many escape sequences, this
        // String::reserve() call can be a useful heuristic.
        result.reserve(content.size());

        auto current = content.cbegin();
        while (current != content.cend()) {
            if (*current == '\\') {
                ++current;
                assert(current != content.cend() && "Unexpected EOF in string literal.");

                auto ch = decode_escape(*current);
                if (!ch) {
                    return std::nullopt;
                }
                result.push_back(*ch);
            } else {
                result.push_back(*current);
            }

            ++current;
        }

        return UdavString{std::move(result)};
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
};

export class IntegerLiteralParser final
{
public:
    explicit IntegerLiteralParser() = default;

    auto parse(StrView literal) -> Option<UdavInteger>
    {
        assert(!literal.empty() && "Integer literal must not be empty.");

        auto input = literal;
        while (input.size() != 1 && input[0] == '0') {
            input = input.substr(1);
        }

        return UdavInteger{boost::multiprecision::cpp_int{input}};
    }
};

TEST_CASE("StringLiteralParser parses correct literals", "[sema]")
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

    auto result = StringLiteralParser{}.parse(input);

    CHECK((result && StrView(*result) == expected));
}

TEST_CASE("StringLiteralParser does not parse invalid escape sequences", "[sema]")
{
    auto input = GENERATE(
        as<StrView>{},
        R"("\x")",
        R"("\5")",
        R"("\q")",
        R"("\foo")",
        R"("\t\0 \2")");

    CHECK(!StringLiteralParser().parse(input));
}

TEST_CASE("IntegerLiteralParser parses correct literals", "[sema]")
{
    struct TC
    {
        StrView input;
        boost::multiprecision::cpp_int expected;
    };

    // clang-format off
    auto [input, expected] = GENERATE(
        TC{"0",          0},
        TC{"0000",       0},
        TC{"0009",       9}, // This must be parsed as decimal
        TC{"0123",       123},
        TC{"1234567890", 1234567890});
    // clang-format on

    auto result = IntegerLiteralParser{}.parse(input);
    CHECK((result && result->value() == expected));
}

} // namespace udav

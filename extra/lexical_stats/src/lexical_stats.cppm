module;

#include "support/numerics.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>

export module udav.lexical_stats;

import udav.lexer;

namespace udav::lexer::stats {

auto is_keyword(TokenKind kind) -> bool
{
    using enum TokenKind;

    switch (kind) {
    case Fun:
    case Return:
    case Pass:
    case Let:
    case If:
    case Elif:
    case Else:
    case While:
        return true;
    default:
        return false;
    }
}

auto is_operator(TokenKind kind) -> bool
{
    using enum TokenKind;

    switch (kind) {
    case Equals:
    case NotEquals:
    case Less:
    case Greater:
    case LessOrEqual:
    case GreaterOrEqual:
    case Plus:
    case Minus:
    case Mul:
    case Div:
    case Modulo:
    case Power:
    case Or:
    case And:
    case BitwiseOr:
    case BitwiseAnd:
    case BitwiseXor:
    case RightShift:
    case LeftShift:
    case Assign:
    case PlusAssign:
    case MinusAssign:
    case MulAssign:
    case DivAssign:
    case ModuloAssign:
    case PowerAssign:
    case BitwiseOrAssign:
    case BitwiseAndAssign:
    case BitwiseXorAssign:
    case RightShiftAssign:
    case LeftShiftAssign:
        return true;
    default:
        return false;
    }
}

export struct LexicalStats
{
    size_t keywords = 0;
    size_t symbols = 0;
    size_t number_literals = 0;
    size_t string_literals = 0;
    size_t operators = 0;
    size_t other_tokens = 0;
};

export auto collect(StrView text) -> LexicalStats
{
    auto stats = LexicalStats{};

    auto lexer = Lexer{text};
    while (true) {
        auto token = lexer.next();

        if (is_keyword(token.kind)) {
            ++stats.keywords;
        } else if (token.kind == TokenKind::Symbol) {
            ++stats.symbols;
        } else if (token.kind == TokenKind::IntegerLiteral) {
            ++stats.number_literals;
        } else if (token.kind == TokenKind::StringLiteral) {
            ++stats.string_literals;
        } else if (is_operator(token.kind)) {
            ++stats.operators;
        } else {
            ++stats.other_tokens;
        }

        if (token.kind == TokenKind::Eof) {
            break;
        }
    }

    return stats;
}

TEST_CASE("stats::collect counts tokens in simple program", "[lex stats]")
{
    auto s = stats::collect(
        "fun main():\n"                           // kw: 1, sym: 1, oth: 4
        "    let x = 1\n"                         // kw: 1, sym: 1, num: 1, op: 1 oth: 2
        "    let y = 2\n"                         // kw: 1, sym: 1, num: 1, op: 1 oth: 1
        "    io.println(\"sum = {}\", x + y)\n"); // sym: 4, str: 1, op: 1, oth: 5
                                                  // oth: 2

    // clang-format off
    CHECK(s.keywords        == size_t{ 3});
    CHECK(s.symbols         == size_t{ 7});
    CHECK(s.number_literals == size_t{ 2});
    CHECK(s.string_literals == size_t{ 1});
    CHECK(s.operators       == size_t{ 3});
    CHECK(s.other_tokens    == size_t{14});
    // clang-format on
}

TEST_CASE("stats::collect counts tokens in FizzBuzz program", "[lex stats]")
{
    auto s = stats::collect(
        "# FizzBuzz program\n"                   // oth: 2
        "fun main():\n"                          // kw: 1, sym: 1, oth: 4
        "    let n = 1\n"                        // kw: 1, sym: 1, num: 1, op: 1, oth: 2
        "    while n <= 100:\n"                  // kw: 1, sym: 1, num: 1, op: 1, oth: 2
        "        if n % 15 == 0:\n"              // kw: 1, sym: 1, num: 2, op: 2, oth: 3
        "            io.println(\"FizzBuzz\")\n" // sym: 2, str: 1, oth: 5
        "        elif n % 3 == 0:\n"             // kw: 1, sym: 1, num: 2, op: 2, oth: 3
        "            io.println(\"Fizz\")\n"     // sym: 2, str: 1, oth: 5
        "        elif n % 5 == 0:\n"             // kw: 1, sym: 1, num: 2, op: 2, oth: 3
        "            io.println(\"Buzz\")\n"     // sym: 2, str: 1, oth: 5
        "        else:\n"                        // kw: 1, oth: 3
        "            io.println(\"{}\", n)\n"    // sym: 3, str: 1, oth: 6
        "        n += 1\n");                     // sym: 1, num: 1, op: 1, oth: 2
                                                 // oth: 3

    // clang-format off
    CHECK(s.keywords        == size_t{ 7});
    CHECK(s.symbols         == size_t{16});
    CHECK(s.number_literals == size_t{ 9});
    CHECK(s.string_literals == size_t{ 4});
    CHECK(s.operators       == size_t{ 9});
    CHECK(s.other_tokens    == size_t{48});
    // clang-format on
}

} // namespace udav::lexer::stats

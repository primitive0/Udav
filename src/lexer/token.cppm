module;

#include <cassert>
#include <ostream>
#include <sstream>

#include <magic_enum/magic_enum.hpp>

#include "support/string.hpp"

#include <doctest/doctest.h>

export module udav.lexer:token;

namespace udav {

export enum class TokenKind {
    // Keywords
    Fun,
    Return,
    Pass,
    Let,
    If,
    Elif,
    Else,
    While,
    Continue,
    Break,
    False,
    True,

    Symbol,

    // Operators
    Not,              // !
    Equals,           // ==
    NotEquals,        // !=
    Less,             // <
    Greater,          // >
    LessOrEqual,      // <=
    GreaterOrEqual,   // >=
    Plus,             // +
    Minus,            // -
    Mul,              // *
    Div,              // /
    Modulo,           // %
    Power,            // **
    Or,               // ||
    And,              // &&
    BitwiseOr,        // |
    BitwiseAnd,       // &
    BitwiseXor,       // ^
    RightShift,       // >>
    LeftShift,        // <<
    Assign,           // =
    PlusAssign,       // +=
    MinusAssign,      // -=
    MulAssign,        // *=
    DivAssign,        // /=
    ModuloAssign,     // %=
    PowerAssign,      // **=
    BitwiseOrAssign,  // |=
    BitwiseAndAssign, // &=
    BitwiseXorAssign, // ^=
    RightShiftAssign, // >>=
    LeftShiftAssign,  // <<=

    // Auxiliary tokens
    Colon,      // :
    Dot,        // .
    Comma,      // ,
    ParenOpen,  // (
    ParenClose, // )

    // Literals
    IntegerLiteral,
    StringLiteral,

    Comment,

    NewLine,
    Indent,
    Dedent,
    Eof,
};

export struct Token final
{
    TokenKind kind;
    StrView span;

    constexpr explicit Token(TokenKind kind, StrView span)
        : kind{kind}
        , span{span}
    {
    }

    constexpr auto operator==(const Token&) const -> bool = default;
};

export auto operator<<(std::ostream& os, TokenKind kind) -> std::ostream&
{
    return os << magic_enum::enum_name(kind);
}

export auto operator<<(std::ostream& os, const Token& token) -> std::ostream&
{
    os << token.kind;

    if (!token.span.empty()) {
        os << " `" << token.span << "`";
    }

    return os;
}

TEST_CASE("udav::TokenKind is formatted to string")
{
    auto ss = std::ostringstream{};

    ss << TokenKind::Fun << "\n"
       << TokenKind::Return << "\n"
       << TokenKind::Power << "\n";

    CHECK(ss.str() == "Fun\nReturn\nPower\n");
}

TEST_CASE("udav::Token is formatted to string")
{
    auto ss = std::ostringstream{};

    ss << Token{TokenKind::Break, "break"} << "\n"
       << Token{TokenKind::Symbol, "foobar"} << "\n";

    CHECK(ss.str() == "Break `break`\nSymbol `foobar`\n");
}

} // namespace udav

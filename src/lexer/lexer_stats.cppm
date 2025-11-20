// TODO: fix module formatting everywhere
module;

#include "support/numerics.hpp"
#include "support/string.hpp"

export module udav.lexer.stats;

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

} // namespace udav::lexer::stats

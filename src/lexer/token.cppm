module;
#include <cassert>
#include <ostream>

#include "support/string.hpp"

export module udav.lexer:token;

namespace udav::lexer {

#define ENUM_ITEM_DECLARE(Item) Item,

#define ENUM_ITEM_CASE(Item) \
    case Item:               \
        return os << #Item;

#define DEFINE_TOKEN_ENUM(EnumName, ENUM_ITEMS)                            \
    export enum class EnumName { ENUM_ITEMS(ENUM_ITEM_DECLARE) };          \
                                                                           \
    export auto operator<<(std::ostream& os, EnumName kind)->std::ostream& \
    {                                                                      \
        using enum EnumName;                                               \
                                                                           \
        switch (kind) {                                                    \
            ENUM_ITEMS(ENUM_ITEM_CASE)                                     \
        default:                                                           \
            assert(false);                                                 \
        }                                                                  \
    }

#define TOKEN_KIND_ITEMS(X)       \
    /* Keywords */                \
    X(Fun)                        \
    X(Return)                     \
    X(Pass)                       \
    X(Let)                        \
    X(If)                         \
    X(Elif)                       \
    X(Else)                       \
    X(While)                      \
    X(Continue)                   \
    X(Break)                      \
    X(False)                      \
    X(True)                       \
                                  \
    X(Symbol)                     \
                                  \
    /* Operators */               \
    X(Not)              /* !   */ \
    X(Equals)           /* ==  */ \
    X(NotEquals)        /* !=  */ \
    X(Less)             /* <   */ \
    X(Greater)          /* >   */ \
    X(LessOrEqual)      /* <=  */ \
    X(GreaterOrEqual)   /* >=  */ \
    X(Plus)             /* +   */ \
    X(Minus)            /* -   */ \
    X(Mul)              /* *   */ \
    X(Div)              /* /   */ \
    X(Modulo)           /* %   */ \
    X(Power)            /* **  */ \
    X(Or)               /* ||  */ \
    X(And)              /* &&  */ \
    X(BitwiseOr)        /* |   */ \
    X(BitwiseAnd)       /* &   */ \
    X(BitwiseXor)       /* ^   */ \
    X(RightShift)       /* >>  */ \
    X(LeftShift)        /* <<  */ \
    X(Assign)           /* =   */ \
    X(PlusAssign)       /* +=  */ \
    X(MinusAssign)      /* -=  */ \
    X(MulAssign)        /* *=  */ \
    X(DivAssign)        /* /=  */ \
    X(ModuloAssign)     /* %=  */ \
    X(PowerAssign)      /* **= */ \
    X(BitwiseOrAssign)  /* |=  */ \
    X(BitwiseAndAssign) /* &=  */ \
    X(BitwiseXorAssign) /* ^=  */ \
    X(RightShiftAssign) /* >>= */ \
    X(LeftShiftAssign)  /* <<= */ \
                                  \
    /* Auxiliary tokens */        \
    X(Colon)      /* :  */        \
    X(Dot)        /* .  */        \
    X(Comma)      /* ,  */        \
    X(ParenOpen)  /* (  */        \
    X(ParenClose) /* )  */        \
                                  \
    /* Literals */                \
    X(IntegerLiteral)             \
    X(StringLiteral)              \
                                  \
    X(Comment)                    \
                                  \
    X(NewLine)                    \
    X(Indent)                     \
    X(Dedent)                     \
    X(Eof)

DEFINE_TOKEN_ENUM(TokenKind, TOKEN_KIND_ITEMS)

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

export auto operator<<(std::ostream& os, const Token& token) -> std::ostream&
{
    os << token.kind;

    if (!token.span.empty()) {
        os << " `" << token.span << "`";
    }

    return os;
}

} // namespace udav::lexer

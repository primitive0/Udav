module;

#include <utility>

export module udav.parsing:token_stream;

import udav.lexer;

namespace udav {

// NOTE: Moving leaves this object in an unspecified state
export class SemanticTokenStream final
{
public:
    explicit SemanticTokenStream(Lexer lexer)
        : lexer_{std::move(lexer)}
        , primed_{false}
        , peeked_{TokenKind::Eof, ""}
    {
    }

    [[nodiscard]]
    auto peek() -> Token
    {
        ensure_primed();
        return peeked_;
    }

    auto advance() -> void
    {
        ensure_primed();
        peeked_ = next_semantic_token();
    }

private:
    Lexer lexer_;
    bool primed_;
    Token peeked_;

    auto ensure_primed() -> void
    {
        if (primed_) {
            return;
        }
        peeked_ = next_semantic_token();
        primed_ = true;
    }

    auto next_semantic_token() -> Token
    {
        auto token = lexer_.next();
        while (token.kind == TokenKind::Comment) {
            token = lexer_.next();
        }
        return token;
    }
};

} // namespace udav

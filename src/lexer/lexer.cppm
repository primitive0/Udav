module;
#include <exception>

#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.lexer;

export import :token;

namespace udav::lexer {

export class LexerException : public std::exception
{
};

export class InconsistentLineEndingException final : public LexerException
{
};

export class Lexer final
{
public:
    explicit Lexer(strview text) {}

    auto next() -> Token
    {
        return Token{TokenKind::Eof, ""};
    }

private:
};

export auto collect_tokens(Lexer lexer) -> Vec<Token>
{
    auto tokens = Vec<Token>{};
    while (true) {
        auto token = lexer.next();
        tokens.push_back(token);
        if (token.kind == TokenKind::Eof) {
            break;
        }
    }
    return tokens;
}

} // namespace udav::lexer

module;
#include <cassert>
#include <exception>
#include <format>
#include <limits>
#include <type_traits>
#include <utility>

#include <utf8.h>

#include "support/deque.hpp"
#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.lexer;

export import :token;

import udav.text.utf8;

// TODO: custom assert macros

namespace udav::lexer {

constexpr char32_t kNoChar32 = std::numeric_limits<char32_t>::max();

// TODO: which exception class should I inherit?
export class LexerException : public std::exception
{
public:
    explicit LexerException()
        : message_{}
    {
    }

    explicit LexerException(String message)
        : message_{std::move(message)}
    {
    }

    auto what() const noexcept -> const char* override
    {
        return message_.c_str();
    }

private:
    String message_;
};

export class InvalidUtf8Exception final : public LexerException
{
};

export class InconsistentDedentException final : public LexerException
{
};

export class UnexpectedCharacterException final : public LexerException
{
public:
    explicit UnexpectedCharacterException(String message_)
        : LexerException(std::move(message_))
    {
    }
};

class TextSplitter final
{
public:
    explicit TextSplitter(StrView text)
        : begin_{text.begin()}
        , cursor_{text.begin()}
        , end_{text.end()}
    {
    }

    [[nodiscard]]
    auto peek() -> char32_t
    {
        if (peeked_ != kNoChar32) {
            return peeked_;
        }

        if (cursor_ == end_) {
            return kNoChar32;
        }

        try {
            peeked_ = utf8::peek_next(cursor_, end_);
        } catch (utf8::exception&) {
            throw InvalidUtf8Exception{};
        }

        return peeked_;
    }

    // TODO: make advance() read peeked_, peek() returns peeked_
    auto advance() -> void
    {
        if (peeked_ != kNoChar32) {
            cursor_ += get_utf8_sequence_size_for_char(peeked_);
            peeked_ = kNoChar32;
            return;
        }

        if (cursor_ == end_) {
            return;
        }

        try {
            utf8::next(cursor_, end_);
        } catch (utf8::exception&) {
            throw InvalidUtf8Exception{};
        }
    }

    auto match(StrView str) -> bool
    {
        assert(!str.empty());

        if (str.size() > end_ - cursor_) {
            return false;
        }
        for (auto i = 0uz; i < str.size(); ++i) {
            if (str[i] != cursor_[i]) {
                return false;
            }
        }

        cursor_ += str.size();
        peeked_ = kNoChar32;
        return true;
    }

    auto split() -> StrView
    {
        auto span = StrView{begin_, cursor_};
        begin_ = cursor_;
        return span;
    }

private:
    static auto get_utf8_sequence_size_for_char(char32_t ch) -> i32
    {
        if (ch < 0x80) {
            return 1;
        } else if (ch < 0x800) {
            return 2;
        } else if (ch < 0x10000) {
            return 3;
        } else if (ch < 0x110000) {
            return 4;
        } else {
            assert(false && "ch must be Unicode code point.");
        }
    }

    const char* begin_;
    const char* cursor_;
    const char* end_;
    char32_t peeked_ = kNoChar32;
};

export class Lexer final
{
public:
    Lexer(Lexer&&) noexcept = default;
    Lexer& operator=(Lexer&&) noexcept = default;

    // TODO: fix formatting
    Lexer(const Lexer&) = delete ("Lexer is move-only type.");
    Lexer& operator=(const Lexer&) = delete ("Lexer is move-only type.");

    explicit Lexer(StrView text)
        : splitter_{text}
    {
    }

    // TODO: provide strong exception guarantee
    // NOTE: currently has no exception guarantees
    auto next() -> Token
    {
        if (!pending_tokens_.empty()) {
            return pop_token();
        }

        if (part_ == PartKind::Beginning) {
            skip_utf8_bom_if_needed();
            part_ = PartKind::Indentation;
        }

        if (splitter_.peek() == kNoChar32) {
            return on_eof();
        }

        return find_next_token();
    }

private:
    enum class PartKind {
        Beginning,
        Indentation,
        Text,
    };

    auto on_eof() -> Token
    {
        if (part_ == PartKind::Text) {
            part_ = PartKind::Indentation;
            return Token{TokenKind::NewLine, ""};
        }

        if (indent_stack_.size() > 1) {
            change_indentation_level(0);
            return pop_token();
        }

        return Token{TokenKind::Eof, ""};
    }

    auto find_next_token() -> Token
    {
        auto num_spaces = skip_spaces();

        if (parse_newline_token() ||
            parse_comment_token()) {
            return pop_token();
        }

        if (part_ == PartKind::Indentation) {
            change_indentation_level(num_spaces);
            part_ = PartKind::Text;
        }

        if (parse_keyword_or_symbol_token() ||
            parse_auxiliary_token() ||
            parse_operator_token() ||
            parse_integer_literal_token() ||
            parse_string_literal_token()) {
            return pop_token();
        }

        throw UnexpectedCharacterException{
            std::format(
                "Unexpected character U+{:04X}.",
                static_cast<u32>(splitter_.peek()))};
    }

    auto parse_newline_token() -> bool
    {
        auto ch = splitter_.peek();
        if (ch != U'\n' && ch != U'\r') {
            return false;
        }

        splitter_.advance();
        if (ch == U'\r' && splitter_.peek() == '\n') {
            splitter_.advance();
        }

        part_ = PartKind::Indentation; // Moving to next line
        push_virtual_token(TokenKind::NewLine);
        return true;
    }

    auto parse_comment_token() -> bool
    {
        if (splitter_.peek() != U'#') {
            return false;
        }

        auto ch = char32_t{};
        do {
            splitter_.advance();
            ch = splitter_.peek();
        } while (ch != U'\n' && ch != U'\r' && ch != kNoChar32);

        part_ = PartKind::Text;
        push_token(TokenKind::Comment);
        return true;
    }

    auto change_indentation_level(i64 new_level) -> void
    {
        if (indent_stack_.back() < new_level) {
            indent_stack_.push_back(new_level);
            push_indent();
            return;
        }

        while (new_level < indent_stack_.back()) {
            indent_stack_.pop_back();
            push_dedent();
        }

        if (new_level != indent_stack_.back()) {
            throw InconsistentDedentException{};
        }

        assert(
            !indent_stack_.empty() &&
            indent_stack_[0] == 0 &&
            "Indentation stack must have 0 as first element.");
    }

    auto parse_keyword_or_symbol_token() -> bool
    {
        auto ch = splitter_.peek();
        if (!udav::text::utf8::is_ascii_alphabetic(ch) && ch != U'_') {
            return false;
        }

        do {
            splitter_.advance();
            ch = splitter_.peek();
        } while (udav::text::utf8::is_ascii_alphanumeric(ch) || ch == '_');

        auto span = splitter_.split();
        pending_tokens_.push_front(
            Token{
                get_keyword_token_kind(span).value_or(TokenKind::Symbol),
                span});
        return true;
    }

    auto parse_auxiliary_token() -> bool
    {
        auto ch = splitter_.peek();
        if (auto kind = get_auxiliary_token_kind(ch); kind) {
            splitter_.advance();
            push_token(*kind);
            return true;
        }
        return false;
    }

    auto parse_operator_token() -> bool
    {
        if (splitter_.match("**=")) {
            push_token(TokenKind::PowerAssign);
            return true;
        } else if (splitter_.match(">>=")) {
            push_token(TokenKind::RightShiftAssign);
            return true;
        } else if (splitter_.match("<<=")) {
            push_token(TokenKind::LeftShiftAssign);
            return true;
        }

        if (splitter_.match("==")) {
            push_token(TokenKind::Equals);
            return true;
        } else if (splitter_.match("!=")) {
            push_token(TokenKind::NotEquals);
            return true;
        } else if (splitter_.match("<=")) {
            push_token(TokenKind::LessOrEqual);
            return true;
        } else if (splitter_.match(">=")) {
            push_token(TokenKind::GreaterOrEqual);
            return true;
        } else if (splitter_.match("**")) {
            push_token(TokenKind::Power);
            return true;
        } else if (splitter_.match("||")) {
            push_token(TokenKind::Or);
            return true;
        } else if (splitter_.match("&&")) {
            push_token(TokenKind::And);
            return true;
        } else if (splitter_.match(">>")) {
            push_token(TokenKind::RightShift);
            return true;
        } else if (splitter_.match("<<")) {
            push_token(TokenKind::LeftShift);
            return true;
        } else if (splitter_.match("+=")) {
            push_token(TokenKind::PlusAssign);
            return true;
        } else if (splitter_.match("-=")) {
            push_token(TokenKind::MinusAssign);
            return true;
        } else if (splitter_.match("*=")) {
            push_token(TokenKind::MulAssign);
            return true;
        } else if (splitter_.match("/=")) {
            push_token(TokenKind::DivAssign);
            return true;
        } else if (splitter_.match("%=")) {
            push_token(TokenKind::ModuloAssign);
            return true;
        } else if (splitter_.match("|=")) {
            push_token(TokenKind::BitwiseOrAssign);
            return true;
        } else if (splitter_.match("&=")) {
            push_token(TokenKind::BitwiseAndAssign);
            return true;
        } else if (splitter_.match("^=")) {
            push_token(TokenKind::BitwiseXorAssign);
            return true;
        }

        if (splitter_.match("<")) {
            push_token(TokenKind::Less);
            return true;
        } else if (splitter_.match(">")) {
            push_token(TokenKind::Greater);
            return true;
        } else if (splitter_.match("+")) {
            push_token(TokenKind::Plus);
            return true;
        } else if (splitter_.match("-")) {
            push_token(TokenKind::Minus);
            return true;
        } else if (splitter_.match("!")) {
            push_token(TokenKind::Not);
            return true;
        } else if (splitter_.match("*")) {
            push_token(TokenKind::Mul);
            return true;
        } else if (splitter_.match("/")) {
            push_token(TokenKind::Div);
            return true;
        } else if (splitter_.match("%")) {
            push_token(TokenKind::Modulo);
            return true;
        } else if (splitter_.match("|")) {
            push_token(TokenKind::BitwiseOr);
            return true;
        } else if (splitter_.match("&")) {
            push_token(TokenKind::BitwiseAnd);
            return true;
        } else if (splitter_.match("^")) {
            push_token(TokenKind::BitwiseXor);
            return true;
        } else if (splitter_.match("=")) {
            push_token(TokenKind::Assign);
            return true;
        }

        return false;
    }

    auto parse_integer_literal_token() -> bool
    {
        auto ch = splitter_.peek();
        if (!udav::text::utf8::is_ascii_digit(ch)) {
            return false;
        }

        do {
            splitter_.advance();
            ch = splitter_.peek();
        } while (udav::text::utf8::is_ascii_digit(ch));

        push_token(TokenKind::IntegerLiteral);
        return true;
    }

    auto parse_string_literal_token() -> bool
    {
        auto ch = splitter_.peek();
        if (ch != U'"') {
            return false;
        }

        do {
            splitter_.advance();
            ch = splitter_.peek();

            if (ch == U'\n' || ch == U'\r') {
                throw UnexpectedCharacterException{
                    "Line breaks inside string literal are not allowed."};
            }

            if (ch == kNoChar32) {
                throw UnexpectedCharacterException{
                    "EOF inside string literal is not allowed."};
            }
        } while (ch != U'"');
        splitter_.advance();

        push_token(TokenKind::StringLiteral);
        return true;
    }

    auto skip_spaces() -> i64
    {
        auto num_spaces = i64{0};

        while (splitter_.peek() == U' ') {
            ++num_spaces;
            splitter_.advance();
        }

        splitter_.split();
        return num_spaces;
    }

    auto skip_utf8_bom_if_needed() -> void
    {
        if (splitter_.peek() == udav::text::utf8::kBom) {
            splitter_.advance();
            splitter_.split();
        }
    }

    auto push_token(TokenKind kind) -> void
    {
        pending_tokens_.push_front(Token{kind, splitter_.split()});
    }

    auto push_virtual_token(TokenKind kind) -> void
    {
        splitter_.split();
        pending_tokens_.push_front(Token{kind, ""});
    }

    auto push_indent() -> void
    {
        pending_tokens_.push_front(Token{TokenKind::Indent, ""});
    }

    auto push_dedent() -> void
    {
        pending_tokens_.push_front(Token{TokenKind::Dedent, ""});
    }

    auto pop_token() -> Token
    {
        assert(!pending_tokens_.empty() && "Cannot pop empty pending token buffer.");

        auto token = pending_tokens_.back();
        pending_tokens_.pop_back();
        return token;
    }

    static auto get_keyword_token_kind(StrView str) -> Option<TokenKind>
    {
        if (str == "fun") {
            return TokenKind::Fun;
        } else if (str == "return") {
            return TokenKind::Return;
        } else if (str == "pass") {
            return TokenKind::Pass;
        } else if (str == "let") {
            return TokenKind::Let;
        } else if (str == "if") {
            return TokenKind::If;
        } else if (str == "elif") {
            return TokenKind::Elif;
        } else if (str == "else") {
            return TokenKind::Else;
        } else if (str == "while") {
            return TokenKind::While;
        } else if (str == "false") {
            return TokenKind::False;
        } else if (str == "true") {
            return TokenKind::True;
        }

        return std::nullopt;
    }

    static auto get_auxiliary_token_kind(char32_t ch) -> Option<TokenKind>
    {
        switch (ch) {
        case U':':
            return TokenKind::Colon;
        case U'.':
            return TokenKind::Dot;
        case U',':
            return TokenKind::Comma;
        case U'(':
            return TokenKind::ParenOpen;
        case U')':
            return TokenKind::ParenClose;
        default:
            return std::nullopt;
        }
    }

    Deque<Token> pending_tokens_{};

    TextSplitter splitter_;

    PartKind part_ = PartKind::Beginning;
    Vec<i64> indent_stack_{0};
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

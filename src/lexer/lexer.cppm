module;

#include <array>
#include <cassert>
#include <exception>
#include <format>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>

#include <utf8.h>

#include "support/deque.hpp"
#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/pair.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

export module udav.lexer;

export import :token;

import udav.text.utf8;

// TODO: custom assert macros

namespace udav {

constexpr char32_t kNoChar32 = std::numeric_limits<char32_t>::max();

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
public:
    using LexerException::LexerException;

    explicit InvalidUtf8Exception() = default;
};

export class InconsistentDedentException final : public LexerException
{
public:
    using LexerException::LexerException;

    explicit InconsistentDedentException() = default;
};

export class UnexpectedCharacterException final : public LexerException
{
public:
    using LexerException::LexerException;

    explicit UnexpectedCharacterException() = default;
};

// TODO: check move constructor is correct, write test case for this
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
    auto operator=(Lexer&&) noexcept -> Lexer& = default;

    Lexer(const Lexer&) = delete;
    auto operator=(const Lexer&) -> Lexer& = delete;

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
        if (!is_first_symbol_char(ch)) {
            return false;
        }

        do {
            splitter_.advance();
            ch = splitter_.peek();
        } while (is_symbol_char(ch));

        auto span = splitter_.split();
        pending_tokens_.push_front(
            Token{
                get_keyword_token_kind(span).value_or(TokenKind::Symbol),
                span});
        return true;
    }

    auto parse_auxiliary_token() -> bool
    {
        static constexpr auto kTokenTable = std::to_array<Pair<StrView, TokenKind>>({
            Pair{":", TokenKind::Colon},
            Pair{".", TokenKind::Dot},
            Pair{",", TokenKind::Comma},
            Pair{"(", TokenKind::ParenOpen},
            Pair{")", TokenKind::ParenClose},
        });

        return parse_token_using_table(kTokenTable);
    }

    auto parse_operator_token() -> bool
    {
        static constexpr auto kTokenTable = std::to_array<Pair<StrView, TokenKind>>({
            {"**=", TokenKind::PowerAssign},
            {">>=", TokenKind::RightShiftAssign},
            {"<<=", TokenKind::LeftShiftAssign},

            {"==", TokenKind::Equals},
            {"!=", TokenKind::NotEquals},
            {"<=", TokenKind::LessOrEqual},
            {">=", TokenKind::GreaterOrEqual},
            {"**", TokenKind::Power},
            {"||", TokenKind::Or},
            {"&&", TokenKind::And},
            {">>", TokenKind::RightShift},
            {"<<", TokenKind::LeftShift},
            {"+=", TokenKind::PlusAssign},
            {"-=", TokenKind::MinusAssign},
            {"*=", TokenKind::MulAssign},
            {"/=", TokenKind::DivAssign},
            {"%=", TokenKind::ModuloAssign},
            {"|=", TokenKind::BitwiseOrAssign},
            {"&=", TokenKind::BitwiseAndAssign},
            {"^=", TokenKind::BitwiseXorAssign},

            {"<", TokenKind::Less},
            {">", TokenKind::Greater},
            {"+", TokenKind::Plus},
            {"-", TokenKind::Minus},
            {"!", TokenKind::Not},
            {"*", TokenKind::Mul},
            {"/", TokenKind::Div},
            {"%", TokenKind::Modulo},
            {"|", TokenKind::BitwiseOr},
            {"&", TokenKind::BitwiseAnd},
            {"^", TokenKind::BitwiseXor},
            {"=", TokenKind::Assign},
        });

        return parse_token_using_table(kTokenTable);
    }

    template<size_t N>
    auto parse_token_using_table(
        const std::array<Pair<StrView, TokenKind>, N>& table)
        -> bool
    {
        for (const auto& entry : table) {
            if (splitter_.match(entry.first)) {
                push_token(entry.second);
                return true;
            }
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
        splitter_.advance();

        auto escaped = false;
        ch = splitter_.peek();
        while (ch != U'"' || escaped) {
            if (ch == U'\n' || ch == U'\r') {
                throw UnexpectedCharacterException{
                    "Line breaks inside string literal are not allowed."};
            }

            if (ch == kNoChar32) {
                throw UnexpectedCharacterException{
                    "EOF inside string literal is not allowed."};
            }

            escaped = !escaped && ch == U'\\';

            splitter_.advance();
            ch = splitter_.peek();
        }
        splitter_.advance(); // Include closing " into span

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

    static auto is_first_symbol_char(char32_t ch) -> bool
    {
        return udav::text::utf8::is_ascii_alphabetic(ch) || ch == '_';
    }

    static auto is_symbol_char(char32_t ch) -> bool
    {
        return udav::text::utf8::is_ascii_alphanumeric(ch) || ch == '_';
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
        } else if (str == "continue") {
            return TokenKind::Continue;
        } else if (str == "break") {
            return TokenKind::Break;
        } else if (str == "false") {
            return TokenKind::False;
        } else if (str == "true") {
            return TokenKind::True;
        }

        return std::nullopt;
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

namespace {

template<typename R>
auto expect_tokens(StrView code, R&& expected) -> void
{
    CHECK_THAT(
        collect_tokens(Lexer(code)),
        Catch::Matchers::RangeEquals(expected));
}

auto expect_tokens(StrView code, std::initializer_list<Token> expected) -> void
{
    expect_tokens<std::initializer_list<Token>&>(code, expected);
}

auto expect_single_token(StrView code, Token expected_token) -> void
{
    expect_tokens(
        code,
        {
            expected_token,
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

template<typename E>
auto expect_lex_throws(StrView code) -> void
{
    CHECK_THROWS_AS(collect_tokens(Lexer(code)), E);
}

TEST_CASE("Lexer parses empty source text", "[lexer]")
{
    expect_tokens("", {Token{TokenKind::Eof, ""}});
}

TEST_CASE("Lexer ignores BOM at the beginning", "[lexer]")
{
    expect_tokens(
        "\xEF\xBB\xBF"
        "println(\"hello\")\n",
        {
            Token{TokenKind::Symbol, "println"},
            Token{TokenKind::ParenOpen, "("},
            Token{TokenKind::StringLiteral, "\"hello\""},
            Token{TokenKind::ParenClose, ")"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer rejects BOM when not at start of input", "[lexer]")
{
    expect_lex_throws<UnexpectedCharacterException>(
        "fun main():\n"
        "\x{EF}\x{BB}\x{BF}pass\n");

    expect_lex_throws<UnexpectedCharacterException>(
        " \x{EF}\x{BB}\x{BF}fun main():\n");
}

TEST_CASE("Lexer fails on invalid UTF-8", "[lexer]")
{
    expect_lex_throws<InvalidUtf8Exception>("\x80");

    expect_lex_throws<InvalidUtf8Exception>("\xC0\xAF");

    expect_lex_throws<InvalidUtf8Exception>("let x = \xD0");
}

TEST_CASE("Lexer parses keyword tokens", "[lexer]")
{
    // Test case
    struct TC
    {
        StrView keyword;
        TokenKind kind;
    };

    // clang-format off
    auto [keyword, kind] = GENERATE(
        TC{"fun",      TokenKind::Fun},
        TC{"return",   TokenKind::Return},
        TC{"pass",     TokenKind::Pass},
        TC{"let",      TokenKind::Let},
        TC{"if",       TokenKind::If},
        TC{"elif",     TokenKind::Elif},
        TC{"else",     TokenKind::Else},
        TC{"while",    TokenKind::While},
        TC{"continue", TokenKind::Continue},
        TC{"break",    TokenKind::Break},
        TC{"false",    TokenKind::False},
        TC{"true",     TokenKind::True});
    // clang-format on

    expect_single_token(keyword, Token{kind, keyword});
}

TEST_CASE("Lexer parses symbols", "[lexer]")
{
    auto symbol = GENERATE(
        as<StrView>{},
        "l",
        "L",
        "foo",
        "i1234567890",
        "fizz_buzz",
        "_",
        "___",
        "__i__love__udav",
        "CaseCase");

    expect_single_token(symbol, Token{TokenKind::Symbol, symbol});
}

TEST_CASE("Keyword tokens are case dependent", "[lexer]")
{
    struct TC
    {
        StrView word;
        TokenKind kind;
    };

    auto [word, kind] = GENERATE(
        TC{"fun", TokenKind::Fun},
        TC{"fUn", TokenKind::Symbol});

    expect_single_token(word, Token{kind, word});
}

TEST_CASE("Lexer parsers operator tokens", "[lexer]")
{
    struct TC
    {
        StrView op;
        TokenKind kind;
    };

    // clang-format off
    auto [op, kind] = GENERATE(
        TC{"!",   TokenKind::Not},
        TC{"==",  TokenKind::Equals},
        TC{"!=",  TokenKind::NotEquals},
        TC{"<",   TokenKind::Less},
        TC{">",   TokenKind::Greater},
        TC{"<=",  TokenKind::LessOrEqual},
        TC{">=",  TokenKind::GreaterOrEqual},
        TC{"+",   TokenKind::Plus},
        TC{"-",   TokenKind::Minus},
        TC{"*",   TokenKind::Mul},
        TC{"/",   TokenKind::Div},
        TC{"%",   TokenKind::Modulo},
        TC{"**",  TokenKind::Power},
        TC{"||",  TokenKind::Or},
        TC{"&&",  TokenKind::And},
        TC{"|",   TokenKind::BitwiseOr},
        TC{"&",   TokenKind::BitwiseAnd},
        TC{"^",   TokenKind::BitwiseXor},
        TC{">>",  TokenKind::RightShift},
        TC{"<<",  TokenKind::LeftShift},
        TC{"=",   TokenKind::Assign},
        TC{"+=",  TokenKind::PlusAssign},
        TC{"-=",  TokenKind::MinusAssign},
        TC{"*=",  TokenKind::MulAssign},
        TC{"/=",  TokenKind::DivAssign},
        TC{"%=",  TokenKind::ModuloAssign},
        TC{"**=", TokenKind::PowerAssign},
        TC{"|=",  TokenKind::BitwiseOrAssign},
        TC{"&=",  TokenKind::BitwiseAndAssign},
        TC{"^=",  TokenKind::BitwiseXorAssign},
        TC{">>=", TokenKind::RightShiftAssign},
        TC{"<<=", TokenKind::LeftShiftAssign}
    );
    // clang-format on

    expect_single_token(op, Token{kind, op});
}

TEST_CASE("Lexer parses auxiliary tokens", "[lexer]")
{
    struct TC
    {
        StrView text;
        TokenKind kind;
    };

    auto [text, kind] = GENERATE(
        TC{":", TokenKind::Colon},
        TC{".", TokenKind::Dot},
        TC{",", TokenKind::Comma},
        TC{"(", TokenKind::ParenOpen},
        TC{")", TokenKind::ParenClose});

    expect_single_token(text, Token{kind, text});
}

TEST_CASE("Adjacent operator and auxiliary tokens are parsed correctly", "[lexer]")
{
    SECTION("Adjacent operators and compound assignments")
    {
        struct TC
        {
            StrView op;
            TokenKind op_kind;
            TokenKind assign_kind;
        };

        // clang-format off
        auto [op, op_kind, assign_kind] = GENERATE(
            TC{"+",  TokenKind::Plus,       TokenKind::PlusAssign},
            TC{"-",  TokenKind::Minus,      TokenKind::MinusAssign},
            TC{"/",  TokenKind::Div,        TokenKind::DivAssign},
            TC{"%",  TokenKind::Modulo,     TokenKind::ModuloAssign},
            TC{"**", TokenKind::Power,      TokenKind::PowerAssign},
            TC{"^",  TokenKind::BitwiseXor, TokenKind::BitwiseXorAssign},
            TC{">>", TokenKind::RightShift, TokenKind::RightShiftAssign},
            TC{"<<", TokenKind::LeftShift,  TokenKind::LeftShiftAssign});
        // clang-format on

        auto assign = std::format("{}=", op);

        expect_tokens(
            std::format("{0}{0}{0}{0}{1}", op, assign),
            {
                Token{op_kind, op},
                Token{op_kind, op},
                Token{op_kind, op},
                Token{op_kind, op},
                Token{assign_kind, assign},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });
    }

    SECTION("Adjacent relational operators")
    {
        expect_tokens(
            "<==",
            {
                Token{TokenKind::LessOrEqual, "<="},
                Token{TokenKind::Assign, "="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "<=>",
            {
                Token{TokenKind::LessOrEqual, "<="},
                Token{TokenKind::Greater, ">"},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "!==",
            {
                Token{TokenKind::NotEquals, "!="},
                Token{TokenKind::Assign, "="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            ">==",
            {
                Token{TokenKind::GreaterOrEqual, ">="},
                Token{TokenKind::Assign, "="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "<===",
            {
                Token{TokenKind::LessOrEqual, "<="},
                Token{TokenKind::Equals, "=="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            ">===",
            {
                Token{TokenKind::GreaterOrEqual, ">="},
                Token{TokenKind::Equals, "=="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "!===",
            {
                Token{TokenKind::NotEquals, "!="},
                Token{TokenKind::Equals, "=="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });
    }

    SECTION("Adjacent logical operators")
    {
        expect_tokens(
            "!!=",
            {
                Token{TokenKind::Not, "!"},
                Token{TokenKind::NotEquals, "!="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "&&=",
            {
                Token{TokenKind::And, "&&"},
                Token{TokenKind::Assign, "="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });

        expect_tokens(
            "||=",
            {
                Token{TokenKind::Or, "||"},
                Token{TokenKind::Assign, "="},
                Token{TokenKind::NewLine, ""},
                Token{TokenKind::Eof, ""},
            });
    }
}

TEST_CASE("Lexer parses integer literals", "[lexer]")
{
    auto integer = GENERATE(
        as<StrView>{},
        "123",
        "0000", "007",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1234567890",
        "999999999999999999999999999999999999999999999999999999999999999999999999");

    expect_single_token(integer, Token{TokenKind::IntegerLiteral, integer});
}

TEST_CASE("Minus is not part of integer literal token", "[lexer]")
{
    expect_tokens("-42",
        {
            Token{TokenKind::Minus, "-"},
            Token{TokenKind::IntegerLiteral, "42"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer parses string literals", "[lexer]")
{
    auto string = GENERATE(
        as<StrView>{},
        R"("")",
        R"("hello")",
        R"("      ")",
        R"("   spaces  ")",
        R"("\"")",
        R"(" \" backslash \" \" backslashes! \"\"\" slash em \" all!")",
        R"("\n\t\\\"\r\0  \  \  \ \a    \b \c  \\\\\\\\\" mixed!\\\\\\")");

    expect_single_token(string, Token{TokenKind::StringLiteral, string});
}

TEST_CASE("Lexer fails on unterminated string literal", "[lexer]")
{
    expect_lex_throws<UnexpectedCharacterException>("\"hello");
}

TEST_CASE("Line breaks inside string literals are not allowed", "[lexer]")
{
    expect_lex_throws<UnexpectedCharacterException>("\"not\nallowed\"");
    expect_lex_throws<UnexpectedCharacterException>("\"not\rallowed\"");
    expect_lex_throws<UnexpectedCharacterException>("\"not\r\nallowed\"");
}

TEST_CASE("Lexer parses adjacent string literals", "[lexer]")
{
    expect_tokens(
        R"("hello"""  "foo" "bar""baz")",
        {
            Token{TokenKind::StringLiteral, R"("hello")"},
            Token{TokenKind::StringLiteral, R"("")"},
            Token{TokenKind::StringLiteral, R"("foo")"},
            Token{TokenKind::StringLiteral, R"("bar")"},
            Token{TokenKind::StringLiteral, R"("baz")"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer parses comment", "[lexer]")
{
    expect_tokens(
        "#first comment\n"
        "\n"
        "# second comment\n"
        "#\n"
        "#\n"
        "#\n",
        {
            Token{TokenKind::Comment, "#first comment"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Comment, "# second comment"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer parses NewLine token", "[lexer]")
{
    expect_tokens(
        "#\n"
        "#\r\n"
        "#\r"
        "#\n\r",
        {
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""}, // lf
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""}, // crlf
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""}, // cr
            Token{TokenKind::Comment, "#"},
            Token{TokenKind::NewLine, ""}, // lf
            Token{TokenKind::NewLine, ""}, // cr
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Line with only spaces is empty", "[lexer]")
{
    expect_tokens(
        "    \n"
        "        \n"
        " \n",
        {
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer adds final line break if missing", "[lexer]")
{
    expect_tokens(
        "fun",
        {
            Token{TokenKind::Fun, "fun"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Eof, ""},
        });
}

TEST_CASE("Lexer fails on inconsistent dedent", "[lexer]")
{
    expect_lex_throws<InconsistentDedentException>(
        "fun main():\n"
        "    if true:\n"
        "      pass\n"
        "   pass\n"); // Indentation on this line is inconsistent with other lines
}

TEST_CASE("First line can be indented", "[lexer]")
{
    expect_tokens(
        "    fun",
        {
            Token{TokenKind::Indent, ""},
            Token{TokenKind::Fun, "fun"},
            Token{TokenKind::NewLine, ""},
            Token{TokenKind::Dedent, ""},
            Token{TokenKind::Eof, ""},
        });
}

} // namespace

} // namespace udav

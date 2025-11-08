#include <exception>

#include <gtest/gtest.h>

#include "support/numerics.hpp"
#include "support/vector.hpp"

import udav.lexer;

using namespace udav::lexer;

#define EXPECT_TOKENS(code, ...)                               \
    do {                                                       \
        auto tokens = collect_tokens(Lexer{code});             \
        auto expected_tokens = Vec<Token>{__VA_ARGS__};        \
        EXPECT_EQ(tokens.size(), expected_tokens.size());      \
        if (tokens.size() == expected_tokens.size()) {         \
            for (auto i = size_t{0}; i < tokens.size(); ++i) { \
                if (tokens[i] != expected_tokens[i]) {         \
                    ADD_FAILURE()                              \
                        << "Expected " << expected_tokens[i]   \
                        << ", but found " << tokens[i];        \
                    break;                                     \
                }                                              \
            }                                                  \
        }                                                      \
    } while (false)

#define EXPECT_LEX_FAILURE(code, ExceptionType) \
    EXPECT_THROW(collect_tokens(Lexer(code)), ExceptionType)

// NOTE: Я думаю, что от такого количества тестов пострадало
// удобство сопровождения кода. Я также думаю, что тестирование
// каждого токена отдельно не проверяет реальных сценариев
// использования лексера. Вопрос: как сохранить сопровождаемость
// кода и сделать кейсы достаточно реалистичными?

// TODO: maybe make newline tokens greedy?

// TODO: test case: first line can be indented
// TODO: invalid utf-8 encoding test (also bom utf-8 between text)
// TODO: test case: string literal must be closed

// TODO: add more negative tests
// TODO: add test case for trailing spaces and new line token
// TODO: Single line / multi line comment can be empty test case

TEST(LexerTest, LexerParsesEmptyFile)
{
    EXPECT_TOKENS(
        "",
        //
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerIgnoresUtf8BomAtTheBeginning)
{
    EXPECT_TOKENS(
        "\xEF\xBB\xBF"
        "println(\"hello\")\n",
        //
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"hello\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerThrowsOnInconsistentLineEndings)
{
    EXPECT_LEX_FAILURE(
        "println(\"Hello\")\r"
        "println(\"world!\")\r\n",
        InconsistentLineEndingException);

    EXPECT_LEX_FAILURE(
        "println(\"Hello\")\r\n"
        "println(\"world!\")\n",
        InconsistentLineEndingException);

    EXPECT_LEX_FAILURE(
        "println(\"Hello\")\n"
        "println(\"world!\")\r",
        InconsistentLineEndingException);
}

TEST(LexerTest, LexerParsesKeywordTokens)
{
    // fun
    EXPECT_TOKENS(
        "fun do_nothing():\n"
        "    pass\n",
        //
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "do_nothing"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));

    // return
    EXPECT_TOKENS(
        "return 42\n",
        //
        Token(TokenKind::Return, "return"),
        Token(TokenKind::IntegerLiteral, "42"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // pass
    EXPECT_TOKENS(
        "pass\n",
        //
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // let
    EXPECT_TOKENS(
        "let x = 10\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::IntegerLiteral, "10"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // if / elif / else
    EXPECT_TOKENS(
        "if x < 0:\n"
        "    pass\n"
        "elif x > 0:\n"
        "    pass\n"
        "else:\n"
        "    pass\n",
        //
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Less, "<"),
        Token(TokenKind::IntegerLiteral, "0"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::ElseIf, "elif"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Greater, ">"),
        Token(TokenKind::IntegerLiteral, "0"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Else, "else"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));

    // while
    EXPECT_TOKENS(
        "while x < 10:\n",
        Token(TokenKind::While, "while"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Less, "<"),
        Token(TokenKind::IntegerLiteral, "10"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesSymbols)
{
    EXPECT_TOKENS(
        "foo\n",
        //
        Token(TokenKind::Symbol, "foo"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "i1234567890\n",
        //
        Token(TokenKind::Symbol, "i1234567890"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "fizz_buzz\n",
        //
        Token(TokenKind::Symbol, "fizz_buzz"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "__i__love__udav\n",
        //
        Token(TokenKind::Symbol, "__i__love__udav"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "CaseCase\n",
        //
        Token(TokenKind::Symbol, "CaseCase"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, KeywordTokensAreCaseDependent)
{
    EXPECT_TOKENS(
        "fun\n",
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "fUn\n",
        Token(TokenKind::Symbol, "fUn"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesOperatorTokens)
{
    // ==  !=  <  >  <=  >=
    EXPECT_TOKENS(
        "a == b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Equals, "=="),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a != b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::NotEquals, "!="),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a < b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Less, "<"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a > b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Greater, ">"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a <= b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::LessOrEqual, "<="),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a >= b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::GreaterOrEqual, ">="),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // +  -  *  /  %  **
    EXPECT_TOKENS(
        "a + b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Plus, "+"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a - b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Minus, "-"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a * b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Mul, "*"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a / b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Div, "/"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a % b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Modulo, "%"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a ** b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Power, "**"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // ||  &&  |  &  ^  >>  <<
    EXPECT_TOKENS(
        "a || b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Or, "||"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a && b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::And, "&&"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a | b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::BitwiseOr, "|"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a & b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::BitwiseAnd, "&"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a ^ b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::BitwiseXor, "^"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a >> b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::RightShift, ">>"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "a << b\n",
        //
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::LeftShift, "<<"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // =  +=  -=  *=  /=  %=  **=  |=  &=  ^=  >>=  <<=
    EXPECT_TOKENS(
        "y = x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y += x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::PlusAssign, "+="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y -= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::MinusAssign, "-="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y *= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::MulAssign, "*="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y /= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::DivAssign, "/="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y %= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::ModuloAssign, "%="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y **= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::PowerAssign, "**="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y |= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::BitwiseOrAssign, "|="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y &= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::BitwiseAndAssign, "&="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y ^= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::BitwiseXorAssign, "^="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y >>= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::RightShiftAssign, ">>="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "y <<= x\n",
        //
        Token(TokenKind::Symbol, "y"),
        Token(TokenKind::LeftShiftAssign, "<<="),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesAuxiliaryTokens)
{
    // ( ) -> :
    EXPECT_TOKENS(
        "fun foo() -> i32:\n",
        //
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "foo"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Arrow, "->"),
        Token(TokenKind::Symbol, "i32"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // . ,
    EXPECT_TOKENS(
        "fmt.println(a, b)\n",
        //
        Token(TokenKind::Symbol, "fmt"),
        Token(TokenKind::Dot, "."),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Comma, ","),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesIntegerLiterals)
{
    EXPECT_TOKENS(
        "let a = 123\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::IntegerLiteral, "123"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "let b = 007\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "b"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::IntegerLiteral, "007"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "println(1234567890)\n",
        //
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::IntegerLiteral, "1234567890"),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesStringLiterals)
{
    EXPECT_TOKENS(
        "let hello_string = \"hello\"\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "hello_string"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::StringLiteral, "\"hello\""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "\" space space space \" != \"\"\n",
        //
        Token(TokenKind::StringLiteral, "\" space space space \""),
        Token(TokenKind::NotEquals, "!="),
        Token(TokenKind::StringLiteral, "\"\""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "let lf = \"\\n\"\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "lf"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::StringLiteral, "\"\\n\""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LineBreaksInsideStringLiteralsAreNotAllowed)
{
    EXPECT_LEX_FAILURE("let str = \"not\nallowed\"", std::exception);
    EXPECT_LEX_FAILURE("let str = \"not\rallowed\"", std::exception);
    EXPECT_LEX_FAILURE("let str = \"not\r\nallowed\"", std::exception);

    FAIL(); // TODO: make exception type more specific
}

TEST(LexerTest, LexerParsesSingleLineComment)
{
    EXPECT_TOKENS(
        "#first comment\n"
        "\n"
        "#second comment\n",
        //
        Token(TokenKind::Comment, "#first comment"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Comment, "#second comment"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerParsesMultilineComment)
{
    EXPECT_TOKENS(
        "let a #[inline]# = 1\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Comment, "#[inline]#"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::IntegerLiteral, "1"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    EXPECT_TOKENS(
        "#[this is\n"
        "a multiline\n"
        "comment]#\n",
        //
        Token(TokenKind::Comment, "#[this is\na multiline\ncomment]#"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, MultilineCommentsCanBeNested)
{
    EXPECT_TOKENS(
        "let x = #[outer #[inner]# still outer]# 123\n",
        //
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::Comment, "#[ outer #[ inner ]# still outer ]#"),
        Token(TokenKind::IntegerLiteral, "123"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));

    // TODO: more variants (comment on several lines)
}

TEST(LexerTest, LexerParsesNewLineToken)
{
    EXPECT_TOKENS(
        "fun main():\n"
        "    println(\"Y\")\n"
        "    println(\"o\")\n"
        "    println(\"!\")\n",
        //
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "main"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"Y\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"o\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"!\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerEmitsTrailingNewLineEvenIfMissing)
{
    EXPECT_TOKENS(
        "fun main():\n"
        "    println(\"hello world\")",
        //
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "main"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"hello world\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerHandlesIndentation)
{
    EXPECT_TOKENS(
        "fun main():\n"              // 0
        " if true:\n"                // 1
        "  if true:\n"               // 2
        "      if false:\n"          // 6
        "                 pass\n"    // 17
        "  else:\n"                  // 2
        "   if true:\n"              // 3
        "                if true:\n" // 16
        "                  pass\n"   // 18
        "                else:\n"    // 16
        "                 pass\n"    // 17
        "   elif:\n"                 // 3
        "    if false:\n"            // 4
        "           pass\n"          // 11
        "    else:\n"                // 4
        "     pass\n"                // 5
        " pass\n"                    // 1
        "\n"                         // 0
        "let a = 1\n",               // 0
        //
        // fun main():
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "main"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 1
        Token(TokenKind::Indent, ""),
        //  if true:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "true"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 2
        Token(TokenKind::Indent, ""),
        //   if true:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "true"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 6
        Token(TokenKind::Indent, ""),
        //       if false:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "false"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 17
        Token(TokenKind::Indent, ""),
        //                  pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 17 -> 6, then 6 -> 2
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),

        //   else:
        Token(TokenKind::Else, "else"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 3
        Token(TokenKind::Indent, ""),
        //    if true:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "true"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 16
        Token(TokenKind::Indent, ""),
        //                 if true:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "true"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 18
        Token(TokenKind::Indent, ""),
        //                   pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 18 -> 16
        Token(TokenKind::Dedent, ""),
        //                 else:
        Token(TokenKind::Else, "else"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 17
        Token(TokenKind::Indent, ""),
        //                  pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 17 -> 16, then 16 -> 3
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),

        //    elif:
        Token(TokenKind::ElseIf, "elif"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 4
        Token(TokenKind::Indent, ""),
        //     if false:
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "false"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 11
        Token(TokenKind::Indent, ""),
        //            pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 11 -> 4
        Token(TokenKind::Dedent, ""),
        //     else:
        Token(TokenKind::Else, "else"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        // indent to 5
        Token(TokenKind::Indent, ""),
        //      pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 5 -> 4 -> 3 -> 2 -> 1
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),

        //  pass
        Token(TokenKind::Pass, "pass"),
        Token(TokenKind::NewLine, ""),

        // dedent 1 -> 0 (exit fun main)
        Token(TokenKind::Dedent, ""),

        // empty line
        Token(TokenKind::NewLine, ""),

        // let a = 1
        Token(TokenKind::Let, "let"),
        Token(TokenKind::Symbol, "a"),
        Token(TokenKind::Assign, "="),
        Token(TokenKind::IntegerLiteral, "1"),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, CommentsCanBeIndented)
{
    EXPECT_TOKENS(
        "# zero\n"
        " # one\n"
        "  # two\n"
        "     # three\n",
        //
        Token(TokenKind::Comment, "# zero"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::Indent, ""),
        Token(TokenKind::Comment, "# one"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::Indent, ""),
        Token(TokenKind::Comment, "# two"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::Indent, ""),
        Token(TokenKind::Comment, "# three"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerDoesNotDedentIfLineContainsOnlyWhitespaces)
{
    EXPECT_TOKENS(
        "fun main():\n"                        // 0
        "\n"                                   // 0
        "    if x:\n"                          // 4
        "    \n"                               // 4
        "        \n"                           // 8
        "        println(\"x true\")\n"        // 8
        "  \n"                                 // 2
        "    else:"                            // 4
        "\n"                                   // 0
        "                                  \n" // 34
        "        println(\"x false\")"         // 8
        "        \n"                           // 8
        "        \n"                           // 8
        "        \n"                           // 8
        "        \n",                          // 8
        //
        // fun main():
        Token(TokenKind::Fun, "fun"),
        Token(TokenKind::Symbol, "main"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::NewLine, ""),

        //     if x:
        Token(TokenKind::Indent, ""),
        Token(TokenKind::If, "if"),
        Token(TokenKind::Symbol, "x"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::NewLine, ""),
        Token(TokenKind::NewLine, ""),

        //         println("x true")
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"x true\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::NewLine, ""),

        //     else:
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Else, "else"),
        Token(TokenKind::Colon, ":"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::NewLine, ""),

        //         println("x false")
        Token(TokenKind::Indent, ""),
        Token(TokenKind::Symbol, "println"),
        Token(TokenKind::ParenOpen, "("),
        Token(TokenKind::StringLiteral, "\"x false\""),
        Token(TokenKind::ParenClose, ")"),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::NewLine, ""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::NewLine, ""),
        Token(TokenKind::NewLine, ""),

        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Dedent, ""),
        Token(TokenKind::Eof, ""));
}

TEST(LexerTest, LexerThrowsOnInconsistentDedent)
{
    // TODO: make exception type more specific
    EXPECT_LEX_FAILURE(
        "fun main():\n"
        "    if true:\n"
        "      pass\n"
        "   pass\n",
        std::exception);
}

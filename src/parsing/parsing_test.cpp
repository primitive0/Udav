#include <functional>
#include <sstream>
#include <utility>

#include <gtest/gtest.h>

#include "support/string.hpp"

import udav.ast;
import udav.ast.builder;
import udav.parsing;
import udav.lexer;

using namespace udav;
using namespace udav::lexer;

#define EXPECT_AST(code, parse_func, ...)                      \
    do {                                                       \
        auto&& expected_ast_ptr = (__VA_ARGS__);               \
        ast::Node& expected_ast = *expected_ast_ptr;           \
                                                               \
        auto lexer = Lexer{code};                              \
        auto parser = Parser{lexer};                           \
        auto ast_ptr = std::invoke((parse_func), parser);      \
        ast::Node& ast = *ast_ptr;                             \
        if (!ast.equals(expected_ast)) {                       \
            ADD_FAILURE() << "--- Expected ---\n"              \
                          << format_using_stream(expected_ast) \
                          << "--- Actual ---\n"                \
                          << format_using_stream(ast);         \
        }                                                      \
    } while (false)

#define EXPECT_PARSING_FAILURE(code, parse_func, ExceptionType)        \
    do {                                                               \
        auto lexer = Lexer{code};                                      \
        auto parser = Parser{lexer};                                   \
        EXPECT_THROW(std::invoke((parse_func), parser), ExceptionType) \
    } while (false)

template<typename T>
auto format_using_stream(T&& value) -> String
{
    auto ss = std::ostringstream{};
    operator<<(ss, std::forward<T>(value));
    return ss.str();
}

TEST(ParserTest, CanParseExpressions)
{
    // Operator priority is handled in parser code,
    // so we need to have test cases for this.

    using namespace udav::ast::expr_dsl;

    // auto arithmetic = INT("1") * INT("2") + pow(INT("3"), pow(INT("4"), INT("5"))) % INT(6);
    // auto expected = EXPR();
    // EXPECT_AST(
    //     "foo || (1 * 2 + 3 ** 4 ** 5 % 6) == 2 && 4 / 2 >= bar || true",
    //     &Parser::parse_expr,
    //     EXPR(
    //         VAR("foo") ||
    //         (( &&
    //             INT("4") / INT("2") >= VAR("bar")) ||
    //         TRUE()));
}

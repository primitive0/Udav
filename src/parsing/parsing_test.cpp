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
}

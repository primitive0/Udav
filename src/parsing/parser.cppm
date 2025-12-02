module;

#include <cstdlib>

#include "support/unique.hpp"

export module udav.parsing;

import udav.lexer;
import udav.ast;
import udav.ast.builder; // TODO: this should not be imported here

using namespace udav::lexer;

namespace udav {

class ParserBase
{
protected:
    explicit ParserBase(Lexer& lexer)
        : lexer_{lexer}
    {
    }

private:
    Lexer& lexer_;
};

class ExpressionParser : protected ParserBase
{
public:
    auto parse_expr() -> Unique<ast::Expr>
    {
        using namespace udav::ast::expr_dsl;

        return EXPR(VAR("1") + VAR("2"));
    }

protected:
    explicit ExpressionParser(Lexer& lexer)
        : ParserBase{lexer}
    {
    }

private:
};

export class Parser final : public ExpressionParser
{
public:
    explicit Parser(Lexer& lexer)
        : ExpressionParser{lexer}
    {
    }

    auto parse_program() -> Unique<ast::Program>
    {
        // This is a stub
        return ast::BuildProgram{}
            .func(ast::BuildFunction{}
                    .name("main")
                    .body(ast::BuildBlock{}
                            .stmt(std::make_unique<ast::PassStmt>())
                            .build())
                    .build())
            .build_unique();
    }

    auto parse_function() -> Unique<ast::Function>
    {
        std::abort();
    }

    auto parse_block() -> Unique<ast::Block>
    {
        std::abort();
    }

    auto parse_stmt() -> Unique<ast::Stmt>
    {
        std::abort();
    }

    auto parse_let_stmt() -> Unique<ast::LetStmt>
    {
        std::abort();
    }

    auto parse_variable_decl() -> Unique<ast::VariableDecl>
    {
        std::abort();
    }

    auto parse_assign_stmt() -> Unique<ast::AssignStmt>
    {
        std::abort();
    }

    auto parse_pass_stmt() -> Unique<ast::PassStmt>
    {
        std::abort();
    }

    auto parse_return_stmt() -> Unique<ast::ReturnStmt>
    {
        std::abort();
    }

    auto parse_call_stmt() -> Unique<ast::CallStmt>
    {
        std::abort();
    }

    auto parse_if_stmt() -> Unique<ast::IfStmt>
    {
        std::abort();
    }

    auto parse_while_stmt() -> Unique<ast::WhileStmt>
    {
        std::abort();
    }
};

}; // namespace udav

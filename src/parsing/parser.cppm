module;

#include <exception>
#include <utility>

#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.parsing;

import udav.ast;

export import :token_stream;

namespace udav {

export class ParserException : public std::exception
{
public:
    explicit ParserException()
        : message_{}
    {
    }

    explicit ParserException(String message)
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

export class UnexpectedTokenException : public ParserException
{
    using ParserException::ParserException;
};

class ParserBase
{
protected:
    SemanticTokenStream& stream_;

    explicit ParserBase(SemanticTokenStream& stream)
        : stream_{stream}
    {
    }

    auto expect_newline() -> void
    {
        expect(TokenKind::NewLine);
        skip_newlines();
    }

    auto skip_newlines() -> void
    {
        while (stream_.peek().kind == TokenKind::NewLine) {
            stream_.advance();
        }
    }

    auto expect(TokenKind kind) -> Token
    {
        if (stream_.peek().kind != kind) {
            // TODO: add simple diagnostics
            throw UnexpectedTokenException{};
        }

        auto token = stream_.peek();
        stream_.advance();
        return token;
    }
};

// TODO: remove, use functions
#define PARSE_LEFT_BINARY_EXPR(parse_term, ...)                  \
    {                                                            \
        auto v = [](auto value) {                                \
            return Option<ast::BinaryOperation>{value};          \
        };                                                       \
        auto get_op = (__VA_ARGS__);                             \
                                                                 \
        auto lhs = (parse_term);                                 \
        while (true) {                                           \
            auto op = get_op();                                  \
            if (!op) {                                           \
                break;                                           \
            }                                                    \
            stream_.advance();                                   \
                                                                 \
            auto rhs = (parse_term);                             \
                                                                 \
            auto bin_expr = std::make_unique<ast::BinaryExpr>(); \
            bin_expr->op = *op;                                  \
            bin_expr->left = std::move(lhs);                     \
            bin_expr->right = std::move(rhs);                    \
                                                                 \
            lhs = std::move(bin_expr);                           \
        }                                                        \
        return lhs;                                              \
    }

class ExpressionParser : protected ParserBase
{
public:
    // expr_list =
    //     expr , { "," , expr } ;
    auto parse_expr_list() -> Vec<Unique<ast::Expr>>
    {
        auto exprs = Vec<Unique<ast::Expr>>{};

        exprs.push_back(parse_expr());
        while (stream_.peek().kind == TokenKind::Comma) {
            stream_.advance();
            exprs.push_back(parse_expr());
        }

        return exprs;
    }

    //expr =
    //    or_expr ;
    auto parse_expr() -> Unique<ast::Expr>
    {
        return parse_or_expr();
    }

    //or_expr =
    //    and_expr |
    //    or_expr , "||" , and_expr ;
    auto parse_or_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_and_expr(), [&]() {
            if (stream_.peek().kind == TokenKind::Or) {
                return v(ast::BinaryOperation::Or);
            }
            return v(std::nullopt);
        });
    }

    //and_expr =
    //   relational_expr |
    //  and_expr , "&&" , relational_expr ;
    auto parse_and_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_relational_expr(), [&]() {
            if (stream_.peek().kind == TokenKind::And) {
                return v(ast::BinaryOperation::And);
            }
            return v(std::nullopt);
        });
    }

    //relational_expr =
    //   bitwise_or_expr |
    //   relational_expr , relational_op , bitwise_or_expr ;
    //
    //   relational_op =
    //       "==" |
    //       "!=" |
    //       ">=" |
    //       "<=" |
    //       ">" |
    //       "<" ;
    auto parse_relational_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_bitwise_or_expr(), [&]() {
            switch (stream_.peek().kind) {
            case TokenKind::Equals:
                return v(ast::BinaryOperation::Equals);

            case TokenKind::NotEquals:
                return v(ast::BinaryOperation::NotEquals);

            case TokenKind::Less:
                return v(ast::BinaryOperation::Less);

            case TokenKind::Greater:
                return v(ast::BinaryOperation::Greater);

            case TokenKind::LessOrEqual:
                return v(ast::BinaryOperation::LessOrEqual);

            case TokenKind::GreaterOrEqual:
                return v(ast::BinaryOperation::GreaterOrEqual);

            default:
                return v(std::nullopt);
            }
        });
    }

    //bitwise_or_expr =
    //    bitwise_xor_expr |
    //    bitwise_or_expr , "|" , bitwise_xor_expr ;
    auto parse_bitwise_or_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_bitwise_xor_expr(), [&]() {
            if (stream_.peek().kind == TokenKind::BitwiseOr) {
                return v(ast::BinaryOperation::BitwiseOr);
            }
            return v(std::nullopt);
        });
    }

    //bitwise_xor_expr =
    //    bitwise_and_expr |
    //    bitwise_xor_expr , "^" , bitwise_and_expr ;
    auto parse_bitwise_xor_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_bitwise_and_expr(), [&]() {
            if (stream_.peek().kind == TokenKind::BitwiseXor) {
                return v(ast::BinaryOperation::BitwiseXor);
            }
            return v(std::nullopt);
        });
    }

    //bitwise_and_expr =
    //    shift_expr |
    //    bitwise_and_expr , "&" , shift_expr ;
    auto parse_bitwise_and_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_shift_expr(), [&]() {
            if (stream_.peek().kind == TokenKind::BitwiseAnd) {
                return v(ast::BinaryOperation::BitwiseAnd);
            }
            return v(std::nullopt);
        });
    }

    //shift_expr =
    //    add_expr |
    //    shift_expr , shift_op , add_expr ;
    auto parse_shift_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_add_expr(), [&]() {
            switch (stream_.peek().kind) {
            case TokenKind::LeftShift:
                return v(ast::BinaryOperation::LeftShift);

            case TokenKind::RightShift:
                return v(ast::BinaryOperation::RightShift);

            default:
                return v(std::nullopt);
            }
        });
    }

    //add_expr =
    //    mul_expr |
    //    add_expr , add_op , mul_expr ;
    auto parse_add_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_mul_expr(), [&]() {
            switch (stream_.peek().kind) {
            case TokenKind::Plus:
                return v(ast::BinaryOperation::Plus);

            case TokenKind::Minus:
                return v(ast::BinaryOperation::Minus);

            default:
                return v(std::nullopt);
            }
        });
    }

    // mul_expr =
    //    unary_expr |
    //    mul_expr , mul_op , unary_expr ;
    auto parse_mul_expr() -> Unique<ast::Expr>
    {
        PARSE_LEFT_BINARY_EXPR(parse_unary_expr(), [&]() {
            switch (stream_.peek().kind) {
            case TokenKind::Mul:
                return v(ast::BinaryOperation::Mul);

            case TokenKind::Div:
                return v(ast::BinaryOperation::Div);

            case TokenKind::Modulo:
                return v(ast::BinaryOperation::Modulo);

            default:
                return v(std::nullopt);
            }
        });
    }

    //unary_expr =
    //    power_expr |
    //    unary_op , power_expr ;
    auto parse_unary_expr() -> Unique<ast::Expr>
    {
        auto op = get_unary_op_kind(stream_.peek().kind);
        if (!op) {
            return parse_power_expr();
        }
        stream_.advance();

        auto unary_expr = std::make_unique<ast::UnaryExpr>();
        unary_expr->op = *op;
        unary_expr->expr = parse_power_expr();
        return unary_expr;
    }

    // power_expr =
    //     primary_expr |
    //     primary_expr , "**" , power_expr ;
    auto parse_power_expr() -> Unique<ast::Expr>
    {
        auto root = parse_primary_expr();

        if (stream_.peek().kind == TokenKind::Power) {
            stream_.advance();

            auto bin_expr = std::make_unique<ast::BinaryExpr>();
            bin_expr->op = ast::BinaryOperation::Power;
            bin_expr->left = std::move(root);
            bin_expr->right = parse_power_expr();
            root = std::move(bin_expr);
        }

        return root;
    }

    //  primary_expr =
    //     INTEGER_LITERAL |
    //     STRING_LITERAL |
    //     "true" |
    //     "false" |
    //     SYMBOL |
    //     SYMBOL , "(" , [ expr_list ] , ")" |
    //     "(" , expr , ")" ;
    auto parse_primary_expr() -> Unique<ast::Expr>
    {
        auto token = stream_.peek();
        stream_.advance();

        switch (token.kind) {
        case TokenKind::IntegerLiteral:
            return match_literal_expr<ast::IntegerExpr>(token.span);

        case TokenKind::StringLiteral:
            return match_literal_expr<ast::StringExpr>(token.span);

        case TokenKind::False:
        case TokenKind::True:
            return match_bool_expr(token.kind == TokenKind::True);

        case TokenKind::Symbol:
            return match_variable_or_call_expr(token.span);

        case TokenKind::ParenOpen: {
            auto expr = parse_expr();
            expect(TokenKind::ParenClose);
            return expr;
        }

        default:
            throw UnexpectedTokenException{};
        }
    }

protected:
    explicit ExpressionParser(SemanticTokenStream& stream)
        : ParserBase{stream}
    {
    }

    auto match_call_info(StrView function) -> ast::CallInfo
    {
        auto call_info = ast::CallInfo{};
        call_info.function = function;

        if (stream_.peek().kind == TokenKind::ParenClose) {
            // Empty argument list
            stream_.advance();
        } else {
            call_info.args = parse_expr_list();
            expect(TokenKind::ParenClose);
        }

        return call_info;
    }

private:
    template<typename T>
    auto match_literal_expr(StrView literal) -> Unique<T>
    {
        auto literal_expr = std::make_unique<T>();
        literal_expr->literal = literal;
        return literal_expr;
    }

    auto match_bool_expr(bool value) -> Unique<ast::BoolExpr>
    {
        auto bool_expr = std::make_unique<ast::BoolExpr>();
        bool_expr->value = value;
        return bool_expr;
    }

    auto match_variable_or_call_expr(StrView target) -> Unique<ast::Expr>
    {
        // TODO: add advance_if function or something similar?
        if (stream_.peek().kind == TokenKind::ParenOpen) {
            stream_.advance();

            auto call_expr = std::make_unique<ast::CallExpr>();
            call_expr->call = match_call_info(target);
            return call_expr;
        } else {
            auto var_expr = std::make_unique<ast::VariableExpr>();
            var_expr->name = target;
            return var_expr;
        }
    }

    static auto get_unary_op_kind(TokenKind kind) -> Option<ast::UnaryOperation>
    {
        switch (kind) {
        case TokenKind::Not:
            return ast::UnaryOperation::Not;

        case TokenKind::Minus:
            return ast::UnaryOperation::Minus;

        default:
            return std::nullopt;
        }
    }
};

export class Parser final : public ExpressionParser
{
public:
    explicit Parser(SemanticTokenStream& stream)
        : ExpressionParser{stream}
    {
    }

    // program =
    //     [ newline ] , { function } , EOF ;
    auto parse_program() -> Unique<ast::Program>
    {
        skip_newlines();

        auto program = std::make_unique<ast::Program>();
        while (stream_.peek().kind != TokenKind::Eof) {
            program->functions.push_back(parse_function());
        }
        stream_.advance();

        return program;
    }

    // function =
    //     "fun" , SYMBOL , "(" , [ argument_list ] , ")" , ":" , block ;
    auto parse_function() -> ast::Function
    {
        auto function = ast::Function{};

        expect(TokenKind::Fun);
        function.name = expect(TokenKind::Symbol).span;

        expect(TokenKind::ParenOpen);

        if (stream_.peek().kind != TokenKind::ParenClose) {
            function.args = parse_argument_list();
        }

        expect(TokenKind::ParenClose);
        expect(TokenKind::Colon);

        function.body = parse_block();

        return function;
    }

    //argument_list =
    //    SYMBOL , { "," , SYMBOL } ;
    auto parse_argument_list() -> Vec<StrView>
    {
        auto args = Vec<StrView>{};

        args.push_back(expect(TokenKind::Symbol).span);
        while (stream_.peek().kind == TokenKind::Comma) {
            stream_.advance();
            args.push_back(expect(TokenKind::Symbol).span);
        }

        return args;
    }

    // block =
    //    newline , INDENT , { stmt } , DEDENT ;
    auto parse_block() -> ast::Block
    {
        auto block = ast::Block{};

        expect_newline();
        expect(TokenKind::Indent);

        while (stream_.peek().kind != TokenKind::Dedent) {
            block.stmts.push_back(parse_stmt());
        }
        stream_.advance();

        return block;
    }

    // stmt =
    //    "let" , let_stmt |
    //    SYMBOL , "(" , [ expr_list ] , ")" , newline |
    //    SYMBOL , assign_op , expr , newline  |
    //    "pass" , newline |
    //    "continue" , newline |
    //    "break" , newline |
    //    "return" , [ expr ] , newline |
    //    "if" if_stmt |
    //    "while" while_stmt ;
    auto parse_stmt() -> Unique<ast::Stmt>
    {
        auto token = stream_.peek();
        stream_.advance();

        switch (token.kind) {
        case TokenKind::Let:
            return parse_let_stmt();

        case TokenKind::Symbol:
            return match_call_or_assign_stmt(token.span);

        case TokenKind::Pass:
            return match_keyword_stmt<ast::PassStmt>();

        case TokenKind::Continue:
            return match_keyword_stmt<ast::ContinueStmt>();

        case TokenKind::Break:
            return match_keyword_stmt<ast::BreakStmt>();

        case TokenKind::Return:
            return match_return_stmt();

        case TokenKind::If:
            return parse_if_stmt();

        case TokenKind::While:
            return parse_while_stmt();

        default:
            throw UnexpectedTokenException{};
        }
    }

    //let_stmt =
    //    variable_decl , { "," , variable_decl } , newline ;
    auto parse_let_stmt() -> Unique<ast::LetStmt>
    {
        auto let_stmt = std::make_unique<ast::LetStmt>();

        let_stmt->decls.push_back(parse_variable_decl());
        while (stream_.peek().kind == TokenKind::Comma) {
            stream_.advance();
            let_stmt->decls.push_back(parse_variable_decl());
        }
        expect_newline();

        return let_stmt;
    }

    //variable_decl =
    //    SYMBOL , "=" , expr ;
    auto parse_variable_decl() -> ast::VariableDecl
    {
        auto decl = ast::VariableDecl{};
        decl.name = expect(TokenKind::Symbol).span;
        expect(TokenKind::Assign);
        decl.value = parse_expr();
        return decl;
    }

    //if_stmt =
    //    expr , ":" , block ,
    //    { "elif" , expr , ":" , block } ,
    //    [ "else" , ":" , block ] ;
    auto parse_if_stmt() -> Unique<ast::IfStmt>
    {
        auto if_stmt = std::make_unique<ast::IfStmt>();

        while (true) {
            auto& branch = if_stmt->branches.emplace_back();

            branch.condition = parse_expr();
            expect(TokenKind::Colon);

            branch.body = parse_block();

            if (stream_.peek().kind != TokenKind::Elif) {
                break;
            }
            stream_.advance();
        }

        if (stream_.peek().kind == TokenKind::Else) {
            stream_.advance();
            expect(TokenKind::Colon);

            if_stmt->else_block = parse_block();
        }

        return if_stmt;
    }

    //while_stmt =
    //    expr , ":" , block ;
    auto parse_while_stmt() -> Unique<ast::WhileStmt>
    {
        auto while_stmt = std::make_unique<ast::WhileStmt>();
        while_stmt->condition = parse_expr();
        expect(TokenKind::Colon);
        while_stmt->body = parse_block();
        return while_stmt;
    }

private:
    auto match_call_or_assign_stmt(StrView target) -> Unique<ast::Stmt>
    {
        auto token = stream_.peek();

        if (token.kind == TokenKind::ParenOpen) {
            stream_.advance();
            return match_call_stmt(target);
        }

        if (auto assign_kind = get_assign_kind(token.kind); assign_kind) {
            stream_.advance();
            return match_assign_stmt(target, *assign_kind);
        }

        throw UnexpectedTokenException{};
    }

    auto match_call_stmt(StrView target) -> Unique<ast::CallStmt>
    {
        auto call_stmt = std::make_unique<ast::CallStmt>();
        call_stmt->call = match_call_info(target);
        expect_newline();
        return call_stmt;
    }

    auto match_assign_stmt(
        StrView target,
        ast::AssignKind kind)
        -> Unique<ast::AssignStmt>
    {
        auto assign_stmt = std::make_unique<ast::AssignStmt>();
        assign_stmt->kind = kind;
        assign_stmt->target = target;
        assign_stmt->value = parse_expr();
        expect_newline();
        return assign_stmt;
    }

    template<typename T>
    auto match_keyword_stmt() -> Unique<T>
    {
        auto stmt = std::make_unique<T>();
        expect_newline();
        return stmt;
    }

    auto match_return_stmt() -> Unique<ast::ReturnStmt>
    {
        auto return_stmt = std::make_unique<ast::ReturnStmt>();

        if (stream_.peek().kind != TokenKind::NewLine) {
            return_stmt->value = parse_expr();
        }
        expect_newline();

        return return_stmt;
    }

    static auto get_assign_kind(TokenKind kind) -> Option<ast::AssignKind>
    {
        switch (kind) {
        case TokenKind::Assign:
            return ast::AssignKind::Assign;

        case TokenKind::PlusAssign:
            return ast::AssignKind::PlusAssign;

        case TokenKind::MinusAssign:
            return ast::AssignKind::MinusAssign;

        case TokenKind::MulAssign:
            return ast::AssignKind::MulAssign;

        case TokenKind::DivAssign:
            return ast::AssignKind::DivAssign;

        case TokenKind::ModuloAssign:
            return ast::AssignKind::ModuloAssign;

        case TokenKind::PowerAssign:
            return ast::AssignKind::PowerAssign;

        case TokenKind::BitwiseOrAssign:
            return ast::AssignKind::BitwiseOrAssign;

        case TokenKind::BitwiseAndAssign:
            return ast::AssignKind::BitwiseAndAssign;

        case TokenKind::BitwiseXorAssign:
            return ast::AssignKind::BitwiseXorAssign;

        case TokenKind::RightShiftAssign:
            return ast::AssignKind::RightShiftAssign;

        case TokenKind::LeftShiftAssign:
            return ast::AssignKind::LeftShiftAssign;

        default:
            return std::nullopt;
        }
    }
};

} // namespace udav

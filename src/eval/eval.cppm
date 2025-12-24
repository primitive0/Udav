module;

#include <cassert>
#include <exception>
#include <iostream>

#include "support/hash_map.hpp"
#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.eval;

import udav.ast;
import udav.sema;
import udav.runtime;
import udav.eval.variable_table;

namespace udav {

export class EvalException : public std::exception
{
public:
    explicit EvalException()
        : message_{}
    {
    }

    explicit EvalException(String message)
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

export class FunctionEvaluator final : private ast::Visitor
{
public:
    explicit FunctionEvaluator(ast::Program& program)
        : program_{program}
    {
    }

    auto eval(ast::Function& function, const Vec<UdavValue>& args) -> UdavValue
    {
        if (function.native_callable) {
            return function.native_callable(args);
        }

        // TODO: populate var table

        function.accept(*this);

        if (function_result_) {
            auto result = std::move(*function_result_);
            function_result_.reset();
            return result;
        } else {
            return UdavValue{UdavNull{}};
        }
    }

private:
    auto visit(ast::Function& function) -> void override
    {
        for (auto& stmt : function.body.stmts) {
            stmt->accept(*this);
            if (should_exit_function()) {
                break;
            }
        }
    }

    auto visit(ast::LetStmt& let_stmt) -> void override
    {
        for (auto& decl : let_stmt.decls) {
            decl.accept(*this);
        }
    }

    auto visit(ast::VariableDecl& var_decl) -> void override
    {
        var_table_.declare(var_decl.name, eval_expression(*var_decl.value));
    }

    auto visit(ast::AssignStmt& assign_stmt) -> void override
    {
        if (assign_stmt.kind != ast::AssignKind::Assign) {
            assert(false && "WIP.");
        }

        auto target = var_table_.get(assign_stmt.target);
        if (!target) {
            throw EvalException{};
        }
        *target = eval_expression(*assign_stmt.value);
    }

    auto visit(ast::PassStmt& pass_stmt) -> void override
    {
        // Do nothing on pass statement
    }

    auto visit(ast::ReturnStmt& return_stmt) -> void override
    {
        if (return_stmt.value) {
            function_result_ = eval_expression(**return_stmt.value);
        } else {
            function_result_ = UdavValue{UdavNull{}};
        }
    }

    auto visit(ast::CallStmt& call_stmt) -> void override
    {
        auto _ = call_function(call_stmt.call);
    }

    auto should_exit_function() -> bool
    {
        return function_result_.has_value();
    }

    auto eval_expression(ast::Expr& expr) -> UdavValue
    {
        expr.accept(*this);

        assert(expr_result_ && "Evaluation of expression did not happen.");
        auto result = std::move(*expr_result_);
        expr_result_.reset();
        return result;
    }

    auto visit(ast::UnaryExpr& unary_expr) -> void override
    {
        switch (unary_expr.op) {
        case ast::UnaryOperation::Minus:
            expr_result_ = apply_unary_minus(eval_expression(*unary_expr.expr));
            break;

        case ast::UnaryOperation::Not:
            assert(false && "WIP.");

        default:
            assert(false && "Unreachable.");
        }
    }

    auto visit(ast::BinaryExpr& bin_expr) -> void override
    {
        auto lhs = eval_expression(*bin_expr.left);
        auto rhs = eval_expression(*bin_expr.right);

        switch (bin_expr.op) {
        case ast::BinaryOperation::Plus:
            expr_result_ = apply_plus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Minus:
            expr_result_ = apply_minus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Mul:
            expr_result_ = apply_mul(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Div:
            expr_result_ = apply_div(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Modulo:
            expr_result_ = apply_modulo(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Power:
            expr_result_ = apply_power(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseOr:
            expr_result_ = apply_bitwise_or(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseAnd:
            expr_result_ = apply_bitwise_and(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseXor:
            expr_result_ = apply_bitwise_xor(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::RightShift:
            expr_result_ = apply_right_shift(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::LeftShift:
            expr_result_ = apply_left_shift(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Or:
        case ast::BinaryOperation::And:
        case ast::BinaryOperation::Equals:
        case ast::BinaryOperation::NotEquals:
        case ast::BinaryOperation::Less:
        case ast::BinaryOperation::Greater:
        case ast::BinaryOperation::LessOrEqual:
        case ast::BinaryOperation::GreaterOrEqual:
            assert(false && "WIP.");
            break;
        default:
            assert(false && "Unreachable.");
        }
    }

    auto visit(ast::VariableExpr& expr) -> void override
    {
        auto value = var_table_.get(expr.name);
        if (!value) {
            throw EvalException{};
        }
        expr_result_ = UdavValue{*value};
    }
    auto visit(ast::IntegerExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit(ast::StringExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit(ast::BoolExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit(ast::NullExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit_literal(ast::LiteralExpr& expr) -> void
    {
        assert(expr.runtime_value && "Must contain runtime value.");
        expr_result_ = UdavValue{*expr.runtime_value};
    }

    auto visit(ast::CallExpr& call_expr) -> void override
    {
        expr_result_ = call_function(call_expr.call);
    }

    auto call_function(ast::CallInfo& call_info) -> UdavValue
    {
        if (var_table_.get(call_info.function)) {
            throw EvalException{};
        }

        auto entry = program_.function_map.find(call_info.function);
        if (entry == program_.function_map.end()) {
            throw EvalException{};
        }
        auto& function = *entry->second;

        auto evaluated_args = Vec<UdavValue>{};
        for (auto& arg : call_info.args) {
            evaluated_args.push_back(eval_expression(*arg));
        }

        return FunctionEvaluator{program_}.eval(function, evaluated_args);
    }

    static auto apply_unary_minus(UdavValue value) -> UdavValue
    {
        auto integer = value.down_cast<UdavInteger>();
        if (!integer) {
            throw EvalException{};
        }
        integer->negate();
        return value;
    }

    static auto apply_plus(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs += rhs;
        });
    }

    static auto apply_minus(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs -= rhs;
        });
    }

    static auto apply_mul(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs *= rhs;
        });
    }

    static auto apply_div(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs /= rhs;
        });
    }

    static auto apply_modulo(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs %= rhs;
        });
    }

    static auto apply_power(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs.pow(rhs);
        });
    }

    static auto apply_bitwise_or(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs |= rhs;
        });
    }

    static auto apply_bitwise_and(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs &= rhs;
        });
    }

    static auto apply_bitwise_xor(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs ^= rhs;
        });
    }

    static auto apply_right_shift(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs >>= rhs;
        });
    }

    static auto apply_left_shift(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        return act_on_int(lhs, rhs, [](UdavInteger& lhs, UdavInteger& rhs) {
            lhs <<= rhs;
        });
    }

    // TODO: rename
    template<typename F>
    static auto act_on_int(UdavValue lhs, UdavValue rhs, F action) -> UdavValue
    {
        auto lhs_int = lhs.down_cast<UdavInteger>();
        auto rhs_int = rhs.down_cast<UdavInteger>();
        if (!lhs_int || !rhs_int) {
            throw EvalException{};
        }
        action(*lhs_int, *rhs_int);
        return lhs;
    }

    ast::Program& program_;
    VariableTable var_table_{};
    Option<UdavValue> function_result_{};
    Option<UdavValue> expr_result_{};
};

} // namespace udav

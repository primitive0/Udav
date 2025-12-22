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

class EvalException : public std::exception
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

class ExpressionEvaluator final : private ast::Visitor
{
public:
    explicit ExpressionEvaluator(const VariableTable& var_table)
        : var_table_{var_table}
    {
    }

    auto eval(ast::Expr& expr) -> UdavValue
    {
        expr.accept(*this);
        return take_result();
    }

private:
    auto visit(ast::UnaryExpr& unary_expr) -> void override
    {
        unary_expr.expr->accept(*this);

        switch (unary_expr.op) {
        case ast::UnaryOperation::Minus:
            result_ = apply_unary_minus(take_result());
            break;

        case ast::UnaryOperation::Not:
            assert(false && "WIP.");

        default:
            assert(false && "Unreachable.");
        }
    }

    auto visit(ast::BinaryExpr& bin_expr) -> void override
    {
        bin_expr.left->accept(*this);
        auto lhs = take_result();

        bin_expr.right->accept(*this);
        auto rhs = take_result();

        switch (bin_expr.op) {
        case ast::BinaryOperation::Plus:
            result_ = apply_plus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Minus:
            result_ = apply_minus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Mul:
            result_ = apply_mul(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Div:
            result_ = apply_div(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Modulo:
            result_ = apply_modulo(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Power:
            result_ = apply_power(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseOr:
            result_ = apply_bitwise_or(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseAnd:
            result_ = apply_bitwise_and(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseXor:
            result_ = apply_bitwise_xor(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::RightShift:
            result_ = apply_right_shift(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::LeftShift:
            result_ = apply_left_shift(std::move(lhs), std::move(rhs));
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
        result_ = UdavValue{*value};
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

    auto visit_literal(ast::LiteralExpr& expr) -> void
    {
        assert(expr.runtime_value && "Must contain runtime value.");
        result_ = UdavValue{*expr.runtime_value};
    }

    auto visit(ast::CallExpr&) -> void override
    {
        assert(false && "WIP.");
    }

    auto take_result() -> UdavValue
    {
        assert(result_ && "Evaluation of expression did not happen.");
        auto v = std::move(*result_);
        result_.reset();
        return v;
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

    const VariableTable& var_table_;
    Option<UdavValue> result_;
};

export class Evaluator final : private ast::Visitor
{
public:
    explicit Evaluator(ast::Program& program)
        : program_{program}
    {
    }

    auto eval() -> void
    {
        auto funcs = resolve_functions(program_);
        if (auto main = funcs.find("main"); main != funcs.cend()) {
            visit(*main->second);
        }
    }

private:
    auto visit(ast::Function& func) -> void override
    {
        for (auto& stmt : func.body.stmts) {
            stmt->accept(*this);
        }
    }

    auto visit(ast::LetStmt& stmt) -> void override
    {
        for (auto& decl : stmt.decls) {
            decl.accept(*this);
        }
    }

    auto visit(ast::VariableDecl& decl) -> void override
    {
        auto value = ExpressionEvaluator{var_table_}.eval(*decl.value);
        var_table_.declare(decl.name, std::move(value));
    }

    auto visit(ast::AssignStmt& stmt) -> void override
    {
        if (stmt.kind != ast::AssignKind::Assign) {
            assert(false && "WIP.");
        }

        auto target = var_table_.get(stmt.target);
        if (!target) {
            throw EvalException{};
        }
        *target = ExpressionEvaluator{var_table_}.eval(*stmt.value);
    }

    auto visit(ast::CallStmt& stmt) -> void override
    {
        call_function(stmt.call);
    }

    auto call_function(ast::CallInfo& info) -> void
    {
        if (info.function == "println") {
            call_println(info.args);
        } else if (info.function == "print") {
            call_print(info.args);
        } else {
            throw EvalException{};
        }
    }

    auto call_println(Vec<Unique<ast::Expr>>& args) -> void
    {
        format_args_to_buffer(args);
        std::cout << buffer_ << "\n";
    }

    auto call_print(Vec<Unique<ast::Expr>>& args) -> void
    {
        format_args_to_buffer(args);
        std::cout << buffer_ << std::flush;
    }

    auto format_args_to_buffer(Vec<Unique<ast::Expr>>& args) -> void
    {
        buffer_.clear();
        for (auto& arg : args) {
            auto value = ExpressionEvaluator{var_table_}.eval(*arg);
            buffer_.append(value.format());
        }
    }

    auto visit(ast::StringExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit(ast::IntegerExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit(ast::BoolExpr& expr) -> void override
    {
        visit_literal(expr);
    }

    auto visit_literal(ast::LiteralExpr& expr) -> void
    {
        assert(expr.runtime_value && "Must contain runtime value.");
        buffer_.append(expr.runtime_value->format());
    }

    static auto resolve_functions(
        ast::Program& program)
        -> HashMap<StrView, ast::Function*>
    {
        auto funcs = HashMap<StrView, ast::Function*>{};

        for (auto& func : program.functions) {
            auto [_, success] = funcs.insert({func.name, &func});
            if (!success) {
                throw EvalException{};
            }
        }

        return funcs;
    }

    ast::Program& program_;

    VariableTable var_table_{};
    String buffer_{};
};

} // namespace udav

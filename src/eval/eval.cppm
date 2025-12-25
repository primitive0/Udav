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

class ExprHelper final
{
public:
    explicit ExprHelper() = delete;

    static auto apply_unary_minus(UdavValue value) -> UdavValue
    {
        ExprHelper::expect_type<UdavInteger>(value).negate();
        return value;
    }

    static auto apply_logical_not(UdavValue value) -> UdavValue
    {
        ExprHelper::expect_type<UdavBoolean>(value).apply_not();
        return value;
    }

    static auto apply_plus(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l += r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_minus(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l -= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_mul(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l *= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_div(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l /= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_modulo(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l %= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_power(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l.pow(r);
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_bitwise_or(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l |= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_bitwise_and(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l &= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_bitwise_xor(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l ^= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_right_shift(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l >>= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    static auto apply_left_shift(UdavValue lhs, UdavValue rhs) -> UdavValue
    {
        match_operands(
            lhs, rhs,
            [](UdavInteger& l, UdavInteger& r) {
                l <<= r;
            },
            [](UdavString&, UdavString&) {
                throw EvalException{};
            },
            [](UdavBoolean&, UdavBoolean&) {
                throw EvalException{};
            });

        return lhs;
    }

    template<typename OnInts, typename OnStrings, typename OnBools>
    static auto match_operands(
        UdavValue& lhs, UdavValue& rhs,
        OnInts act_on_ints,
        OnStrings act_on_strings,
        OnBools act_on_bools)
        -> void
    {
        if (auto left_int = lhs.down_cast<UdavInteger>()) {
            if (auto right_int = rhs.down_cast<UdavInteger>()) {
                act_on_ints(*left_int, *right_int);
                return;
            }
        }

        if (auto left_str = lhs.down_cast<UdavString>()) {
            if (auto right_str = rhs.down_cast<UdavString>()) {
                act_on_strings(*left_str, *right_str);
                return;
            }
        }

        if (auto left_bool = lhs.down_cast<UdavBoolean>()) {
            if (auto right_bool = rhs.down_cast<UdavBoolean>()) {
                act_on_bools(*left_bool, *right_bool);
                return;
            }
        }

        // Attempted operating on null
        throw EvalException{};
    }

    template<typename T>
    static auto expect_type(UdavValue& value) -> T&
    {
        auto inner = value.down_cast<T>();
        if (!inner) {
            throw EvalException{};
        }
        return *inner;
    }
};

export class FunctionEvaluator final : private ast::Visitor
{
public:
    // TODO: forward args
    static auto eval(
        ast::Program& program,
        ast::Function& function,
        const Vec<UdavValue>& args)
        -> UdavValue
    {
        if (function.native_callable) {
            return function.native_callable(args);
        }

        if (function.params.size() != args.size()) {
            throw EvalException{};
        }

        auto evaluator = FunctionEvaluator{program};

        for (auto i = 0uz; i < args.size(); ++i) {
            // Duplicate parameter names are filtered at semantic analysis stage
            auto _ = evaluator.var_table_.declare(
                function.params[i].name, UdavValue{args[i]});
        }

        function.accept(evaluator);

        if (evaluator.function_result_) {
            return std::move(*evaluator.function_result_);
        } else {
            return UdavValue{UdavNull{}};
        }
    }

private:
    explicit FunctionEvaluator(ast::Program& program)
        : program_{program}
    {
    }

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
        auto success = var_table_.declare(
            var_decl.name,
            eval_expr(*var_decl.value));
        if (!success) {
            throw EvalException{};
        }
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
        *target = eval_expr(*assign_stmt.value);
    }

    auto visit(ast::PassStmt& pass_stmt) -> void override
    {
        // Do nothing on pass statement
    }

    auto visit(ast::ReturnStmt& return_stmt) -> void override
    {
        if (return_stmt.value) {
            function_result_ = eval_expr(**return_stmt.value);
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

    auto eval_expr(ast::Expr& expr) -> UdavValue
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
            expr_result_ = ExprHelper::apply_unary_minus(eval_expr(*unary_expr.expr));
            break;

        case ast::UnaryOperation::Not:
            expr_result_ = ExprHelper::apply_logical_not(eval_expr(*unary_expr.expr));
            break;

        default:
            assert(false && "Unreachable.");
        }
    }

    auto visit(ast::BinaryExpr& bin_expr) -> void override
    {
        if (bin_expr.op == ast::BinaryOperation::Or) {
            visit_or_expr(bin_expr);
            return;
        } else if (bin_expr.op == ast::BinaryOperation::And) {
            visit_and_expr(bin_expr);
            return;
        }

        auto lhs = eval_expr(*bin_expr.left);
        auto rhs = eval_expr(*bin_expr.right);

        switch (bin_expr.op) {
        case ast::BinaryOperation::Plus:
            expr_result_ = ExprHelper::apply_plus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Minus:
            expr_result_ = ExprHelper::apply_minus(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Mul:
            expr_result_ = ExprHelper::apply_mul(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Div:
            expr_result_ = ExprHelper::apply_div(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Modulo:
            expr_result_ = ExprHelper::apply_modulo(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::Power:
            expr_result_ = ExprHelper::apply_power(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseOr:
            expr_result_ = ExprHelper::apply_bitwise_or(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseAnd:
            expr_result_ = ExprHelper::apply_bitwise_and(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::BitwiseXor:
            expr_result_ = ExprHelper::apply_bitwise_xor(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::RightShift:
            expr_result_ = ExprHelper::apply_right_shift(std::move(lhs), std::move(rhs));
            break;

        case ast::BinaryOperation::LeftShift:
            expr_result_ = ExprHelper::apply_left_shift(std::move(lhs), std::move(rhs));
            break;

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

    auto visit_or_expr(ast::BinaryExpr& or_expr) -> void
    {
        auto lhs = eval_expr(*or_expr.left);
        if (ExprHelper::expect_type<UdavBoolean>(lhs)) {
            expr_result_ = std::move(lhs);
        } else {
            auto rhs = eval_expr(*or_expr.right);
            ExprHelper::expect_type<UdavBoolean>(rhs);
            expr_result_ = std::move(rhs);
        }
    }

    auto visit_and_expr(ast::BinaryExpr& and_expr) -> void
    {
        auto lhs = eval_expr(*and_expr.left);
        if (!ExprHelper::expect_type<UdavBoolean>(lhs)) {
            expr_result_ = std::move(lhs);
        } else {
            auto rhs = eval_expr(*and_expr.right);
            ExprHelper::expect_type<UdavBoolean>(rhs);
            expr_result_ = std::move(rhs);
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
            evaluated_args.push_back(eval_expr(*arg));
        }

        return FunctionEvaluator::eval(program_, function, evaluated_args);
    }

    ast::Program& program_;
    VariableTable var_table_{};
    Option<UdavValue> function_result_{};
    Option<UdavValue> expr_result_{};
};

} // namespace udav

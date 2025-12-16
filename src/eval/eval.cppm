module;

#include <cassert>
#include <exception>
#include <iostream>

#include "support/hash_map.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.eval;

import udav.ast;
import udav.sema;

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
            arg->accept(*this);
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
        assert(expr.annotation && "Must be non-null.");

        const auto& annotation = static_cast<LiteralAnnotation&>(*expr.annotation);
        buffer_.append(annotation.value.format());
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

    String buffer_{};
};

} // namespace udav

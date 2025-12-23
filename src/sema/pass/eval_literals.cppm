module;

#include <utility>

#include "support/unique.hpp"

export module udav.sema.pass.eval_literals;

import udav.runtime;
import udav.ast;
import udav.sema.common;
import udav.sema.literal_parsing;

namespace udav {

export class EvalLiteralsPass final : private ast::RecursiveVisitor
{
public:
    explicit EvalLiteralsPass() = default;

    auto process(ast::Node& node) -> void
    {
        node.accept(*this);
    }

private:
    auto visit(ast::StringExpr& expr) -> void override
    {
        auto value = string_parser_.parse(expr.literal);
        if (!value) {
            throw PassException{};
        }
        expr.runtime_value = UdavValue{std::move(*value)};
    }

    auto visit(ast::IntegerExpr& expr) -> void override
    {
        auto value = integer_parser_.parse(expr.literal);
        if (!value) {
            throw PassException{};
        }
        expr.runtime_value = UdavValue{std::move(*value)};
    }

    auto visit(ast::BoolExpr& expr) -> void override
    {
        expr.runtime_value = UdavValue{UdavBoolean{expr.value}};
    }

    StringLiteralParser string_parser_{};
    IntegerLiteralParser integer_parser_{};
};

} // namespace udav

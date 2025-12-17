module;

#include <utility>

#include "support/unique.hpp"

export module udav.sema:eval_literals_pass;

import udav.runtime;
import udav.ast;
import udav.sema.literal_parsing;

import :exceptions;
import :annotations;

namespace udav {

// TODO: move into module udav.sema.pass.eval_literals
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

        annotate_literal(expr, UdavValue{std::move(*value)});
    }

    auto visit(ast::IntegerExpr& expr) -> void override
    {
        auto value = integer_parser_.parse(expr.literal);
        if (!value) {
            throw PassException{};
        }

        annotate_literal(expr, UdavValue{std::move(*value)});
    }

    auto visit(ast::BoolExpr& expr) -> void override
    {
        annotate_literal(expr, UdavValue{UdavBoolean{expr.value}});
    }

    auto annotate_literal(ast::LiteralExpr& expr, UdavValue value) -> void
    {
        expr.annotation = std::make_unique<LiteralAnnotation>(std::move(value));
    }

    StringLiteralParser string_parser_{};
    IntegerLiteralParser integer_parser_{};
};

} // namespace udav

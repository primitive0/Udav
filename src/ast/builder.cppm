module;

#include <cassert>
#include <utility>

#include "support/string.hpp"
#include "support/unique.hpp"

export module udav.ast.builder;

import udav.ast;

namespace udav::ast {

namespace expr_dsl {

export class ExprDsl final
{
public:
    ExprDsl(const ExprDsl&) = delete;
    ExprDsl& operator=(const ExprDsl&) = delete;

    ExprDsl(ExprDsl&&) = delete;
    ExprDsl& operator=(ExprDsl&&) = delete;

    explicit ExprDsl(Unique<Expr> expr)
        : expr_{std::move(expr)}
    {
    }

    friend auto operator-(ExprDsl e) -> ExprDsl
    {
        auto node = std::make_unique<UnaryExpr>();
        node->op_ = UnaryOperation::Minus;
        node->expr_ = std::move(e.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator!(ExprDsl e) -> ExprDsl
    {
        auto node = std::make_unique<UnaryExpr>();
        node->op_ = UnaryOperation::Not;
        node->expr_ = std::move(e.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator+(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Plus;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator-(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Minus;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator*(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Mul;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator/(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Div;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator%(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Modulo;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator==(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Equals;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator!=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::NotEquals;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Less;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::LessOrEqual;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Greater;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::GreaterOrEqual;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator||(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Or;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator&&(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::And;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator|(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::BitwiseOr;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator&(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::BitwiseAnd;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator^(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::BitwiseXor;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<<(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::LeftShift;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>>(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::RightShift;
        node->left_ = std::move(lhs.expr_);
        node->right_ = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto pow(ExprDsl base, ExprDsl exp) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op_ = BinaryOperation::Power;
        node->left_ = std::move(base.expr_);
        node->right_ = std::move(exp.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto EXPR(ExprDsl) -> Unique<Expr>;

private:
    Unique<Expr> expr_{};
};

export auto VAR(StrView name) -> ExprDsl
{
    auto node = std::make_unique<VariableExpr>();
    node->name_ = name;
    return ExprDsl{std::move(node)};
}

export auto INT(StrView literal) -> ExprDsl
{
    auto node = std::make_unique<IntegerExpr>();
    node->literal_ = literal;
    return ExprDsl{std::move(node)};
}

export auto STR(StrView literal) -> ExprDsl
{
    auto node = std::make_unique<StringExpr>();
    node->literal_ = literal;
    return ExprDsl{std::move(node)};
}

export auto BOOL(bool value) -> ExprDsl
{
    auto node = std::make_unique<BoolExpr>();
    node->value_ = value;
    return ExprDsl{std::move(node)};
}

export auto FALSE() -> ExprDsl
{
    return BOOL(false);
}

export auto TRUE() -> ExprDsl
{
    return BOOL(true);
}

export auto EXPR(ExprDsl e) -> Unique<Expr>
{
    assert(e.expr_ != nullptr && "Cannot build empty ExprDsl.");
    return std::move(e.expr_);
}

}; // namespace expr_dsl

// TODO: validate data before building class

template<typename T>
class Builder
{
public:
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    Builder(Builder&&) = delete;
    Builder& operator=(Builder&&) = delete;

    auto build() && -> T
    {
        return std::move(value_);
    }

    auto build_unique() && -> Unique<T>
    {
        return std::make_unique<T>(std::move(value_));
    }

protected:
    explicit Builder() = default;

    T value_{};
};

export class BuildBlock final : public Builder<Block>
{
public:
    explicit BuildBlock() = default;

    auto stmt(Unique<Stmt> stmt) -> BuildBlock&
    {
        value_.stmts_.push_back(std::move(stmt));
        return *this;
    }
};

export class BuildFunction final : public Builder<Function>
{
public:
    explicit BuildFunction() = default;

    auto name(StrView name) -> BuildFunction&
    {
        value_.name_ = name;
        return *this;
    }

    auto arg(StrView argument) -> BuildFunction&
    {
        value_.arguments_.push_back(argument);
        return *this;
    }

    auto body(Block body) -> BuildFunction&
    {
        value_.body_ = std::move(body);
        return *this;
    }
};

export class BuildProgram final : public Builder<Program>
{
public:
    explicit BuildProgram() = default;

    auto func(Function function) -> BuildProgram&
    {
        value_.functions_.push_back(std::move(function));
        return *this;
    }
};

export class BuildLetStmt final : public Builder<LetStmt>
{
public:
    explicit BuildLetStmt() = default;

    auto decl(StrView name, Unique<Expr> value) -> BuildLetStmt&
    {
        VariableDecl decl{};
        decl.name_ = name;
        decl.value_ = std::move(value);
        value_.decls_.push_back(std::move(decl));
        return *this;
    }
};

export class BuildAssignStmt final : public Builder<AssignStmt>
{
public:
    explicit BuildAssignStmt(AssignKind kind)
    {
        value_.kind_ = kind;
    }

    auto target(StrView target) -> BuildAssignStmt&
    {
        value_.target_ = target;
        return *this;
    }

    auto value(Unique<Expr> value) -> BuildAssignStmt&
    {
        value_.value_ = std::move(value);
        return *this;
    }
};

export class BuildReturnStmt final : public Builder<ReturnStmt>
{
public:
    explicit BuildReturnStmt() = default;

    auto value(Unique<Expr> value) -> BuildReturnStmt&
    {
        value_.value_ = std::move(value);
        return *this;
    }
};

export class BuildCallStmt final : public Builder<CallStmt>
{
public:
    explicit BuildCallStmt() = default;

    auto function(StrView function) -> BuildCallStmt&
    {
        value_.call_.function_ = function;
        return *this;
    }

    auto arg(Unique<Expr> argument) -> BuildCallStmt&
    {
        value_.call_.arguments_.push_back(std::move(argument));
        return *this;
    }
};

export class BuildBranch final : public Builder<Branch>
{
public:
    explicit BuildBranch() = default;

    auto condition(Unique<Expr> condition) -> BuildBranch&
    {
        value_.condition_ = std::move(condition);
        return *this;
    }

    auto body(Block body) -> BuildBranch&
    {
        value_.body_ = std::move(body);
        return *this;
    }
};

export class BuildIfStmt final : public Builder<IfStmt>
{
public:
    explicit BuildIfStmt() = default;

    auto branch(Branch branch) -> BuildIfStmt&
    {
        value_.branches_.push_back(std::move(branch));
        return *this;
    }

    auto else_branch(Block block) -> BuildIfStmt&
    {
        value_.else_branch_ = std::move(block);
        return *this;
    }
};

export class BuildWhileStmt final : public Builder<WhileStmt>
{
public:
    explicit BuildWhileStmt() = default;

    auto condition(Unique<Expr> condition) -> BuildWhileStmt&
    {
        value_.condition_ = std::move(condition);
        return *this;
    }

    auto body(Block body) -> BuildWhileStmt&
    {
        value_.body_ = std::move(body);
        return *this;
    }
};

} // namespace udav::ast

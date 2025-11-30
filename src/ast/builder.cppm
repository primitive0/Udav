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
        node->op = UnaryOperation::Minus;
        node->expr = std::move(e.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator!(ExprDsl e) -> ExprDsl
    {
        auto node = std::make_unique<UnaryExpr>();
        node->op = UnaryOperation::Not;
        node->expr = std::move(e.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator+(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Plus;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator-(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Minus;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator*(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Mul;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator/(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Div;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator%(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Modulo;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator==(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Equals;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator!=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::NotEquals;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Less;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::LessOrEqual;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Greater;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>=(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::GreaterOrEqual;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator||(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Or;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator&&(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::And;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator|(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::BitwiseOr;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator&(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::BitwiseAnd;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator^(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::BitwiseXor;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator<<(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::LeftShift;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto operator>>(ExprDsl lhs, ExprDsl rhs) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::RightShift;
        node->left = std::move(lhs.expr_);
        node->right = std::move(rhs.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto pow(ExprDsl base, ExprDsl exp) -> ExprDsl
    {
        auto node = std::make_unique<BinaryExpr>();
        node->op = BinaryOperation::Power;
        node->left = std::move(base.expr_);
        node->right = std::move(exp.expr_);
        return ExprDsl{std::move(node)};
    }

    friend auto EXPR(ExprDsl) -> Unique<Expr>;

private:
    Unique<Expr> expr_{};
};

export auto VAR(StrView name) -> ExprDsl
{
    auto node = std::make_unique<VariableExpr>();
    node->name = name;
    return ExprDsl{std::move(node)};
}

export auto INT(StrView literal) -> ExprDsl
{
    auto node = std::make_unique<IntegerExpr>();
    node->literal = literal;
    return ExprDsl{std::move(node)};
}

export auto STR(StrView literal) -> ExprDsl
{
    auto node = std::make_unique<StringExpr>();
    node->literal = literal;
    return ExprDsl{std::move(node)};
}

export auto BOOL(bool value) -> ExprDsl
{
    auto node = std::make_unique<BoolExpr>();
    node->value = value;
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
        value_.stmts.push_back(std::move(stmt));
        return *this;
    }
};

export class BuildFunction final : public Builder<Function>
{
public:
    explicit BuildFunction() = default;

    auto name(StrView name) -> BuildFunction&
    {
        value_.name = name;
        return *this;
    }

    auto arg(StrView arg) -> BuildFunction&
    {
        value_.args.push_back(arg);
        return *this;
    }

    auto body(Block body) -> BuildFunction&
    {
        value_.body = std::move(body);
        return *this;
    }
};

export class BuildProgram final : public Builder<Program>
{
public:
    explicit BuildProgram() = default;

    auto func(Function function) -> BuildProgram&
    {
        value_.functions.push_back(std::move(function));
        return *this;
    }
};

export class BuildVariableDecl final : public Builder<VariableDecl>
{
public:
    explicit BuildVariableDecl() = default;

    auto name(StrView name) -> BuildVariableDecl&
    {
        value_.name = name;
        return *this;
    }

    auto value(Unique<Expr> value) -> BuildVariableDecl&
    {
        value_.value = std::move(value);
        return *this;
    }
};

export class BuildLetStmt final : public Builder<LetStmt>
{
public:
    explicit BuildLetStmt() = default;

    auto decl(StrView name, Unique<Expr> value) -> BuildLetStmt&
    {
        VariableDecl var_decl{};
        var_decl.name = name;
        var_decl.value = std::move(value);
        return decl(std::move(var_decl));
    }

    auto decl(VariableDecl decl) -> BuildLetStmt&
    {
        value_.decls.push_back(std::move(decl));
        return *this;
    }
};

export class BuildAssignStmt final : public Builder<AssignStmt>
{
public:
    explicit BuildAssignStmt(AssignKind kind)
    {
        value_.kind = kind;
    }

    auto target(StrView target) -> BuildAssignStmt&
    {
        value_.target = target;
        return *this;
    }

    auto value(Unique<Expr> value) -> BuildAssignStmt&
    {
        value_.value = std::move(value);
        return *this;
    }
};

export class BuildReturnStmt final : public Builder<ReturnStmt>
{
public:
    explicit BuildReturnStmt() = default;

    auto value(Unique<Expr> value) -> BuildReturnStmt&
    {
        value_.value = std::move(value);
        return *this;
    }
};

export class BuildCallStmt final : public Builder<CallStmt>
{
public:
    explicit BuildCallStmt() = default;

    auto function(StrView function) -> BuildCallStmt&
    {
        value_.call.function = function;
        return *this;
    }

    auto arg(Unique<Expr> arg) -> BuildCallStmt&
    {
        value_.call.args.push_back(std::move(arg));
        return *this;
    }
};

export class BuildBranch final : public Builder<Branch>
{
public:
    explicit BuildBranch() = default;

    auto condition(Unique<Expr> condition) -> BuildBranch&
    {
        value_.condition = std::move(condition);
        return *this;
    }

    auto body(Block body) -> BuildBranch&
    {
        value_.body = std::move(body);
        return *this;
    }
};

export class BuildIfStmt final : public Builder<IfStmt>
{
public:
    explicit BuildIfStmt() = default;

    auto branch(Branch branch) -> BuildIfStmt&
    {
        value_.branches.push_back(std::move(branch));
        return *this;
    }

    auto else_branch(Block block) -> BuildIfStmt&
    {
        value_.else_branch = std::move(block);
        return *this;
    }
};

export class BuildWhileStmt final : public Builder<WhileStmt>
{
public:
    explicit BuildWhileStmt() = default;

    auto condition(Unique<Expr> condition) -> BuildWhileStmt&
    {
        value_.condition = std::move(condition);
        return *this;
    }

    auto body(Block body) -> BuildWhileStmt&
    {
        value_.body = std::move(body);
        return *this;
    }
};

} // namespace udav::ast

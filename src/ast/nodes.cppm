module;

#include <functional>

#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.ast:nodes;

import udav.support.convert;

import :visitor;

namespace udav::ast {

export enum class UnaryOperation {
    Minus,
    Not,
};

export enum class BinaryOperation {
    Equals,
    NotEquals,
    Less,
    Greater,
    LessOrEqual,
    GreaterOrEqual,
    Plus,
    Minus,
    Mul,
    Div,
    Modulo,
    Power,
    Or,
    And,
    BitwiseOr,
    BitwiseAnd,
    BitwiseXor,
    RightShift,
    LeftShift,
};

export enum class AssignKind {
    Assign,
    PlusAssign,
    MinusAssign,
    MulAssign,
    DivAssign,
    ModuloAssign,
    PowerAssign,
    BitwiseOrAssign,
    BitwiseAndAssign,
    BitwiseXorAssign,
    RightShiftAssign,
    LeftShiftAssign,
};

export struct Annotation
{
public:
    virtual ~Annotation() = default;
};

export struct Node
{
public:
    Unique<Annotation> annotation;

    virtual ~Node() = default;

    Node(const Node&) = delete;
    auto operator=(const Node&) -> Node& = delete;

    template<typename T>
    auto annotation_as() -> T&
    {
        return static_cast<T&>(*annotation);
    }

    template<typename T>
    auto annotation_as() const -> const T&
    {
        return static_cast<const T&>(*annotation);
    }

    virtual auto accept(Visitor& v) -> void = 0;
    virtual auto accept_children(Visitor& v) -> void {}

    virtual auto equals(const Node& rhs) const -> bool = 0;

    template<typename T, typename U>
    static auto check_equal(const Vec<T>& lhs, const Vec<U>& rhs) -> bool
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (auto i = 0uz; i < lhs.size(); ++i) {
            const auto& left = support::as_ref(lhs[i]);
            const auto& right = support::as_ref(rhs[i]);
            if (!left.equals(right)) {
                return false;
            }
        }
        return true;
    }

    template<typename T, typename U>
    static auto check_equal(const Option<T>& lhs, const Option<U>& rhs) -> bool
    {
        if (lhs && rhs) {
            return support::as_ref(*lhs).equals(support::as_ref(*rhs));
        } else {
            return !lhs && !rhs;
        }
    }

protected:
    Node(Node&&) = default;
    auto operator=(Node&&) -> Node& = default;

    explicit Node() = default;
};

template<typename Base, typename Derived>
struct ConcreteNode : public Base
{
public:
    auto accept(Visitor& v) -> void override
    {
        v.visit(self());
    }

    auto equals(const Node& rhs) const -> bool override
    {
        using OverloadedFuncPtrType = auto (Derived::*)(const Derived&) const->bool;

        auto same_type_rhs = dynamic_cast<const Derived*>(&rhs);
        if (!same_type_rhs) {
            return false;
        }

        auto overloaded_equals = static_cast<OverloadedFuncPtrType>(&Derived::equals);
        return std::mem_fn(overloaded_equals)(self(), *same_type_rhs);
    }

protected:
    ConcreteNode(ConcreteNode&&) = default;
    auto operator=(ConcreteNode&&) -> ConcreteNode& = default;

    explicit ConcreteNode() = default;

private:
    auto self() const -> const Derived&
    {
        return static_cast<const Derived&>(*this);
    }

    auto self() -> Derived&
    {
        return static_cast<Derived&>(*this);
    }
};

// Expression nodes

export struct Expr : public Node
{
protected:
    Expr(Expr&&) = default;
    auto operator=(Expr&&) -> Expr& = default;

    explicit Expr() = default;
};

export struct CallInfo final
{
    StrView function{};
    Vec<Unique<Expr>> args{};

    explicit CallInfo() = default;

    auto accept_children(Visitor& v) -> void
    {
        for (auto& arg : args) {
            arg->accept(v);
        }
    }

    auto equals(const CallInfo& rhs) const -> bool
    {
        return function == rhs.function &&
               Node::check_equal(args, rhs.args);
    }
};

export struct UnaryExpr final : public ConcreteNode<Expr, UnaryExpr>
{
    UnaryOperation op{};
    Unique<Expr> expr{};

    explicit UnaryExpr() = default;

    auto accept_children(Visitor& v) -> void override
    {
        expr->accept(v);
    }

    using Node::equals;

    auto equals(const UnaryExpr& rhs) const -> bool
    {
        return op == rhs.op && expr->equals(*rhs.expr);
    }
};

export struct BinaryExpr final : public ConcreteNode<Expr, BinaryExpr>
{
    BinaryOperation op{};
    Unique<Expr> left{};
    Unique<Expr> right{};

    explicit BinaryExpr() = default;

    auto accept_children(Visitor& v) -> void override
    {
        left->accept(v);
        right->accept(v);
    }

    using Node::equals;

    auto equals(const BinaryExpr& rhs) const -> bool
    {
        return op == rhs.op &&
               left->equals(*rhs.left) &&
               right->equals(*rhs.right);
    }
};

export struct LiteralExpr : public Expr
{
protected:
    LiteralExpr(LiteralExpr&&) = default;
    auto operator=(LiteralExpr&&) -> LiteralExpr& = default;

    explicit LiteralExpr() = default;
};

export struct IntegerExpr final : public ConcreteNode<LiteralExpr, IntegerExpr>
{
    StrView literal{};

    explicit IntegerExpr() = default;

    using Node::equals;

    auto equals(const IntegerExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

export struct StringExpr final : public ConcreteNode<LiteralExpr, StringExpr>
{
    StrView literal{};

    explicit StringExpr() = default;

    using Node::equals;

    auto equals(const StringExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

export struct BoolExpr final : public ConcreteNode<LiteralExpr, BoolExpr>
{
    bool value{};

    explicit BoolExpr() = default;

    using Node::equals;

    auto equals(const BoolExpr& rhs) const -> bool
    {
        return value == rhs.value;
    }
};

export struct CallExpr final : public ConcreteNode<Expr, CallExpr>
{
    CallInfo call{};

    explicit CallExpr() = default;

    auto accept_children(Visitor& v) -> void override
    {
        call.accept_children(v);
    }

    using Node::equals;

    auto equals(const CallExpr& rhs) const -> bool
    {
        return call.equals(rhs.call);
    }
};

export struct VariableExpr final : public ConcreteNode<Expr, VariableExpr>
{
    StrView name{};

    explicit VariableExpr() = default;

    using Node::equals;

    auto equals(const VariableExpr& rhs) const -> bool
    {
        return name == rhs.name;
    }
};

// Statements

export struct Stmt : public Node
{
protected:
    Stmt(Stmt&&) = default;
    auto operator=(Stmt&&) -> Stmt& = default;

    explicit Stmt() = default;
};

export struct Block final : public ConcreteNode<Node, Block>
{
    Vec<Unique<Stmt>> stmts{};

    explicit Block() = default;

    auto accept_children(Visitor& v) -> void override
    {
        for (auto& stmt : stmts) {
            stmt->accept(v);
        }
    }

    using Node::equals;

    auto equals(const Block& rhs) const -> bool
    {
        return check_equal(stmts, rhs.stmts);
    }
};

export struct VariableDecl final : public ConcreteNode<Node, VariableDecl>
{
    StrView name{};
    Unique<Expr> value{};

    explicit VariableDecl() = default;

    auto accept_children(Visitor& v) -> void override
    {
        value->accept(v);
    }

    using Node::equals;

    auto equals(const VariableDecl& rhs) const -> bool
    {
        return name == rhs.name && value->equals(*rhs.value);
    }
};

export struct LetStmt final : public ConcreteNode<Stmt, LetStmt>
{
    Vec<VariableDecl> decls{};

    explicit LetStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        for (auto& decl : decls) {
            decl.accept(v);
        }
    }

    using Node::equals;

    auto equals(const LetStmt& rhs) const -> bool
    {
        return check_equal(decls, rhs.decls);
    }
};

export struct AssignStmt final : public ConcreteNode<Stmt, AssignStmt>
{
    AssignKind kind{};
    StrView target{};
    Unique<Expr> value{};

    explicit AssignStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        value->accept(v);
    }

    using Node::equals;

    auto equals(const AssignStmt& rhs) const -> bool
    {
        return kind == rhs.kind &&
               target == rhs.target &&
               value->equals(*rhs.value);
    }
};

export struct PassStmt final : public ConcreteNode<Stmt, PassStmt>
{
    explicit PassStmt() = default;

    using Node::equals;

    auto equals([[maybe_unused]] const PassStmt& rhs) const -> bool
    {
        return true;
    }
};

export struct ContinueStmt final : public ConcreteNode<Stmt, ContinueStmt>
{
    explicit ContinueStmt() = default;

    using Node::equals;

    auto equals([[maybe_unused]] const ContinueStmt& rhs) const -> bool
    {
        return true;
    }
};

export struct BreakStmt final : public ConcreteNode<Stmt, BreakStmt>
{
    explicit BreakStmt() = default;

    using Node::equals;

    auto equals([[maybe_unused]] const BreakStmt& rhs) const -> bool
    {
        return true;
    }
};

export struct ReturnStmt final : public ConcreteNode<Stmt, ReturnStmt>
{
    Option<Unique<Expr>> value{};

    explicit ReturnStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        if (value) {
            (*value)->accept(v);
        }
    }

    using Node::equals;

    auto equals(const ReturnStmt& rhs) const -> bool
    {
        return check_equal(value, rhs.value);
    }
};

export struct CallStmt final : public ConcreteNode<Stmt, CallStmt>
{
    CallInfo call{};

    explicit CallStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        call.accept_children(v);
    }

    using Node::equals;

    auto equals(const CallStmt& rhs) const -> bool
    {
        return call.equals(rhs.call);
    }
};

// TODO: inherit from Node
export struct Branch final
{
    Unique<Expr> condition{};
    Block body{};

    explicit Branch() = default;

    auto accept_children(Visitor& v) -> void
    {
        condition->accept(v);
        body.accept(v);
    }

    auto equals(const Branch& rhs) const -> bool
    {
        return condition->equals(*rhs.condition) &&
               body.equals(rhs.body);
    }
};

export struct IfStmt final : public ConcreteNode<Stmt, IfStmt>
{
    Vec<Branch> branches{};
    Option<Block> else_branch{};

    explicit IfStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        for (auto& branch : branches) {
            branch.accept_children(v);
        }
        if (else_branch) {
            (*else_branch).accept(v);
        }
    }

    using Node::equals;

    auto equals(const IfStmt& rhs) const -> bool
    {
        return check_equal(branches, rhs.branches) &&
               check_equal(else_branch, rhs.else_branch);
    }
};

export struct WhileStmt final : public ConcreteNode<Stmt, WhileStmt>
{
    Unique<Expr> condition{};
    Block body{};

    explicit WhileStmt() = default;

    auto accept_children(Visitor& v) -> void override
    {
        condition->accept(v);
        body.accept(v);
    }

    using Node::equals;

    auto equals(const WhileStmt& rhs) const -> bool
    {
        return condition->equals(*rhs.condition) &&
               body.equals(rhs.body);
    }
};

// Top level nodes

export struct Function final : public ConcreteNode<Node, Function>
{
    StrView name{};
    Vec<StrView> args{}; // TODO: add FunctionArg node
    Block body{};

    explicit Function() = default;

    auto accept_children(Visitor& v) -> void override
    {
        body.accept(v);
    }

    using Node::equals;

    auto equals(const Function& rhs) const -> bool
    {
        return name == rhs.name &&
               args == rhs.args &&
               body.equals(rhs.body);
    }
};

export struct Program final : public ConcreteNode<Node, Program>
{
    Vec<Function> functions{};

    explicit Program() = default;

    auto accept_children(Visitor& v) -> void override
    {
        for (auto& function : functions) {
            function.accept(v);
        }
    }

    using Node::equals;

    auto equals(const Program& rhs) const -> bool
    {
        return check_equal(functions, rhs.functions);
    }
};

} // namespace udav::ast

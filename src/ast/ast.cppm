module;

#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.ast;

import udav.support.convert;

namespace udav::ast {

// AST node declarations

export struct Node;

export struct Expr;
export struct LiteralExpr;
export struct UnaryExpr;
export struct BinaryExpr;
export struct IntegerExpr;
export struct StringExpr;
export struct BoolExpr;
export struct CallExpr;
export struct VariableExpr;

export struct Stmt;
export struct Block;
export struct VariableDecl;
export struct LetStmt;
export struct AssignStmt;
export struct PassStmt;
export struct ContinueStmt;
export struct BreakStmt;
export struct ReturnStmt;
export struct CallStmt;
export struct Branch;
export struct IfStmt;
export struct WhileStmt;

export struct Function;
export struct Program;

// ---------------------------

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

export class Visitor
{
public:
    virtual ~Visitor() = default;

    Visitor(const Visitor&) = default;
    Visitor& operator=(const Visitor&) = default;

    Visitor(Visitor&&) = default;
    Visitor& operator=(Visitor&&) = default;

    explicit Visitor() = default;

    virtual auto visit(Program&) -> void {}
    virtual auto visit(Function&) -> void {}
    virtual auto visit(Block&) -> void {}

    virtual auto visit(VariableDecl&) -> void {}
    virtual auto visit(LetStmt&) -> void {}
    virtual auto visit(AssignStmt&) -> void {}
    virtual auto visit(PassStmt&) -> void {}
    virtual auto visit(ContinueStmt&) -> void {}
    virtual auto visit(BreakStmt&) -> void {}
    virtual auto visit(ReturnStmt&) -> void {}
    virtual auto visit(CallStmt&) -> void {}
    virtual auto visit(IfStmt&) -> void {}
    virtual auto visit(WhileStmt&) -> void {}

    virtual auto visit(UnaryExpr&) -> void {}
    virtual auto visit(BinaryExpr&) -> void {}
    virtual auto visit(IntegerExpr&) -> void {}
    virtual auto visit(StringExpr&) -> void {}
    virtual auto visit(BoolExpr&) -> void {}
    virtual auto visit(CallExpr&) -> void {}
    virtual auto visit(VariableExpr&) -> void {}
};

// Node base class

struct Node
{
public:
    virtual ~Node() = default;

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    virtual auto accept(Visitor& v) -> void = 0;

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
    Node& operator=(Node&&) = default;

    explicit Node() = default;
};

template<typename Base, typename Derived>
struct Leaf : public Base
{
public:
    auto accept(Visitor& v) -> void override
    {
        v.visit(self());
    }

    auto equals(const Node& rhs) const -> bool override
    {
        auto same_type_rhs = dynamic_cast<const Derived*>(&rhs);
        if (same_type_rhs == nullptr) {
            return false;
        }
        return self().equals(*same_type_rhs);
    }

protected:
    Leaf(Leaf&&) = default;
    Leaf& operator=(Leaf&&) = default;

    explicit Leaf() = default;

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

struct Expr : public Node
{
protected:
    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;

    explicit Expr() = default;
};

export struct CallInfo final
{
    StrView function{};
    Vec<Unique<Expr>> args{};

    explicit CallInfo() = default;

    auto equals(const CallInfo& rhs) const -> bool
    {
        return function == rhs.function &&
               Node::check_equal(args, rhs.args);
    }
};

struct UnaryExpr final : public Leaf<Expr, UnaryExpr>
{
    UnaryOperation op{};
    Unique<Expr> expr{};

    explicit UnaryExpr() = default;

    auto equals(const UnaryExpr& rhs) const -> bool
    {
        return op == rhs.op && expr->equals(*rhs.expr);
    }
};

struct BinaryExpr final : public Leaf<Expr, BinaryExpr>
{
    BinaryOperation op{};
    Unique<Expr> left{};
    Unique<Expr> right{};

    explicit BinaryExpr() = default;

    auto equals(const BinaryExpr& rhs) const -> bool
    {
        return op == rhs.op &&
               left->equals(*rhs.left) &&
               right->equals(*rhs.right);
    }
};

struct LiteralExpr : public Expr
{
protected:
    LiteralExpr(LiteralExpr&&) = default;
    LiteralExpr& operator=(LiteralExpr&&) = default;

    explicit LiteralExpr() = default;
};

struct IntegerExpr final : public Leaf<LiteralExpr, IntegerExpr>
{
    StrView literal{};

    explicit IntegerExpr() = default;

    auto equals(const IntegerExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

struct StringExpr final : public Leaf<LiteralExpr, StringExpr>
{
    StrView literal{};

    explicit StringExpr() = default;

    auto equals(const StringExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

struct BoolExpr final : public Leaf<LiteralExpr, BoolExpr>
{
    bool value{};

    explicit BoolExpr() = default;

    auto equals(const BoolExpr& rhs) const -> bool
    {
        return value == rhs.value;
    }
};

struct CallExpr final : public Leaf<Expr, CallExpr>
{
    CallInfo call{};

    explicit CallExpr() = default;

    auto equals(const CallExpr& rhs) const -> bool
    {
        return call.equals(rhs.call);
    }
};

struct VariableExpr final : public Leaf<Expr, VariableExpr>
{
    StrView name{};

    explicit VariableExpr() = default;

    auto equals(const VariableExpr& rhs) const -> bool
    {
        return name == rhs.name;
    }
};

// Statements

struct Stmt : public Node
{
protected:
    Stmt(Stmt&&) = default;
    Stmt& operator=(Stmt&&) = default;

    explicit Stmt() = default;
};

struct Block final : public Leaf<Node, Block>
{
    Vec<Unique<Stmt>> stmts{};

    explicit Block() = default;

    auto equals(const Block& rhs) const -> bool
    {
        return check_equal(stmts, rhs.stmts);
    }
};

struct VariableDecl final : public Leaf<Node, VariableDecl>
{
    StrView name{};
    Unique<Expr> value{};

    explicit VariableDecl() = default;

    auto equals(const VariableDecl& rhs) const -> bool
    {
        return name == rhs.name && value->equals(*rhs.value);
    }
};

struct LetStmt final : public Leaf<Stmt, LetStmt>
{
    Vec<VariableDecl> decls{};

    explicit LetStmt() = default;

    auto equals(const LetStmt& rhs) const -> bool
    {
        return check_equal(decls, rhs.decls);
    }
};

struct AssignStmt final : public Leaf<Stmt, AssignStmt>
{
    AssignKind kind{};
    StrView target{};
    Unique<Expr> value{};

    explicit AssignStmt() = default;

    auto equals(const AssignStmt& rhs) const -> bool
    {
        return kind == rhs.kind &&
               target == rhs.target &&
               value->equals(*rhs.value);
    }
};

struct PassStmt final : public Leaf<Stmt, PassStmt>
{
    explicit PassStmt() = default;

    auto equals(const PassStmt& rhs) const -> bool
    {
        return true;
    }
};

struct ContinueStmt final : public Leaf<Stmt, ContinueStmt>
{
    explicit ContinueStmt() = default;

    auto equals(const ContinueStmt& rhs) const -> bool
    {
        return true;
    }
};

struct BreakStmt final : public Leaf<Stmt, BreakStmt>
{
    explicit BreakStmt() = default;

    auto equals(const BreakStmt& rhs) const -> bool
    {
        return true;
    }
};

struct ReturnStmt final : public Leaf<Stmt, ReturnStmt>
{
    Unique<Expr> value{};

    explicit ReturnStmt() = default;

    auto equals(const ReturnStmt& rhs) const -> bool
    {
        return value->equals(*rhs.value);
    }
};

struct CallStmt final : public Leaf<Stmt, CallStmt>
{
    CallInfo call{};

    explicit CallStmt() = default;

    auto equals(const CallStmt& rhs) const -> bool
    {
        return call.equals(rhs.call);
    }
};

export struct Branch final
{
    Unique<Expr> condition{};
    Block body{};

    explicit Branch() = default;

    auto equals(const Branch& rhs) const -> bool
    {
        return condition->equals(*rhs.condition) &&
               body.equals(rhs.body);
    }
};

struct IfStmt final : public Leaf<Stmt, IfStmt>
{
    Vec<Branch> branches{};
    Option<Block> else_branch{};

    explicit IfStmt() = default;

    auto equals(const IfStmt& rhs) const -> bool
    {
        return check_equal(branches, rhs.branches) &&
               check_equal(else_branch, rhs.else_branch);
    }
};

struct WhileStmt final : public Leaf<Stmt, WhileStmt>
{
    Unique<Expr> condition{};
    Block body{};

    explicit WhileStmt() = default;

    auto equals(const WhileStmt& rhs) const -> bool
    {
        return condition->equals(*rhs.condition) &&
               body.equals(rhs.body);
    }
};

// Top level nodes

struct Function final : public Leaf<Node, Function>
{
    StrView name{};
    Vec<StrView> args{};
    Block body{};

    explicit Function() = default;

    auto equals(const Function& rhs) const -> bool
    {
        return name == rhs.name &&
               args == rhs.args &&
               body.equals(rhs.body);
    }
};

struct Program final : public Leaf<Node, Program>
{
    Vec<Function> functions{};

    explicit Program() = default;

    auto equals(const Program& rhs) const -> bool
    {
        return check_equal(functions, rhs.functions);
    }
};

} // namespace udav::ast

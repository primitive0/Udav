module;

#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.ast;

// TODO: sort classes
namespace udav::ast {

export class Node;

export class Expr;
export class LiteralExpr;
export class UnaryExpr;
export class BinaryExpr;
export class IntegerExpr;
export class StringExpr;
export class BoolExpr;
export class CallExpr;
export class VariableExpr;

export class Stmt;
export class Block;
export class VariableDecl;
export class LetStmt;
export class AssignStmt;
export class PassStmt;
export class ContinueStmt;
export class BreakStmt;
export class ReturnStmt;
export class CallStmt;
export class Branch;
export class IfStmt;
export class WhileStmt;

export class Function;
export class Program;

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

    virtual auto visit(UnaryExpr&) -> void {}
    virtual auto visit(BinaryExpr&) -> void {}
    virtual auto visit(IntegerExpr&) -> void {}
    virtual auto visit(StringExpr&) -> void {}
    virtual auto visit(BoolExpr&) -> void {}
    virtual auto visit(CallExpr&) -> void {}
    virtual auto visit(VariableExpr&) -> void {}

    virtual auto visit(LetStmt&) -> void {}
    virtual auto visit(AssignStmt&) -> void {}
    virtual auto visit(PassStmt&) -> void {}
    virtual auto visit(ContinueStmt&) -> void {}
    virtual auto visit(BreakStmt&) -> void {}
    virtual auto visit(ReturnStmt&) -> void {}
    virtual auto visit(CallStmt&) -> void {}
    virtual auto visit(IfStmt&) -> void {}
    virtual auto visit(WhileStmt&) -> void {}
};

template<typename Base, typename Derived>
class Leaf : public Base
{
public:
    auto accept(Visitor& v) -> void override
    {
        v.visit(static_cast<Derived&>(*this));
    }

protected:
    Leaf(Leaf&&) = default;
    Leaf& operator=(Leaf&&) = default;

    Leaf() = default;
};

class Node
{
public:
    virtual ~Node() = default;

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    virtual auto accept(Visitor&) -> void = 0;

protected:
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;

    Node() = default;
};

class Expr : public Node
{
protected:
    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;

    explicit Expr() = default;
};

class CallInfo final
{
public:
    explicit CallInfo() = default;

    StrView function_{};
    Vec<Unique<Expr>> arguments_{};
};

export enum class UnaryOperation {
    Minus,
    Not,
};

class UnaryExpr final : public Leaf<Expr, UnaryExpr>
{
public:
    explicit UnaryExpr() = default;

    UnaryOperation op_{};
    Unique<Expr> expr_{};
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

class BinaryExpr final : public Leaf<Expr, BinaryExpr>
{
public:
    explicit BinaryExpr() = default;

    BinaryOperation op_{};
    Unique<Expr> left_{};
    Unique<Expr> right_{};
};

class LiteralExpr : public Expr
{
protected:
    LiteralExpr(LiteralExpr&&) = default;
    LiteralExpr& operator=(LiteralExpr&&) = default;

    explicit LiteralExpr() = default;
};

class IntegerExpr final : public Leaf<LiteralExpr, IntegerExpr>
{
public:
    explicit IntegerExpr() = default;

    StrView literal_{};
};

class StringExpr final : public Leaf<LiteralExpr, StringExpr>
{
public:
    explicit StringExpr() = default;

    StrView literal_{};
};

class BoolExpr final : public Leaf<LiteralExpr, BoolExpr>
{
public:
    explicit BoolExpr() = default;

    bool value_{};
};

class CallExpr final : public Leaf<Expr, CallExpr>
{
public:
    explicit CallExpr() = default;

    CallInfo call_{};
};

class VariableExpr final : public Leaf<Expr, VariableExpr>
{
public:
    explicit VariableExpr() = default;

    StrView name_{};
};

class Stmt : public Node
{
protected:
    Stmt(Stmt&&) = default;
    Stmt& operator=(Stmt&&) = default;

    explicit Stmt() = default;
};

class Block final : public Leaf<Node, Block>
{
public:
    explicit Block() = default;

    Vec<Unique<Stmt>> stmts_{};
};

class VariableDecl final
{
public:
    explicit VariableDecl() = default;

    StrView name_{};
    Unique<Expr> value_{};
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

class LetStmt final : public Leaf<Stmt, LetStmt>
{
public:
    explicit LetStmt() = default;

    Vec<VariableDecl> decls_{};
};

class AssignStmt final : public Leaf<Stmt, AssignStmt>
{
public:
    explicit AssignStmt() = default;

    AssignKind kind_{};
    StrView target_{};
    Unique<Expr> value_{};
};

class PassStmt final : public Leaf<Stmt, PassStmt>
{
public:
    explicit PassStmt() = default;
};

class ContinueStmt final : public Leaf<Stmt, ContinueStmt>
{
public:
    explicit ContinueStmt() = default;
};

class BreakStmt final : public Leaf<Stmt, BreakStmt>
{
public:
    explicit BreakStmt() = default;
};

class ReturnStmt final : public Leaf<Stmt, ReturnStmt>
{
public:
    explicit ReturnStmt() = default;

    Unique<Expr> value_{};
};

class CallStmt final : public Leaf<Stmt, CallStmt>
{
public:
    explicit CallStmt() = default;

    CallInfo call_{};
};

class Branch final
{
public:
    explicit Branch() = default;

    Unique<Expr> condition_{};
    Block body_{};
};

class IfStmt final : public Leaf<Stmt, IfStmt>
{
public:
    explicit IfStmt() = default;

    Vec<Branch> branches_{};
    Option<Block> else_branch_{};
};

class WhileStmt final : public Leaf<Stmt, WhileStmt>
{
public:
    explicit WhileStmt() = default;

    Unique<Expr> condition_{};
    Block body_{};
};

class Function final : public Leaf<Node, Function>
{
public:
    explicit Function() = default;

    StrView name_{};
    Vec<StrView> arguments_{};
    Block body_{};
};

class Program final : public Leaf<Node, Program>
{
public:
    explicit Program() = default;

    Vec<Function> functions_{};
};

} // namespace udav::ast

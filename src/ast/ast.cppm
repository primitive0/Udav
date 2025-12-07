module;

#include <cassert>
#include <functional>
#include <ostream>

#include "support/numerics.hpp"
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
        using OverloadedFuncPtrType = auto (Derived::*)(const Derived&) const->bool;

        auto same_type_rhs = dynamic_cast<const Derived*>(&rhs);
        if (same_type_rhs == nullptr) {
            return false;
        }

        auto overloaded_equals = static_cast<OverloadedFuncPtrType>(&Derived::equals);
        return std::mem_fn(overloaded_equals)(self(), *same_type_rhs);
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

    using Node::equals;

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

    using Node::equals;

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

    using Node::equals;

    auto equals(const IntegerExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

struct StringExpr final : public Leaf<LiteralExpr, StringExpr>
{
    StrView literal{};

    explicit StringExpr() = default;

    using Node::equals;

    auto equals(const StringExpr& rhs) const -> bool
    {
        return literal == rhs.literal;
    }
};

struct BoolExpr final : public Leaf<LiteralExpr, BoolExpr>
{
    bool value{};

    explicit BoolExpr() = default;

    using Node::equals;

    auto equals(const BoolExpr& rhs) const -> bool
    {
        return value == rhs.value;
    }
};

struct CallExpr final : public Leaf<Expr, CallExpr>
{
    CallInfo call{};

    explicit CallExpr() = default;

    using Node::equals;

    auto equals(const CallExpr& rhs) const -> bool
    {
        return call.equals(rhs.call);
    }
};

struct VariableExpr final : public Leaf<Expr, VariableExpr>
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

    using Node::equals;

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

    using Node::equals;

    auto equals(const VariableDecl& rhs) const -> bool
    {
        return name == rhs.name && value->equals(*rhs.value);
    }
};

struct LetStmt final : public Leaf<Stmt, LetStmt>
{
    Vec<VariableDecl> decls{};

    explicit LetStmt() = default;

    using Node::equals;

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

    using Node::equals;

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

    using Node::equals;

    auto equals(const PassStmt& rhs) const -> bool
    {
        return true;
    }
};

struct ContinueStmt final : public Leaf<Stmt, ContinueStmt>
{
    explicit ContinueStmt() = default;

    using Node::equals;

    auto equals(const ContinueStmt& rhs) const -> bool
    {
        return true;
    }
};

struct BreakStmt final : public Leaf<Stmt, BreakStmt>
{
    explicit BreakStmt() = default;

    using Node::equals;

    auto equals(const BreakStmt& rhs) const -> bool
    {
        return true;
    }
};

struct ReturnStmt final : public Leaf<Stmt, ReturnStmt>
{
    Option<Unique<Expr>> value{};

    explicit ReturnStmt() = default;

    using Node::equals;

    auto equals(const ReturnStmt& rhs) const -> bool
    {
        return check_equal(value, rhs.value);
    }
};

struct CallStmt final : public Leaf<Stmt, CallStmt>
{
    CallInfo call{};

    explicit CallStmt() = default;

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

    using Node::equals;

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

    using Node::equals;

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
    Vec<StrView> args{}; // TODO: add FunctionArg node
    Block body{};

    explicit Function() = default;

    using Node::equals;

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

    using Node::equals;

    auto equals(const Program& rhs) const -> bool
    {
        return check_equal(functions, rhs.functions);
    }
};

} // namespace udav::ast

namespace udav {

// TODO: move class data members up to access modifier in ALL CLASSES
class AstPrinter final : public ast::Visitor
{
public:
    explicit AstPrinter(std::ostream& os)
        : os_{os}
    {
    }

    auto visit(ast::Program& node) -> void override
    {
        begin_node("Program");
        auto g = IndentGuard{*this};
        print_children("functions", node.functions);
    }

    auto visit(ast::Function& node) -> void override
    {
        begin_node("Function");
        auto g = IndentGuard{*this};
        print_field("name", node.name);
        print_vec_strings("args", node.args);
        print_children("body", node.body);
    }

    auto visit(ast::Block& node) -> void override
    {
        begin_node("Block");
        auto g = IndentGuard{*this};
        print_children("stmts", node.stmts);
    }

    auto visit(ast::VariableDecl& node) -> void override
    {
        begin_node("VariableDecl");
        auto g = IndentGuard{*this};
        print_field("name", node.name);
        print_children("value", node.value);
    }

    auto visit(ast::LetStmt& node) -> void override
    {
        begin_node("LetStmt");
        auto g = IndentGuard{*this};
        print_children("decls", node.decls);
    }

    auto visit(ast::AssignStmt& node) -> void override
    {
        begin_node("AssignStmt");
        auto g = IndentGuard{*this};
        print_field("kind", node.kind);
        print_field("target", node.target);
        print_children("value", node.value);
    }

    auto visit(ast::PassStmt&) -> void override
    {
        begin_node("PassStmt");
    }

    auto visit(ast::ContinueStmt&) -> void override
    {
        begin_node("ContinueStmt");
    }

    auto visit(ast::BreakStmt&) -> void override
    {
        begin_node("BreakStmt");
    }

    auto visit(ast::ReturnStmt& node) -> void override
    {
        begin_node("ReturnStmt");
        auto g = IndentGuard{*this};
        print_children("value", node.value);
    }

    auto visit(ast::CallStmt& node) -> void override
    {
        begin_node("CallStmt");
        auto g = IndentGuard{*this};
        print_field("function", node.call.function);
        print_children("args", node.call.args);
    }

    auto visit(ast::IfStmt& node) -> void override
    {
        begin_node("IfStmt");
        auto g = IndentGuard{*this};
        print_children("branches", node.branches);
        print_children("else_branch", node.else_branch);
    }

    auto visit(ast::WhileStmt& node) -> void override
    {
        begin_node("WhileStmt");
        auto g = IndentGuard{*this};
        print_children("condition", node.condition);
        print_children("body", node.body);
    }

    auto visit(ast::UnaryExpr& node) -> void override
    {
        begin_node("UnaryExpr");
        auto g = IndentGuard{*this};
        print_field("op", node.op);
        print_children("expr", node.expr);
    }

    auto visit(ast::BinaryExpr& node) -> void override
    {
        begin_node("BinaryExpr");
        auto g = IndentGuard{*this};
        print_field("op", node.op);
        print_children("left", node.left);
        print_children("right", node.right);
    }

    auto visit(ast::IntegerExpr& node) -> void override
    {
        begin_node("IntegerExpr");
        auto g = IndentGuard{*this};
        print_field("literal", node.literal);
    }

    auto visit(ast::StringExpr& node) -> void override
    {
        begin_node("StringExpr");
        auto g = IndentGuard{*this};
        print_field("literal", node.literal);
    }

    auto visit(ast::BoolExpr& node) -> void override
    {
        begin_node("BoolExpr");
        auto g = IndentGuard{*this};
        print_field("value", node.value);
    }

    auto visit(ast::CallExpr& node) -> void override
    {
        begin_node("CallExpr");
        auto g = IndentGuard{*this};
        print_field("function", node.call.function);
        print_children("args", node.call.args);
    }

    auto visit(ast::VariableExpr& node) -> void override
    {
        begin_node("VariableExpr");
        auto g = IndentGuard{*this};
        print_field("name", node.name);
    }

private:
    class IndentGuard
    {
    public:
        explicit IndentGuard(AstPrinter& printer)
            : printer_{printer}
        {
            ++printer_.indent_;
        }

        ~IndentGuard()
        {
            --printer_.indent_;
        }

    private:
        AstPrinter& printer_;
    };

    auto print_node(ast::Node& node) -> void
    {
        node.accept(*this);
    }

    // TODO: remove, when ast::Branch will become ast::Node
    auto print_node(ast::Branch& branch) -> void
    {
        begin_node("Branch");
        auto g = IndentGuard{*this};
        print_children("condition", branch.condition);
        print_children("body", branch.body);
    }

    auto print_field(StrView name, StrView value) -> void
    {
        print_indent();
        os_ << name << " = " << value << '\n';
    }

    auto print_field(StrView name, bool value) -> void
    {
        print_indent();
        os_ << name << " = " << (value ? "true" : "false") << '\n';
    }

    template<typename V>
    auto print_field(StrView name, V value) -> void
    {
        print_indent();
        os_ << name << " = " << to_string(value) << '\n';
    }

    template<typename T>
    auto print_children(StrView name, T& child) -> void
    {
        begin_field_block(name);
        auto g = IndentGuard{*this};
        print_node(support::as_ref(child));
    }

    template<typename T>
    auto print_children(StrView name, Vec<T>& vec) -> void
    {
        begin_field_block(name);
        auto g = IndentGuard{*this};
        for (auto& elem : vec) {
            print_node(support::as_ref(elem));
        }
    }

    template<typename T>
    auto print_children(StrView name, Option<T>& opt) -> void
    {
        begin_field_block(name);
        auto g = IndentGuard{*this};
        if (opt) {
            begin_field_block("some");
            auto g2 = IndentGuard{*this};
            print_node(support::as_ref(*opt));
        } else {
            print_indent();
            os_ << "none\n";
        }
    }

    auto print_vec_strings(StrView name, const Vec<StrView>& vec) -> void
    {
        begin_field_block(name);
        auto g = IndentGuard{*this};
        for (auto s : vec) {
            print_indent();
            os_ << s << '\n';
        }
    }

    auto begin_node(StrView name) -> void
    {
        print_indent();
        os_ << name << '\n';
    }

    auto begin_field_block(StrView name) -> void
    {
        print_indent();
        os_ << name << ":\n";
    }

    auto print_indent() -> void
    {
        for (auto i = 0uz; i < indent_; ++i) {
            os_ << "  ";
        }
    }

    // TODO: automatically generate this for enum
    static auto to_string(ast::UnaryOperation op) -> const char*
    {
        using enum ast::UnaryOperation;

        switch (op) {
        case Minus:
            return "Minus";
        case Not:
            return "Not";
        default:
            assert(false && "Switch is not exhaustive.");
        }
    }

    // TODO: automatically generate this for enum
    static auto to_string(ast::BinaryOperation op) -> const char*
    {
        using enum ast::BinaryOperation;

        switch (op) {
        case Equals:
            return "Equals";
        case NotEquals:
            return "NotEquals";
        case Less:
            return "Less";
        case Greater:
            return "Greater";
        case LessOrEqual:
            return "LessOrEqual";
        case GreaterOrEqual:
            return "GreaterOrEqual";
        case Plus:
            return "Plus";
        case Minus:
            return "Minus";
        case Mul:
            return "Mul";
        case Div:
            return "Div";
        case Modulo:
            return "Modulo";
        case Power:
            return "Power";
        case Or:
            return "Or";
        case And:
            return "And";
        case BitwiseOr:
            return "BitwiseOr";
        case BitwiseAnd:
            return "BitwiseAnd";
        case BitwiseXor:
            return "BitwiseXor";
        case RightShift:
            return "RightShift";
        case LeftShift:
            return "LeftShift";
        default:
            assert(false && "Switch is not exhaustive.");
        }
    }

    // TODO: automatically generate this for enum
    static auto to_string(ast::AssignKind kind) -> const char*
    {
        using enum ast::AssignKind;

        switch (kind) {
        case Assign:
            return "Assign";
        case PlusAssign:
            return "PlusAssign";
        case MinusAssign:
            return "MinusAssign";
        case MulAssign:
            return "MulAssign";
        case DivAssign:
            return "DivAssign";
        case ModuloAssign:
            return "ModuloAssign";
        case PowerAssign:
            return "PowerAssign";
        case BitwiseOrAssign:
            return "BitwiseOrAssign";
        case BitwiseAndAssign:
            return "BitwiseAndAssign";
        case BitwiseXorAssign:
            return "BitwiseXorAssign";
        case RightShiftAssign:
            return "RightShiftAssign";
        case LeftShiftAssign:
            return "LeftShiftAssign";
        default:
            assert(false && "Switch is not exhaustive.");
        }
    }

    std::ostream& os_;
    size_t indent_{0};
};

export auto operator<<(std::ostream& os, ast::Node& node) -> std::ostream&
{
    auto printer = AstPrinter{os};
    node.accept(printer);
    return os;
}

} // namespace udav

module;

#include <functional>
#include <ostream>

#include <magic_enum/magic_enum.hpp>

#include "support/option.hpp"
#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

export module udav.ast:nodes;

import udav.support.convert;

import :visitor;

namespace udav::ast {

export enum class NodeKind {
    // Top level
    Program,
    Function,

    // Statements
    Block,
    LetStmt,
    VariableDecl,
    AssignStmt,
    PassStmt,
    ContinueStmt,
    BreakStmt,
    ReturnStmt,
    CallStmt,
    IfStmt,
    WhileStmt,

    // Expressions
    UnaryExpr,
    BinaryExpr,
    IntegerExpr,
    StringExpr,
    BoolExpr,
    CallExpr,
    VariableExpr
};

export auto operator<<(std::ostream& os, NodeKind kind) -> std::ostream&
{
    return os << magic_enum::enum_name(kind);
}

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

    virtual auto node_kind() const -> NodeKind = 0;

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::UnaryExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::BinaryExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::IntegerExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::StringExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::BoolExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::CallExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::VariableExpr;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::Block;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::VariableDecl;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::LetStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::AssignStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::PassStmt;
    }

    using Node::equals;

    auto equals([[maybe_unused]] const PassStmt& rhs) const -> bool
    {
        return true;
    }
};

export struct ContinueStmt final : public ConcreteNode<Stmt, ContinueStmt>
{
    explicit ContinueStmt() = default;

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::ContinueStmt;
    }

    using Node::equals;

    auto equals([[maybe_unused]] const ContinueStmt& rhs) const -> bool
    {
        return true;
    }
};

export struct BreakStmt final : public ConcreteNode<Stmt, BreakStmt>
{
    explicit BreakStmt() = default;

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::BreakStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::ReturnStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::CallStmt;
    }

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
    Option<Block> else_branch{}; // TODO: rename to else_block

    explicit IfStmt() = default;

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::IfStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::WhileStmt;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::Function;
    }

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

    auto node_kind() const -> NodeKind override
    {
        return NodeKind::Program;
    }

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

namespace {

class TraceVisitor final : public Visitor
{
public:
    using Trace = Vec<NodeKind>;

    explicit TraceVisitor(Trace& trace)
        : trace_{trace}
    {
    }

    // clang-format off
    auto visit(Program&)      -> void override { trace_.push_back(NodeKind::Program); }
    auto visit(Function&)     -> void override { trace_.push_back(NodeKind::Function); }

    auto visit(Block&)        -> void override { trace_.push_back(NodeKind::Block); }
    auto visit(VariableDecl&) -> void override { trace_.push_back(NodeKind::VariableDecl); }
    auto visit(LetStmt&)      -> void override { trace_.push_back(NodeKind::LetStmt); }
    auto visit(AssignStmt&)   -> void override { trace_.push_back(NodeKind::AssignStmt); }
    auto visit(PassStmt&)     -> void override { trace_.push_back(NodeKind::PassStmt); }
    auto visit(ContinueStmt&) -> void override { trace_.push_back(NodeKind::ContinueStmt); }
    auto visit(BreakStmt&)    -> void override { trace_.push_back(NodeKind::BreakStmt); }
    auto visit(ReturnStmt&)   -> void override { trace_.push_back(NodeKind::ReturnStmt); }
    auto visit(CallStmt&)     -> void override { trace_.push_back(NodeKind::CallStmt); }
    auto visit(IfStmt&)       -> void override { trace_.push_back(NodeKind::IfStmt); }
    auto visit(WhileStmt&)    -> void override { trace_.push_back(NodeKind::WhileStmt); }

    auto visit(UnaryExpr&)    -> void override { trace_.push_back(NodeKind::UnaryExpr); }
    auto visit(BinaryExpr&)   -> void override { trace_.push_back(NodeKind::BinaryExpr); }
    auto visit(IntegerExpr&)  -> void override { trace_.push_back(NodeKind::IntegerExpr); }
    auto visit(StringExpr&)   -> void override { trace_.push_back(NodeKind::StringExpr); }
    auto visit(BoolExpr&)     -> void override { trace_.push_back(NodeKind::BoolExpr); }
    auto visit(CallExpr&)     -> void override { trace_.push_back(NodeKind::CallExpr); }
    auto visit(VariableExpr&) -> void override { trace_.push_back(NodeKind::VariableExpr); }
    // clang-format on

    Trace& trace_;
};

auto get_node_trace(Node& node) -> Vec<NodeKind>
{
    auto trace = Vec<NodeKind>{};
    auto visitor = TraceVisitor{trace};
    node.accept(visitor);
    return trace;
}

auto get_node_children_trace(Node& node) -> Vec<NodeKind>
{
    auto trace = Vec<NodeKind>{};
    auto visitor = TraceVisitor{trace};
    node.accept_children(visitor);
    return trace;
}

TEMPLATE_TEST_CASE("AST nodes are visited", "[ast]",
    // Top level
    Program, Function,

    // Statements
    Block, LetStmt, VariableDecl, AssignStmt, PassStmt, ContinueStmt, BreakStmt,
    ReturnStmt, CallStmt, IfStmt, WhileStmt,

    // Expressions
    UnaryExpr, BinaryExpr, IntegerExpr, StringExpr, BoolExpr, CallExpr, VariableExpr)
{
    using namespace Catch::Matchers;

    auto node = TestType{};
    CHECK_THAT(get_node_trace(node), RangeEquals({node.node_kind()}));
}

TEST_CASE("Program node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto program = Program{};
    program.functions.emplace_back();
    program.functions.emplace_back();
    program.functions.emplace_back();

    CHECK_THAT(
        get_node_children_trace(program),
        RangeEquals({
            NodeKind::Function,
            NodeKind::Function,
            NodeKind::Function,
        }));
}

TEST_CASE("Function node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto function = Function{};

    CHECK_THAT(get_node_children_trace(function), RangeEquals({NodeKind::Block}));
}

TEST_CASE("Block node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto block = Block{};
    block.stmts.push_back(std::make_unique<LetStmt>());
    block.stmts.push_back(std::make_unique<CallStmt>());
    block.stmts.push_back(std::make_unique<IfStmt>());
    block.stmts.push_back(std::make_unique<WhileStmt>());

    CHECK_THAT(
        get_node_children_trace(block),
        RangeEquals({
            NodeKind::LetStmt,
            NodeKind::CallStmt,
            NodeKind::IfStmt,
            NodeKind::WhileStmt,
        }));
}

TEST_CASE("LetStmt node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto let_stmt = LetStmt{};
    let_stmt.decls.emplace_back();
    let_stmt.decls.emplace_back();

    CHECK_THAT(
        get_node_children_trace(let_stmt),
        RangeEquals({
            NodeKind::VariableDecl,
            NodeKind::VariableDecl,
        }));
}

TEST_CASE("VariableDecl node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto var_decl = VariableDecl{};
    var_decl.value = std::make_unique<IntegerExpr>();

    CHECK_THAT(get_node_children_trace(var_decl), RangeEquals({NodeKind::IntegerExpr}));
}

TEST_CASE("AssignStmt node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto assign_stmt = AssignStmt{};
    assign_stmt.value = std::make_unique<BoolExpr>();

    CHECK_THAT(get_node_children_trace(assign_stmt), RangeEquals({NodeKind::BoolExpr}));
}

TEST_CASE("ReturnStmt node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto return_nothing_stmt = ReturnStmt{};
    CHECK(get_node_children_trace(return_nothing_stmt).empty());

    auto return_something_stmt = ReturnStmt{};
    return_something_stmt.value = std::make_unique<BinaryExpr>();
    CHECK_THAT(
        get_node_children_trace(return_something_stmt),
        RangeEquals({NodeKind::BinaryExpr}));
}

TEST_CASE("CallStmt node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto call_stmt = CallStmt{};
    call_stmt.call.args.push_back(std::make_unique<UnaryExpr>());
    call_stmt.call.args.push_back(std::make_unique<VariableExpr>());
    call_stmt.call.args.push_back(std::make_unique<BinaryExpr>());

    CHECK_THAT(
        get_node_children_trace(call_stmt),
        RangeEquals({
            NodeKind::UnaryExpr,
            NodeKind::VariableExpr,
            NodeKind::BinaryExpr,
        }));
}

TEST_CASE("IfStmt node children are visited", "[ast][!mayfail]")
{
    FAIL("TODO: implement this test case, when Branch inherits Node");
}

TEST_CASE("WhileStmt node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto while_stmt = WhileStmt{};
    while_stmt.condition = std::make_unique<CallExpr>();

    CHECK_THAT(
        get_node_children_trace(while_stmt),
        RangeEquals({
            NodeKind::CallExpr,
            NodeKind::Block,
        }));
}

TEST_CASE("UnaryExpr node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto unary_expr = UnaryExpr{};
    unary_expr.expr = std::make_unique<StringExpr>();

    CHECK_THAT(get_node_children_trace(unary_expr), RangeEquals({NodeKind::StringExpr}));
}

TEST_CASE("BinaryExpr node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto binary_expr = BinaryExpr{};
    binary_expr.left = std::make_unique<VariableExpr>();
    binary_expr.right = std::make_unique<UnaryExpr>();

    CHECK_THAT(
        get_node_children_trace(binary_expr),
        RangeEquals({
            NodeKind::VariableExpr,
            NodeKind::UnaryExpr,
        }));
}

TEST_CASE("CallExpr node children are visited", "[ast]")
{
    using namespace Catch::Matchers;

    auto call_expr = CallExpr{};
    call_expr.call.args.push_back(std::make_unique<BoolExpr>());
    call_expr.call.args.push_back(std::make_unique<CallExpr>());
    call_expr.call.args.push_back(std::make_unique<StringExpr>());

    CHECK_THAT(
        get_node_children_trace(call_expr),
        RangeEquals({
            NodeKind::BoolExpr,
            NodeKind::CallExpr,
            NodeKind::StringExpr,
        }));
}

} // namespace

} // namespace udav::ast

module;

#include <ostream>

#include <magic_enum/magic_enum.hpp>

#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.ast:format;

import udav.support.convert;

import :visitor;
import :nodes;

// TODO: move to udav::ast namespace
namespace udav {

class AstFormatter final : public ast::Visitor
{
public:
    explicit AstFormatter(std::ostream& os)
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

    auto visit(ast::Branch& branch) -> void override
    {
        begin_node("Branch");
        auto g = IndentGuard{*this};
        print_children("condition", branch.condition);
        print_children("body", branch.body);
    }

    auto visit(ast::IfStmt& node) -> void override
    {
        begin_node("IfStmt");
        auto g = IndentGuard{*this};
        print_children("branches", node.branches);
        print_children("else_block", node.else_block);
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

    auto visit(ast::NullExpr& node) -> void override
    {
        begin_node("NullExpr");
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
        explicit IndentGuard(AstFormatter& printer)
            : printer_{printer}
        {
            ++printer_.indent_;
        }

        ~IndentGuard()
        {
            --printer_.indent_;
        }

    private:
        AstFormatter& printer_;
    };

    auto print_node(ast::Node& node) -> void
    {
        node.accept(*this);
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

    static auto to_string(ast::UnaryOperation op) -> StrView
    {
        return magic_enum::enum_name(op);
    }

    static auto to_string(ast::BinaryOperation op) -> StrView
    {
        return magic_enum::enum_name(op);
    }

    static auto to_string(ast::AssignKind kind) -> StrView
    {
        return magic_enum::enum_name(kind);
    }

    std::ostream& os_;
    size_t indent_{0};
};

export auto operator<<(std::ostream& os, ast::Node& node) -> std::ostream&
{
    auto printer = AstFormatter{os};
    node.accept(printer);
    return os;
}

} // namespace udav

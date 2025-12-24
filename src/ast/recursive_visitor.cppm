export module udav.ast:recursive_visitor;

import :visitor;
import :nodes;

namespace udav::ast {

export class RecursiveVisitor : public Visitor
{
public:
    // clang-format off
    auto visit(Program& n)      -> void override { n.accept_children(*this); }
    auto visit(Function& n)     -> void override { n.accept_children(*this); }
    auto visit(Block& n)        -> void override { n.accept_children(*this); }

    auto visit(VariableDecl& n) -> void override { n.accept_children(*this); }
    auto visit(LetStmt& n)      -> void override { n.accept_children(*this); }
    auto visit(AssignStmt& n)   -> void override { n.accept_children(*this); }
    auto visit(PassStmt& n)     -> void override { n.accept_children(*this); }
    auto visit(ContinueStmt& n) -> void override { n.accept_children(*this); }
    auto visit(BreakStmt& n)    -> void override { n.accept_children(*this); }
    auto visit(ReturnStmt& n)   -> void override { n.accept_children(*this); }
    auto visit(CallStmt& n)     -> void override { n.accept_children(*this); }
    auto visit(Branch& n)       -> void override { n.accept_children(*this); }
    auto visit(IfStmt& n)       -> void override { n.accept_children(*this); }
    auto visit(WhileStmt& n)    -> void override { n.accept_children(*this); }

    auto visit(UnaryExpr& n)    -> void override { n.accept_children(*this); }
    auto visit(BinaryExpr& n)   -> void override { n.accept_children(*this); }
    auto visit(IntegerExpr& n)  -> void override { n.accept_children(*this); }
    auto visit(StringExpr& n)   -> void override { n.accept_children(*this); }
    auto visit(BoolExpr& n)     -> void override { n.accept_children(*this); }
    auto visit(NullExpr& n)     -> void override { n.accept_children(*this); }
    auto visit(CallExpr& n)     -> void override { n.accept_children(*this); }
    auto visit(VariableExpr& n) -> void override { n.accept_children(*this); }
    // clang-format on

protected:
    RecursiveVisitor(const RecursiveVisitor&) = default;
    auto operator=(const RecursiveVisitor&) -> RecursiveVisitor& = default;

    RecursiveVisitor(RecursiveVisitor&&) = default;
    auto operator=(RecursiveVisitor&&) -> RecursiveVisitor& = default;

    explicit RecursiveVisitor() = default;
};

} // namespace udav::ast

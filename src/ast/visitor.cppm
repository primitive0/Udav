module;

#include <cassert>

export module udav.ast:visitor;

namespace udav::ast {

export class Visitor
{
public:
    virtual ~Visitor() = default;

    // clang-format off
    virtual auto visit(struct Program&)      -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct Function&)     -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct Block&)        -> void { assert(false && "Not implemented."); }

    virtual auto visit(struct VariableDecl&) -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct LetStmt&)      -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct AssignStmt&)   -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct PassStmt&)     -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct ContinueStmt&) -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct BreakStmt&)    -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct ReturnStmt&)   -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct CallStmt&)     -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct Branch&)       -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct IfStmt&)       -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct WhileStmt&)    -> void { assert(false && "Not implemented."); }

    virtual auto visit(struct UnaryExpr&)    -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct BinaryExpr&)   -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct IntegerExpr&)  -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct StringExpr&)   -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct BoolExpr&)     -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct CallExpr&)     -> void { assert(false && "Not implemented."); }
    virtual auto visit(struct VariableExpr&) -> void { assert(false && "Not implemented."); }
    // clang-format on

protected:
    Visitor(const Visitor&) = default;
    auto operator=(const Visitor&) -> Visitor& = default;

    Visitor(Visitor&&) = default;
    auto operator=(Visitor&&) -> Visitor& = default;

    explicit Visitor() = default;
};

} // namespace udav::ast

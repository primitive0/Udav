export module udav.sema;

import udav.ast;

export import :exceptions;
export import :eval_literals_pass;

namespace udav {

export auto perform_semantic_analysis(ast::Program& program_node) -> void
{
    EvalLiteralsPass{}.process(program_node);
}

} // namespace udav

export module udav.sema;

export import udav.sema.common;

import udav.ast;
import udav.sema.pass.eval_literals;

namespace udav {

export auto perform_semantic_analysis(ast::Program& program_node) -> void
{
    EvalLiteralsPass{}.process(program_node);
}

} // namespace udav

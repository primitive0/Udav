export module udav.sema;

export import udav.sema.common;

import udav.ast;
import udav.sema.pass.eval_literals;
import udav.sema.pass.collect_functions;
import udav.sema.pass.enforce_context_dependent_rules;

namespace udav {

export class SemanticAnalyzer final
{
public:
    static auto run(ast::Program& program) -> void
    {
        EvalLiteralsPass{}.process(program);
        CollectFunctionsPass{}.process(program);
        EnforceContextDependentRulesPass{}.process(program);
    }

private:
    explicit SemanticAnalyzer() = default;
};

} // namespace udav

module;

#include "support/hash_set.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.sema.pass.enforce_context_dependent_rules;

import udav.ast;
import udav.sema.common;

namespace udav {

export class EnforceContextDependentRulesPass final : private ast::RecursiveVisitor
{
public:
    explicit EnforceContextDependentRulesPass() = default;

    auto process(ast::Program& program) -> void
    {
        program.accept(*this);
    }

private:
    auto visit(ast::Function& function) -> void override
    {
        // TODO: move this string to constant
        if (function.name == "main") {
            check_main_function(function);
        } else {
            ensure_no_duplicate_parameters(function);
        }

        ast::RecursiveVisitor::visit(function);
    }

    auto ensure_no_duplicate_parameters(ast::Function& function) -> void
    {
        auto parameter_name_set = HashSet<StrView>{};
        for (const auto& parameter : function.params) {
            auto [_, success] = parameter_name_set.insert(parameter.name);
            if (!success) {
                throw PassException{};
            }
        }
    }

    auto check_main_function(ast::Function& function) -> void
    {
        if (!function.params.empty()) {
            throw PassException{};
        }
    }
};

} // namespace udav

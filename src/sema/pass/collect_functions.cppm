module;

#include "support/hash_map.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.sema.pass.collect_functions;

import udav.ast;
import udav.sema.common;

namespace udav {

export class CollectFunctionsPass final
{
public:
    explicit CollectFunctionsPass() = default;

    auto process(ast::Program& program) -> void
    {
        for (auto& func : program.functions) {
            auto [_, success] = program.function_map.insert({func.name, &func});
            if (!success) {
                throw PassException{};
            }
        }
    }
};

} // namespace udav

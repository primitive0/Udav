module;

#include <cassert>

#include "support/hash_map.hpp"
#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.eval.variable_table;

import udav.runtime;

namespace udav {

export class VariableTable final
{
public:
    explicit VariableTable()
    {
        scopes_.emplace_back();
    }

    [[nodiscard]]
    auto declare(StrView name, UdavValue value) -> bool
    {
        auto& current_scope = scopes_.back();
        if (current_scope.contains(name)) {
            return false;
        }

        current_scope.emplace(name, std::move(value));
        return true;
    }

    // TODO: refactor it
    template<typename Self>
    decltype(auto) get(this Self&& self, StrView name) // NOLINT(modernize-use-trailing-return-type)
    {
        for (auto it = self.scopes_.rbegin(); it != self.scopes_.rend(); ++it) {
            auto target_var = it->find(name);
            if (target_var != it->end()) {
                return &target_var->second;
            }
        }
        return decltype(&self.scopes_.rbegin()->find(name)->second){nullptr};
    }

    auto push_scope() -> void
    {
        scopes_.emplace_back();
    }

    auto pop_scope() -> void
    {
        scopes_.pop_back();
        assert(!scopes_.empty() && "Cannot pop global scope.");
    }

private:
    using Scope = HashMap<StrView, UdavValue>;
    Vec<Scope> scopes_;
};

} // namespace udav

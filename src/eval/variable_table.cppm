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

    auto get(StrView name) -> UdavValue*
    {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto target_var = it->find(name);
            if (target_var != it->end()) {
                return &target_var->second;
            }
        }
        return nullptr;
    }

    [[nodiscard]]
    auto declare(StrView name, UdavValue value) -> bool
    {
        if (get(name)) {
            return false;
        }

        scopes_.back().emplace(name, std::move(value));
        return true;
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

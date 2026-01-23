module;

#include <utility>

#include "support/string.hpp"
#include "support/variant.hpp"

export module udav.runtime.value;

import udav.support.functional;
import udav.runtime.string;
import udav.runtime.integer;
import udav.runtime.boolean;
import udav.runtime.null;

namespace udav {

export class UdavValue final
{
public:
    // TODO: fix copy and move constructors

    template<typename Self, typename... Cases>
    decltype(auto) visit(this Self&& self, Cases... cases) // NOLINT(modernize-use-trailing-return-type)
    {
        return std::visit(support::Overloaded{cases...}, self.inner_);
    }

    template<typename T, typename Self>
    auto down_cast(this Self&& self)
    {
        return std::get_if<T>(&self.inner_);
    }

    explicit UdavValue(UdavString string)
        : inner_{std::move(string)}
    {
    }

    explicit UdavValue(UdavInteger integer)
        : inner_{std::move(integer)}
    {
    }

    explicit UdavValue(UdavBoolean boolean)
        : inner_{std::move(boolean)}
    {
    }

    explicit UdavValue(UdavNull null)
        : inner_{std::move(null)}
    {
    }

    auto operator==(const UdavValue& value) const -> bool
    {
        return inner_ == value.inner_;
    }


    auto format() const -> String
    {
        return visit(
            [](const UdavString& str) { return String{StrView{str}}; },
            [](const UdavInteger& integer) { return integer.format(); },
            [](const UdavBoolean& boolean) { return boolean.format(); },
            [](const UdavNull& null) { return null.format(); });
    }

    auto is_null() const -> bool
    {
        return std::holds_alternative<UdavNull>(inner_);
    }

private:
    using Value = Variant<
        UdavString,
        UdavInteger,
        UdavBoolean,
        UdavNull>;

    Value inner_;
};

// TODO: add tests for UdavValue

} // namespace udav

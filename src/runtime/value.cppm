module;

#include <utility>

#include "support/string.hpp"
#include "support/variant.hpp"

export module udav.runtime:value;

import udav.support.functional;

import :string;
import :integer;

namespace udav {

export class UdavValue final
{
public:
    // TODO: fix copy and move constructors

    template<typename Self, typename... Cases>
    decltype(auto) visit(this Self&& self, Cases... cases) // NOLINT(modernize-use-trailing-return-type)
    {
        return self.inner_.visit(support::Overloaded{cases...});
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

    auto format() const -> String
    {
        return visit(
            [](const UdavString& str) { return String{StrView{str}}; },
            [](const UdavInteger& integer) { return integer.format(); });
    }

private:
    using Value = Variant<
        UdavString,
        UdavInteger>;

    Value inner_;
};

// TODO: add tests for UdavValue

} // namespace udav

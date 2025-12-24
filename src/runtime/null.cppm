module;

#include "support/string.hpp"

export module udav.runtime.null;

namespace udav {

export class UdavNull final
{
public:
    explicit UdavNull(const UdavNull&) = default; // NOLINT(google-explicit-constructor)
    auto operator=(const UdavNull&) -> UdavNull& = delete("Use copy constructor.");

    UdavNull(UdavNull&&) = default;
    auto operator=(UdavNull&&) -> UdavNull& = default;

    explicit UdavNull() = default;

    auto format() const -> String
    {
        return "null";
    }
};

} // namespace udav

module;

#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>

export module udav.runtime.boolean;

namespace udav {

export class UdavBoolean final
{
public:
    UdavBoolean(const UdavBoolean&) = default;
    auto operator=(const UdavBoolean&) -> UdavBoolean& = default;

    UdavBoolean(UdavBoolean&&) = default;
    auto operator=(UdavBoolean&&) -> UdavBoolean& = default;

    explicit UdavBoolean(bool value)
        : value_{value}
    {
    }

    explicit operator bool() const
    {
        return value_;
    }

    auto format() const -> String
    {
        return value_ ? "true" : "false";
    }

    auto apply_not() -> void
    {
        value_ = !value_;
    }

private:
    bool value_;
};

TEST_CASE("UdavBoolean is formatted", "[runtime]")
{
    CHECK(UdavBoolean{false}.format() == "false");
    CHECK(UdavBoolean{true}.format() == "true");
}

} // namespace udav

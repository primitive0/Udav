module;

#include <limits>
#include <utility>

#include <boost/multiprecision/cpp_int.hpp>

#include "support/numerics.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.runtime.integer;

import udav.runtime.exceptions;

namespace udav {

export class UdavInteger final
{
public:
    explicit UdavInteger(const UdavInteger&) = default; // NOLINT(google-explicit-constructor)
    auto operator=(const UdavInteger&) -> UdavInteger& = delete("Use copy constructor.");

    UdavInteger(UdavInteger&&) = default;
    auto operator=(UdavInteger&&) -> UdavInteger& = default;

    explicit UdavInteger(boost::multiprecision::cpp_int value)
        : value_{std::move(value)}
    {
    }

    auto operator==(const UdavInteger& rhs) const -> bool = default;

    auto format() const -> String
    {
        return value_.convert_to<String>();
    }

    auto value() -> boost::multiprecision::cpp_int&
    {
        return value_;
    }

    auto value() const -> const boost::multiprecision::cpp_int&
    {
        return value_;
    }

    auto negate() -> void
    {
        value_ *= -1;
    }

    auto operator+=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ += rhs.value_;
        return *this;
    }

    auto operator-=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ -= rhs.value_;
        return *this;
    }

    auto operator*=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ *= rhs.value_;
        return *this;
    }

    auto operator/=(const UdavInteger& rhs) -> UdavInteger&
    {
        if (rhs.value_ == 0) {
            throw UdavRuntimeException{"Can't divide by zero."};
        }

        value_ /= rhs.value_;
        return *this;
    }

    auto operator%=(const UdavInteger& rhs) -> UdavInteger&
    {
        if (rhs.value_ <= 0) {
            throw UdavRuntimeException{"Modulo divisor must be positive."};
        }

        value_ %= rhs.value_;
        return *this;
    }

    auto pow(const UdavInteger& rhs) -> void
    {
        if (rhs.value_ < 0) {
            throw UdavRuntimeException{"Exponent must be non-negative."};
        }
        if (rhs.value_ > std::numeric_limits<i64>::max()) {
            throw UdavRuntimeException{"Exponent is too big."};
        }
        auto exp = rhs.value_.convert_to<i64>();

        value_ = boost::multiprecision::pow(value_, exp);
    }

    auto operator|=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ |= rhs.value_;
        return *this;
    }

    auto operator^=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ ^= rhs.value_;
        return *this;
    }

    auto operator&=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ &= rhs.value_;
        return *this;
    }

    auto operator>>=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ >>= get_shift_count(rhs.value_);
        return *this;
    }

    auto operator<<=(const UdavInteger& rhs) -> UdavInteger&
    {
        value_ <<= get_shift_count(rhs.value_);
        return *this;
    }

    static auto get_shift_count(const boost::multiprecision::cpp_int& big_int) -> i64
    {
        if (big_int < 0) {
            throw UdavRuntimeException{"Shift count must be non-negative."};
        }
        if (big_int > std::numeric_limits<i64>::max()) {
            throw UdavRuntimeException{"Shift count is too big."};
        }
        return big_int.convert_to<i64>();
    }

private:
    boost::multiprecision::cpp_int value_;
};

TEST_CASE("UdavInteger is constructed and formatted", "[runtime]")
{
    struct TC
    {
        boost::multiprecision::cpp_int input;
        StrView format;
    };

    // clang-format off
    auto [input, format] = GENERATE(
        TC{0,          "0"},
        TC{1234567890, "1234567890"},
        TC{-123,       "-123"});
    // clang-format on

    const auto udav_integer = UdavInteger{
        boost::multiprecision::cpp_int{input}};

    CHECK(udav_integer.value() == input);
    CHECK(udav_integer.format() == format);
}

} // namespace udav

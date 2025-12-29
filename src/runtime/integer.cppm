module;

#include <compare>
#include <limits>
#include <utility>

#include <boost/multiprecision/cpp_int.hpp>

#include "support/numerics.hpp"
#include "support/option.hpp"
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

    auto operator<=>(const UdavInteger& rhs) const -> std::strong_ordering
    {
        if (value_ < rhs.value_) {
            return std::strong_ordering::less;
        } else if (value_ > rhs.value_) {
            return std::strong_ordering::greater;
        } else {
            return std::strong_ordering::equal;
        }
    }

    auto format() const -> String
    {
        return value_.convert_to<String>();
    }

    auto to_i64() const -> i64
    {
        if (value_ < std::numeric_limits<i64>::min() ||
            std::numeric_limits<i64>::max() < value_) {
            throw UdavRuntimeException{"UdavInteger is too big to be converted to i64."};
        }
        return value_.convert_to<i64>();
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

    static auto parse_decimal(StrView input) -> Option<UdavInteger>
    {
        if (input.empty()) {
            return std::nullopt;
        }

        auto negative = false;
        if (input[0] == '-') {
            negative = true;
            input = input.substr(1);
        }
        if (input.empty()) {
            return std::nullopt;
        }

        while (input.size() != 1 && input[0] == '0') {
            input = input.substr(1);
        }

        for (auto ch : input) {
            if (ch < '0' || '9' < ch) {
                return std::nullopt;
            }
        }

        auto integer = UdavInteger{boost::multiprecision::cpp_int{input}};
        if (negative) {
            integer.negate();
        }
        return integer;
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

TEST_CASE("UdavInteger::parse_decimal parses decimal integers", "[runtime]")
{
    struct TC
    {
        StrView input;
        boost::multiprecision::cpp_int expected;
    };

    // clang-format off
    auto [input, expected] = GENERATE(
        TC{"0",          0},
        TC{"-0",         0},
        TC{"0000",       0},
        TC{"-0000",      0},
        TC{"0009",       9}, // This must be parsed as decimal
        TC{"0123",       123},
        TC{"-0123",      -123},
        TC{"1234567890", 1234567890},
        TC{"-1",         -1},
        TC{"-42",        -42});
    // clang-format on

    auto result = UdavInteger::parse_decimal(input);
    CHECK((result && result->value() == expected));
}

TEST_CASE("UdavInteger::parse_decimal does not parse invalid integers", "[runtime]")
{
    auto input = GENERATE(
        as<StrView>{},
        "",
        "-",
        "aaa",
        "0b010"
        "0b00F",
        "-0x1",
        "-0b1");

    CHECK(!UdavInteger::parse_decimal(input));
}

} // namespace udav

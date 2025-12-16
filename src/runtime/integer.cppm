module;

#include <utility>

#include <boost/multiprecision/cpp_int.hpp>

#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.runtime.integer;

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

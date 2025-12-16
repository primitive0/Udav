module;

#include <utility>

#include "support/shared.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.runtime.string;

namespace udav {

export class UdavString final
{
public:
    explicit UdavString(const UdavString&) = default; // NOLINT(google-explicit-constructor)
    auto operator=(const UdavString&) -> UdavString& = delete("Use copy constructor.");

    UdavString(UdavString&&) = default;
    auto operator=(UdavString&&) -> UdavString& = default;

    explicit UdavString()
        : data_{std::make_shared<String>()}
    {
    }

    explicit UdavString(StrView str)
        : data_{std::make_shared<String>(str)}
    {
    }

    explicit UdavString(String str)
        : data_{std::make_shared<String>(std::move(str))}
    {
    }

    explicit operator StrView() const
    {
        return *data_;
    }

private:
    Shared<String> data_;
};

TEST_CASE("UdavString default constructor creates empty string", "[runtime]")
{
    const auto udav_string = UdavString{};
    CHECK(StrView(udav_string) == "");
}

TEST_CASE("UdavString is created from StrView and converted back", "[runtime]")
{
    using namespace std::string_view_literals;

    auto input = GENERATE(
        as<StrView>{},
        "",
        "foobar");

    const auto udav_string = UdavString{input};
    CHECK(StrView(udav_string) == input);
}

TEST_CASE("UdavString is created from String", "[runtime]")
{
    auto str = String{"foo"};

    const auto udav_string = UdavString{std::move(str)};

    CHECK(str.empty());
    CHECK(StrView(udav_string) == "foo");
}

} // namespace udav

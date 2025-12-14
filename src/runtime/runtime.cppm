module;

#include <array>
#include <utility>

#include "support/option.hpp"
#include "support/shared.hpp"
#include "support/string.hpp"
#include "support/variant.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.runtime;

import udav.support.functional;

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

    // NOTE: This is intentionally implicit.
    operator StrView() const // NOLINT(google-explicit-constructor)
    {
        return *data_;
    }

private:
    Shared<String> data_;
};

TEST_CASE("UdavString default constructor creates empty string", "[runtime]")
{
    const auto udav_string = UdavString{};
    CHECK(static_cast<StrView>(udav_string) == "");
}

TEST_CASE("UdavString is created from StrView and converted back", "[runtime]")
{
    using namespace std::string_view_literals;

    auto input = GENERATE(
        as<StrView>{},
        "",
        "foobar");

    const auto udav_string = UdavString{input};
    CHECK(udav_string == input);
}

TEST_CASE("UdavString is created from String", "[runtime]")
{
    auto str = String{"foo"};

    auto udav_string = UdavString{std::move(str)};

    CHECK(str.empty());
    CHECK(static_cast<StrView>(udav_string) == "foo");
}

export class UdavValue final
{
public:
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

    // TODO: use something like std::borrow::Cow from rust?
    auto format() const -> String
    {
        return visit(
            [](const UdavString& str) { return String{str}; });
    }

private:
    using Value = Variant<
        UdavString>;

    Value inner_;
};

// TODO: add tests for UdavValue

} // namespace udav

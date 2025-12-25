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

    explicit UdavString(const char* cstr)
        : data_{std::make_shared<String>(cstr)}
    {
    }

    auto operator==(const UdavString& rhs) const -> bool
    {
        return *data_ == *rhs.data_;
    }

    explicit operator StrView() const
    {
        return *data_;
    }

    auto operator+=(const UdavString& rhs) -> UdavString&
    {
        // TODO: ensure no aliasing problem corrupts memory here
        get_mut() += *rhs.data_;
        return *this;
    }

    auto size() const -> size_t
    {
        return data_->size();
    }

private:
    auto get_mut() -> String&
    {
        if (!data_.unique()) {
            data_ = std::make_shared<String>(*data_);
        }
        return *data_;
    }

    Shared<String> data_;
};

TEST_CASE("UdavString default constructor creates empty string", "[runtime]")
{
    const auto udav_string = UdavString{};
    CHECK(StrView(udav_string).empty());
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

    CHECK(StrView(udav_string) == "foo");
}

} // namespace udav

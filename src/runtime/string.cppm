module;

#include <algorithm>
#include <limits>
#include <utility>

#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/shared.hpp"
#include "support/string.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

export module udav.runtime.string;

namespace udav {

export class UdavString final
{
public:
    static constexpr size_t kNoPos = std::numeric_limits<size_t>::max();

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

    auto find(const UdavString& substr) const -> size_t
    {
        return find(StrView{substr});
    }

    auto find(StrView substr) const -> size_t
    {
        auto index = StrView(*this).find(substr);
        return index != StrView::npos ? index : kNoPos;
    }

    auto reverse() -> void
    {
        auto& data = get_mut();
        std::reverse(data.begin(), data.end());
    }

    auto substr(size_t start, size_t count) const -> Option<UdavString>
    {
        if (start > size()) {
            return std::nullopt;
        }
        return UdavString{data_->substr(start, count)};
    }

private:
    auto get_mut() -> String&
    {
        if (data_.use_count() != 1) {
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

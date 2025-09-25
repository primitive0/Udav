#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "Example.hpp"

using namespace std::literals::string_view_literals;

class FilterDuplicateLinesTest : public ::testing::Test {
protected:
    auto SetUp() -> void override
    {
        auto test_id = std::string{::testing::UnitTest::GetInstance()
                                       ->current_test_info()
                                       ->name()};
        source_path_ = std::filesystem::temp_directory_path() / (test_id + "_source.txt");
        dest_path_ = std::filesystem::temp_directory_path() / (test_id + "_dest.txt");
    }

    auto TearDown() -> void override
    {
        std::filesystem::remove(source_path_);
        std::filesystem::remove(dest_path_);
    }

    auto create_file(std::filesystem::path const& path, std::string_view content) -> void
    {
        auto file = std::ofstream{path};
        ASSERT_TRUE(file.is_open());
        file << content;
    }

    auto read_file(std::filesystem::path const& path, std::string& content) -> void
    {
        auto file = std::ifstream{path};
        ASSERT_TRUE(file.is_open());
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
        content = buffer.str();
    }

    std::filesystem::path source_path_;
    std::filesystem::path dest_path_;
};

TEST_F(FilterDuplicateLinesTest, RemovesDuplicateLinesAndPreservesOrder)
{
    constexpr std::string_view kSource = "banana\n"
                                         "apple\n"
                                         "cherry\n"
                                         "apple\n"
                                         "banana\n";

    constexpr std::string_view kExpected = "banana\n"
                                           "apple\n"
                                           "cherry\n";

    create_file(source_path_, kSource);

    example::filter_duplicate_lines(source_path_.string(), dest_path_.string());

    auto content = std::string{};
    read_file(dest_path_, content);
    EXPECT_EQ(content, kExpected);
}

TEST_F(FilterDuplicateLinesTest, HandlesEmptySourceFile)
{
    create_file(source_path_, "");

    example::filter_duplicate_lines(source_path_.string(), dest_path_.string());

    auto content = std::string{};
    read_file(dest_path_, content);
    EXPECT_EQ(content, "");
}

TEST_F(FilterDuplicateLinesTest, HandlesFileWithNoDuplicates)
{
    constexpr std::string_view kSource = "line 1\n"
                                         "line 2\n"
                                         "line 3\n";

    create_file(source_path_, kSource);

    example::filter_duplicate_lines(source_path_.string(), dest_path_.string());

    auto content = std::string{};
    read_file(dest_path_, content);
    EXPECT_EQ(content, kSource);
}

TEST_F(FilterDuplicateLinesTest, HandlesFileWhereAllLinesAreDuplicates)
{
    const std::string_view kSource = "repeat\n"
                                     "repeat\n"
                                     "repeat\n"
                                     "repeat\n";

    const std::string_view kExpected = "repeat\n";

    create_file(source_path_, kSource);

    example::filter_duplicate_lines(source_path_.string(), dest_path_.string());

    auto content = std::string{};
    read_file(dest_path_, content);
    EXPECT_EQ(content, kExpected);
}

TEST_F(FilterDuplicateLinesTest, OverwritesExistingDestinationFile)
{
    constexpr std::string_view kSource = "new\ncontent\n";
    constexpr std::string_view kDestination = "this old content must be overwritten";

    create_file(source_path_, kSource);
    create_file(dest_path_, kDestination);

    example::filter_duplicate_lines(source_path_.string(), dest_path_.string());

    auto content = std::string{};
    read_file(dest_path_, content);
    EXPECT_EQ(content, kSource);
}

TEST_F(FilterDuplicateLinesTest, ThrowsWhenSourceAndDestinationAreSameFile)
{
    create_file(source_path_, "content");

    ASSERT_THROW(([&]() {
                     example::filter_duplicate_lines(source_path_.string(),
                                                     source_path_.string());
                 })(),
                 std::runtime_error);
}

TEST_F(FilterDuplicateLinesTest, ThrowsWhenSourceFileDoesNotExist)
{
    ASSERT_THROW(([&]() {
                     example::filter_duplicate_lines(source_path_.string(),
                                                     dest_path_.string());
                 })(),
                 std::runtime_error);
}

TEST(ParseUrlEncodedTest, HandlesEmptyString)
{
    EXPECT_EQ(example::parse_url_encoded(""), "");
}

TEST(ParseUrlEncodedTest, KeepsNormalTextUnchanged)
{
    constexpr std::string_view kPlainText = "Simple text. 123. ~_-\n";

    EXPECT_EQ(example::parse_url_encoded(kPlainText), kPlainText);
}

TEST(ParseUrlEncodedTest, DecodesFullyPercentEncodedString)
{
    EXPECT_EQ(example::parse_url_encoded("%48%65%6C%6C%6F"), "Hello");
    EXPECT_EQ(example::parse_url_encoded("%20%21%22"), " !\"");
}

TEST(ParseUrlEncodedTest, DecodesMixedContent)
{
    EXPECT_EQ(example::parse_url_encoded("Hello%20World%21"), "Hello World!");
}

TEST(ParseUrlEncodedTest, HandlesDifferentHexadecimalCases)
{
    EXPECT_EQ(example::parse_url_encoded("%3a"), ":");
    EXPECT_EQ(example::parse_url_encoded("%3A"), ":");
    EXPECT_EQ(example::parse_url_encoded("%aA%bB%cC%dD%eE%fF"), "\xAA\xBB\xCC\xDD\xEE\xFF");
}

TEST(ParseUrlEncodedTest, DecodesBoundaryHexValues)
{
    EXPECT_EQ(example::parse_url_encoded("%00%FF"), "\x00\xFF"sv);
}

TEST(ParseUrlEncodedTest, ThrowsOnTrailingPercent)
{
    EXPECT_THROW(example::parse_url_encoded("%"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("some-text%"), example::ParsingException);
}

TEST(ParseUrlEncodedTest, ThrowsOnIncompletePercentSequence)
{
    EXPECT_THROW(example::parse_url_encoded("%a"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("starts_ok%1"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("%a%20"), example::ParsingException);
}

TEST(ParseUrlEncodedTest, ThrowsOnInvalidHexCharacters)
{
    EXPECT_THROW(example::parse_url_encoded("%zz"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("%GG"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("%%"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("%1G"), example::ParsingException);
    EXPECT_THROW(example::parse_url_encoded("%G1"), example::ParsingException);
}

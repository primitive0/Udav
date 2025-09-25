#ifndef EXAMPLE_H_
#define EXAMPLE_H_

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

namespace example {

class ParsingException : std::exception {};

namespace __detail {

inline auto parse_hexadecimal_digit(char ch) -> int32_t
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    } else if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }

    throw ParsingException{};
}

inline auto parse_percent_octet_data(std::string_view str, size_t& cursor) -> char
{
    auto octets = str.substr(cursor);
    if (octets.size() < 2) {
        throw ParsingException{};
    }

    auto high_nibble = parse_hexadecimal_digit(octets[0]);
    auto low_nibble = parse_hexadecimal_digit(octets[1]);
    auto ch = static_cast<char>(high_nibble * 16 + low_nibble);

    cursor += 2;
    return ch;
}

} // namespace __detail

inline auto filter_duplicate_lines(std::string_view source,
                                   std::string_view destination) -> void
{
    if (std::filesystem::exists(source) &&
        std::filesystem::exists(destination) &&
        std::filesystem::equivalent(source, destination)) {
        throw std::runtime_error{"Source and destination files must be different."};
    }

    auto input = std::ifstream{std::string{source}};
    if (!input.is_open()) {
        throw std::runtime_error{"Failed to open source file."};
    }

    auto output = std::ofstream{std::string{destination}};
    if (!output.is_open()) {
        throw std::runtime_error{"Failed to open destination file."};
    }

    auto line_set = std::unordered_set<std::string>{};
    auto line = std::string{};
    while (std::getline(input, line)) {
        auto [_, inserted] = line_set.insert(line);
        if (inserted) {
            output << line << "\n";

            if (output.bad()) {
                throw std::runtime_error{"Error while writing to destination file."};
            }
        }
    }

    if (input.bad()) {
        throw std::runtime_error{"Error while reading source file."};
    }
}

inline auto parse_url_encoded(std::string_view str) -> std::string
{
    constexpr char kPrecentChar = '%';

    auto result = std::string{};

    auto cursor = size_t{0};
    while (cursor != str.size()) {
        if (str[cursor] == kPrecentChar) {
            ++cursor;
            result += __detail::parse_percent_octet_data(str, cursor);
        } else {
            result += str[cursor];
            ++cursor;
        }
    }

    return result;
}

} // namespace example

#endif // EXAMPLE_H_

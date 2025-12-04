export module udav.text.utf8;

namespace udav::text::utf8 {

export constexpr char32_t kBom = U'\xFEFF';

export auto is_ascii_lowercase(char32_t ch) -> bool
{
    return U'a' <= ch && ch <= U'z';
}

export auto is_ascii_uppercase(char32_t ch) -> bool
{
    return U'A' <= ch && ch <= U'Z';
}

export auto is_ascii_alphabetic(char32_t ch) -> bool
{
    return is_ascii_lowercase(ch) || is_ascii_uppercase(ch);
}

export auto is_ascii_digit(char32_t ch) -> bool
{
    return U'0' <= ch && ch <= U'9';
}

export auto is_ascii_alphanumeric(char32_t ch) -> bool
{
    return is_ascii_alphabetic(ch) || is_ascii_digit(ch);
}

} // namespace udav::text::utf8

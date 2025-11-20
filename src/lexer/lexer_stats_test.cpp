#include <gtest/gtest.h>

#include "support/string.hpp"

import udav.lexer;
import udav.lexer.stats;

using namespace udav::lexer;

TEST(LexicalStatsTest, CountSimpleProgram)
{
    auto s = stats::collect(
        "fun main():\n"                           // kw: 1, sym: 1, oth: 4
        "    let x = 1\n"                         // kw: 1, sym: 1, num: 1, op: 1 oth: 2
        "    let y = 2\n"                         // kw: 1, sym: 1, num: 1, op: 1 oth: 1
        "    io.println(\"sum = {}\", x + y)\n"); // sym: 4, str: 1, op: 1, oth: 7

    // clang-format off
    EXPECT_EQ(s.keywords,        size_t{ 3});
    EXPECT_EQ(s.symbols,         size_t{ 7});
    EXPECT_EQ(s.number_literals, size_t{ 2});
    EXPECT_EQ(s.string_literals, size_t{ 1});
    EXPECT_EQ(s.operators,       size_t{ 3});
    EXPECT_EQ(s.other_tokens,    size_t{14});
    // clang-format on
}

TEST(LexicalStatsTest, CountFizzBuzzProgram)
{
    auto s = stats::collect(
        "# FizzBuzz program\n"                   // oth: 2
        "fun main():\n"                          // kw: 1, sym: 1, oth: 4
        "    let n = 1\n"                        // kw: 1, sym: 1, num: 1, op: 1, oth: 2
        "    while n <= 100:\n"                  // kw: 1, sym: 1, num: 1, op: 1, oth: 1
        "        if n % 15 == 0:\n"              // kw: 1, sym: 1, num: 2, op: 2, oth: 3
        "            io.println(\"FizzBuzz\")\n" // sym: 2, str: 1, oth: 4
        "        elif n % 3 == 0:\n"             // kw: 1, sym: 1, num: 2, op: 2, oth: 2
        "            io.println(\"Fizz\")\n"     // sym: 2, str: 1, oth: 4
        "        elif n % 5 == 0:\n"             // kw: 1, sym: 1, num: 2, op: 2, oth: 2
        "            io.println(\"Buzz\")\n"     // sym: 2, str: 1, oth: 4
        "        else:\n"                        // kw: 1, oth: 2
        "            io.println(\"{}\", n)\n"    // sym: 3, str: 1, oth: 5
        "        n += 1\n");                     // sym: 1, num: 1, op: 1, oth: 4

    // clang-format off
    EXPECT_EQ(s.keywords,        size_t{ 7});
    EXPECT_EQ(s.symbols,         size_t{16});
    EXPECT_EQ(s.number_literals, size_t{ 9});
    EXPECT_EQ(s.string_literals, size_t{ 4});
    EXPECT_EQ(s.operators,       size_t{ 9});
    EXPECT_EQ(s.other_tokens,    size_t{48});
    // clang-format on
}

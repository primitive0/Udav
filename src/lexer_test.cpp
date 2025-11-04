#include <gtest/gtest.h>

import udav.lexer;

TEST(LexerTest, Get42Returns42)
{
    EXPECT_EQ(udav::lexer::get_42(), 42);
}

#include <gtest/gtest.h>

#include "minidb/common/config.h"

TEST(HelloDBMS, EngineBoots)
{
    EXPECT_TRUE(true);
}

TEST(HelloDBMS, BasicMath)
{
    EXPECT_EQ(2 + 2, 4);
}

TEST(ConfigTest, PageSizeIsCorrect)
{
    EXPECT_EQ(minidb::PAGE_SIZE, 4096);
}

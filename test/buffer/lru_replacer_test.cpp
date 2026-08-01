#include <gtest/gtest.h>

#include "minidb/buffer/lru_replacer.h"

namespace minidb
{

// ---------------------------------------------------------
// Empty replacer
// ---------------------------------------------------------

TEST(LRUReplacerTest, EmptyReplacer)
{
    LRUReplacer replacer(10);

    frame_id_t victim;

    EXPECT_FALSE(replacer.Victim(&victim));
    EXPECT_EQ(replacer.Size(), 0);
}

// ---------------------------------------------------------
// Single victim
// ---------------------------------------------------------

TEST(LRUReplacerTest, SingleVictim)
{
    LRUReplacer replacer(10);

    replacer.Unpin(3);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));

    EXPECT_EQ(victim, 3);
    EXPECT_EQ(replacer.Size(), 0);
}

// ---------------------------------------------------------
// Least Recently Used ordering
// ---------------------------------------------------------

TEST(LRUReplacerTest, LeastRecentlyUsedOrder)
{
    LRUReplacer replacer(10);

    replacer.Unpin(1);
    replacer.Unpin(2);
    replacer.Unpin(3);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 1);

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 2);

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 3);

    EXPECT_FALSE(replacer.Victim(&victim));
}

// ---------------------------------------------------------
// Pin removes frame
// ---------------------------------------------------------

TEST(LRUReplacerTest, PinRemovesFrame)
{
    LRUReplacer replacer(10);

    replacer.Unpin(1);
    replacer.Unpin(2);

    replacer.Pin(1);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));

    EXPECT_EQ(victim, 2);

    EXPECT_FALSE(replacer.Victim(&victim));
}

// ---------------------------------------------------------
// Unpin adds frame
// ---------------------------------------------------------

TEST(LRUReplacerTest, UnpinAddsFrame)
{
    LRUReplacer replacer(10);

    replacer.Pin(7);
    replacer.Unpin(7);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));

    EXPECT_EQ(victim, 7);
}

// ---------------------------------------------------------
// Duplicate Unpin handling
// ---------------------------------------------------------

TEST(LRUReplacerTest, DuplicateUnpinHandling)
{
    LRUReplacer replacer(10);

    replacer.Unpin(5);
    replacer.Unpin(5);
    replacer.Unpin(5);

    EXPECT_EQ(replacer.Size(), 1);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));

    EXPECT_EQ(victim, 5);

    EXPECT_FALSE(replacer.Victim(&victim));
}

// ---------------------------------------------------------
// Interleaved access
// ---------------------------------------------------------

TEST(LRUReplacerTest, InterleavedAccess)
{
    LRUReplacer replacer(10);

    replacer.Unpin(1);
    replacer.Unpin(2);

    replacer.Pin(1);

    replacer.Unpin(1);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 2);

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 1);

    EXPECT_FALSE(replacer.Victim(&victim));
}

// ---------------------------------------------------------
// Size bookkeeping
// ---------------------------------------------------------

TEST(LRUReplacerTest, SizeTracking)
{
    LRUReplacer replacer(10);

    EXPECT_EQ(replacer.Size(), 0);

    replacer.Unpin(1);
    EXPECT_EQ(replacer.Size(), 1);

    replacer.Unpin(2);
    EXPECT_EQ(replacer.Size(), 2);

    replacer.Pin(1);
    EXPECT_EQ(replacer.Size(), 1);

    frame_id_t victim;

    bool success = replacer.Victim(&victim);
    EXPECT_TRUE(success);
    EXPECT_EQ(replacer.Size(), 0);
}

// ---------------------------------------------------------
// Pin on non-existent frame
// ---------------------------------------------------------

TEST(LRUReplacerTest, PinNonResidentFrame)
{
    LRUReplacer replacer(10);

    replacer.Pin(42);

    EXPECT_EQ(replacer.Size(), 0);

    frame_id_t victim;

    EXPECT_FALSE(replacer.Victim(&victim));
}

// ---------------------------------------------------------
// Reinsert after eviction
// ---------------------------------------------------------

TEST(LRUReplacerTest, ReinsertAfterVictim)
{
    LRUReplacer replacer(10);

    replacer.Unpin(4);

    frame_id_t victim;

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 4);

    replacer.Unpin(4);

    ASSERT_TRUE(replacer.Victim(&victim));
    EXPECT_EQ(victim, 4);

    EXPECT_FALSE(replacer.Victim(&victim));
}

} // namespace minidb

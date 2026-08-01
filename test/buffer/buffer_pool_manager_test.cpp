#include <cstdio>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.h"
#include "minidb/disk/disk_manager.h"

namespace minidb
{

class BufferPoolManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        db_file_ = "buffer_pool_test.db";

        disk_manager_ = std::make_unique<DiskManager>(db_file_);

        buffer_pool_ = std::make_unique<BufferPoolManager>(2, disk_manager_.get());
    }

    void TearDown() override
    {
        buffer_pool_.reset();
        disk_manager_.reset();

        std::remove(db_file_.c_str());
    }

    std::string db_file_;

    std::unique_ptr<DiskManager> disk_manager_;

    std::unique_ptr<BufferPoolManager> buffer_pool_;
};

//
// ---------------------------------------------------------
// Page creation
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, NewPageReturnsValidPage)
{
    page_id_t page_id;

    auto page = buffer_pool_->NewPage(&page_id);

    ASSERT_TRUE(page);

    EXPECT_NE(*page, nullptr);
}

TEST_F(BufferPoolManagerTest, NewPageAllocatesUniquePageIds)
{
    page_id_t id1;
    page_id_t id2;

    ASSERT_TRUE(buffer_pool_->NewPage(&id1));
    ASSERT_TRUE(buffer_pool_->NewPage(&id2));

    EXPECT_NE(id1, id2);
}

//
// ---------------------------------------------------------
// Fetching
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, FetchExistingPage)
{
    page_id_t page_id;

    auto page1 = buffer_pool_->NewPage(&page_id);

    ASSERT_TRUE(page1);

    ASSERT_TRUE(buffer_pool_->UnpinPage(page_id, false));

    auto page2 = buffer_pool_->FetchPage(page_id);

    ASSERT_TRUE(page2);

    EXPECT_EQ(*page1, *page2);
}

TEST_F(BufferPoolManagerTest, FetchInvalidPageFails)
{
    auto page = buffer_pool_->FetchPage(INVALID_PAGE_ID);

    EXPECT_FALSE(page);
}

//
// ---------------------------------------------------------
// Pin count handling
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, MultipleFetchesIncreasePinCount)
{
    page_id_t page_id;

    ASSERT_TRUE(buffer_pool_->NewPage(&page_id));

    ASSERT_TRUE(buffer_pool_->FetchPage(page_id));
    ASSERT_TRUE(buffer_pool_->FetchPage(page_id));

    EXPECT_TRUE(buffer_pool_->UnpinPage(page_id, false));

    EXPECT_TRUE(buffer_pool_->UnpinPage(page_id, false));

    EXPECT_TRUE(buffer_pool_->UnpinPage(page_id, false));
}

TEST_F(BufferPoolManagerTest, InvalidUnpinFails)
{
    EXPECT_FALSE(buffer_pool_->UnpinPage(INVALID_PAGE_ID, false));
}

//
// ---------------------------------------------------------
// Dirty pages
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, FlushPageSucceeds)
{
    page_id_t page_id;

    ASSERT_TRUE(buffer_pool_->NewPage(&page_id));

    ASSERT_TRUE(buffer_pool_->UnpinPage(page_id, true));

    EXPECT_TRUE(buffer_pool_->FlushPage(page_id));
}

TEST_F(BufferPoolManagerTest, FlushAllPagesSucceeds)
{
    page_id_t id1;
    page_id_t id2;

    ASSERT_TRUE(buffer_pool_->NewPage(&id1));
    ASSERT_TRUE(buffer_pool_->NewPage(&id2));

    ASSERT_TRUE(buffer_pool_->UnpinPage(id1, true));

    ASSERT_TRUE(buffer_pool_->UnpinPage(id2, true));

    EXPECT_TRUE(buffer_pool_->FlushAllPages());
}

//
// ---------------------------------------------------------
// Eviction
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, PoolFullWhenAllPagesPinned)
{
    page_id_t id1;
    page_id_t id2;
    page_id_t id3;

    ASSERT_TRUE(buffer_pool_->NewPage(&id1));
    ASSERT_TRUE(buffer_pool_->NewPage(&id2));

    auto page = buffer_pool_->NewPage(&id3);

    EXPECT_FALSE(page);
}

TEST_F(BufferPoolManagerTest, FrameReuseAfterEviction)
{
    page_id_t id1;
    page_id_t id2;
    page_id_t id3;

    ASSERT_TRUE(buffer_pool_->NewPage(&id1));
    ASSERT_TRUE(buffer_pool_->NewPage(&id2));

    ASSERT_TRUE(buffer_pool_->UnpinPage(id1, false));

    ASSERT_TRUE(buffer_pool_->NewPage(&id3));
}

TEST_F(BufferPoolManagerTest, FetchLoadsEvictedPage)
{
    page_id_t id1;
    page_id_t id2;
    page_id_t id3;

    ASSERT_TRUE(buffer_pool_->NewPage(&id1));
    ASSERT_TRUE(buffer_pool_->NewPage(&id2));

    ASSERT_TRUE(buffer_pool_->UnpinPage(id1, true));

    ASSERT_TRUE(buffer_pool_->NewPage(&id3));
}

//
// ---------------------------------------------------------
// Stress
// ---------------------------------------------------------
//

TEST_F(BufferPoolManagerTest, RepeatedFetchUnpin)
{
    page_id_t page_id;

    ASSERT_TRUE(buffer_pool_->NewPage(&page_id));

    for (int i = 0; i < 100; ++i)
    {
        ASSERT_TRUE(buffer_pool_->FetchPage(page_id));

        ASSERT_TRUE(buffer_pool_->UnpinPage(page_id, false));
    }
}

} // namespace minidb

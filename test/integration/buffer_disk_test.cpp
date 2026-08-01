#include <cstdio>
#include <string>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.h"
#include "minidb/disk/disk_manager.h"
#include "minidb/storage/page.h"
#include "minidb/storage/serializer.h"
#include "minidb/storage/tuple.h"

namespace minidb
{

class BufferDiskTest : public ::testing::Test
{
  protected:
    void SetUp() override { filename_ = "buffer_disk_test.db"; }

    void TearDown() override { std::remove(filename_.c_str()); }

    std::string filename_;
};

TEST_F(BufferDiskTest, DirtyPageSurvivesEviction)
{
    page_id_t first_page_id;
    page_id_t second_page_id;
    page_id_t third_page_id;

    RID rid;

    Tuple original;

    original.AddInteger(999);
    original.AddString("dirty eviction test");

    {
        DiskManager disk_manager(filename_);

        /*
         * Small pool to force eviction.
         */
        BufferPoolManager bpm(2, &disk_manager);

        /*
         * Create first page.
         */

        auto first_page = bpm.NewPage(&first_page_id);

        ASSERT_TRUE(first_page.Ok());

        Page* page = first_page.Value();

        auto bytes = Serializer::Serialize(original);

        auto rid_result = page->InsertRecord(bytes.data(), bytes.size());

        ASSERT_TRUE(rid_result.has_value());

        rid = rid_result.value();

        /*
         * Dirty page becomes evictable.
         */

        ASSERT_TRUE(bpm.UnpinPage(first_page_id, true).Ok());

        /*
         * Fill remaining frame.
         */

        ASSERT_TRUE(bpm.NewPage(&second_page_id).Ok());

        ASSERT_TRUE(bpm.UnpinPage(second_page_id, true).Ok());

        /*
         * Force eviction.
         *
         * first_page should be selected by LRU.
         */

        ASSERT_TRUE(bpm.NewPage(&third_page_id).Ok());

        ASSERT_TRUE(bpm.UnpinPage(third_page_id, true).Ok());

        /*
         * Reload first page.
         *
         * This requires:
         *
         * eviction
         * dirty flush
         * disk read
         */

        auto recovered_page = bpm.FetchPage(first_page_id);

        ASSERT_TRUE(recovered_page.Ok());

        Page* recovered = recovered_page.Value();

        auto record = recovered->GetRecord(rid);

        ASSERT_TRUE(record.has_value());

        Tuple result = Serializer::Deserialize(record.value());

        EXPECT_EQ(result, original);

        ASSERT_TRUE(bpm.UnpinPage(first_page_id, false).Ok());
    }
}

} // namespace minidb

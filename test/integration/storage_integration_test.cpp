#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.h"
#include "minidb/disk/disk_manager.h"
#include "minidb/storage/page.h"
#include "minidb/storage/serializer.h"
#include "minidb/storage/tuple.h"

namespace minidb
{

class StorageIntegrationTest : public ::testing::Test
{
  protected:
    void SetUp() override { filename_ = "storage_integration_test.db"; }

    void TearDown() override { std::remove(filename_.c_str()); }

    std::string filename_;
};

TEST_F(StorageIntegrationTest, TupleSurvivesFullStoragePipeline)
{
    page_id_t page_id;

    RID rid;

    Tuple original;

    /*
     * Create test tuple.
     */

    original.AddInteger(123);
    original.AddString("MiniDB storage test");

    /*
     * -----------------------------
     * Write path
     *
     * Tuple
     *   |
     * Serializer
     *   |
     * Page
     *   |
     * BufferPoolManager
     *   |
     * DiskManager
     *
     * -----------------------------
     */

    {
        DiskManager disk_manager(filename_);

        BufferPoolManager bpm(2, &disk_manager);

        auto page_result = bpm.NewPage(&page_id);

        ASSERT_TRUE(page_result.Ok());

        Page* page = page_result.Value();

        /*
         * Serialize tuple.
         */

        std::vector<std::byte> bytes = Serializer::Serialize(original);

        ASSERT_FALSE(bytes.empty());

        /*
         * Store serialized tuple.
         */

        auto rid_result = page->InsertRecord(bytes.data(), bytes.size());

        ASSERT_TRUE(rid_result.has_value());

        rid = rid_result.value();

        /*
         * Release page.
         *
         * Dirty=true ensures disk write.
         */

        ASSERT_TRUE(bpm.UnpinPage(page_id, true).Ok());

        ASSERT_TRUE(bpm.FlushAllPages().Ok());
    }

    /*
     * -----------------------------
     * Read path
     *
     * DiskManager
     *   |
     * BufferPoolManager
     *   |
     * Page
     *   |
     * Serializer
     *   |
     * Tuple
     *
     * -----------------------------
     */

    {
        DiskManager disk_manager(filename_);

        BufferPoolManager bpm(2, &disk_manager);

        auto page_result = bpm.FetchPage(page_id);

        ASSERT_TRUE(page_result.Ok());

        Page* page = page_result.Value();

        auto record = page->GetRecord(rid);

        ASSERT_TRUE(record.has_value());

        Tuple recovered;

        ASSERT_NO_THROW(recovered = Serializer::Deserialize(record.value()));

        EXPECT_EQ(recovered, original);

        ASSERT_TRUE(bpm.UnpinPage(page_id, false).Ok());
    }
}

} // namespace minidb

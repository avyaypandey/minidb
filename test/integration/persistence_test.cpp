#include <cstdio>
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

TEST(PersistenceTest, DataSurvivesRestart)
{
    const std::string filename = "persistence_test.db";

    page_id_t page_id;

    RID rid;

    Tuple original;

    original.AddInteger(-42);
    original.AddString("persistent data");

    /*
     * First database instance.
     *
     * Write data and flush.
     */

    {
        DiskManager disk_manager(filename);

        BufferPoolManager bpm(2, &disk_manager);

        auto page_result = bpm.NewPage(&page_id);

        ASSERT_TRUE(page_result.Ok());

        Page* page = page_result.Value();

        auto serialized = Serializer::Serialize(original);

        auto rid_result = page->InsertRecord(serialized.data(), serialized.size());

        ASSERT_TRUE(rid_result.has_value());

        rid = rid_result.value();

        ASSERT_TRUE(bpm.UnpinPage(page_id, true).Ok());

        ASSERT_TRUE(bpm.FlushAllPages().Ok());
    }

    /*
     * Simulate restart.
     *
     * New DiskManager.
     * New BufferPoolManager.
     */

    {
        DiskManager disk_manager(filename);

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

    std::remove(filename.c_str());
}

} // namespace minidb

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "minidb/disk/disk_manager.h"
#include "minidb/storage/page.h"
#include "minidb/storage/serializer.h"
#include "minidb/storage/tuple.h"

namespace minidb
{

class PageDiskTest : public ::testing::Test
{
  protected:
    void SetUp() override { filename_ = "page_disk_test.db"; }

    void TearDown() override { std::remove(filename_.c_str()); }

    std::string filename_;
};

TEST_F(PageDiskTest, WritePageReadPage)
{
    page_id_t page_id;

    std::vector<std::byte> original_bytes;

    /*
     * Write phase.
     */

    {
        DiskManager disk_manager(filename_);

        auto page_id_result = disk_manager.AllocatePage();

        ASSERT_TRUE(page_id_result.Ok());

        page_id = page_id_result.Value();

        Page page;

        /*
         * Create real page contents.
         */

        Tuple tuple;

        tuple.AddInteger(100);
        tuple.AddString("disk manager test");

        original_bytes = Serializer::Serialize(tuple);

        auto rid = page.InsertRecord(original_bytes.data(), original_bytes.size());

        ASSERT_TRUE(rid.has_value());

        ASSERT_TRUE(
            disk_manager.WritePage(page_id, reinterpret_cast<const char*>(page.GetData())).Ok());
    }

    /*
     * Read phase.
     *
     * New DiskManager instance to simulate reopen.
     */

    {
        DiskManager disk_manager(filename_);

        Page recovered;

        ASSERT_TRUE(
            disk_manager.ReadPage(page_id, reinterpret_cast<char*>(recovered.GetData())).Ok());

        auto record = recovered.GetRecord(RID{0, 0});

        ASSERT_TRUE(record.has_value());

        Tuple recovered_tuple = Serializer::Deserialize(record.value());

        Tuple expected_tuple;

        expected_tuple.AddInteger(100);
        expected_tuple.AddString("disk manager test");

        EXPECT_EQ(recovered_tuple, expected_tuple);
    }
}

} // namespace minidb

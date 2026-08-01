#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "minidb/common/config.h"
#include "minidb/common/status.h"
#include "minidb/disk/disk_manager.h"

namespace minidb
{

class DiskManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        db_file_ = "disk_manager_test.db";

        disk_manager_ = std::make_unique<DiskManager>(db_file_);
    }

    void TearDown() override
    {
        disk_manager_.reset();

        std::remove(db_file_.c_str());
    }

    std::string db_file_;

    std::unique_ptr<DiskManager> disk_manager_;
};

// ---------------------------------------------------------
// Allocation
// ---------------------------------------------------------

TEST_F(DiskManagerTest, AllocateSequentialPages)
{
    auto p1 = disk_manager_->AllocatePage();
    auto p2 = disk_manager_->AllocatePage();
    auto p3 = disk_manager_->AllocatePage();

    ASSERT_TRUE(p1);
    ASSERT_TRUE(p2);
    ASSERT_TRUE(p3);

    EXPECT_EQ(*p1, 0);
    EXPECT_EQ(*p2, 1);
    EXPECT_EQ(*p3, 2);
}

// ---------------------------------------------------------
// Single page read/write
// ---------------------------------------------------------

TEST_F(DiskManagerTest, WriteSinglePage)
{
    auto page_result = disk_manager_->AllocatePage();

    ASSERT_TRUE(page_result);

    page_id_t page_id = *page_result;

    char write_buffer[PAGE_SIZE];

    memset(write_buffer, 'A', PAGE_SIZE);

    EXPECT_TRUE(disk_manager_->WritePage(page_id, write_buffer));

    char read_buffer[PAGE_SIZE];

    memset(read_buffer, 0, PAGE_SIZE);

    EXPECT_TRUE(disk_manager_->ReadPage(page_id, read_buffer));

    EXPECT_EQ(memcmp(write_buffer, read_buffer, PAGE_SIZE), 0);
}

TEST_F(DiskManagerTest, ReadSinglePage)
{
    auto page_result = disk_manager_->AllocatePage();

    ASSERT_TRUE(page_result);

    page_id_t page_id = *page_result;

    char expected[PAGE_SIZE];

    for (size_t i = 0; i < PAGE_SIZE; i++)
    {
        expected[i] = static_cast<char>(i % 256);
    }

    ASSERT_TRUE(disk_manager_->WritePage(page_id, expected));

    char actual[PAGE_SIZE];

    ASSERT_TRUE(disk_manager_->ReadPage(page_id, actual));

    EXPECT_EQ(memcmp(expected, actual, PAGE_SIZE), 0);
}

// ---------------------------------------------------------
// Multiple pages
// ---------------------------------------------------------

TEST_F(DiskManagerTest, WriteMultiplePages)
{
    auto page1 = disk_manager_->AllocatePage();
    auto page2 = disk_manager_->AllocatePage();

    ASSERT_TRUE(page1);
    ASSERT_TRUE(page2);

    char buffer1[PAGE_SIZE];
    char buffer2[PAGE_SIZE];

    memset(buffer1, 'X', PAGE_SIZE);
    memset(buffer2, 'Y', PAGE_SIZE);

    ASSERT_TRUE(disk_manager_->WritePage(*page1, buffer1));

    ASSERT_TRUE(disk_manager_->WritePage(*page2, buffer2));

    char result1[PAGE_SIZE];
    char result2[PAGE_SIZE];

    ASSERT_TRUE(disk_manager_->ReadPage(*page1, result1));

    ASSERT_TRUE(disk_manager_->ReadPage(*page2, result2));

    EXPECT_EQ(memcmp(buffer1, result1, PAGE_SIZE), 0);

    EXPECT_EQ(memcmp(buffer2, result2, PAGE_SIZE), 0);
}

// ---------------------------------------------------------
// Overwrite
// ---------------------------------------------------------

TEST_F(DiskManagerTest, OverwriteExistingPage)
{
    auto page = disk_manager_->AllocatePage();

    ASSERT_TRUE(page);

    char first[PAGE_SIZE];
    char second[PAGE_SIZE];

    memset(first, 'A', PAGE_SIZE);
    memset(second, 'B', PAGE_SIZE);

    ASSERT_TRUE(disk_manager_->WritePage(*page, first));

    ASSERT_TRUE(disk_manager_->WritePage(*page, second));

    char result[PAGE_SIZE];

    ASSERT_TRUE(disk_manager_->ReadPage(*page, result));

    EXPECT_EQ(memcmp(second, result, PAGE_SIZE), 0);
}

// ---------------------------------------------------------
// Persistence
// ---------------------------------------------------------

TEST(DiskManagerPersistenceTest, DataPersistsAcrossRestart)
{
    const std::string filename = "disk_manager_restart_test.db";

    page_id_t page_id;

    {
        DiskManager manager(filename);

        auto page = manager.AllocatePage();

        ASSERT_TRUE(page);

        page_id = *page;

        char buffer[PAGE_SIZE];

        memset(buffer, 'Z', PAGE_SIZE);

        ASSERT_TRUE(manager.WritePage(page_id, buffer));
    }

    {
        DiskManager manager(filename);

        char buffer[PAGE_SIZE];

        ASSERT_TRUE(manager.ReadPage(page_id, buffer));

        EXPECT_EQ(buffer[0], 'Z');
    }

    std::remove(filename.c_str());
}

// ---------------------------------------------------------
// Error handling
// ---------------------------------------------------------

TEST_F(DiskManagerTest, InvalidPageAccess)
{
    char buffer[PAGE_SIZE];

    auto read_status = disk_manager_->ReadPage(INVALID_PAGE_ID, buffer);

    EXPECT_FALSE(read_status);

    auto write_status = disk_manager_->WritePage(INVALID_PAGE_ID, buffer);

    EXPECT_FALSE(write_status);
}

} // namespace minidb

#include <gtest/gtest.h>

#include "minidb/common/config.h"
#include "minidb/storage/page.h"

namespace minidb {

    TEST(PageTest, InitializesEmptyPage)
    {
        Page page;

        EXPECT_EQ(page.NumSlots(), 0);
        EXPECT_EQ(page.FreeSpaceOffset(), PAGE_SIZE);
    }

    TEST(PageTest, InitializesFullFreeSpace)
    {
        Page page;

        EXPECT_GT(page.FreeSpace(), 0);

        EXPECT_EQ(
            page.FreeSpace(),
                  PAGE_SIZE - 4
        );
    }

    TEST(PageTest, InsertsSingleRecord)
    {
        Page page;

        std::vector<std::byte> record = {
            std::byte{0x01},
            std::byte{0x02},
            std::byte{0x03},
            std::byte{0x04}
        };

        auto rid = page.InsertRecord(
            record.data(),
            record.size()
        );

        ASSERT_TRUE(rid.has_value());

        EXPECT_EQ(rid->page_id, 0);
        EXPECT_EQ(rid->slot_id, 0);

        EXPECT_EQ(page.NumSlots(), 1);
    }

    TEST(PageTest, RetrievesInsertedRecord)
    {
        Page page;

        std::vector<std::byte> record = {
            std::byte{0x01},
            std::byte{0x02},
            std::byte{0x03},
            std::byte{0x04}
        };

        auto rid = page.InsertRecord(
            record.data(),
                                     record.size()
        );

        ASSERT_TRUE(rid.has_value());

        auto result = page.GetRecord(*rid);

        ASSERT_TRUE(result.has_value());

        EXPECT_EQ(result.value(), record);
    }


    TEST(PageTest, RetrievesMultipleRecords)
    {
        Page page;

        std::vector<std::byte> first = {
            std::byte{0xAA},
            std::byte{0xBB}
        };

        std::vector<std::byte> second = {
            std::byte{0x10},
            std::byte{0x20},
            std::byte{0x30},
            std::byte{0x40}
        };

        std::vector<std::byte> third = {
            std::byte{0xFF}
        };

        auto first_rid = page.InsertRecord(
            first.data(),
                                           first.size()
        );

        auto second_rid = page.InsertRecord(
            second.data(),
                                            second.size()
        );

        auto third_rid = page.InsertRecord(
            third.data(),
                                           third.size()
        );

        ASSERT_TRUE(first_rid.has_value());
        ASSERT_TRUE(second_rid.has_value());
        ASSERT_TRUE(third_rid.has_value());

        EXPECT_EQ(page.GetRecord(*first_rid).value(), first);
        EXPECT_EQ(page.GetRecord(*second_rid).value(), second);
        EXPECT_EQ(page.GetRecord(*third_rid).value(), third);
    }


    TEST(PageTest, RejectsInvalidSlotID)
    {
        Page page;

        RID invalid{
            0,
            100
        };

        auto result = page.GetRecord(invalid);

        EXPECT_FALSE(result.has_value());
    }


    TEST(PageTest, RejectsInvalidPageID)
    {
        Page page;

        RID invalid{
            99,
            0
        };

        auto result = page.GetRecord(invalid);

        EXPECT_FALSE(result.has_value());
    }

    TEST(PageTest, DeletesRecord)
    {
        Page page;

        std::vector<std::byte> record = {
            std::byte{0x01},
            std::byte{0x02},
            std::byte{0x03}
        };

        auto rid = page.InsertRecord(
            record.data(),
                                     record.size()
        );

        ASSERT_TRUE(rid.has_value());

        EXPECT_TRUE(page.DeleteRecord(*rid));

        auto result = page.GetRecord(*rid);

        EXPECT_FALSE(result.has_value());
    }


    TEST(PageTest, DeleteDoesNotAffectOtherRecords)
    {
        Page page;

        std::vector<std::byte> first = {
            std::byte{0x10}
        };

        std::vector<std::byte> second = {
            std::byte{0x20},
            std::byte{0x30}
        };

        std::vector<std::byte> third = {
            std::byte{0x40},
            std::byte{0x50},
            std::byte{0x60}
        };

        auto first_rid = page.InsertRecord(
            first.data(),
                                           first.size()
        );

        auto second_rid = page.InsertRecord(
            second.data(),
                                            second.size()
        );

        auto third_rid = page.InsertRecord(
            third.data(),
                                           third.size()
        );

        ASSERT_TRUE(first_rid.has_value());
        ASSERT_TRUE(second_rid.has_value());
        ASSERT_TRUE(third_rid.has_value());

        EXPECT_TRUE(page.DeleteRecord(*second_rid));

        EXPECT_EQ(page.GetRecord(*first_rid).value(), first);

        EXPECT_FALSE(page.GetRecord(*second_rid).has_value());

        EXPECT_EQ(page.GetRecord(*third_rid).value(), third);
    }


    TEST(PageTest, RejectsInvalidDeleteRID)
    {
        Page page;

        RID invalid{
            0,
            100
        };

        EXPECT_FALSE(page.DeleteRecord(invalid));
    }


    TEST(PageTest, RejectsDeletingSameRecordTwice)
    {
        Page page;

        std::vector<std::byte> record = {
            std::byte{0xAA}
        };

        auto rid = page.InsertRecord(
            record.data(),
                                     record.size()
        );

        ASSERT_TRUE(rid.has_value());

        EXPECT_TRUE(page.DeleteRecord(*rid));

        EXPECT_FALSE(page.DeleteRecord(*rid));
    }

} // namespace minidb

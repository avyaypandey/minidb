#include <gtest/gtest.h>

#include "minidb/storage/tuple.h"

namespace minidb
{

TEST(FieldTest, StoresIntegerValue)
{
    Field field(42);

    const auto& value = field.GetValue();

    ASSERT_TRUE(std::holds_alternative<int32_t>(value));

    EXPECT_EQ(std::get<int32_t>(value), 42);
}

TEST(FieldTest, StoresStringValue)
{
    Field field("MiniDB");

    const auto& value = field.GetValue();

    ASSERT_TRUE(std::holds_alternative<std::string>(value));

    EXPECT_EQ(std::get<std::string>(value), "MiniDB");
}

TEST(TupleTest, AddsFieldsInOrder)
{
    Tuple tuple;

    tuple.AddInteger(100);
    tuple.AddString("hello");

    ASSERT_EQ(tuple.Size(), 2);

    const auto& fields = tuple.Fields();

    ASSERT_TRUE(std::holds_alternative<int32_t>(fields[0].GetValue()));

    EXPECT_EQ(std::get<int32_t>(fields[0].GetValue()), 100);

    ASSERT_TRUE(std::holds_alternative<std::string>(fields[1].GetValue()));

    EXPECT_EQ(std::get<std::string>(fields[1].GetValue()), "hello");
}

TEST(TupleTest, DifferentTuplesCompareUnequal)
{
    Tuple first;

    first.AddInteger(42);

    Tuple second;

    second.AddInteger(100);

    EXPECT_FALSE(first == second);
}

} // namespace minidb

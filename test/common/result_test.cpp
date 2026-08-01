#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "minidb/common/result.h"

using namespace minidb;

namespace
{

struct Dummy
{
    int value;

    int Increment() { return ++value; }
};

} // namespace

// -----------------------------------------------------------------------------
// Success construction
// -----------------------------------------------------------------------------

TEST(ResultTest, ConstructSuccess)
{
    Result<int> result(42);

    EXPECT_TRUE(result.Ok());
    EXPECT_TRUE(result);

    EXPECT_EQ(result.Value(), 42);
    EXPECT_EQ(*result, 42);
}

TEST(ResultTest, BoolConversion)
{
    Result<int> result(100);

    EXPECT_TRUE(result);
}

TEST(ResultTest, DereferenceOperator)
{
    Result<int> result(5);

    EXPECT_EQ(*result, 5);

    *result = 10;

    EXPECT_EQ(result.Value(), 10);
}

TEST(ResultTest, ArrowOperator)
{
    Result<Dummy> result(Dummy{5});

    EXPECT_EQ(result->value, 5);

    EXPECT_EQ(result->Increment(), 6);
    EXPECT_EQ(result->value, 6);
}

// -----------------------------------------------------------------------------
// Error construction
// -----------------------------------------------------------------------------
// TODO: Re-enable once a real error StatusCode exists.
//
// TEST(ResultTest, ConstructError)
// {
//     Result<int> result(Status(StatusCode::DISK_ERROR));
//     ...
// }

// -----------------------------------------------------------------------------
// Copy semantics
// -----------------------------------------------------------------------------

TEST(ResultTest, CopyConstructor)
{
    Result<std::string> original(std::string("MiniDB"));

    Result<std::string> copy(original);

    EXPECT_TRUE(copy);
    EXPECT_EQ(copy.Value(), "MiniDB");
}

TEST(ResultTest, CopyAssignment)
{
    Result<std::string> lhs(std::string("Old"));
    Result<std::string> rhs(std::string("New"));

    lhs = rhs;

    EXPECT_TRUE(lhs);
    EXPECT_EQ(lhs.Value(), "New");
}

// -----------------------------------------------------------------------------
// Move semantics
// -----------------------------------------------------------------------------

TEST(ResultTest, MoveConstructor)
{
    Result<std::unique_ptr<int>> result(std::make_unique<int>(123));

    Result<std::unique_ptr<int>> moved(std::move(result));

    ASSERT_TRUE(moved);
    EXPECT_EQ(**moved, 123);
}

TEST(ResultTest, MoveAssignment)
{
    Result<std::unique_ptr<int>> lhs(std::make_unique<int>(1));

    Result<std::unique_ptr<int>> rhs(std::make_unique<int>(999));

    lhs = std::move(rhs);

    ASSERT_TRUE(lhs);
    EXPECT_EQ(**lhs, 999);
}

// -----------------------------------------------------------------------------
// Assertion tests
// -----------------------------------------------------------------------------

#ifndef NDEBUG

TEST(ResultTest, ValueAssertsOnError)
{
    EXPECT_DEATH(
        {
            Result<int> result(Status(StatusCode::OK));
            result.Value();
        },
        "");
}

TEST(ResultTest, DereferenceAssertsOnError)
{
    EXPECT_DEATH(
        {
            Result<int> result(Status(StatusCode::OK));
            *result;
        },
        "");
}

TEST(ResultTest, ArrowOperatorAssertsOnError)
{
    EXPECT_DEATH(
        {
            Result<Dummy> result(Status(StatusCode::OK));
            result->Increment();
        },
        "");
}

#endif

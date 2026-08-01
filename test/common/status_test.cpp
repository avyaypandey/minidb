#include <gtest/gtest.h>

#include "minidb/common/status.h"

using namespace minidb;

TEST(StatusTest, DefaultConstructorIsOk)
{
    Status status;

    EXPECT_TRUE(status.Ok());
    EXPECT_TRUE(static_cast<bool>(status));
    EXPECT_EQ(status.Code(), StatusCode::OK);
}

TEST(StatusTest, ConstructFromStatusCode)
{
    Status status(StatusCode::OK);

    EXPECT_TRUE(status.Ok());
    EXPECT_TRUE(status);
    EXPECT_EQ(status.Code(), StatusCode::OK);
}

TEST(StatusTest, FactoryCreatesOkStatus)
{
    Status status = Status::OkStatus();

    EXPECT_TRUE(status.Ok());
    EXPECT_TRUE(status);
    EXPECT_EQ(status.Code(), StatusCode::OK);
}

TEST(StatusTest, CopyConstructor)
{
    Status original(StatusCode::OK);

    Status copy(original);

    EXPECT_EQ(copy.Code(), original.Code());
    EXPECT_EQ(copy.Ok(), original.Ok());
}

TEST(StatusTest, CopyAssignment)
{
    Status lhs;
    Status rhs(StatusCode::OK);

    lhs = rhs;

    EXPECT_EQ(lhs.Code(), rhs.Code());
    EXPECT_EQ(lhs.Ok(), rhs.Ok());
}

TEST(StatusTest, MoveConstructor)
{
    Status original(StatusCode::OK);

    Status moved(std::move(original));

    EXPECT_TRUE(moved.Ok());
    EXPECT_EQ(moved.Code(), StatusCode::OK);
}

TEST(StatusTest, MoveAssignment)
{
    Status lhs;
    Status rhs(StatusCode::OK);

    lhs = std::move(rhs);

    EXPECT_TRUE(lhs.Ok());
    EXPECT_EQ(lhs.Code(), StatusCode::OK);
}

#include <gtest/gtest.h>

#include "minidb/storage/serializer.h"
#include "minidb/storage/tuple.h"

namespace minidb {

    TEST(SerializerTest, SerializesEmptyTuple)
    {
        Tuple tuple;

        auto bytes = Serializer::Serialize(tuple);

        Tuple result = Serializer::Deserialize(bytes);

        EXPECT_EQ(result, tuple);
    }

    TEST(SerializerTest, SerializesIntegerField)
    {
        Tuple tuple;

        tuple.AddInteger(42);

        auto bytes = Serializer::Serialize(tuple);

        Tuple result = Serializer::Deserialize(bytes);

        EXPECT_EQ(result, tuple);
    }

    TEST(SerializerTest, SerializesStringField)
    {
        Tuple tuple;

        tuple.AddString("MiniDB");

        auto bytes = Serializer::Serialize(tuple);

        Tuple result = Serializer::Deserialize(bytes);

        EXPECT_EQ(result, tuple);
    }

    TEST(SerializerTest, SerializesMixedTuple)
    {
        Tuple tuple;

        tuple.AddInteger(100);
        tuple.AddString("storage engine");
        tuple.AddInteger(-50);

        auto bytes = Serializer::Serialize(tuple);

        Tuple result = Serializer::Deserialize(bytes);

        EXPECT_EQ(result, tuple);
    }

    TEST(SerializerTest, SerializesDifferentStringLengths)
    {
        Tuple tuple;

        tuple.AddString("a");
        tuple.AddString("short string");
        tuple.AddString(
            "this is a much longer string value"
        );

        auto bytes = Serializer::Serialize(tuple);

        Tuple result = Serializer::Deserialize(bytes);

        EXPECT_EQ(result, tuple);
    }

    TEST(SerializerTest, ProducesSerializedBytes)
    {
        Tuple tuple;

        tuple.AddInteger(10);

        auto bytes = Serializer::Serialize(tuple);

        EXPECT_FALSE(bytes.empty());
    }

    TEST(SerializerTest, RejectsTruncatedTuple)
    {
        std::vector<std::byte> bytes = {
            std::byte{0x01}
        };

        EXPECT_THROW(
            Serializer::Deserialize(bytes),
                     std::runtime_error
        );
    }

    TEST(SerializerTest, RejectsTruncatedString)
    {
        std::vector<std::byte> bytes = {
            std::byte{0x01}, // field count low byte
            std::byte{0x00}, // field count high byte

            std::byte{0x01}, // STRING type

            std::byte{0x05}, // length = 5
            std::byte{0x00},
            std::byte{0x00},
            std::byte{0x00},

            std::byte{'h'}
        };

        EXPECT_THROW(
            Serializer::Deserialize(bytes),
                     std::runtime_error
        );
    }

} // namespace minidb

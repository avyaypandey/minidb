#include "minidb/storage/serializer.h"

#include <cstdint>
#include <stdexcept>

namespace minidb
{

namespace
{

/**
 * @brief Writes a uint16_t value in little-endian format.
 */
void WriteUInt16(std::vector<std::byte>& buffer, uint16_t value)
{
    buffer.push_back(static_cast<std::byte>(value & 0xFF));

    buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
}

/**
 * @brief Writes a uint32_t value in little-endian format.
 */
void WriteUInt32(std::vector<std::byte>& buffer, uint32_t value)
{
    for (int i = 0; i < 4; i++)
    {
        buffer.push_back(static_cast<std::byte>((value >> (8 * i)) & 0xFF));
    }
}

bool HasBytes(const std::vector<std::byte>& buffer, std::size_t offset, std::size_t amount)
{
    return offset <= buffer.size() && amount <= buffer.size() - offset;
}

/**
 * @brief Reads a uint16_t value.
 */
uint16_t ReadUInt16(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    if (!HasBytes(buffer, offset, sizeof(uint16_t)))
    {
        throw std::runtime_error("Unexpected end of serialized data");
    }

    uint16_t value =
        static_cast<uint16_t>(buffer[offset]) | (static_cast<uint16_t>(buffer[offset + 1]) << 8);

    offset += sizeof(uint16_t);

    return value;
}

/**
 * @brief Reads a uint32_t value.
 */
uint32_t ReadUInt32(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    if (!HasBytes(buffer, offset, sizeof(uint32_t)))
    {
        throw std::runtime_error("Unexpected end of serialized data");
    }

    uint32_t value = 0;

    for (int i = 0; i < 4; i++)
    {
        value |= static_cast<uint32_t>(buffer[offset + i]) << (8 * i);
    }

    offset += sizeof(uint32_t);

    return value;
}

} // namespace

/**
 * @brief Serializes a tuple into MiniDB binary format.
 */
std::vector<std::byte> Serializer::Serialize(const Tuple& tuple)
{
    std::vector<std::byte> buffer;

    WriteUInt16(buffer, static_cast<uint16_t>(tuple.Size()));

    for (const Field& field : tuple.Fields())
    {
        const auto& value = field.GetValue();

        if (std::holds_alternative<int32_t>(value))
        {
            buffer.push_back(static_cast<std::byte>(FieldType::INTEGER));

            WriteUInt32(buffer, static_cast<uint32_t>(std::get<int32_t>(value)));
        }
        else if (std::holds_alternative<std::string>(value))
        {
            buffer.push_back(static_cast<std::byte>(FieldType::STRING));

            const std::string& str = std::get<std::string>(value);

            WriteUInt32(buffer, static_cast<uint32_t>(str.size()));

            for (char c : str)
            {
                buffer.push_back(static_cast<std::byte>(c));
            }
        }
    }

    return buffer;
}

/**
 * @brief Deserializes MiniDB binary format into a tuple.
 */
Tuple Serializer::Deserialize(const std::vector<std::byte>& data)
{
    Tuple tuple;

    std::size_t offset = 0;

    uint16_t field_count = ReadUInt16(data, offset);

    for (uint16_t i = 0; i < field_count; i++)
    {
        if (!HasBytes(data, offset, 1))
        {
            throw std::runtime_error("Unexpected end of serialized data");
        }

        FieldType type = static_cast<FieldType>(data[offset++]);

        switch (type)
        {
        case FieldType::INTEGER:
        {
            int32_t value = static_cast<int32_t>(ReadUInt32(data, offset));

            tuple.AddInteger(value);

            break;
        }

        case FieldType::STRING:
        {
            uint32_t length = ReadUInt32(data, offset);

            if (!HasBytes(data, offset, length))
            {
                throw std::runtime_error("String exceeds serialized buffer");
            }

            std::string value;

            value.reserve(length);

            for (uint32_t j = 0; j < length; j++)
            {
                value.push_back(static_cast<char>(data[offset++]));
            }

            tuple.AddString(value);

            break;
        }

        default:
            throw std::runtime_error("Unknown field type");
        }
    }

    return tuple;
}

} // namespace minidb

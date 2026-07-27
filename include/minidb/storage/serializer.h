#pragma once

#include <cstddef>
#include <vector>

#include "minidb/storage/tuple.h"

namespace minidb
{

/**
 * @brief Converts tuples to and from a portable binary representation.
 *
 * Serializer defines MiniDB's binary tuple format. It does not
 * manage pages or storage locations.
 *
 * The serialized format uses:
 *
 * Tuple:
 *   - uint16_t field count
 *
 * Field:
 *   - uint8_t field type
 *   - field data
 *
 * Supported field types:
 *   - int32_t
 *   - string
 */
class Serializer
{
  public:
    /**
     * @brief Serializes a tuple into raw bytes.
     *
     * @param tuple Tuple to serialize.
     *
     * @return Binary representation of the tuple.
     */
    static std::vector<std::byte> Serialize(const Tuple& tuple);

    /**
     * @brief Deserializes raw bytes into a tuple.
     *
     * @param data Serialized tuple bytes.
     *
     * @return Reconstructed tuple.
     */
    static Tuple Deserialize(const std::vector<std::byte>& data);

  private:
    enum class FieldType : uint8_t
    {
        INTEGER = 0,
        STRING = 1
    };
};

} // namespace minidb

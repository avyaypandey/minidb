#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace minidb {

    /**
     * @brief Represents a single value stored inside a tuple.
     *
     * Fields are kept independent from storage representation.
     * Serialization is handled separately by Serializer.
     */
    class Field {
    public:
        using Value = std::variant<int32_t, std::string>;

        /**
         * @brief Creates an integer field.
         */
        explicit Field(int32_t value);

        /**
         * @brief Creates a string field.
         */
        explicit Field(std::string value);

        /**
         * @brief Returns the stored value.
         */
        const Value& GetValue() const;

        /**
         * @brief Compares two fields for equality.
         */
        bool operator==(const Field& other) const;

    private:
        Value value_;
    };


    /**
     * @brief Represents a collection of fields forming a tuple.
     *
     * Tuple stores logical data only. It does not know about pages,
     * offsets, or binary storage formats.
     */
    class Tuple {
    public:
        /**
         * @brief Creates an empty tuple.
         */
        Tuple() = default;

        /**
         * @brief Adds an integer field to the tuple.
         */
        void AddInteger(int32_t value);

        /**
         * @brief Adds a string field to the tuple.
         */
        void AddString(const std::string& value);

        /**
         * @brief Returns all fields contained in the tuple.
         */
        const std::vector<Field>& Fields() const;

        /**
         * @brief Returns the number of fields in the tuple.
         */
        std::size_t Size() const;

        /**
         * @brief Compares two tuples for equality.
         */
        bool operator==(const Tuple& other) const;

    private:
        std::vector<Field> fields_;
    };

} // namespace minidb

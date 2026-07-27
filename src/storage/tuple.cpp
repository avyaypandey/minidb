#include "minidb/storage/tuple.h"

namespace minidb
{

/**
 * @brief Creates an integer field.
 *
 * @param value Integer value stored by the field.
 */
Field::Field(int32_t value) : value_(value) {}

/**
 * @brief Creates a string field.
 *
 * @param value String value stored by the field.
 */
Field::Field(std::string value) : value_(std::move(value)) {}

/**
 * @brief Returns the value stored in this field.
 *
 * @return Stored field value.
 */
const Field::Value& Field::GetValue() const { return value_; }

/**
 * @brief Compares two fields.
 *
 * Fields are equal when both their types and values match.
 */
bool Field::operator==(const Field& other) const { return value_ == other.value_; }

/**
 * @brief Adds an integer field to the tuple.
 *
 * @param value Integer value to append.
 */
void Tuple::AddInteger(int32_t value) { fields_.emplace_back(value); }

/**
 * @brief Adds a string field to the tuple.
 *
 * @param value String value to append.
 */
void Tuple::AddString(const std::string& value) { fields_.emplace_back(value); }

/**
 * @brief Returns the tuple fields.
 *
 * @return Constant reference to stored fields.
 */
const std::vector<Field>& Tuple::Fields() const { return fields_; }

/**
 * @brief Returns the number of fields in the tuple.
 *
 * @return Number of stored fields.
 */
std::size_t Tuple::Size() const { return fields_.size(); }

/**
 * @brief Compares two tuples.
 *
 * Tuples are equal when they contain the same number of fields
 * and each field matches in the same position.
 */
bool Tuple::operator==(const Tuple& other) const { return fields_ == other.fields_; }

} // namespace minidb

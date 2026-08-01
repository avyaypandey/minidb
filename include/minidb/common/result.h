#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

#include "minidb/common/status.h"

namespace minidb
{

/**
 * @brief A container holding either a successful value of type T or a failure Status.
 *
 * The Result class provides a type-safe, zero-overhead way to return either
 * the outcome of an operation or an error code. It mimics smart-pointer semantics
 * for clean access to the underlying value when successful.
 *
 * @tparam T The type of the value returned on success.
 */
template <typename T> class [[nodiscard]] Result
{
  public:
    using value_type = T;

    /**
     * @brief Construct a successful Result with a copy of the value.
     * @param value The success value to store.
     */
    Result(const T& value) : storage_(value) {}

    /**
     * @brief Construct a successful Result by moving the value.
     * @param value The success value to move into storage.
     */
    Result(T&& value) : storage_(std::move(value)) {}

    /**
     * @brief Construct a failed Result from a Status error.
     * @details Asserts that the provided status is actually an error (not OK).
     * @param error The failure Status.
     */
    Result(Status error) : storage_(error)
    {
        assert(!error.Ok() && "Successful Status should not construct an error Result.");
    }

    // Rule of Zero/Five
    Result(const Result&) = default;
    Result(Result&&) noexcept = default;

    Result& operator=(const Result&) = default;
    Result& operator=(Result&&) noexcept = default;

    ~Result() = default;

    // -------------------------------------------------

    /**
     * @brief Check if the Result contains a successful value.
     * @return true if successful, false if it contains an error.
     */
    [[nodiscard]]
    bool Ok() const noexcept
    {
        return std::holds_alternative<T>(storage_);
    }

    /**
     * @brief Convenience boolean conversion operator.
     * @return true if successful, false if it contains an error.
     */
    [[nodiscard]]
    explicit operator bool() const noexcept
    {
        return Ok();
    }

    // -------------------------------------------------

    /**
     * @brief Access the underlying success value (mutable).
     * @details Asserts that the Result is in a successful state.
     * @return Reference to the stored value.
     */
    T& Value()
    {
        assert(Ok() && "Attempted to access value of failed Result.");
        return std::get<T>(storage_);
    }

    /**
     * @brief Access the underlying success value (immutable).
     * @details Asserts that the Result is in a successful state.
     * @return Const reference to the stored value.
     */
    const T& Value() const
    {
        assert(Ok() && "Attempted to access value of failed Result.");
        return std::get<T>(storage_);
    }

    /**
     * @brief Access the underlying error Status.
     * @details Asserts that the Result is actually in a failed state.
     * @return Const reference to the stored Status.
     */
    const Status& Error() const
    {
        assert(!Ok() && "Attempted to access error of successful Result.");
        return std::get<Status>(storage_);
    }

    // -------------------------------------------------

    /**
     * @brief Dereference operator to access the success value.
     * @return Reference to the stored value.
     */
    T& operator*() { return Value(); }

    /**
     * @brief Const dereference operator to access the success value.
     * @return Const reference to the stored value.
     */
    const T& operator*() const { return Value(); }

    /**
     * @brief Member access operator for the success value.
     * @return Pointer to the stored value.
     */
    T* operator->() { return &Value(); }

    /**
     * @brief Const member access operator for the success value.
     * @return Const pointer to the stored value.
     */
    const T* operator->() const { return &Value(); }

  private:
    std::variant<T, Status> storage_;
};

} // namespace minidb

#pragma once

#include <cassert>

namespace minidb
{

/**
 * @brief Represents the status code of an operation within minidb.
 */
enum class StatusCode
{
    OK = 0,
    INVALID_PAGE_ID,
    DISK_ERROR,
    PAGE_NOT_FOUND,
    ALL_PAGES_PINNED,
    BUFFER_POOL_FULL
};

/**
 * @brief Represents the outcome of an operation, indicating success or failure.
 *
 * The Status class wraps a StatusCode and provides a lightweight, compile-time
 * friendly way to check for success or failure without relying on exceptions.
 */
class [[nodiscard]] Status
{
  public:
    /**
     * @brief Construct a default Status representing success (OK).
     */
    constexpr Status() noexcept : code_(StatusCode::OK) {}

    /**
     * @brief Construct a Status with a specific StatusCode.
     * @param code The status code to set.
     */
    constexpr Status(StatusCode code) noexcept : code_(code) {}

    /**
     * @brief Check if the status represents success.
     * @return true if the code is StatusCode::OK, false otherwise.
     */
    [[nodiscard]]
    constexpr bool Ok() const noexcept
    {
        return code_ == StatusCode::OK;
    }

    /**
     * @brief Get the underlying status code.
     * @return The StatusCode enum value.
     */
    [[nodiscard]]
    constexpr StatusCode Code() const noexcept
    {
        return code_;
    }

    /**
     * @brief Convenience boolean conversion operator.
     * @return true if successful (OK), false otherwise.
     */
    constexpr explicit operator bool() const noexcept { return Ok(); }

    // ---------- Convenience factories ----------

    /**
     * @brief Create a success Status object.
     * @return A Status initialized with StatusCode::OK.
     */
    static constexpr Status OkStatus() noexcept { return Status(StatusCode::OK); }

    /**
     * @brief Create an invalid page ID error Status object.
     * @return A Status initialized with StatusCode::INVALID_PAGE_ID.
     */
    static constexpr Status InvalidPageId() noexcept { return Status(StatusCode::INVALID_PAGE_ID); }

    /**
     * @brief Create a disk error Status object.
     * @return A Status initialized with StatusCode::DISK_ERROR.
     */
    static constexpr Status DiskError() noexcept { return Status(StatusCode::DISK_ERROR); }

    /**
     * @brief Create a page not found error Status object.
     * @return A Status initialized with StatusCode::PAGE_NOT_FOUND.
     */
    static constexpr Status PageNotFound() noexcept { return Status(StatusCode::PAGE_NOT_FOUND); }

    /**
     * @brief Create an all pages pinned error Status object.
     * @return A Status initialized with StatusCode::ALL_PAGES_PINNED.
     */
    static constexpr Status AllPagesPinned() noexcept
    {
        return Status(StatusCode::ALL_PAGES_PINNED);
    }

    /**
     * @brief Create a buffer pool full error Status object.
     * @return A Status initialized with StatusCode::BUFFER_POOL_FULL.
     */
    static constexpr Status BufferPoolFull() noexcept
    {
        return Status(StatusCode::BUFFER_POOL_FULL);
    }

  private:
    StatusCode code_;
};

} // namespace minidb

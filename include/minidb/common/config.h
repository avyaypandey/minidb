#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace minidb
{

/**
 * @brief Type definition for unique page identifiers.
 */
using page_id_t = int32_t;

/**
 * @brief Type definition for unique frame identifiers in the buffer pool.
 */
using frame_id_t = int32_t;

/**
 * @brief Constant representing an invalid or unassigned page ID.
 */
constexpr page_id_t INVALID_PAGE_ID = -1;

/**
 * @brief Constant representing an invalid or unassigned frame ID.
 */
constexpr frame_id_t INVALID_FRAME_ID = -1;

/**
 * @brief Size of a single database page in bytes (default: 4KB).
 */
constexpr std::size_t PAGE_SIZE = 4096;

} // namespace minidb

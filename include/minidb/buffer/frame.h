#pragma once

#include <cstdint>

#include "minidb/common/config.h"
#include "minidb/common/status.h"
#include "minidb/storage/page.h"

namespace minidb
{

/**
 * @brief Represents a single frame in the buffer pool.
 *
 * A frame stores one in-memory page together with the runtime metadata
 * required by the BufferPoolManager.
 *
 * A frame is purely an in-memory concept. It does not own disk resources
 * and is not responsible for page replacement or persistence.
 *
 * Responsibilities:
 *  - Store one cached Page.
 *  - Track which page is currently loaded.
 *  - Maintain the page pin count.
 *  - Track whether the page has been modified.
 *
 * The BufferPoolManager owns the lifecycle of a frame and is responsible
 * for updating its metadata.
 */
struct Frame
{

    /**
     * @brief Cached page contents.
     *
     * For pages loaded from disk, DiskManager populates this object.
     *
     * For newly created pages, the Page constructor initializes the page
     * into a valid empty state.
     */
    Page page;

    /**
     * @brief Identifier of the page currently stored in this frame.
     *
     * INVALID_PAGE_ID indicates that the frame is currently unused.
     */
    page_id_t page_id = INVALID_PAGE_ID;

    /**
     * @brief Number of active users of this page.
     *
     * A frame may only be selected for eviction when pin_count == 0.
     */
    std::uint32_t pin_count = 0;

    /**
     * @brief Indicates whether the page has been modified.
     *
     * Dirty pages must be written back to disk before eviction.
     */
    bool dirty = false;
};

} // namespace minidb

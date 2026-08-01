#pragma once

#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "minidb/buffer/frame.h"
#include "minidb/buffer/lru_replacer.h"
#include "minidb/common/result.h"
#include "minidb/common/status.h"
#include "minidb/disk/disk_manager.h"

namespace minidb
{

/**
 * @brief Manages the in-memory buffer pool of MiniDB.
 *
 * BufferPoolManager is responsible for coordinating:
 *
 *  - Page caching
 *  - Frame allocation
 *  - Page table management
 *  - Pin/unpin tracking
 *  - Dirty page handling
 *  - Page eviction
 *  - Disk interaction
 *
 * BufferPoolManager does not understand:
 *
 *  - Tuple layout
 *  - Serialization
 *  - Page record semantics
 *
 * Those responsibilities belong to Page, Tuple, and Serializer.
 *
 *
 * Frame invariants:
 *
 * 1. A page exists in at most one frame.
 * 2. Every cached page has exactly one page table entry.
 * 3. Frames with pin_count > 0 are never evictable.
 * 4. Frames inside the LRU replacer always have pin_count == 0.
 * 5. Dirty pages must be flushed before eviction.
 */
class BufferPoolManager
{
  public:
    /**
     * @brief Create a new buffer pool manager.
     *
     * @param pool_size Number of frames available.
     * @param disk_manager Persistent storage manager.
     */
    BufferPoolManager(std::size_t pool_size, DiskManager* disk_manager);

    ~BufferPoolManager();

    BufferPoolManager(const BufferPoolManager&) = delete;
    BufferPoolManager& operator=(const BufferPoolManager&) = delete;

    BufferPoolManager(BufferPoolManager&&) = delete;
    BufferPoolManager& operator=(BufferPoolManager&&) = delete;

    /**
     * @brief Fetch a page into the buffer pool.
     *
     * If the page is already cached:
     *
     *      - increment pin count
     *      - return existing page
     *
     * Otherwise:
     *
     *      - acquire a free frame or evict a victim
     *      - flush victim if dirty
     *      - read page from disk
     *      - cache it
     *
     * @param page_id Page identifier.
     *
     * @return
     *      Page pointer on success.
     *      Error status on failure.
     */
    [[nodiscard]]
    Result<Page*> FetchPage(page_id_t page_id);

    /**
     * @brief Create a new page.
     *
     * The returned page starts pinned.
     *
     * New pages are considered dirty because their contents exist
     * only in memory until flushed.
     *
     * @param[out] page_id Receives allocated page identifier.
     *
     * @return
     *      Newly created page on success.
     *      Error status on failure.
     */
    [[nodiscard]]
    Result<Page*> NewPage(page_id_t* page_id);

    /**
     * @brief Release a page reference.
     *
     * Decrements the pin count. When the pin count reaches zero,
     * the page becomes eligible for eviction.
     *
     * @param page_id Page identifier.
     * @param dirty Whether the caller modified the page.
     *
     * @return Success or failure status.
     */
    [[nodiscard]]
    Status UnpinPage(page_id_t page_id, bool dirty);

    /**
     * @brief Flush one page to disk.
     *
     * Dirty pages are written using DiskManager and marked clean.
     *
     * @param page_id Page identifier.
     *
     * @return Success or failure status.
     */
    [[nodiscard]]
    Status FlushPage(page_id_t page_id);

    /**
     * @brief Flush every dirty page in the buffer pool.
     *
     * @return Success or failure status.
     */
    [[nodiscard]]
    Status FlushAllPages();

  private:
    /**
     * @brief Find a frame available for reuse.
     *
     * Selection order:
     *
     * 1. Free frame list.
     * 2. LRU victim.
     *
     * @param[out] frame_id Selected frame.
     *
     * @return Success or failure status.
     */
    [[nodiscard]]
    Status FindAvailableFrame(frame_id_t* frame_id);

    /**
     * @brief Evict the contents of a frame.
     *
     * If dirty:
     *      write page to disk.
     *
     * Then:
     *      remove page table entry.
     *      reset metadata.
     *
     * @param frame_id Frame to evict.
     *
     * @return Success or failure status.
     */
    [[nodiscard]]
    Status EvictFrame(frame_id_t frame_id);

    /**
     * @brief Get the frame associated with a page.
     *
     * @param page_id Page identifier.
     *
     * @return Frame identifier if present.
     */
    [[nodiscard]]
    Result<frame_id_t> GetFrameId(page_id_t page_id);

  private:
    /**
     * @brief All frames managed by this buffer pool.
     */
    std::vector<Frame> frames_;

    /**
     * @brief Maps page IDs to resident frames.
     */
    std::unordered_map<page_id_t, frame_id_t> page_table_;

    /**
     * @brief Frames that have never been assigned or are available.
     */
    std::list<frame_id_t> free_list_;

    /**
     * @brief Replacement policy for evictable frames.
     */
    LRUReplacer replacer_;

    /**
     * @brief Persistent page storage.
     */
    DiskManager* disk_manager_;

    /**
     * @brief Protects buffer pool metadata.
     *
     * The latch protects:
     *
     * - page table
     * - free list
     * - frame metadata
     * - eviction operations
     *
     * It does not protect modifications made directly to Page contents
     * by callers.
     */
    mutable std::mutex latch_;
};

} // namespace minidb

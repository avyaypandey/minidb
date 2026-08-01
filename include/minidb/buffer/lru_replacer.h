#pragma once

#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>

#include "minidb/common/config.h"

namespace minidb
{

/**
 * @brief Least Recently Used (LRU) replacement policy.
 *
 * The LRUReplacer maintains the set of evictable buffer pool frames.
 * It is used by the BufferPoolManager to determine which frame should
 * be evicted when the buffer pool is full.
 *
 * Only unpinned frames are tracked by the replacer. Pinned frames are
 * considered in-use and are therefore not eligible for eviction.
 *
 * Ordering:
 *
 *  Front of the list:
 *      Most recently unpinned frame.
 *
 *  Back of the list:
 *      Least recently used frame.
 *
 * Thread Safety:
 *
 * All public member functions are thread-safe.
 */
class LRUReplacer
{
  public:
    /**
     * @brief Construct an empty LRU replacer.
     *
     * @param num_frames Maximum number of frames that may be tracked.
     */
    explicit LRUReplacer(std::size_t num_frames);

    ~LRUReplacer() = default;

    LRUReplacer(const LRUReplacer&) = delete;
    LRUReplacer& operator=(const LRUReplacer&) = delete;

    LRUReplacer(LRUReplacer&&) = delete;
    LRUReplacer& operator=(LRUReplacer&&) = delete;

    /**
     * @brief Select the least recently used frame for eviction.
     *
     * If an evictable frame exists, it is removed from the replacer
     * and returned through @p frame_id.
     *
     * @param[out] frame_id Receives the selected victim frame.
     *
     * @return
     *      true  - a victim frame was found.
     *      false - no evictable frame exists.
     */
    [[nodiscard]]
    bool Victim(frame_id_t* frame_id);

    /**
     * @brief Remove a frame from the replacer.
     *
     * This is typically called when a frame becomes pinned and should
     * no longer be considered for eviction.
     *
     * Calling Pin() on a frame that is not currently tracked has no effect.
     *
     * @param frame_id Frame to remove.
     */
    void Pin(frame_id_t frame_id);

    /**
     * @brief Add a frame to the replacer.
     *
     * This is typically called when a frame's pin count reaches zero,
     * making it eligible for eviction.
     *
     * If the frame is already present, its position is updated to the
     * most recently used position.
     *
     * @param frame_id Frame to add.
     */
    void Unpin(frame_id_t frame_id);

    /**
     * @brief Return the number of evictable frames.
     *
     * @return Number of frames currently tracked.
     */
    [[nodiscard]]
    std::size_t Size() const;

  private:
    /**
     * @brief Maximum number of frames managed by the replacer.
     */
    std::size_t capacity_;

    /**
     * @brief LRU ordering.
     *
     * Front:
     *      Most recently used.
     *
     * Back:
     *      Least recently used.
     */
    std::list<frame_id_t> lru_list_;

    /**
     * @brief Maps frame identifiers to their position in the LRU list.
     */
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> positions_;

    /**
     * @brief Protects all internal data structures.
     */
    mutable std::mutex latch_;
};

} // namespace minidb

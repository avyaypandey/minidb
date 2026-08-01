#include "minidb/buffer/lru_replacer.h"

#include <cassert>

namespace minidb
{

LRUReplacer::LRUReplacer(std::size_t num_frames) : capacity_(num_frames) {}

bool LRUReplacer::Victim(frame_id_t* frame_id)
{
    std::lock_guard<std::mutex> lock(latch_);

    if (lru_list_.empty())
    {
        return false;
    }

    *frame_id = lru_list_.back();

    positions_.erase(*frame_id);
    lru_list_.pop_back();

    return true;
}

void LRUReplacer::Pin(frame_id_t frame_id)
{
    std::lock_guard<std::mutex> lock(latch_);

    auto it = positions_.find(frame_id);

    if (it == positions_.end())
    {
        return;
    }

    lru_list_.erase(it->second);
    positions_.erase(it);
}

void LRUReplacer::Unpin(frame_id_t frame_id)
{
    std::lock_guard<std::mutex> lock(latch_);

    auto it = positions_.find(frame_id);

    /*
     * If the frame is already tracked, remove it first so that
     * it can be reinserted as the most recently used frame.
     */
    if (it != positions_.end())
    {
        lru_list_.erase(it->second);
        positions_.erase(it);
    }

    lru_list_.push_front(frame_id);
    positions_[frame_id] = lru_list_.begin();

    assert(lru_list_.size() == positions_.size());
    assert(lru_list_.size() <= capacity_);
}

std::size_t LRUReplacer::Size() const
{
    std::lock_guard<std::mutex> lock(latch_);

    return lru_list_.size();
}

} // namespace minidb

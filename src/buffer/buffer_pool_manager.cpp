#include "minidb/buffer/buffer_pool_manager.h"

#include <cstring>

namespace minidb
{

BufferPoolManager::BufferPoolManager(std::size_t pool_size, DiskManager* disk_manager)
    : frames_(pool_size), replacer_(pool_size), disk_manager_(disk_manager)
{
    for (frame_id_t i = 0; i < static_cast<frame_id_t>(pool_size); i++)
    {
        free_list_.push_back(i);
    }
}

BufferPoolManager::~BufferPoolManager() { (void)FlushAllPages(); }

Result<Page*> BufferPoolManager::FetchPage(page_id_t page_id)
{
    if (page_id == INVALID_PAGE_ID)
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    std::lock_guard<std::mutex> lock(latch_);

    /*
     * Case 1:
     * Page already exists in memory.
     */
    auto table_it = page_table_.find(page_id);

    if (table_it != page_table_.end())
    {

        frame_id_t frame_id = table_it->second;

        Frame& frame = frames_[frame_id];

        frame.pin_count++;

        replacer_.Pin(frame_id);

        return Result<Page*>(&frame.page);
    }

    /*
     * Case 2:
     * Need a new frame.
     */
    frame_id_t frame_id;

    Status status = FindAvailableFrame(&frame_id);

    if (!status.Ok())
    {
        return status;
    }

    Frame& frame = frames_[frame_id];

    /*
     * If this frame previously contained a page,
     * remove the old mapping.
     */
    if (frame.page_id != INVALID_PAGE_ID)
    {

        if (frame.dirty)
        {

            Status flush = disk_manager_->WritePage(
                frame.page_id, reinterpret_cast<const char*>(frame.page.GetData()));

            if (!flush.Ok())
            {
                return flush;
            }
        }

        page_table_.erase(frame.page_id);
    }

    /*
     * Load requested page from disk.
     */
    Status read_status =
        disk_manager_->ReadPage(page_id, reinterpret_cast<char*>(frame.page.GetData()));

    if (!read_status.Ok())
    {
        return read_status;
    }

    frame.page_id = page_id;
    frame.pin_count = 1;
    frame.dirty = false;

    page_table_[page_id] = frame_id;

    return Result<Page*>(&frame.page);
}

Result<Page*> BufferPoolManager::NewPage(page_id_t* page_id)
{
    if (page_id == nullptr)
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    std::lock_guard<std::mutex> lock(latch_);

    auto result = disk_manager_->AllocatePage();

    if (!result.Ok())
    {
        return result.Error();
    }

    *page_id = result.Value();

    frame_id_t frame_id;

    Status status = FindAvailableFrame(&frame_id);

    if (!status.Ok())
    {
        return status;
    }

    Frame& frame = frames_[frame_id];

    if (frame.page_id != INVALID_PAGE_ID)
    {

        if (frame.dirty)
        {

            Status flush = disk_manager_->WritePage(
                frame.page_id, reinterpret_cast<const char*>(frame.page.GetData()));

            if (!flush.Ok())
            {
                return flush;
            }
        }

        page_table_.erase(frame.page_id);
    }

    /*
     * Page constructor establishes the empty-page invariant.
     */
    frame.page = Page();

    frame.page_id = *page_id;
    frame.pin_count = 1;
    frame.dirty = true;

    page_table_[*page_id] = frame_id;

    return Result<Page*>(&frame.page);
}

Status BufferPoolManager::UnpinPage(page_id_t page_id, bool dirty)
{
    std::lock_guard<std::mutex> lock(latch_);

    auto it = page_table_.find(page_id);

    if (it == page_table_.end())
    {
        return Status(StatusCode::PAGE_NOT_FOUND);
    }

    Frame& frame = frames_[it->second];

    if (frame.pin_count == 0)
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    frame.pin_count--;

    if (dirty)
    {
        frame.dirty = true;
    }

    if (frame.pin_count == 0)
    {
        replacer_.Unpin(it->second);
    }

    return Status::OkStatus();
}

Status BufferPoolManager::FlushPage(page_id_t page_id)
{
    std::lock_guard<std::mutex> lock(latch_);

    auto it = page_table_.find(page_id);

    if (it == page_table_.end())
    {
        return Status(StatusCode::PAGE_NOT_FOUND);
    }

    Frame& frame = frames_[it->second];

    if (!frame.dirty)
    {
        return Status::OkStatus();
    }

    Status status =
        disk_manager_->WritePage(page_id, reinterpret_cast<const char*>(frame.page.GetData()));

    if (status.Ok())
    {
        frame.dirty = false;
    }

    return status;
}

Status BufferPoolManager::FlushAllPages()
{
    std::lock_guard<std::mutex> lock(latch_);

    for (auto& frame : frames_)
    {

        if (frame.page_id == INVALID_PAGE_ID)
        {
            continue;
        }

        if (!frame.dirty)
        {
            continue;
        }

        Status status = disk_manager_->WritePage(
            frame.page_id, reinterpret_cast<const char*>(frame.page.GetData()));

        if (!status.Ok())
        {
            return status;
        }

        frame.dirty = false;
    }

    return Status::OkStatus();
}

Status BufferPoolManager::FindAvailableFrame(frame_id_t* frame_id)
{
    if (!free_list_.empty())
    {

        *frame_id = free_list_.front();

        free_list_.pop_front();

        return Status::OkStatus();
    }

    if (replacer_.Victim(frame_id))
    {
        return Status::OkStatus();
    }

    return Status(StatusCode::ALL_PAGES_PINNED);
}

Status BufferPoolManager::EvictFrame(frame_id_t frame_id)
{
    Frame& frame = frames_[frame_id];

    if (frame.pin_count != 0)
    {
        return Status(StatusCode::BUFFER_POOL_FULL);
    }

    if (frame.dirty)
    {

        Status status = disk_manager_->WritePage(
            frame.page_id, reinterpret_cast<const char*>(frame.page.GetData()));

        if (!status.Ok())
        {
            return status;
        }
    }

    page_table_.erase(frame.page_id);

    frame.page_id = INVALID_PAGE_ID;
    frame.pin_count = 0;
    frame.dirty = false;

    return Status::OkStatus();
}

Result<frame_id_t> BufferPoolManager::GetFrameId(page_id_t page_id)
{
    auto it = page_table_.find(page_id);

    if (it == page_table_.end())
    {
        return Status(StatusCode::PAGE_NOT_FOUND);
    }

    return it->second;
}

} // namespace minidb

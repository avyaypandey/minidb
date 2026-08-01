#include "minidb/disk/disk_manager.h"

namespace minidb
{

DiskManager::DiskManager(const std::string& db_file)
{
    db_file_.open(db_file, std::ios::in | std::ios::out | std::ios::binary);

    /*
     * If the file does not exist, create it first.
     */
    if (!db_file_.is_open())
    {
        db_file_.clear();

        std::ofstream create_file(db_file, std::ios::binary);

        create_file.close();

        db_file_.open(db_file, std::ios::in | std::ios::out | std::ios::binary);
    }

    InitializePageAllocation();
}

DiskManager::~DiskManager()
{
    if (db_file_.is_open())
    {
        db_file_.close();
    }
}

Status DiskManager::ReadPage(page_id_t page_id, char* buffer)
{
    if (!IsValidPage(page_id))
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    std::lock_guard<std::mutex> lock(io_latch_);

    const auto offset = static_cast<std::streamoff>(page_id * PAGE_SIZE);

    db_file_.seekg(offset, std::ios::beg);

    if (!db_file_)
    {
        db_file_.clear();

        return Status(StatusCode::DISK_ERROR);
    }

    db_file_.read(buffer, PAGE_SIZE);

    if (db_file_.gcount() != PAGE_SIZE)
    {
        db_file_.clear();

        return Status(StatusCode::DISK_ERROR);
    }

    return Status::OkStatus();
}

Status DiskManager::WritePage(page_id_t page_id, const char* buffer)
{
    if (!IsValidPage(page_id))
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    std::lock_guard<std::mutex> lock(io_latch_);

    const auto offset = static_cast<std::streamoff>(page_id * PAGE_SIZE);

    db_file_.seekp(offset, std::ios::beg);

    if (!db_file_)
    {
        db_file_.clear();

        return Status(StatusCode::DISK_ERROR);
    }

    db_file_.write(buffer, PAGE_SIZE);

    if (!db_file_)
    {
        db_file_.clear();

        return Status(StatusCode::DISK_ERROR);
    }

    /*
     * For this milestone, every write is durable.
     *
     * BufferPoolManager will decide later when writes happen.
     */
    db_file_.flush();

    if (!db_file_)
    {
        db_file_.clear();

        return Status(StatusCode::DISK_ERROR);
    }

    return Status::OkStatus();
}

Result<page_id_t> DiskManager::AllocatePage()
{
    page_id_t page_id = next_page_id_.fetch_add(1);

    return page_id;
}

Status DiskManager::DeallocatePage(page_id_t page_id)
{
    if (!IsValidPage(page_id))
    {
        return Status(StatusCode::INVALID_PAGE_ID);
    }

    /*
     * Physical file shrinking is intentionally not implemented.
     *
     * Future versions may maintain free-page metadata.
     */
    return Status::OkStatus();
}

bool DiskManager::IsValidPage(page_id_t page_id) const { return page_id >= 0; }

void DiskManager::InitializePageAllocation()
{
    std::error_code error;

    if (!db_file_.is_open())
    {
        next_page_id_ = 0;
        return;
    }

    /*
     * Find existing database size.
     */
    db_file_.seekg(0, std::ios::end);

    auto file_size = db_file_.tellg();

    db_file_.clear();

    if (file_size <= 0)
    {
        next_page_id_ = 0;
        return;
    }

    next_page_id_ = static_cast<page_id_t>(file_size / PAGE_SIZE);
}

} // namespace minidb

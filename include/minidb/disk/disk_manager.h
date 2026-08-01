#pragma once

#include <atomic>
#include <fstream>
#include <mutex>
#include <string>

#include "minidb/common/config.h"
#include "minidb/common/result.h"
#include "minidb/common/status.h"

namespace minidb
{

/**
 * @brief Manages persistent storage of fixed-size database pages.
 *
 * DiskManager is the lowest layer of the storage system. It is responsible for:
 *
 *  - Managing the database file.
 *  - Translating page identifiers into file offsets.
 *  - Reading raw page bytes from disk.
 *  - Writing raw page bytes to disk.
 *  - Allocating new page identifiers.
 *
 * DiskManager does not understand higher-level storage concepts such as:
 *
 *  - tuples
 *  - schemas
 *  - records
 *  - slots
 *  - buffer pool frames
 *
 * Pages are treated as opaque PAGE_SIZE byte blocks.
 *
 * Thread safety:
 *
 * Public operations are thread-safe. Internal file operations are protected
 * using an internal mutex because stream seek/read/write operations share
 * mutable state.
 */
class DiskManager
{
  public:
    /**
     * @brief Opens or creates a database file.
     *
     * @param db_file Path to the database file.
     */
    explicit DiskManager(const std::string& db_file);

    /**
     * @brief Closes the database file.
     */
    ~DiskManager();

    /**
     * DiskManager manages a unique file resource and cannot be copied.
     */
    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    /**
     * @brief Reads one complete page from disk.
     *
     * The page is copied into the provided buffer.
     *
     * Requirements:
     *
     *  - page_id must refer to a valid allocated page.
     *  - buffer must have PAGE_SIZE bytes available.
     *
     * @param page_id Page identifier to read.
     * @param buffer Destination buffer.
     *
     * @return Status::OkStatus() on success.
     *
     * Possible errors:
     *
     *  - INVALID_PAGE_ID
     *  - DISK_ERROR
     */
    Status ReadPage(page_id_t page_id, char* buffer);

    /**
     * @brief Writes one complete page to disk.
     *
     * The contents of buffer replace the existing page contents.
     *
     * @param page_id Page identifier to write.
     * @param buffer Source buffer containing PAGE_SIZE bytes.
     *
     * @return Status::OkStatus() on success.
     *
     * Possible errors:
     *
     *  - INVALID_PAGE_ID
     *  - DISK_ERROR
     */
    Status WritePage(page_id_t page_id, const char* buffer);

    /**
     * @brief Allocates a new page identifier.
     *
     * Allocation only reserves a page identifier.
     *
     * DiskManager does not initialize page contents.
     * The caller is responsible for creating and initializing the page
     * before it is persisted.
     *
     * @return Newly allocated page_id_t on success.
     *
     * Possible errors:
     *
     *  - DISK_ERROR
     */
    Result<page_id_t> AllocatePage();

    /**
     * @brief Releases a page identifier.
     *
     * Currently this operation may only update internal allocation metadata.
     * Physical file shrinking is not required.
     *
     * @param page_id Page identifier to release.
     *
     * @return Status::OkStatus() on success.
     *
     * Possible errors:
     *
     *  - INVALID_PAGE_ID
     */
    Status DeallocatePage(page_id_t page_id);

  private:
    /**
     * @brief Checks whether a page identifier is valid.
     */
    bool IsValidPage(page_id_t page_id) const;

    /**
     * @brief Initializes the next page identifier after opening an existing DB.
     */
    void InitializePageAllocation();

  private:
    /*
     * Database file stream.
     */
    std::fstream db_file_;

    /*
     * Next available page identifier.
     */
    std::atomic<page_id_t> next_page_id_;

    /*
     * Protects file stream operations.
     */
    mutable std::mutex io_latch_;
};

} // namespace minidb

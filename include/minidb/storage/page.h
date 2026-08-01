#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "minidb/common/config.h"
#include "minidb/storage/rid.h"

namespace minidb
{

/**
 * @brief Represents a single fixed-size slotted page.
 *
 * A Page stores variable-length records as raw bytes. It manages
 * page metadata, the slot directory, and record placement, but is
 * unaware of tuple schemas or application-level objects.
 */
class Page
{
  public:
    /**
     * @brief Creates an empty page with initialized metadata.
     */
    Page();

    /**
     * @brief Inserts a serialized record into the page.
     *
     * @param record Serialized record bytes.
     * @return RID identifying the inserted record, or std::nullopt if
     *         insufficient free space is available.
     */
    std::optional<RID> InsertRecord(const std::byte* record, std::size_t size);

    /**
     * @brief Retrieves a serialized record by its RID.
     *
     * @param rid Record identifier.
     * @return The stored bytes if the record exists and is not deleted.
     */
    std::optional<std::vector<std::byte>> GetRecord(RID rid) const;

    /**
     * @brief Marks a record as deleted.
     *
     * Slot identifiers remain stable after deletion.
     *
     * @return true if the record was successfully marked deleted.
     */
    bool DeleteRecord(RID rid);

    /// Returns the number of slots currently stored in the page.
    uint16_t NumSlots() const;

    /// Returns the current free-space pointer.
    uint16_t FreeSpaceOffset() const;

    /// Returns the remaining usable space in the page.
    std::size_t FreeSpace() const;

    char* GetData() { return reinterpret_cast<char*>(data_.data()); }

    const char* GetData() const { return reinterpret_cast<const char*>(data_.data()); }

  private:
    struct Slot
    {
        uint16_t offset;
        uint16_t length;
        uint16_t flags;
    };

    static_assert(sizeof(Slot) == 6, "Slot layout must remain exactly 6 bytes.");

    static constexpr uint16_t SLOT_DELETED = 0x0001;

    static constexpr std::size_t NUM_SLOTS_OFFSET = 0;
    static constexpr std::size_t FREE_SPACE_OFFSET = sizeof(uint16_t);
    static constexpr std::size_t HEADER_SIZE = 2 * sizeof(uint16_t);
    static constexpr std::size_t SLOT_SIZE = sizeof(uint16_t) * 3;

    /// Reads a 16-bit value from the page header.
    uint16_t ReadUInt16(std::size_t offset) const;

    /// Writes a 16-bit value into the page header.
    void WriteUInt16(std::size_t offset, uint16_t value);

    void SetNumSlots(uint16_t value);
    void SetFreeSpaceOffset(uint16_t value);

    /// Reads a slot entry from the slot directory.
    Slot ReadSlot(uint16_t slot_id) const;

    /// Writes a slot entry into the slot directory.
    void WriteSlot(uint16_t slot_id, const Slot& slot);

    std::array<std::byte, PAGE_SIZE> data_;
};

} // namespace minidb

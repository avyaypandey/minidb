#include "minidb/storage/page.h"

#include <cstring>

namespace minidb {

    /**
     * @brief Creates an empty page with a deterministic initial state.
     *
     * A newly created page:
     * - contains only zeroed bytes
     * - has no slots
     * - has the free-space pointer at the end of the page
     */
    Page::Page() {
        data_.fill(std::byte{0});

        SetNumSlots(0);
        SetFreeSpaceOffset(PAGE_SIZE);
    }

    /**
     * @brief Inserts a serialized record into the page.
     *
     * Allocates space for the record growing upwards from the end of the page
     * and adds a new entry to the slot directory growing downwards from the header.
     * Ensures strict bounds checking to prevent collisions between the slot directory
     * and the free space.
     *
     * @param record Pointer to the raw bytes of the serialized record.
     * @param size   Size of the record in bytes.
     * @return std::optional<RID> The Record ID of the inserted record if successful,
     *         or std::nullopt if there is insufficient free space.
     */
    std::optional<RID> Page::InsertRecord(
        const std::byte* record,
        std::size_t size)
    {
        uint16_t num_slots = NumSlots();
        uint16_t free_offset = FreeSpaceOffset();

        std::size_t slot_directory_end =
        HEADER_SIZE + (num_slots * SLOT_SIZE);

        if (free_offset < size)
        {
            return std::nullopt;
        }

        uint16_t new_free_offset =
        static_cast<uint16_t>(free_offset - size);

        if (new_free_offset <
            slot_directory_end + SLOT_SIZE)
        {
            return std::nullopt;
        }

        // Copy record bytes into page
        std::memcpy(
            data_.data() + new_free_offset,
                    record,
                    size
        );

        Slot slot;
        slot.offset = new_free_offset;
        slot.length = static_cast<uint16_t>(size);
        slot.flags = 0;

        WriteSlot(num_slots, slot);

        SetNumSlots(num_slots + 1);
        SetFreeSpaceOffset(new_free_offset);

        return RID{
            0,
            num_slots
        };
    }

    /**
     * @brief Retrieves a serialized record from the page.
     *
     * The page uses the RID to locate the corresponding slot entry.
     * The slot stores the record's byte offset and length, which are
     * used to copy the raw record bytes out of the page.
     *
     * The Page layer does not interpret record contents.
     *
     * @param rid Record identifier.
     *
     * @return A copy of the stored record bytes if the RID is valid
     *         and the record has not been deleted. Returns std::nullopt
     *         otherwise.
     */
    std::optional<std::vector<std::byte>> Page::GetRecord(RID rid) const
    {
        if (rid.page_id != 0)
        {
            return std::nullopt;
        }

        if (rid.slot_id >= NumSlots())
        {
            return std::nullopt;
        }

        Slot slot = ReadSlot(rid.slot_id);

        if (slot.flags & SLOT_DELETED)
        {
            return std::nullopt;
        }

        std::vector<std::byte> record(slot.length);

        std::memcpy(
            record.data(),
                    data_.data() + slot.offset,
                    slot.length
        );

        return record;
    }

    /**
     * @brief Marks a record as deleted using tombstone deletion.
     *
     * Deletion does not remove the record bytes or modify slot identifiers.
     * The slot remains in place, but future reads will treat it as invalid.
     *
     * This keeps RIDs stable and allows future page compaction to be added
     * without changing the external interface.
     *
     * @param rid Identifier of the record to delete.
     *
     * @return true if the record was successfully marked deleted,
     *         false if the RID is invalid or already deleted.
     */
    bool Page::DeleteRecord(RID rid)
    {
        if (rid.page_id != 0)
        {
            return false;
        }

        if (rid.slot_id >= NumSlots())
        {
            return false;
        }

        Slot slot = ReadSlot(rid.slot_id);

        if (slot.flags & SLOT_DELETED)
        {
            return false;
        }

        slot.flags |= SLOT_DELETED;

        WriteSlot(rid.slot_id, slot);

        return true;
    }

    /**
     * @brief Returns the number of slots currently stored in the page.
     */
    uint16_t Page::NumSlots() const {
        return ReadUInt16(NUM_SLOTS_OFFSET);
    }

    /**
     * @brief Returns the current byte offset where free space begins.
     */
    uint16_t Page::FreeSpaceOffset() const {
        return ReadUInt16(FREE_SPACE_OFFSET);
    }

    /**
     * @brief Calculates the remaining usable space between the slot directory
     *        and the free space offset.
     */
    std::size_t Page::FreeSpace() const {
        uint16_t num_slots = NumSlots();
        uint16_t free_space_offset = FreeSpaceOffset();

        std::size_t directory_size = HEADER_SIZE + (num_slots * SLOT_SIZE);

        if (free_space_offset <= directory_size) {
            return 0;
        }

        return free_space_offset - directory_size;
    }

    /**
     * @brief Reads a 16-bit unsigned integer from the given byte offset.
     */
    uint16_t Page::ReadUInt16(std::size_t offset) const {
        uint16_t value = 0;
        std::memcpy(&value, data_.data() + offset, sizeof(uint16_t));
        return value;
    }

    /**
     * @brief Writes a 16-bit unsigned integer into the given byte offset.
     */
    void Page::WriteUInt16(std::size_t offset, uint16_t value) {
        std::memcpy(data_.data() + offset, &value, sizeof(uint16_t));
    }

    /**
     * @brief Updates the slot count metadata in the page header.
     */
    void Page::SetNumSlots(uint16_t value) {
        WriteUInt16(NUM_SLOTS_OFFSET, value);
    }

    /**
     * @brief Updates the free space offset pointer in the page header.
     */
    void Page::SetFreeSpaceOffset(uint16_t value) {
        WriteUInt16(FREE_SPACE_OFFSET, value);
    }

    /**
     * @brief Reads a slot entry from the slot directory at the specified slot ID.
     */
    Page::Slot Page::ReadSlot(uint16_t slot_id) const {
        std::size_t offset = HEADER_SIZE + (slot_id * SLOT_SIZE);
        Slot slot;
        std::memcpy(&slot.offset, data_.data() + offset, sizeof(uint16_t));
        std::memcpy(&slot.length, data_.data() + offset + 2, sizeof(uint16_t));
        std::memcpy(&slot.flags, data_.data() + offset + 4, sizeof(uint16_t));
        return slot;
    }

    /**
     * @brief Writes a slot entry into the slot directory at the specified slot ID.
     */
    void Page::WriteSlot(uint16_t slot_id, const Slot& slot) {
        std::size_t offset = HEADER_SIZE + (slot_id * SLOT_SIZE);
        std::memcpy(data_.data() + offset, &slot.offset, sizeof(uint16_t));
        std::memcpy(data_.data() + offset + 2, &slot.length, sizeof(uint16_t));
        std::memcpy(data_.data() + offset + 4, &slot.flags, sizeof(uint16_t));
    }

} // namespace minidb

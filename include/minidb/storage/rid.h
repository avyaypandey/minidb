#pragma once

#include <cstdint>

namespace minidb
{

struct RID
{
    uint32_t page_id{0};
    uint32_t slot_id{0};

    bool operator==(const RID& other) const
    {
        return page_id == other.page_id && slot_id == other.slot_id;
    }
};

} // namespace minidb

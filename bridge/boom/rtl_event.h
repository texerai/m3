/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef RTL_EVENT_H_
#define RTL_EVENT_H_

// C++ libraries.
#include <stdint.h>

// Local libraries.
#include "memop_info.h"

namespace m3
{
    enum class RTLEvent
    {
        kUndefined,
        kCreateMemop,
        kAddMemopAddress,
        kPerformLoad,
        kAddStoreData,
        kCompleteStore,
        kCommitMemop,
        kUpdateCacheLineState,
        kUpdateCacheLineData
    };

    struct RTLEventData
    {
        RTLEvent event = RTLEvent::kUndefined;
        MemopType memop_type = MemopType::kUndefined;
        MemopFormat data_format = MemopFormat::kUndefined;
        uint32_t hart_id = 0;
        uint32_t rob_id = 0;
        uint32_t load_dest_reg = 0;
        uint32_t memop_size = 0;
        uint64_t address = 0;
        uint32_t memop_id = 0;
        uint64_t load_rtl_data = 0;
        uint64_t store_data = 0;
        uint64_t timestamp = 0;

        // Cache line related data.
        uint32_t coherence_state = 0;
        uint32_t way_id = 0;
        uint32_t cache_line_id = 0;
        uint64_t tag = 0;
    };
}
#endif
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
        kSendDcacheAmo,
        kCompleteStore,
        kCommitMemop,
        kCompleteAmo,
        kUpdateCacheLineState,
        kUpdateCacheLineData,
        KRecoveryRob
    };

    struct RTLEventData
    {
        RTLEvent event = RTLEvent::kUndefined;
        MemopType memop_type = MemopType::kUndefined;
        AmoType amo_type = AmoType::kUndefined;
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
        uint32_t store_buffer_id = 0;
        uint32_t amo_rob_id = 0;
        uint32_t complete_amo_rob_id = 0;


        // Cache line related data.
        uint32_t coherence_state = 0;
        uint32_t way_id = 0;
        uint32_t cache_line_id = 0;
        uint64_t tag = 0;

        // Rob recovery related data.
        uint32_t rob_head = 0;
        int      xcpt     = 0;
    };
}
#endif
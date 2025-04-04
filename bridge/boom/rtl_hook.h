/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef RTL_HOOK_H_
#define RTL_HOOK_H_

// C++ libraries.
#include <map>
#include <stdint.h>

namespace m3
{
    // RTL Hooks exposed from BOOM to the M3 model.
    enum class RtlHook
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

    // Hooks can be triggered at the same clock cycle.
    // Prioritize the hooks. The bridge function of the hook
    // with higher priority will execute first.
    static std::map<RtlHook, uint32_t> kRtlHookPriority = {
        { RtlHook::kCreateMemop, 0 },
        { RtlHook::kAddMemopAddress, 0 },
        { RtlHook::kPerformLoad, 0 },
        { RtlHook::kAddStoreData, 0 },
        { RtlHook::kCompleteStore, 0 },
        { RtlHook::kCommitMemop, 0 },
        { RtlHook::kUpdateCacheLineState, 0 },
        { RtlHook::kUpdateCacheLineData, 0 }
    };

    // Data that is passed from the RTL to the M3 model when
    // the hook is triggered.
    struct RtlHookData
    {
        RtlHook event = RtlHook::kUndefined;
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

        // Debugging signals, used for assertions
        // and double checking.
        bool is_load = false;
        bool is_store = false;
        bool is_amo = false;
    };
}
#endif
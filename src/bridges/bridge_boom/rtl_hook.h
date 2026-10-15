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
        kFlushRob,
        kBranchMispredict,
        kBranchResolve,
        kBranchPredictionStart,
        kUpdateCacheLineState,
        kUpdateCacheLineData
    };

    // Hooks can be triggered at the same clock cycle.
    // Prioritize the hooks. The bridge function of the hook
    // with higher priority will execute first; hooks of equal
    // priority execute in the order they were registered.
    //
    // Within a cycle, store addresses/data must be known before loads
    // perform (a load can forward from a store whose data arrives in
    // the same cycle), and loads must perform before they commit.
    // Events keyed by ROB id that share a cycle with a CreateMemop for
    // the same id belong to the previous occupant of that ROB entry
    // (BOOM never produces address/data/completion in the enqueue
    // cycle), so CreateMemop runs last.
    static std::map<RtlHook, uint32_t> kRtlHookPriority = {
        { RtlHook::kAddMemopAddress, 3 },
        { RtlHook::kAddStoreData, 3 },
        { RtlHook::kPerformLoad, 2 },
        { RtlHook::kCommitMemop, 1 },
        { RtlHook::kCompleteStore, 0 },
        { RtlHook::kUpdateCacheLineState, 0 },
        { RtlHook::kUpdateCacheLineData, 0 },
        { RtlHook::kFlushRob, 0 },
        { RtlHook::kBranchMispredict, 0 },
        { RtlHook::kBranchResolve, 0 },
        { RtlHook::kBranchPredictionStart, 0 },
        { RtlHook::kCreateMemop, 0 }
    };

    // Data that is passed from the RTL to the M3 model when
    // the hook is triggered.
    struct RtlHookData
    {
        RtlHook event = RtlHook::kUndefined;
        uint32_t hart_id = 0;
        uint32_t rob_id = 0;
        uint32_t memop_size = 0;
        uint32_t rv_instruction = 0;
        uint32_t branch_mask = 0;
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

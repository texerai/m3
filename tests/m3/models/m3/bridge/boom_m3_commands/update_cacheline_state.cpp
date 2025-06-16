/*
* Copyright (c) 2023-2024 Micro Architecture Santa Cruz
* and Texer.ai. All rights reserved.
*/
#include "update_cacheline_state.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "rvutils.h"
#include "state.h"
#include "debug_utils.hpp"

// C++ libraries.
#include <cassert>
#include <fmt/core.h>

namespace m3
{
    static const uint32_t kMesiMState = 3;

    UpdateCachelineState::UpdateCachelineState(const RTLEventData& data)
    {
        data_ = data;
    }

    bool UpdateCachelineState::Execute(State& state)
    {
        bool should_abort = false;

        M3Cores& m3cores = state.m3cores;
        Cache& cache = state.cache;
        auto& beyond_core_stores = state.beyond_core_stores;
        RTLEventData& d = data_;

        cache.UpdateMetaData(d.way_id, d.cache_line_id, d.coherence_state, d.tag);
        DEBUG_LOG(fmt::format(
            "Update cacheline state, state: {}, tag: 0x{:x}, count: {}",
            static_cast<int>(d.coherence_state),
            d.tag,
            beyond_core_stores[d.hart_id].count(d.tag)
        ), debug::VerbosityLevel::Low);

        // What if gets invalidated?
        // What if it was a store miss and the data never got
        // written to the cache because it was invalidated by
        // coherence mechanisms?
        if (d.coherence_state == kMesiMState)
        {
            if (beyond_core_stores[d.hart_id].count(d.tag) > 0)
            {
                m3cores[d.hart_id].st_globally_perform(beyond_core_stores[d.hart_id][d.tag]);
                beyond_core_stores[d.hart_id].erase(d.tag);
                was_globally_performed_ = true;
            }
            else
            {
                DEBUG_LOG(
                    "Warning: changing cache line state, but M3 never seen it",
                    debug::VerbosityLevel::Warning);
            }
        }

        return !should_abort;
    }
}

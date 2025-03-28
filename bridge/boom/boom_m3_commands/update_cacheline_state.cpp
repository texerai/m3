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
#include "tracer.h"

// C++ libraries.
#include <cassert>

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

        // What if gets invalidated?
        // What if it was a store miss and the data never got
        // written to the cache because it was invalidated by
        // coherence mechanisms?
        if (d.coherence_state == kMesiMState)
        {
            if (beyond_core_stores[d.hart_id].count(d.tag) > 0)
            {
                const auto& m3id = state.beyond_core_stores[d.hart_id][d.tag].m3id;
                assert(m3id > 0);
                m3cores[d.hart_id].st_globally_perform(m3id);
                beyond_core_stores[d.hart_id].erase(d.tag);
                was_globally_performed_ = true;
                performed_id_ = static_cast<uint64_t>(m3id);
            }
            else
            {
                //assert(false);
                //should_abort = true;
                std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
            }
        }

        return !should_abort;
    }

    bool UpdateCachelineState::Execute(State& state, Tracer& m3tracer)
    {
        bool is_executed = Execute(state);
        if (is_executed && was_globally_performed_)
        {
            auto& d = data_;
            m3tracer.RecordEvent(m3::MemoryEvent::kStoreGlobalPerform, d.hart_id, d.tag,
                0, 0, performed_id_,
                static_cast<uint32_t>(MemopType::kStore), d.timestamp);
        }

        return is_executed;
    }
}
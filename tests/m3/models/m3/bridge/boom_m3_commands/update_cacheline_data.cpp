/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "update_cacheline_data.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "rvutils.h"
#include "state.h"

// C++ libraries.
#include <cassert>

namespace m3
{
    static const uint32_t kMesiMState = 3;

    UpdateCachelineData::UpdateCachelineData(const RTLEventData& data)
    {
        data_ = data;
    }

    bool UpdateCachelineData::Execute(State& state)
    {
        /*M3Cores& m3cores = state.m3cores;
        Cache& cache = state.cache;
        auto& beyond_core_stores = state.beyond_core_stores;
        RTLEventData& d = data_;

        uint32_t cache_line_id = d.address >> 6;
        auto& cache_line = cache.banks[d.way_id][d.cache_line_id];
        if (cache_line.coh_state == kMesiMState)
        {
            if (beyond_core_stores[d.hart_id].count(cache_line.tag) > 0)
            {
                const auto& m3id = state.beyond_core_stores[d.hart_id][cache_line.tag].m3id;
                assert(m3id > 0);
                m3cores[d.hart_id].st_globally_perform(m3id);
                beyond_core_stores[d.hart_id].erase(cache_line.tag);
                was_globally_performed_ = true;
                performed_id_ = static_cast<uint64_t>(m3id);
            }
            else
            {
                //assert(false);
                std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
            }
        }*/

        return true;
    }
}
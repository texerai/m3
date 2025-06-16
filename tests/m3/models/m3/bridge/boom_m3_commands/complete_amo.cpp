/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "complete_amo.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "rvutils.h"
#include "state.h"

// C++ libraries.
#include <cassert>

namespace m3
{
    CompleteAmo::CompleteAmo(const RTLEventData& data)
    {
        data_ = data;
    }

    bool CompleteAmo::Execute(State& state)
    {
        RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;
        auto& beyond_core_stores = state.beyond_core_stores;
        MemopInfo* completing_memop;
        
        
        /*// Get the entry.
        for( int i = 0; i < (state.in_core_memops[d.hart_id]).size(); i++ ){ //[TODO]check if this works and if this amo is in_core_memops
            MemopInfo& aux = state.in_core_memops[d.hart_id][i];
            if( aux.rob_id == d.complete_amo_rob_id ){ //should this be aux.rob_id or aux.amo_rob_id?
                *completing_memop = aux;
                break;
            }
        }

        assert(completing_memop->memop_type == MemopType::kAmo);

        // Set safe this store.
        m3cores[d.hart_id].set_safe(completing_memop->m3id);

        //[TODO] Check: doing the same as for CompleteStore
        // The store can be globally performed soon.
        // The stores that go to the same cache line can be merged.
        uint64_t cache_line_tag = completing_memop->address >> 6;
        bool is_first = beyond_core_stores[d.hart_id].find(cache_line_tag) == beyond_core_stores[d.hart_id].end();
        if (!is_first)
        {
            // Merge what was there previously with the new coming store.
            MemopInfo& prev_memop = beyond_core_stores[d.hart_id][cache_line_tag];
            m3cores[d.hart_id].st_locally_merged(prev_memop.m3id, completing_memop->m3id);
        }
        else
        {
            completing_memop->completed_time = d.timestamp;
            beyond_core_stores[d.hart_id][cache_line_tag] = *completing_memop;
        }*/

        return true;

    }
}

/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "complete_store.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "state.h"

namespace m3
{
    CompleteStore::CompleteStore(const RTLEventData& data)
    {
        data_ = data;
        priority_ = 1;
    }

    bool CompleteStore::Execute(State& state)
    {
        /*RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;
        auto& beyond_core_stores = state.beyond_core_stores;
        MemopInfo* completing_memop;

        // Get the entry.
        for( int i = 0; i < (state.in_core_memops[d.hart_id]).size(); i++ ){ //[TODO]check if this works
            MemopInfo& aux = state.in_core_memops[d.hart_id][i];
            if( aux.store_buffer_id == d.store_buffer_id ){
                *completing_memop = aux;
                break;
            }
        }

        assert(completing_memop->memop_type == MemopType::kStore);

        // Set safe this store.
        m3cores[d.hart_id].set_safe(completing_memop->m3id);

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
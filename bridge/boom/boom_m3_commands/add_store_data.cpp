/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "add_store_data.h"

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
    AddStoreData::AddStoreData(const RtlHookData& data)
    {
        data_ = data;
    }

    bool AddStoreData::Execute(State& state)
    {
        RtlHookData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Get the M3 entry based on ROB ID.
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Skip if already set.
        if (!memop_info.is_data_valid)
        {
            // Save data.
            memop_info.store_data = d.store_data;
            memop_info.is_data_valid = true;

            // Locally perform the store.
            // So that the loads could forward the values locally.
            if (memop_info.CanBePerformed())
            {
                auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                m3cores[d.hart_id].st_locally_perform(m3id);
            }
        }

        return true;
    }

    bool AddStoreData::Execute(State& state, Tracer& m3tracer)
    {
        bool ret = Execute(state);

        RtlHookData& d = data_;
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        if (memop_info.CanBePerformed())
        {
            m3tracer.RecordEvent(m3::MemoryEvent::kStoreLocalPerform, d.hart_id, memop_info.address,
                memop_info.store_data, 0, static_cast<uint64_t>(memop_info.m3id),
                static_cast<uint32_t>(memop_info.memop_type), d.timestamp);
        }

        return ret;
    }
}
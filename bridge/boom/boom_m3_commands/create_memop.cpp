/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "create_memop.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "state.h"
#include "tracer.h"

namespace m3
{
    CreateMemop::CreateMemop(const RtlHookData& data)
    {
        data_ = data;
    }

    bool CreateMemop::Execute(State& state)
    {
        RtlHookData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Get the entry.
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        // Remove the entry from M3 if previously allocated entry
        // did not complete.
        if (!memop_info.committed && !memop_info.is_just_created)
        {
            m3cores[d.hart_id].nuke(memop_info.m3id);
        }

        // Drop the previous information.
        memop_info.Invalidate();

        // New entry in m3.
        memop_info.is_just_created = false;
        memop_info.m3id = m3cores[d.hart_id].inorder();
        memop_info.memop_type = d.memop_type;
        memop_info.rob_id = d.rob_id;
        memop_info.load_dest_reg = d.load_dest_reg;

        return true;
    }

    bool CreateMemop::Execute(State& state, Tracer& m3tracer)
    {
        bool ret = Execute(state);

        RtlHookData& d = data_;
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];
        m3tracer.RecordEvent(m3::MemoryEvent::kMemopCreated, d.hart_id, 0, 0, 0,
            static_cast<uint64_t>(memop_info.m3id),
            static_cast<uint32_t>(memop_info.memop_type), d.timestamp);

        return ret;
    }
}
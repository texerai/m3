/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "send_dcache_amo.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "rvutils.h"
#include "state.h"

// C++ libraries.
#include <cassert>

namespace m3
{
    SendDcacheAmo::SendDcacheAmo(const RTLEventData& data)
    {
        data_ = data;
    }

    bool SendDcacheAmo::Execute(State& state)
    {
        /*RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Should be created by this time.
        assert(state.in_core_memops[d.hart_id].count(d.rob_id) > 0);
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        assert(memop_info.memop_type == MemopType::kAmo);

        // Get the reference.
        Inst_id m3id = Inst_id(memop_info.m3id);
        auto& d_load = m3cores[d.hart_id].ld_data_ref(m3id);
        auto& d_store = m3cores[d.hart_id].st_data_ref(m3id);

        //[TODO] What should be done in this event???
        //AMOs are locally performed at CommitMemop
        //d.amo_rob_id ??*/

        return true;
    }
}

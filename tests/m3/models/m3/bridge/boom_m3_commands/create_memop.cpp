/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

// C++ libraries.
#include "fmt/core.h"

// Local libraries.
#include "create_memop.h"
#include "memop_info.h"
#include "rtl_event.h"
#include "state.h"
#include "debug_utils.hpp"

namespace m3
{
    CreateMemop::CreateMemop(const RTLEventData& data)
    {
        data_ = data;
    }

    bool CreateMemop::Execute(State& state)
    {
        RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Get the entry.
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        DEBUG_LOG(fmt::format(
            "Create operation, hart_id: {}, rtl rob_id: {}, memop_type: {}",
            d.hart_id, d.rob_id, int(memop_info.memop_type)
        ), debug::VerbosityLevel::Low);

        // Drop the previous information.
        memop_info.Invalidate();

        // New entry in m3.
        memop_info.m3id = m3cores[d.hart_id].inorder(d.rob_id);
        memop_info.memop_type = d.memop_type;
        memop_info.amo_type = d.amo_type;
        memop_info.rob_id = d.rob_id;

        DEBUG_LOG(fmt::format(
            "Create operation, m3 rob_id: {}",
            memop_info.m3id
        ), debug::VerbosityLevel::Low);
        //m3cores[d.hart_id].add_rob_entry(d.rob_id);
        return true;
    }
}
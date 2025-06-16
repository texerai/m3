/*
* Copyright (c) 2023 Micro Architecture Santa Cruz
* and Texer.ai. All rights reserved.
*/
#include "perform_load.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "rvutils.h"
#include "state.h"
#include "debug_utils.hpp"

// C++ libraries.
#include <cassert>
#include "fmt/core.h"

namespace m3
{
    PerformLoad::PerformLoad(const RTLEventData& data)
    {
        data_ = data;
    }

    bool PerformLoad::Execute(State& state)
    {
        RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Get memop info based on ROB ID.
        DEBUG_ASSERT(state.in_core_memops[d.hart_id].count(d.rob_id) > 0,
            fmt::format("in_core_memops entry not found, hart_id: {}, m3 rob_id: {}",
            d.hart_id, d.rob_id));
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Perform the load.
        auto &d_load = m3cores[d.hart_id].ld_data_ref(m3id);
        m3cores[d.hart_id].ld_perform(m3id);

        uint64_t model_data = d_load.get_data(memop_info.address, memop_info.size);

        DEBUG_LOG(fmt::format("Perform load, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, data: 0x{:x}, addr: 0x{:x}, size: {}",
            d.hart_id, d.rob_id, m3id, model_data, memop_info.address, memop_info.size), debug::VerbosityLevel::Low);

        return true;
    }
}

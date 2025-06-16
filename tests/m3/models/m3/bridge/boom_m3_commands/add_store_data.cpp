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
#include "debug_utils.hpp"

// C++ libraries.
#include <cassert>
#include "fmt/core.h"

namespace m3
{
    AddStoreData::AddStoreData(const RTLEventData& data)
    {
        data_ = data;
        priority_ = 1;
    }

    bool AddStoreData::Execute(State& state)
    {
        RTLEventData& d = data_;
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
            DEBUG_LOG(fmt::format(
                "Add store data, data: 0x{:x}, addr: 0x{:x}, size: {}, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}",
                memop_info.store_data, memop_info.address, memop_info.size, d.hart_id, d.rob_id, m3id
            ), debug::VerbosityLevel::Low);            

            // Locally perform the store.
            // So that the loads could forward the values locally.
            if (memop_info.CanBePerformed())
            {
                auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                m3cores[d.hart_id].st_locally_perform(m3id);
                DEBUG_LOG("Add store data, performing local store", debug::VerbosityLevel::Low);
            }
        }

        return true;
    }
}
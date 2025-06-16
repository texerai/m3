/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "add_address.h"

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
    AddAddress::AddAddress(const RTLEventData& data)
    {
        data_ = data;
        priority_ = 1;
    }

    bool AddAddress::Execute(State& state)
    {
        RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Should be created by this time.
        DEBUG_ASSERT(state.in_core_memops[d.hart_id].count(d.rob_id) > 0,
            fmt::format("in_core_memops entry not found, hart_id: {}, m3 rob_id: {}",
            d.hart_id, d.rob_id));
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        /*
        //AddMemopAddress interface doesn't have the type anymore, so rely on memop_info.memop_type
        if (d.memop_type == MemopType::kLoad)
        {
            assert(memop_info.memop_type == MemopType::kLoad);
        }
        else
        {
            assert(memop_info.memop_type == MemopType::kStore
                || memop_info.memop_type == MemopType::kAmo);
        }
        */

        // Calculate memop size.
        //uint64_t byte_size = RVUtils::inst_size_to_byte_size(d.memop_size);

        // Get the reference.
        Inst_id m3id = Inst_id(memop_info.m3id);
        auto& d_load = m3cores[d.hart_id].ld_data_ref(m3id);
        auto& d_store = m3cores[d.hart_id].st_data_ref(m3id);

        memop_info.size = d.memop_size; //byte_size;
        memop_info.address = d.address;
        memop_info.is_address_valid = true;

        switch(memop_info.memop_type) //switch (d.memop_type)
        {
            case MemopType::kLoad:
                DEBUG_LOG("Add memop load ", debug::VerbosityLevel::Low);
                d_load.add_addr(d.address, d.memop_size);
                break;
            case MemopType::kStore:
                DEBUG_LOG("Add memop store ", debug::VerbosityLevel::Low);
                d_store.add_addr(d.address, d.memop_size);
                // TODO: check if we ever use stq_id.
                //memop_info.stq_id = d.memop_id;
                break;
            case MemopType::kAmo:
                DEBUG_LOG("Add memop amo ", debug::VerbosityLevel::Low);
                d_store.add_addr(d.address, d.memop_size);
                d_load.add_addr(d.address, d.memop_size);
                //memop_info.stq_id = d.memop_id;
                break;
            default:
                DEBUG_ASSERT(false, fmt::format(
                    "Not memory instruction type, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}",
                    d.hart_id, d.rob_id, memop_info.m3id
                ));
                break;
        }
        DEBUG_LOG(fmt::format(
            "hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, addr: 0x{:x}, size: {}",
            d.hart_id, d.rob_id, memop_info.m3id, d.address, d.memop_size
        ), debug::VerbosityLevel::Low);        

        // Locally perform the store.
        // So that the loads could forward the values locally.
        if (memop_info.CanBePerformed())
        {
            auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
            d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
            m3cores[d.hart_id].st_locally_perform(m3id);
            DEBUG_LOG("Perform local store", debug::VerbosityLevel::Low);
        }

        return true;
    }
}

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
#include "tracer.h"

// C++ libraries.
#include <cassert>

namespace m3
{
    AddAddress::AddAddress(const RtlHookData& data)
    {
        data_ = data;
    }

    bool AddAddress::Execute(State& state)
    {
        RtlHookData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Should be created by this time.
        assert(state.in_core_memops[d.hart_id].count(d.rob_id) > 0);
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];
        if (d.memop_type == MemopType::kLoad)
        {
            assert(memop_info.memop_type == MemopType::kLoad);
        }
        else
        {
            assert(memop_info.memop_type == MemopType::kStore
                || memop_info.memop_type == MemopType::kAmo);
        }

        // Calculate memop size.
        uint64_t byte_size = RVUtils::inst_size_to_byte_size(d.memop_size);

        // Get the reference.
        Inst_id m3id = Inst_id(memop_info.m3id);
        auto& d_load = m3cores[d.hart_id].ld_data_ref(m3id);
        auto& d_store = m3cores[d.hart_id].st_data_ref(m3id);

        memop_info.size = byte_size;
        memop_info.address = d.address;
        memop_info.is_address_valid = true;
        switch (d.memop_type)
        {
            case MemopType::kLoad:
                d_load.add_addr(d.address, byte_size);
                break;
            case MemopType::kStore:
                d_store.add_addr(d.address, byte_size);
                // TODO: check if we ever use stq_id.
                memop_info.stq_id = d.memop_id;
                break;
            case MemopType::kAmo:
                d_store.add_addr(d.address, byte_size);
                d_load.add_addr(d.address, byte_size);
                memop_info.stq_id = d.memop_id;
                break;
            default:
                assert(false);
                break;
        }

        // Locally perform the store.
        // So that the loads could forward the values locally.
        if (memop_info.CanBePerformed())
        {
            auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
            d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
            m3cores[d.hart_id].st_locally_perform(m3id);
        }

        return true;
    }

    bool AddAddress::Execute(State& state, Tracer& m3tracer)
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

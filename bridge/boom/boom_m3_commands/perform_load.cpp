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
#include "tracer.h"

// C++ libraries.
#include <cassert>

namespace m3
{
    PerformLoad::PerformLoad(const RtlHookData& data)
    {
        data_ = data;
    }

    bool PerformLoad::Execute(State& state)
    {
        RtlHookData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // Get memop info based on ROB ID.
        assert(state.in_core_memops[d.hart_id].count(d.rob_id) > 0);
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Perform the load.
        auto &d_load = m3cores[d.hart_id].ld_data_ref(m3id);
        m3cores[d.hart_id].ld_perform(m3id);

        uint64_t model_data = d_load.get_data(memop_info.address, memop_info.size);
        uint64_t rtl_data = d.load_rtl_data;

        memop_info.load_model_data = model_data;
        memop_info.load_rtl_data = rtl_data;
        m3::MemoryEvent tracer_event = m3::MemoryEvent::kLoadPerform;
        int64_t model_data_signed = RVUtils::sign_extend(model_data, memop_info.size);
        int64_t loaded_data_signed = RVUtils::sign_extend(rtl_data, memop_info.size);

        // Check the load data.
        bool is_success = model_data_signed == loaded_data_signed;
        if (!is_success)
        {
            memop_info.check_failed = true;
            tracer_event = m3::MemoryEvent::kLoadPerformFailed;
            std::cout << "Error: Load Perform Failed (iid: ";
            std::cout << static_cast<uint32_t>(memop_info.m3id) << ")" << std::endl;
            std::cout << "  DUT: " << rtl_data << ", Model: " << model_data;
            std::cout << ", Address: " << memop_info.address << std::endl;
        }

        return is_success;
    }

    bool PerformLoad::Execute(State& state, Tracer& m3tracer)
    {
        bool is_success = Execute(state);

        RtlHookData& d = data_;
        MemopInfo& memop_info = state.in_core_memops[d.hart_id][d.rob_id];

        // Trace for debugging purposes.
        MemoryEvent tracer_event = MemoryEvent::kLoadPerform;
        if (!is_success)
        {
            tracer_event = MemoryEvent::kLoadPerformFailed;
        }
        int64_t model_data_signed = RVUtils::sign_extend(memop_info.load_model_data, memop_info.size);
        int64_t loaded_data_signed = RVUtils::sign_extend(memop_info.load_rtl_data, memop_info.size);
        m3tracer.RecordEvent(tracer_event, d.hart_id, memop_info.address, model_data_signed, loaded_data_signed,
            static_cast<uint64_t>(memop_info.m3id), static_cast<uint32_t>(memop_info.memop_type), d.timestamp);

        return is_success;
    }
}
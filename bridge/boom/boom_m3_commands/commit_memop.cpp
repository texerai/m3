/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "commit_memop.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "state.h"
#include "tracer.h"

namespace m3
{
    constexpr bool isDebugOn = true;

    CommitMemop::CommitMemop(const RtlHookData& data, std::shared_ptr<dromajo_t> core_model_ptr)
    {
        data_ = data;
        core_model_ptr_ = core_model_ptr;
    }

    bool CommitMemop::Execute(State& state)
    {
        bool should_abort = false;
        M3Cores& m3cores = state.m3cores;
        auto& in_core_memops = state.in_core_memops;
        auto& d = data_;

        // Get the M3 entry based on ROB ID.
        assert(in_core_memops[d.hart_id].count(d.rob_id) > 0);
        MemopInfo& memop_info = in_core_memops[d.hart_id][d.rob_id];
        assert(memop_info.memop_type != MemopType::kUndefined);
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Set this safe. Point of no return: instruction commited.
        // Stores are set safe not at commit, but when they succeed
        // writing the cacheline. Memops besides stores can be
        // marked safe.
        memop_info.committed = true;
        if (memop_info.memop_type != MemopType::kStore)
        {
            m3cores[d.hart_id].set_safe(m3id);
        }

        // Load data check.
        if (memop_info.memop_type == MemopType::kLoad)
        {
            assert(core_model_ptr_ != nullptr);
            if (!memop_info.check_failed)
            {
                bool is_fp = memop_info.load_dest_reg > 31;
                uint32_t rd = memop_info.load_dest_reg % 32;
                core_model_ptr_->update_register(d.hart_id, rd, memop_info.load_rtl_data, is_fp);
            }
            else
            {
                should_abort = true;
                if constexpr (isDebugOn)
                {
                    std::cout << "Error: Load commiting with a mismatched performed data.\n";
                    std::cout << " - Model data: " << memop_info.load_model_data << "\n";
                    std::cout << " - RTL data: " << memop_info.load_rtl_data << "\n";
                }
            }
        }
        // AMOs should get visible at commit. Commit of AMO
        // guarantees that the data in the cache.
        else if (memop_info.memop_type == MemopType::kAmo)
        {
            auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
            d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
            m3cores[d.hart_id].st_locally_perform(m3id);
            m3cores[d.hart_id].st_globally_perform(m3id);
        }

        return !should_abort;
    }

    bool CommitMemop::Execute(State& state, Tracer& m3tracer)
    {
        M3Cores& m3cores = state.m3cores;
        auto& in_core_memops = state.in_core_memops;
        auto& d = data_;
        MemopInfo& memop_info = in_core_memops[d.hart_id][d.rob_id];

        // Execute.
        bool is_executed = Execute(state);

        // Trace for debugging purposes.
        uint64_t model_data = memop_info.load_model_data;
        uint64_t rtl_data = memop_info.load_rtl_data;
        if (memop_info.memop_type != MemopType::kLoad)
        {
            model_data = memop_info.store_data;
            rtl_data = 0;
        }
        m3tracer.RecordEvent(m3::MemoryEvent::kMemopCommited, d.hart_id, memop_info.address,
            model_data, rtl_data, static_cast<uint64_t>(memop_info.m3id),
            static_cast<uint32_t>(memop_info.memop_type), d.timestamp);

        return is_executed;
    }
}
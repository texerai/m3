/*
* Copyright (c) 2023-2024 Micro Architecture Santa Cruz
* and Texer.ai. All rights reserved.
*/
#include "commit_memop.h"

// Local libraries.
#include "memop_info.h"
#include "rtl_event.h"
#include "state.h"
#include "rvamo_utils.h"
#include "debug_utils.hpp"

namespace m3
{
    constexpr bool isDebugOn = true;

    CommitMemop::CommitMemop(const RTLEventData& data)
    {
        data_ = data;
    }

    bool CommitMemop::Execute(State& state)
    {
        bool should_abort = false;
        M3Cores& m3cores = state.m3cores;
        auto& in_core_memops = state.in_core_memops;
        auto& d = data_;

        // Get the M3 entry based on ROB ID.
        DEBUG_ASSERT(in_core_memops[d.hart_id].count(d.rob_id) > 0,
            "no incore_memops entry found");
        MemopInfo& memop_info = in_core_memops[d.hart_id][d.rob_id];
        //if(memop_info.memop_type != MemopType::kUndefined) {
            //memop_info.store_buffer_id = d.store_buffer_id; //[TODO] is this correct? is this a pointer that updates the in_core_memop from state?
            //assert(memop_info.memop_type != MemopType::kUndefined);
            Inst_id m3id = Inst_id(memop_info.m3id);

            DEBUG_LOG(fmt::format(
                "Commit mem operation, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, xcpt: {}",
                d.hart_id, d.rob_id, m3id, d.xcpt
            ), debug::VerbosityLevel::Low);

            // exception case, keep entry but don't commit
            // No extra checking is necesary cause:
            // 1- in next cycle in spike, this entry will be flushed
            // 2- when there's an exception, it is the only instruction in the commit window
            if (d.xcpt) {
                return 1;
            }

            // Set this safe. Point of no return: instruction commited.
            // Stores are set safe not at commit, but when they succeed
            // writing the cacheline. Memops besides stores can be
            // marked safe.
            memop_info.committed = true;
            if (memop_info.memop_type != MemopType::kStore)
            {
                m3cores[d.hart_id].set_safe(m3id);
                DEBUG_LOG(", set_safe ", debug::VerbosityLevel::Low);
            }

            // AMOs should get visible at commit. Commit of AMO
            // guarantees that the data in the cache.
            if (memop_info.memop_type == MemopType::kAmo)
            {
                // Perform amo operation locally
                // Sizes for load and store operation are the same
                Gold_data temp_data;
                // get gold_data
                temp_data.add_addr(memop_info.address, memop_info.size);
                temp_data.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                auto &d_load = m3cores[d.hart_id].ld_data_ref(m3id);
                // obtain gol_data value
                uint64_t load_data = d_load.get_data(memop_info.address, memop_info.size);
                uint64_t store_data = temp_data.get_data(memop_info.address, memop_info.size);
                // calculate amo store value
                DEBUG_ASSERT(memop_info.amo_type != AmoType::kUndefined,
                "Undefined amo type");
                DEBUG_ASSERT((memop_info.size == 4) || (memop_info.size == 8),
                "Wrong amo access size");

                DEBUG_LOG(fmt::format(
                    "perform amo: type: {} size: {} load_data: 0x{:x} store_data: 0x{:x}",
                    static_cast<int>(memop_info.amo_type), memop_info.size, load_data, store_data
                ), debug::VerbosityLevel::Low);

                uint64_t amo_store_data = RiscV_AMO::execute(memop_info.amo_type, memop_info.size, load_data, store_data);
                // Store amo operation result
                auto &d_store = m3cores[d.hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, amo_store_data);
                m3cores[d.hart_id].st_locally_perform(m3id);
                m3cores[d.hart_id].st_globally_perform(m3id);
                // Display debug information
                uint64_t cache_line_tag = memop_info.address >> 6;

                DEBUG_LOG(fmt::format(
                    ", perform amo op, tag: 0x{:x}, addr: 0x{:x} data: 0x{:x} size: {}",
                    cache_line_tag, memop_info.address, amo_store_data, memop_info.size
                ), debug::VerbosityLevel::Low);
            }
            if (memop_info.memop_type == MemopType::kStore)
            {
                //assert(memop_info.memop_type == MemopType::kStore);
                auto& beyond_core_stores = state.beyond_core_stores;

                // Set safe this store.
                m3cores[d.hart_id].set_safe(m3id);
                DEBUG_LOG(", set_safe", debug::VerbosityLevel::Low);

                // The store can be globally performed soon.
                // The stores that go to the same cache line can be merge
                uint64_t cache_line_tag = memop_info.address >> 6;
                bool is_first = beyond_core_stores[d.hart_id].find(cache_line_tag) == beyond_core_stores[d.hart_id].end();
                if (!is_first)
                {
                    // Merge what was there previously with the new coming store.
                    beyond_core_stores[d.hart_id][cache_line_tag].push_back(memop_info.m3id);
                    DEBUG_LOG(", Merge store buffer", debug::VerbosityLevel::Low);
                }
                else
                {
                    memop_info.completed_time = d.timestamp;
                    beyond_core_stores[d.hart_id][cache_line_tag].push_back(memop_info.m3id);
                    DEBUG_LOG(", Set new value store buffer", debug::VerbosityLevel::Low);
                }
            }
        //}

        return !should_abort;
    }
}

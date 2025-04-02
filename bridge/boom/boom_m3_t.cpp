/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "boom_m3_t.h"

// C++ libraries.
#include <cassert>
#include <memory>
#include <unordered_map>
#include <functional>

// Local libraries.
#include "memory.hpp"
#include "rtl_hook.h"
#include "rvutils.h"
#include "state.h"
#include "tracer.h"

extern uint8_t dromajo_get_byte_direct(uint64_t paddr);
static Memory mem(dromajo_get_byte_direct);

namespace m3
{
    // Commands in the model have priority. If two commands
    // were registered at the same clock cycle the commands
    // with higher priority need to be executed first.
    // This constant represents the highest command
    // priority in the model.
    static const uint32_t kMaxPriority = 1;

    // Tracer.
    static const uint32_t kTracerBufferSize = 1024;
    static Tracer m3tracer;
    static State state;

    namespace commands
    {
        static bool CreateMemop(const RtlHookData& data, State& state)
        {

            M3Cores& m3cores = state.m3cores;

            // Get the entry.
            MemopInfo& memop_info = state.in_core_memops[data.hart_id][data.rob_id];

            // Remove the entry from M3 if previously allocated entry
            // did not complete.
            if (!memop_info.committed && !memop_info.is_just_created)
            {
                m3cores[data.hart_id].nuke(memop_info.m3id);
            }

            // Drop the previous information.
            memop_info.Invalidate();

            // New entry in m3.
            memop_info.is_just_created = false;
            memop_info.m3id = m3cores[data.hart_id].inorder();
            memop_info.rob_id = data.rob_id;
            memop_info.load_dest_reg = data.load_dest_reg;
            // Define memop type: store, load, amo, etc.

            return true;
        }

        static bool CompleteStore(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;
            auto& beyond_core_stores = state.beyond_core_stores;

            // Get the entry.
            MemopInfo& completing_memop = state.in_core_memops[data.hart_id][data.rob_id];
            assert(completing_memop.memop_type == MemopType::kStore);

            // Set safe this store.
            m3cores[data.hart_id].set_safe(completing_memop.m3id);

            // The store can be globally performed soon.
            // The stores that go to the same cache line can be merged.
            uint64_t cache_line_tag = completing_memop.address >> 12;
            bool is_first = beyond_core_stores[data.hart_id].find(cache_line_tag) == beyond_core_stores[data.hart_id].end();
            if (!is_first)
            {
                // Merge what was there previously with the new coming store.
                MemopInfo& prev_memop = beyond_core_stores[data.hart_id][cache_line_tag];
                m3cores[data.hart_id].st_locally_merged(prev_memop.m3id, completing_memop.m3id);
            }
            else
            {
                completing_memop.completed_time = data.timestamp;
                beyond_core_stores[data.hart_id][cache_line_tag] = completing_memop;
            }

            return true;
        }

        static std::map<RtlHook, std::function<bool(const RtlHookData&, State&)>> kRtlHookCommands = {
            { RtlHook::kCreateMemop, CreateMemop }
        };
    }

    // Implementation struct.
    struct BridgeBoom::BridgeBoomImpl
    {
        std::vector<RtlHookData> rtl_hook_data;
    };

    void BridgeBoom::Init(uint32_t ncores)
    {
        if (pimpl_ == nullptr)
        {
            pimpl_ = new BridgeBoomImpl();

            for (uint32_t i = 0; i < ncores; ++i)
            {
                state.m3cores.emplace_back(mem, i);
            }

            std::cout << "Goldmem Info: Goldmem initialized successfully." << std::endl;
            m3tracer.Init(kTracerBufferSize);
        }
    }

    void BridgeBoom::Close()
    {
        m3tracer.SaveTrace("m3trace.txt", m3::TraceFileFormat::kJson);
    }

    void BridgeBoom::RegisterEvent(const RtlHookData& data)
    {
        pimpl_->rtl_hook_data.push_back(data);
    }

    bool BridgeBoom::ServeRegisteredEvents()
    {
        int32_t execution_priority = kMaxPriority;
        while (execution_priority >= 0)
        {
            for (auto data : pimpl_->rtl_hook_data)
            {
                uint32_t priority = kRtlHookPriority[data.event];
                if (priority == execution_priority)
                {
                    bool is_success = commands::kRtlHookCommands[data.event](data, state);
                }
            }
            --execution_priority;
        }
        pimpl_->rtl_hook_data.clear();
        return true;
    }
}
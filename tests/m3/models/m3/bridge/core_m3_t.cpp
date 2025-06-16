/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "core_m3_t.h"

// C++ libraries.
#include <cassert>
#include <memory>
#include <unordered_map>
#include "fmt/core.h"

// Local libraries.
#include "Gold_mem.hpp"
#include "rvutils.h"
//#include "tracer.h"
#include "debug_utils.hpp"

// M3 commands.
#include "m3command.h"
#include "commit_memop.h"
#include "complete_store.h"
#include "create_memop.h"
#include "add_address.h"
#include "add_store_data.h"
#include "perform_load.h"
#include "update_cacheline_state.h"
#include "update_cacheline_data.h"
#include "send_dcache_amo.h"
#include "complete_amo.h"
#include "recovery_rob.h"

namespace m3
{
    // Commands in the model have priority. If two commands
    // were registered at the same clock cycle the commands
    // with higher priority need to be executed first.
    // This constant represents the highest command
    // priority in the model.
    static const uint32_t kMaxPriority = 2;

    // Tracer.
    /*static const uint32_t kTracerBufferSize = 1024;
    static Tracer m3tracer;*/
    
    State* state = nullptr;
    std::shared_ptr<m3::core_m3_t> m3_ptr = nullptr;
    Gold_mem mem;

    // Implementation struct.
    struct core_m3_t::core_m3_impl
    {
        std::vector<std::shared_ptr<IM3Command>> commands;
    };

    void core_m3_t::init(uint32_t ncores, debug::VerbosityLevel level, debug::ExecutionMode mode)
    {
        if (pimpl_ == nullptr)
        {
            // required variables for spike run method when using spike with m3 hooks
            // use default core
            core_id = 0;
            // only access to memory by default (no hooks, load elf)
            rob_id  = -1;
            pimpl_ = new core_m3_impl();
            state = new State();
            mem = Gold_mem([](uint64_t) { return 0; });

            for (uint32_t i = 0; i < ncores; ++i)
            {
                state->m3cores.emplace_back(mem, i);
            }

            // debug info set
            debug::Settings::getInstance().setVerbosity(level);
            debug::Settings::getInstance().setMode(mode);

            DEBUG_LOG(
                fmt::format("M3 initialized succesfully, ncores {}, verbosity level is {}, execution mode is {}",
                ncores, level, mode), debug::VerbosityLevel::None);
            //m3tracer.Init(kTracerBufferSize);
        }
    }

    void core_m3_t::close()
    {
        delete pimpl_;
        delete state;
        pimpl_ = nullptr;  // Optional, but good practice.
        state = nullptr;
        //m3tracer.SaveTrace("m3trace.txt", m3::TraceFileFormat::kJson);
    }

    void core_m3_t::register_event(const RTLEventData& data)
    {
        switch (data.event)
        {
        case RTLEvent::kCreateMemop:
            pimpl_->commands.push_back(std::make_shared<CreateMemop>(data));
            break;
        case RTLEvent::kAddMemopAddress:
            pimpl_->commands.push_back(std::make_shared<AddAddress>(data));
            break;
        case RTLEvent::kAddStoreData:
            pimpl_->commands.push_back(std::make_shared<AddStoreData>(data));
            break;
        case RTLEvent::kSendDcacheAmo:
            pimpl_->commands.push_back(std::make_shared<SendDcacheAmo>(data));
            break;
        case RTLEvent::kPerformLoad:
            pimpl_->commands.push_back(std::make_shared<PerformLoad>(data));
            break;
        case RTLEvent::kCompleteStore:
            pimpl_->commands.push_back(std::make_shared<CompleteStore>(data));
            break;
        case RTLEvent::kCompleteAmo:
            pimpl_->commands.push_back(std::make_shared<CompleteAmo>(data));
            break;
        case RTLEvent::kCommitMemop:
            pimpl_->commands.push_back(std::make_shared<CommitMemop>(data));
            break;
        case RTLEvent::kUpdateCacheLineState:
            pimpl_->commands.push_back(std::make_shared<UpdateCachelineState>(data));
            break;
        case RTLEvent::kUpdateCacheLineData:
            pimpl_->commands.push_back(std::make_shared<UpdateCachelineData>(data));
            break;
        case RTLEvent::KRecoveryRob:
            pimpl_->commands.push_back(std::make_shared<RecoveryRob>(data));
            break;
        default:
            DEBUG_ASSERT(false, "Trying to register non valid command");
            break;
        }
    }

    bool core_m3_t::serve_registered_events()
    {
        int32_t execution_priority = kMaxPriority;
        while (execution_priority >= 0)
        {
            for (auto command : pimpl_->commands)
            {
                if (command->GetPriority() == execution_priority)
                {
                    bool is_good = command->Execute(*state);
                    //bool is_good = command->Execute(state, m3tracer);
                    if (!is_good)
                    {
                        return false;
                    }
                }
            }
            --execution_priority;
        }
        pimpl_->commands.clear();
        return true;
    }
}
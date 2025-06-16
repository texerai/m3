/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "boom_m3_t.h"

// C++ libraries.
#include <cassert>
#include <memory>
#include <unordered_map>

// Local libraries.
#include "dromajo_t.h"
#include "Gold_mem.hpp"
#include "rvutils.h"
#include "state.h"
#include "tracer.h"

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

// m3 memory interface.
extern uint8_t dromajo_get_byte_direct(uint64_t paddr);
static Gold_mem mem(dromajo_get_byte_direct);

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

    // Implementation struct.
    struct boom_m3_t::boom_m3_impl
    {
        std::vector<std::shared_ptr<IM3Command>> commands;
        std::shared_ptr<dromajo_t> core_model_ptr;
    };

    void boom_m3_t::init(uint32_t ncores, std::shared_ptr<dromajo_t> core_model_ptr)
    {
        if (pimpl_ == nullptr)
        {
            pimpl_ = new boom_m3_impl();
            pimpl_->core_model_ptr = core_model_ptr;

            for (uint32_t i = 0; i < ncores; ++i)
            {
                state.m3cores.emplace_back(mem, i);
            }

            std::cout << "Goldmem Info: Goldmem initialized successfully." << std::endl;
            m3tracer.Init(kTracerBufferSize);
        }
    }

    void boom_m3_t::close()
    {
        m3tracer.SaveTrace("m3trace.txt", m3::TraceFileFormat::kJson);
    }

    void boom_m3_t::register_event(const RTLEventData& data)
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
        case RTLEvent::kPerformLoad:
            pimpl_->commands.push_back(std::make_shared<PerformLoad>(data));
            break;
        case RTLEvent::kCompleteStore:
            pimpl_->commands.push_back(std::make_shared<CompleteStore>(data));
            break;
        case RTLEvent::kCommitMemop:
            pimpl_->commands.push_back(std::make_shared<CommitMemop>(data, pimpl_->core_model_ptr));
            break;
        case RTLEvent::kUpdateCacheLineState:
            pimpl_->commands.push_back(std::make_shared<UpdateCachelineState>(data));
            break;
        case RTLEvent::kUpdateCacheLineData:
            pimpl_->commands.push_back(std::make_shared<UpdateCachelineData>(data));
            break;
        default:
            assert(false);
            break;
        }
    }

    bool boom_m3_t::serve_registered_events()
    {
        int32_t execution_priority = kMaxPriority;
        while (execution_priority >= 0)
        {
            for (auto command : pimpl_->commands)
            {
                if (command->GetPriority() == execution_priority)
                {
                    bool is_good = command->Execute(state, m3tracer);
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
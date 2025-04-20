/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "bridge_boom.h"

// C++ libraries.
#include <cassert>
#include <memory>
#include <unordered_map>
#include <functional>

// Local libraries.
#include "rtl_hook.h"
#include "state.h"
#include "m3/memory.hpp"
#include "utils/rvutils.h"
#include "utils/tracer.h"

// External function defined in the processor model
// to allow Memory class manipulate.
static Memory mem;

// External function defined in the processor model
// which allows updating register values.
// For exmple void dromajo_update_register(uint32_t hart_id,
// uint32_t destination_reg, uint64_t write_data, bool is_fp_register);
static std::function<void(uint32_t, uint32_t, uint64_t, bool)> UpdateProcessorRegister;

namespace m3
{
    // Debugging.
    constexpr bool kIsDebugOn = true;

    // Commands in the model have priority. If two commands
    // were registered at the same clock cycle the commands
    // with higher priority need to be executed first.
    // This constant represents the highest command
    // priority in the model.
    static const uint32_t kMaxPriority = 1;

    // Global state.
    static State state;

    // Define bridge functions under this namespace.
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
            memop_info.instruction = data.rv_instruction;
            memop_info.load_dest_reg = RVUtils::get_destination_from_load(data.rv_instruction);            

            // Double check the memop type.
            if (data.is_amo)
            {
                memop_info.memop_type = MemopType::kAmo;
            }
            else if (data.is_store)
            {
                memop_info.memop_type = MemopType::kStore;
            }
            else if (data.is_load)
            {
                memop_info.memop_type = MemopType::kLoad;
            }

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

        static bool CommitMemop(const RtlHookData& data, State& state)
        {
            bool should_abort = false;
            M3Cores& m3cores = state.m3cores;
            auto& in_core_memops = state.in_core_memops;

            // Get the M3 entry based on ROB ID.
            assert(in_core_memops[data.hart_id].count(data.rob_id) > 0);
            MemopInfo& memop_info = in_core_memops[data.hart_id][data.rob_id];
            assert(memop_info.memop_type != MemopType::kUndefined);
            Inst_id m3id = Inst_id(memop_info.m3id);

            // Set this safe. Point of no return: instruction commited.
            // Stores are set safe not at commit, but when they succeed
            // writing the cacheline. Memops besides stores can be
            // marked safe.
            memop_info.committed = true;
            if (memop_info.memop_type != MemopType::kStore)
            {
                m3cores[data.hart_id].set_safe(m3id);
            }

            // Load data check.
            if (memop_info.memop_type == MemopType::kLoad)
            {
                if (!memop_info.check_failed)
                {
                    bool is_fp = memop_info.load_dest_reg > 31;
                    uint32_t rd = memop_info.load_dest_reg % 32;
                    UpdateProcessorRegister(data.hart_id, rd, memop_info.load_rtl_data, is_fp);
                }
                else
                {
                    should_abort = true;
                    if constexpr (kIsDebugOn)
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
                auto &d_store = m3cores[data.hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                m3cores[data.hart_id].st_locally_perform(m3id);
                m3cores[data.hart_id].st_globally_perform(m3id);
            }

            return !should_abort;
        }

        static bool AddAddress(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Should be created by this time.
            assert(state.in_core_memops[data.hart_id].count(data.rob_id) > 0);
            MemopInfo& memop_info = state.in_core_memops[data.hart_id][data.rob_id];
            if (data.is_load)
            {
                assert(memop_info.memop_type == MemopType::kLoad);
            }
            else
            {
                assert(memop_info.memop_type == MemopType::kStore
                    || memop_info.memop_type == MemopType::kAmo);
            }

            // Calculate memop size.
            uint64_t byte_size = RVUtils::inst_size_to_byte_size(data.memop_size);

            // Get the reference.
            Inst_id m3id = Inst_id(memop_info.m3id);
            auto& d_load = m3cores[data.hart_id].ld_data_ref(m3id);
            auto& d_store = m3cores[data.hart_id].st_data_ref(m3id);

            memop_info.size = byte_size;
            memop_info.address = data.address;
            memop_info.is_address_valid = true;
            switch (memop_info.memop_type)
            {
                case MemopType::kLoad:
                    d_load.add_addr(data.address, byte_size);
                    break;
                case MemopType::kStore:
                    d_store.add_addr(data.address, byte_size);
                    // TODO: check if we ever use stq_id.
                    memop_info.stq_id = data.memop_id;
                    break;
                case MemopType::kAmo:
                    d_store.add_addr(data.address, byte_size);
                    d_load.add_addr(data.address, byte_size);
                    memop_info.stq_id = data.memop_id;
                    break;
                default:
                    assert(false);
                    break;
            }

            // Locally perform the store.
            // So that the loads could forward the values locally.
            if (memop_info.CanBePerformed())
            {
                auto &d_store = m3cores[data.hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                m3cores[data.hart_id].st_locally_perform(m3id);
            }

            return true;
        }

        static bool AddStoreData(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Get the M3 entry based on ROB ID.
            MemopInfo& memop_info = state.in_core_memops[data.hart_id][data.rob_id];
            Inst_id m3id = Inst_id(memop_info.m3id);

            // Skip if already set.
            if (!memop_info.is_data_valid)
            {
                // Save data.
                memop_info.store_data = data.store_data;
                memop_info.is_data_valid = true;

                // Locally perform the store.
                // So that the loads could forward the values locally.
                if (memop_info.CanBePerformed())
                {
                    auto &d_store = m3cores[data.hart_id].st_data_ref(m3id);
                    d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                    m3cores[data.hart_id].st_locally_perform(m3id);
                }
            }

            return true;
        }

        static bool PerformLoad(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Get memop info based on ROB ID.
            assert(state.in_core_memops[data.hart_id].count(data.rob_id) > 0);
            MemopInfo& memop_info = state.in_core_memops[data.hart_id][data.rob_id];
            Inst_id m3id = Inst_id(memop_info.m3id);

            // Perform the load.
            auto &d_load = m3cores[data.hart_id].ld_data_ref(m3id);
            m3cores[data.hart_id].ld_perform(m3id);

            uint64_t model_data = d_load.get_data(memop_info.address, memop_info.size);
            uint64_t rtl_data = data.load_rtl_data;

            memop_info.load_model_data = model_data;
            memop_info.load_rtl_data = rtl_data;
            int64_t model_data_signed = RVUtils::sign_extend(model_data, memop_info.size);
            int64_t loaded_data_signed = RVUtils::sign_extend(rtl_data, memop_info.size);

            // Check the load data.
            bool is_success = model_data_signed == loaded_data_signed;
            if (!is_success)
            {
                memop_info.check_failed = true;
                std::cout << "Error: Load Perform Failed (iid: ";
                std::cout << static_cast<uint32_t>(memop_info.m3id) << ")" << std::endl;
                std::cout << "  DUT: " << std::hex << rtl_data << ", Model: " << model_data;
                std::cout << ", Address: " << std::hex << memop_info.address << std::endl;
            }

            return is_success;
        }

        static bool UpdateCachelineData(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;
            Cache& cache = state.cache;
            auto& beyond_core_stores = state.beyond_core_stores;

            uint32_t cache_line_id = data.address >> 6;
            auto& cache_line = cache.banks[data.way_id][data.cache_line_id];
            if (cache_line.coh_state == kBoomMesiStateValue)
            {
                if (beyond_core_stores[data.hart_id].count(cache_line.tag) > 0)
                {
                    const auto& m3id = state.beyond_core_stores[data.hart_id][cache_line.tag].m3id;
                    assert(m3id > 0);
                    m3cores[data.hart_id].st_globally_perform(m3id);
                    beyond_core_stores[data.hart_id].erase(cache_line.tag);
                }
                else
                {
                    //assert(false);
                    std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
                }
            }

            return true;
        }

        bool UpdateCachelineState(const RtlHookData& data, State& state)
        {
            bool should_abort = false;

            M3Cores& m3cores = state.m3cores;
            Cache& cache = state.cache;
            auto& beyond_core_stores = state.beyond_core_stores;

            cache.UpdateMetaData(data.way_id, data.cache_line_id, data.coherence_state, data.tag);

            // What if gets invalidated?
            // What if it was a store miss and the data never got
            // written to the cache because it was invalidated by
            // coherence mechanisms?
            if (data.coherence_state == kBoomMesiStateValue)
            {
                if (beyond_core_stores[data.hart_id].count(data.tag) > 0)
                {
                    const auto& m3id = state.beyond_core_stores[data.hart_id][data.tag].m3id;
                    assert(m3id > 0);
                    m3cores[data.hart_id].st_globally_perform(m3id);
                    beyond_core_stores[data.hart_id].erase(data.tag);
                }
                else
                {
                    //assert(false);
                    //should_abort = true;
                    std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
                }
            }

            return !should_abort;
        }

        static std::map<RtlHook, std::function<bool(const RtlHookData&, State&)>> kRtlHookCommands = {
            { RtlHook::kCreateMemop,          CreateMemop },
            { RtlHook::kCompleteStore,        CompleteStore },
            { RtlHook::kCommitMemop,          CommitMemop },
            { RtlHook::kAddMemopAddress,      AddAddress },
            { RtlHook::kAddStoreData,         AddStoreData },
            { RtlHook::kPerformLoad,          PerformLoad },
            { RtlHook::kUpdateCacheLineData,  UpdateCachelineData },
            { RtlHook::kUpdateCacheLineState, UpdateCachelineState }
        };
    }

    // Implementation struct.
    struct BridgeBoom::BridgeBoomImpl
    {
        // Vector of events need to be processed at once.
        // This is to support triggered hooks at the same
        // clock cycle.
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
        }
    }

    void BridgeBoom::Close() { }

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

    void BridgeBoom::SetCallbackGetByte(std::function<uint8_t(uint64_t)> get_byte_function)
    {
        mem.init(get_byte_function);
    }

    void BridgeBoom::SetCallbackUpdateReg(std::function<void(uint32_t, uint32_t, uint64_t, bool)> update_proc_register)
    {
        UpdateProcessorRegister = update_proc_register;
    }
}
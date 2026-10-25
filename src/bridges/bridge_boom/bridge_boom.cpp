/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#include "bridge_boom.h"

// C++ libraries.
#include <algorithm>
#include <cassert>
#include <memory>
#include <functional>
#include <unordered_map>

// Local libraries.
#include "rtl_hook.h"
#include "state.h"
#include "m3/memory.hpp"
#include "utils/rvutils.h"
#include "utils/tracer.h"

// External function defined in the processor model
// to allow Memory class manipulate.
static m3::Memory mem;

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
    // This is the highest priority found in kRtlHookPriority.
    static const int32_t kMaxPriority = []() {
        uint32_t max_priority = 0;
        for (const auto& entry : kRtlHookPriority)
        {
            max_priority = std::max(max_priority, entry.second);
        }
        return static_cast<int32_t>(max_priority);
    }();

    // Global state.
    static State state;

    // M3_BRIDGE_DEBUG=1 narrates the store lifecycle.
    static const bool kBridgeDebug = std::getenv("M3_BRIDGE_DEBUG") != nullptr;
    static uint64_t debug_now = 0;  // timestamp of the event being served

    // Define bridge functions under this namespace.
    namespace commands
    {
        // Oldest tracked memop that carries the given branch bit, and the
        // same for a bit whose branch has just resolved (the mispredict
        // can arrive a cycle after the resolve).
        static bool OldestUnderBranch(const State& state, uint32_t hart_id, uint32_t mask, bool use_resolved, uint64_t& oldest)
        {
            bool found = false;
            auto it = state.branch_trackers.find(hart_id);
            if (it == state.branch_trackers.end())
            {
                return false;
            }
            const auto& records = use_resolved ? it->second.resolved_oldest : it->second.live_oldest;
            for (const auto& bit_and_id : records)
            {
                if ((mask & (1u << bit_and_id.first)) == 0)
                {
                    continue;
                }
                if (!found || bit_and_id.second < oldest)
                {
                    found = true;
                    oldest = bit_and_id.second;
                }
            }
            return found;
        }

        // Drops the memops the model removed from the bridge's own state.
        static void ForgetRemoved(State& state, uint32_t hart_id, const std::set<Inst_id>& removed_ids)
        {
            auto& core_memops = state.in_core_memops[hart_id];
            for (const auto& id : removed_ids) {
                for (auto it = core_memops.begin(); it != core_memops.end(); ++it) {
                    if (it->second.m3id == static_cast<uint64_t>(id))
                        it->second.Invalidate();
                }
            }

            // Records that pointed at removed memops are stale now.
            if (removed_ids.empty())
            {
                return;
            }
            uint64_t nuke_id = *removed_ids.begin();
            auto& tracker = state.branch_trackers[hart_id];
            for (auto* records : {&tracker.live_oldest, &tracker.resolved_oldest})
            {
                for (auto it = records->begin(); it != records->end();)
                {
                    it = it->second >= nuke_id ? records->erase(it) : std::next(it);
                }
            }
        }

        // The store has left the ROB: from now on it is tracked by its cache
        // line tag until the cache reports the line written, at which point
        // every store pending on that tag is globally performed in order.
        static void MoveStoreBeyondCore(uint32_t hart_id, MemopInfo& completing_memop, uint64_t timestamp, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Set safe this store.
            m3cores[hart_id].set_safe(completing_memop.m3id);
            completing_memop.store_succeeded = true;
            completing_memop.completed_time = timestamp;

            if (completing_memop.is_device)
            {
                // Never reaches the cache: visible once the device took it.
                m3cores[hart_id].st_globally_perform(Inst_id(completing_memop.m3id));
                return;
            }

            // Keyed by the full line address (tag and set), not just the
            // tag: lines of the same page are owned independently.
            uint64_t cache_line = completing_memop.address >> 6;
            state.beyond_core_stores[hart_id][cache_line].push_back(completing_memop);
        }

        // The cache line (address >> 6) is now owned by the hart: the stores
        // waiting on it are visible to everybody.
        static void GloballyPerformPendingStores(uint32_t hart_id, uint64_t cache_line, State& state)
        {
            auto& per_hart = state.beyond_core_stores[hart_id];
            auto it = per_hart.find(cache_line);
            if (it == per_hart.end())
            {
                //assert(false);
                std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
                return;
            }

            for (const MemopInfo& pending : it->second)
            {
                assert(pending.m3id > 0);
                if (kBridgeDebug)
                {
                    std::cout << "bridge: hart " << hart_id << " globally performs store iid " << pending.m3id
                              << " addr 0x" << std::hex << pending.address << std::dec << " data " << pending.store_data
                              << " (completed at " << pending.completed_time << ") at " << debug_now << std::endl;
                }
                state.m3cores[hart_id].st_globally_perform(Inst_id(pending.m3id));
            }
            per_hart.erase(it);
        }

        static bool CreateMemop(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Get the entry.
            MemopInfo& memop_info = state.in_core_memops[data.hart_id][data.rob_id];

            // Check if the previously allocated entry completed.
            if (!memop_info.committed && !memop_info.is_just_created)
            {
                std::cout << "ALERT: Previous entry did not complete." << std::endl;
            }

            // Committed stores have already been moved to committed_stores
            // (keyed by STQ entry), so reusing their ROB slot is fine here.

            // Drop the previous information.
            memop_info.Invalidate();

            // New entry in m3.
            memop_info.is_just_created = false;
            memop_info.m3id = m3cores[data.hart_id].inorder();
            memop_info.rob_id = data.rob_id;
            memop_info.instruction = data.rv_instruction;
            memop_info.load_dest_reg = RVUtils::get_destination_from_load(data.rv_instruction);

            // Remember the oldest memop speculated under each branch so a
            // mispredict knows where to start nuking.
            auto& tracker = state.branch_trackers[data.hart_id];
            for (uint32_t bit = 0; bit < 32; ++bit)
            {
                if ((data.branch_mask & (1u << bit)) && tracker.live_oldest.count(bit) == 0)
                {
                    tracker.live_oldest[bit] = memop_info.m3id;
                }
            }

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
            // Committed stores are keyed by STQ entry (memop_id). Events
            // recorded without it (older traces) fall back to the rob -> STQ
            // alias of the most recently committed store with that rob id.
            // Entries stay around after completion because the RTL keeps the
            // succeeded bit up for a while, so a still-pending match wins.
            auto& stores = state.committed_stores[data.hart_id];
            MemopInfo* pending = nullptr;
            bool seen_done = false;
            auto consider = [&](std::unordered_map<uint32_t, MemopInfo>::iterator it)
            {
                if (it == stores.end() || it->second.rob_id != data.rob_id) return;
                if (it->second.store_succeeded) seen_done = true;
                else if (pending == nullptr) pending = &it->second;
            };
            consider(stores.find(data.memop_id));
            auto& aliases = state.committed_rob_to_stq[data.hart_id];
            auto alias = aliases.find(data.rob_id);
            if (alias != aliases.end()) consider(stores.find(alias->second));

            if (pending == nullptr)
            {
                if (seen_done)
                {
                    return true;  // the succeeded bit stays up for a few cycles
                }
                // Not an RTL property: the bridge simply has nothing to
                // complete (e.g. a fence occupying an STQ entry).
                std::cout << "Warning: CompleteStore for hart " << data.hart_id << " rob " << data.rob_id
                          << " stq " << data.memop_id << " does not match a committed store." << std::endl;
                return true;
            }

            if (kBridgeDebug)
            {
                std::cout << "bridge: hart " << data.hart_id << " store iid " << pending->m3id << " addr 0x" << std::hex
                          << pending->address << std::dec << " data " << pending->store_data << " completed at "
                          << data.timestamp << std::endl;
            }
            MoveStoreBeyondCore(data.hart_id, *pending, data.timestamp, state);
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
            m3::Inst_id m3id = m3::Inst_id(memop_info.m3id);

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
            // guarantees that the data in the cache. A store-conditional
            // is an AMO whose store only happens when it succeeded; the
            // RTL's SC result (0 = success) rides in load_rtl_data.
            else if (memop_info.memop_type == MemopType::kAmo)
            {
                bool is_sc = (memop_info.instruction & 0x7f) == 0x2f && (memop_info.instruction >> 27) == 3;
                if (!(is_sc && data.load_rtl_data != 0))
                {
                    auto &d_store = m3cores[data.hart_id].st_data_ref(m3id);
                    d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                    m3cores[data.hart_id].st_locally_perform(m3id);
                    m3cores[data.hart_id].st_globally_perform(m3id);
                }
            }
            // A committed store leaves the ROB but stays in the STQ until
            // the cache takes it; its ROB slot may be reused meanwhile, so
            // track it by STQ entry from now on.
            else if (memop_info.memop_type == MemopType::kStore)
            {
                if (memop_info.is_address_valid)
                {
                    state.committed_stores[data.hart_id][memop_info.stq_id] = memop_info;
                    state.committed_rob_to_stq[data.hart_id][data.rob_id] = memop_info.stq_id;
                }
                in_core_memops[data.hart_id].erase(data.rob_id);
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
            m3::Inst_id m3id = m3::Inst_id(memop_info.m3id);
            auto& d_load = m3cores[data.hart_id].ld_data_ref(m3id);
            auto& d_store = m3cores[data.hart_id].st_data_ref(m3id);

            memop_info.size = byte_size;
            memop_info.address = data.address;
            memop_info.stq_id = data.memop_id;
            memop_info.is_address_valid = true;
            memop_info.is_device = IsDeviceAddress(data.address);
            if (memop_info.is_device)
            {
                d_load.set_device();
                d_store.set_device();
            }
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

            // Get the M3 entry based on ROB ID. The RTL keeps reporting the
            // data while the store sits in the STQ, also after it committed
            // and left the ROB; by then it is tracked by STQ entry instead.
            auto& per_hart = state.in_core_memops[data.hart_id];
            auto it = per_hart.find(data.rob_id);
            if (it == per_hart.end())
            {
                return true;
            }
            MemopInfo& memop_info = it->second;
            m3::Inst_id m3id = m3::Inst_id(memop_info.m3id);

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
            m3::Inst_id m3id = m3::Inst_id(memop_info.m3id);

            // Perform the load.
            auto &d_load = m3cores[data.hart_id].ld_data_ref(m3id);
            m3cores[data.hart_id].ld_perform(m3id);

            uint64_t model_data = d_load.get_data(memop_info.address, memop_info.size);
            uint64_t rtl_data = data.load_rtl_data;

            memop_info.load_model_data = model_data;
            memop_info.load_rtl_data = rtl_data;
            int64_t model_data_signed = RVUtils::sign_extend(model_data, memop_info.size);
            int64_t loaded_data_signed = RVUtils::sign_extend(rtl_data, memop_info.size);

            // Check the load data. Device reads cannot be predicted.
            bool is_success = memop_info.is_device || model_data_signed == loaded_data_signed;
            if (!is_success)
            {
                memop_info.check_failed = true;
                std::cout << "Error: Load Perform Failed (iid: ";
                std::cout << static_cast<uint32_t>(memop_info.m3id) << ", hart " << data.hart_id << ", rob " << data.rob_id
                          << ", timestamp " << data.timestamp << ")" << std::endl;
                std::cout << "  DUT: " << std::hex << rtl_data << ", Model: " << model_data;
                std::cout << ", Address: " << memop_info.address << std::dec << std::endl;
            }

            return is_success;
        }

        static bool FlushRob(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // A ROB flush discards the uncommitted instructions only.
            // Committed stores have left the ROB and keep draining to the
            // cache, so they must stay tracked until they globally perform.
            // Commit is in order, hence everything younger than the oldest
            // uncommitted memop is uncommitted too.
            bool has_uncommitted = false;
            uint64_t oldest_uncommitted = 0;
            for (const auto& entry : state.in_core_memops[data.hart_id])
            {
                const MemopInfo& memop_info = entry.second;
                if (memop_info.is_just_created || memop_info.committed)
                {
                    continue;
                }
                if (!has_uncommitted || memop_info.m3id < oldest_uncommitted)
                {
                    has_uncommitted = true;
                    oldest_uncommitted = memop_info.m3id;
                }
            }

            std::set<Inst_id> removed_ids;
            if (has_uncommitted)
            {
                m3cores[data.hart_id].nuke(Inst_id(oldest_uncommitted), removed_ids, false);
            }

            ForgetRemoved(state, data.hart_id, removed_ids);
            state.branch_trackers[data.hart_id] = BranchTracker();
            return true;
        }

        static bool BranchMispredict(const RtlHookData& data, State& state)
        {
            M3Cores& m3cores = state.m3cores;

            // Everything speculated under a mispredicted branch is squashed.
            // Younger memops all carry the bit, so nuking from the oldest
            // one removes the whole suffix.
            uint64_t oldest = 0;
            bool found = OldestUnderBranch(state, data.hart_id, data.branch_mask, false, oldest);
            if (!found)
            {
                found = OldestUnderBranch(state, data.hart_id, data.branch_mask, true, oldest);
            }

            if (found)
            {
                std::set<Inst_id> removed_ids;
                m3cores[data.hart_id].nuke(Inst_id(oldest), removed_ids, false);
                ForgetRemoved(state, data.hart_id, removed_ids);
            }

            return true;
        }

        static bool BranchResolve(const RtlHookData& data, State& state)
        {
            // The branch is done, but keep its record one step longer:
            // a mispredict for it may still be reported.
            auto& tracker = state.branch_trackers[data.hart_id];
            for (uint32_t bit = 0; bit < 32; ++bit)
            {
                if ((data.branch_mask & (1u << bit)) == 0)
                {
                    continue;
                }
                auto it = tracker.live_oldest.find(bit);
                if (it != tracker.live_oldest.end())
                {
                    tracker.resolved_oldest[bit] = it->second;
                    tracker.live_oldest.erase(it);
                }
                else
                {
                    tracker.resolved_oldest.erase(bit);
                }
            }
            return true;
        }

        static bool BranchPredictionStart(const RtlHookData&, State&)
        {
            // Nothing to do: memops record their branch mask on creation.
            return true;
        }

        static bool UpdateCachelineData(const RtlHookData& data, State& state)
        {
            // The data write carries the byte address within the way:
            // (index << 6) | offset for BOOM's 64-byte lines.
            uint32_t cache_line_id = data.address >> 6;
            const auto& cache_line = state.caches[data.hart_id].banks[data.way_id][cache_line_id];
            if (cache_line.coh_state == kBoomMesiStateValue)
            {
                GloballyPerformPendingStores(data.hart_id, (cache_line.tag << 6) | cache_line_id, state);
            }

            return true;
        }

        bool UpdateCachelineState(const RtlHookData& data, State& state)
        {
            state.caches[data.hart_id].UpdateMetaData(data.way_id, data.cache_line_id, data.coherence_state, data.tag);

            // What if gets invalidated?
            // What if it was a store miss and the data never got
            // written to the cache because it was invalidated by
            // coherence mechanisms?
            if (data.coherence_state == kBoomMesiStateValue)
            {
                GloballyPerformPendingStores(data.hart_id, (data.tag << 6) | data.cache_line_id, state);
            }

            return true;
        }

        static std::map<RtlHook, std::function<bool(const RtlHookData&, State&)>> kRtlHookCommands = {
            { RtlHook::kCreateMemop,            CreateMemop },
            { RtlHook::kCompleteStore,          CompleteStore },
            { RtlHook::kCommitMemop,            CommitMemop },
            { RtlHook::kAddMemopAddress,        AddAddress },
            { RtlHook::kAddStoreData,           AddStoreData },
            { RtlHook::kPerformLoad,            PerformLoad },
            { RtlHook::kFlushRob,               FlushRob },
            { RtlHook::kBranchMispredict,       BranchMispredict },
            { RtlHook::kBranchResolve,          BranchResolve },
            { RtlHook::kBranchPredictionStart,  BranchPredictionStart },
            { RtlHook::kUpdateCacheLineData,    UpdateCachelineData },
            { RtlHook::kUpdateCacheLineState,   UpdateCachelineState }
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
            pimpl_ = std::make_unique<BridgeBoomImpl>();

            for (uint32_t i = 0; i < ncores; ++i)
            {
                state.m3cores.emplace_back(mem, i);
            }

            std::cout << "Goldmem Info: Goldmem initialized successfully." << std::endl;
        }
    }

    void BridgeBoom::DumpStatistics() const
    {
        for (size_t hart_id = 0; hart_id < state.m3cores.size(); ++hart_id)
        {
            size_t pending_stores = 0;
            auto it = state.beyond_core_stores.find(hart_id);
            if (it != state.beyond_core_stores.end())
            {
                for (const auto& tag_and_stores : it->second)
                {
                    pending_stores += tag_and_stores.second.size();
                    for (const auto& pending : tag_and_stores.second)
                    {
                        std::cout << "  pending store m3 iid " << pending.m3id << " addr 0x" << std::hex << pending.address
                                  << std::dec << " size " << pending.size << " completed at " << pending.completed_time << std::endl;
                    }
                }
            }
            std::cout << "m3 stats: hart " << hart_id << " tracked memops " << state.m3cores[hart_id].size()
                      << ", stores waiting for the cache " << pending_stores << std::endl;
            state.m3cores[hart_id].dump_oldest(4);
        }
    }

    void BridgeBoom::Close()
    {
        pimpl_.reset();
    }

    BridgeBoom::BridgeBoom() = default;
    BridgeBoom::~BridgeBoom() = default;

    void BridgeBoom::RegisterEvent(const RtlHookData& data)
    {
        pimpl_->rtl_hook_data.push_back(data);
    }

    void BridgeBoom::AddDeviceRegion(uint64_t begin, uint64_t end)
    {
        g_extra_device_regions.push_back({begin, end});
    }

    bool BridgeBoom::ServeRegisteredEvents()
    {
        bool all_succeeded = true;
        int32_t execution_priority = kMaxPriority;
        while (execution_priority >= 0)
        {
            for (const auto& data : pimpl_->rtl_hook_data)
            {
                int32_t priority = kRtlHookPriority[data.event];
                if (priority == execution_priority)
                {
                    debug_now = data.timestamp;
                    // Keep serving the remaining events of this cycle so the
                    // state stays consistent, but report the failure.
                    all_succeeded &= commands::kRtlHookCommands[data.event](data, state);
                }
            }
            --execution_priority;
        }
        pimpl_->rtl_hook_data.clear();
        return all_succeeded;
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

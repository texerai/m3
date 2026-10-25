/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef STATE_H_
#define STATE_H_

// Local libraries.
#include "m3/memory_marionette.hpp"

// C++ libraries.
#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace m3
{
    using M3Cores = std::vector<MemoryMarionette>;

    const uint32_t kBoomMesiStateValue = 0x3;

    // Uncached device regions of the BOOM SoC (CLINT, PLIC, UART...).
    // Accesses there bypass the data cache: stores are globally performed
    // as soon as they complete, and load values are not checked because
    // the model has no knowledge of the device state.
    struct AddressRange
    {
        uint64_t begin;
        uint64_t end;  // exclusive
    };
    static const AddressRange kBoomDeviceRegions[] = {
        { 0x02000000, 0x02010000 },  // CLINT
        { 0x0c000000, 0x10000000 },  // PLIC
        { 0x54000000, 0x54001000 },  // UART
    };
    // Regions registered at run time by the simulation harness, e.g. the
    // HTIF tohost/fromhost words that the host writes behind the cores'
    // backs (see BridgeBoom::AddDeviceRegion).
    inline std::vector<AddressRange> g_extra_device_regions;

    inline bool IsDeviceAddress(uint64_t address)
    {
        for (const auto& range : kBoomDeviceRegions)
        {
            if (address >= range.begin && address < range.end)
            {
                return true;
            }
        }
        for (const auto& range : g_extra_device_regions)
        {
            if (address >= range.begin && address < range.end)
            {
                return true;
            }
        }
        return false;
    }

    // Structures to keep track of memory operation.
    enum class MemopFormat
    {
        kUndefined,
        kInteger,
        kFloat
    };

    enum class MemopType
    {
        kUndefined,
        kLoad,
        kStore,
        kAmo
    };

    struct MemopInfo
    {
        uint64_t m3id;
        uint64_t size = 0;
        uint64_t address = 0;
        uint64_t load_model_data = 0;
        uint64_t load_rtl_data = 0;
        uint32_t load_dest_reg = 0;
        uint64_t store_data = 0;
        uint32_t stq_id = 0;
        uint32_t rob_id = 0;
        uint32_t instruction = 0;
        uint64_t completed_time = 0;
        MemopType memop_type = MemopType::kUndefined;
        bool check_failed = false;
        bool is_performed = false;
        bool store_succeeded = false;
        bool committed = false;
        bool is_amo = false;
        bool is_address_valid = false;
        bool is_data_valid = false;
        bool is_just_created = true;
        bool is_device = false;

        bool CanBePerformed() const
        {
            return (is_address_valid && is_data_valid)
                && (memop_type == MemopType::kStore)
                && (memop_type != MemopType::kAmo);
        }

        void Invalidate()
        {
            is_amo = false;
            is_address_valid = false;
            is_data_valid = false;
            check_failed = false;
            is_performed = false;
            store_succeeded = false;
            committed = false;
            address = 0;
            is_just_created = true;
            is_device = false;
        }
    };

    // Expose minimum cache configuration to get information
    // about when data gets globally available.
    static const uint32_t kBoomCacheWays = 4;
    struct Cache
    {
        struct CacheLine
        {
            uint64_t tag = 0;
            uint32_t coh_state = 0;
        };
        using CacheWay = std::unordered_map<uint32_t, CacheLine>;
        CacheWay banks[kBoomCacheWays];

        // Record the cache activity.
        void UpdateMetaData(uint32_t way_id, uint32_t cache_line_id, uint32_t new_state, uint32_t new_tag)
        {
            assert(way_id < kBoomCacheWays);
            banks[way_id][cache_line_id].coh_state = new_state;
            banks[way_id][cache_line_id].tag = new_tag;
        }
    };

    // Per hart: for each branch-mask bit, the oldest tracked memop
    // speculated under that branch. "resolved" keeps the record of a
    // branch that has resolved until its bit is reused.
    struct BranchTracker
    {
        std::unordered_map<uint32_t, uint64_t> live_oldest;
        std::unordered_map<uint32_t, uint64_t> resolved_oldest;
    };

    // The structure keeps track of relevant information
    // required for verification of the memory model.
    struct State
    {
        // Branch speculation tracking per hart.
        std::unordered_map<uint32_t, BranchTracker> branch_trackers;

        // Track memory operation activities of each core.
        M3Cores m3cores;

        // Track caching activities.
        Cache cache;

        // Per hart: cache line address (address >> 6) -> committed stores
        // (oldest first) waiting for the cache to report the line written.
        std::unordered_map<uint32_t, std::unordered_map<uint64_t, std::vector<MemopInfo>>> beyond_core_stores;

        // Map rob_id to MemopInfo.
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, MemopInfo>> in_core_memops;

        // Committed stores waiting in the STQ, per hart keyed by STQ entry.
        // The ROB slot of a committed store is free and can be reused by a
        // younger instruction before the store leaves the STQ, so from
        // commit on a store is identified by its STQ entry instead.
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, MemopInfo>> committed_stores;

        // Per hart: rob_id of the last store committed from it -> STQ entry,
        // for completion events that carry no STQ entry (older traces).
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> committed_rob_to_stq;
    };
}

#endif

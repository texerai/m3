/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef STATE_H_
#define STATE_H_

// Local libraries.
#include "memory_marionette.hpp"

// C++ libraries.
#include <stdint.h>
#include <unordered_map>

namespace m3
{
    using M3Cores = std::vector<MemoryMarionette>;

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

    // The structure keeps track of relevant information
    // required for verification of the memory model.
    struct State
    {
        // Track memory operation activities of each core.
        M3Cores m3cores;

        // Track caching activities.
        Cache cache;

        // Map STQ to MemopInfo. STQ IDs are stored beyond commit.
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, MemopInfo>> beyond_core_stores;

        // Map rob_id to MemopInfo.
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, MemopInfo>> in_core_memops;
    };
}

#endif
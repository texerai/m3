/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef STATE_H_
#define STATE_H_

// Local libraries.
#include "memop_info.h"
#include "memory_marionette.hpp"

// C++ libraries.
#include <stdint.h>
#include <unordered_map>

namespace m3
{
    using M3Cores = std::vector<MemoryMarionette>;

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
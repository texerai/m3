// Copyright (c) 2023 Texer.ai. All rights reserved.
#include <svdpi.h>

// C++ libraries.
#include <cassert>
#include <unordered_map>
#include <vector>
#include "fmt/core.h"

// Local libraries.
#include "core_m3_t.h"
#include "rvutils.h"
#include "rtl_event.h"

// Constants.
static const uint32_t kStqWidth = 8; //[TODO] check for #store queues

static uint32_t one_hot_to_regular(uint32_t one_hot_num)
{
    uint32_t reg = 0;
    while (one_hot_num & 0b1 == 0)
    {
        one_hot_num >>= 1;
        reg++;
    }
    return reg;
}

extern "C" void create_memop_inorder_DPI
(
    int hart_id, // 1
    int rob_id, // 5
    int insn, // 32
    long long global_clock // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");


    bool uses_ldq      = RVUtils::is_load(static_cast<uint32_t>(insn));//(uses_q & 0b10) > 0;
    bool uses_stq      = RVUtils::is_store(static_cast<uint32_t>(insn)); //(uses_q & 0b01) > 0;
    bool is_amo        = RVUtils::is_amo(static_cast<uint32_t>(insn));

    m3::MemopType memop_type = m3::MemopType::kUndefined;
    m3::AmoType amo_type = m3::AmoType::kUndefined;
    if (uses_ldq)
    {
        memop_type = m3::MemopType::kLoad;
        DEBUG_ASSERT(!uses_stq, "Is load and store mem instruction");
        DEBUG_ASSERT(!is_amo, "Is load and amo mem instruction");

        // Get the destination register.
        /*load_dest_reg = RVUtils::get_destination_from_load(insn);
        assert(load_dest_reg > 0);*/
    }
    else if (uses_stq)
    {
        memop_type = m3::MemopType::kStore;
        DEBUG_ASSERT(!uses_ldq, "Is store and load mem instruction");
        DEBUG_ASSERT(!is_amo, "Is store and amo mem instruction");
        /*if (is_amo)
        {
            memop_type = m3::MemopType::kAmo;
        }*/
    }
    else if (is_amo) {
        memop_type = m3::MemopType::kAmo;
        DEBUG_ASSERT(!uses_stq, "Is amo and store mem instruction");
        DEBUG_ASSERT(!uses_ldq, "Is amo and load mem instruction");
        if (RVUtils::is_amo_add(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kAdd;
        if (RVUtils::is_amo_swap(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kSwap;
        if (RVUtils::is_amo_xor(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kXor;
        if (RVUtils::is_amo_and(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kAnd;
        if (RVUtils::is_amo_or(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kOr;
        if (RVUtils::is_amo_min(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kMin;
        if (RVUtils::is_amo_max(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kMax;
        if (RVUtils::is_amo_minu(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kMinu;
        if (RVUtils::is_amo_maxu(static_cast<uint32_t>(insn)))
            amo_type = m3::AmoType::kMaxu;

        DEBUG_ASSERT(amo_type != m3::AmoType::kUndefined, "Undefined amo type");
    }
    //assert(memop_type != m3::MemopType::kUndefined);

    // Pack data from the RTL and register the event.
    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kCreateMemop;
    ed.hart_id = static_cast<uint32_t>(hart_id);
    ed.rob_id = static_cast<uint32_t>(rob_id);
    ed.memop_type = memop_type;
    ed.amo_type   = amo_type;
    //ed.load_dest_reg = static_cast<uint32_t>(load_dest_reg);
    ed.timestamp = static_cast<uint64_t>(global_clock);
    m3::m3_ptr->register_event(ed);
}

extern "C" void add_memop_address_DPI
(
    int hart_id,             // 1
    long long memop_address, // 40
    int memop_size,          // 2
    int rob_idx,            // 5
    long long global_clock // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    // Pack data from the RTL and register the event.
    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kAddMemopAddress;
    ed.hart_id = static_cast<uint32_t>(hart_id);
    ed.memop_id = 0; //static_cast<uint32_t>(stq_idx); -> this stq_idx is never used
    ed.rob_id = static_cast<uint32_t>(rob_idx);
    ed.memop_size = static_cast<uint32_t>(memop_size);
    ed.address = static_cast<uint64_t>(memop_address);
    ed.timestamp = static_cast<uint64_t>(global_clock);
    m3::m3_ptr->register_event(ed);
}

extern "C" void i_perform_load_DPI
(
    int hart_id,            // 1
    long long load_data,    // 64
    int rob_idx,            // 5
    long long global_clock  // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    // Pack data from the RTL and register the event.
    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kPerformLoad;
    ed.hart_id = static_cast<uint32_t>(hart_id);
    ed.rob_id = static_cast<uint32_t>(rob_idx);
    ed.load_rtl_data = static_cast<uint64_t>(load_data);
    ed.data_format = m3::MemopFormat::kInteger;
    ed.timestamp = static_cast<uint64_t>(global_clock);
    m3::m3_ptr->register_event(ed);
}

extern "C" void add_store_data_DPI
(
    int hart_id, // 1
    long long stq_data, //64
    int rob_id, //5
    long long global_clock // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kAddStoreData;
    ed.hart_id = hart_id;
    ed.rob_id = rob_id;
    ed.store_data = stq_data;
    ed.timestamp = global_clock;
    m3::m3_ptr->register_event(ed);
}

extern "C" void send_dcache_amo_DPI
(
    int hart_id,
    int amo_rob_id,
    long long global_clock
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kSendDcacheAmo;
    ed.hart_id = hart_id;
    ed.amo_rob_id = amo_rob_id;
    ed.timestamp = global_clock;
    m3::m3_ptr->register_event(ed);
}

extern "C" void commit_memop_DPI
(
    int hart_id,
    int rob_id,
    int store_buffer_id,
    int xcpt, 
    long long global_clock
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kCommitMemop;
    ed.hart_id = hart_id;
    ed.rob_id = rob_id;
    ed.store_buffer_id = store_buffer_id;
    ed.xcpt = xcpt;
    ed.timestamp = global_clock;
    m3::m3_ptr->register_event(ed);
}


extern "C" void complete_store_DPI
(
    int hart_id,
    int store_buffer_id,
    long long global_clock // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kCompleteStore;
    ed.hart_id = static_cast<uint32_t>(hart_id);
    //ed.rob_id = static_cast<uint32_t>(rob_ids[stq_idx]); //[TODO] we now use the store_buffer_id to identify the completed store
    ed.store_buffer_id = store_buffer_id;
    ed.timestamp = static_cast<uint64_t>(global_clock);
    m3::m3_ptr->register_event(ed);
}

extern "C" void complete_amo_DPI
(
    int hart_id,
    int complete_rob_id,
    long long global_clock // 64
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kCompleteAmo;
    ed.hart_id = hart_id;
    ed.complete_amo_rob_id = complete_rob_id;
    ed.timestamp = global_clock;
    m3::m3_ptr->register_event(ed);
}


extern "C" void update_cache_meta_DPI
(
    int hart_id, // 1
    int way_id, // 4
    int cache_line_id, // 6
    int new_coh_state, // 2
    int tag, // 20
    long long global_counter
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    // Convert one-hot way id to regular number.
    way_id = one_hot_to_regular(way_id);

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kUpdateCacheLineState;
    ed.hart_id = hart_id;
    ed.cache_line_id = cache_line_id;
    ed.coherence_state = new_coh_state;
    ed.tag = tag;
    ed.way_id = way_id;
    ed.timestamp = global_counter;
    m3::m3_ptr->register_event(ed);
}


extern "C" void update_cache_data_DPI
(
    int hart_id, // 1
    int way_id, // 4
    int data_write_address, // 12
    long long global_counter
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    // Convert one-hot way id to regular number.
    way_id = one_hot_to_regular(way_id);

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::kUpdateCacheLineData;
    ed.hart_id = hart_id;
    ed.address = data_write_address;
    ed.way_id = way_id;
    ed.timestamp = global_counter;
    m3::m3_ptr->register_event(ed);
}

// This method fulshes entry 
extern "C" void rob_recovery_DPI
(
    int hart_id, // 1
    int rob_head, // 4
    int rob_id,
    long long global_counter
)
{
    DEBUG_ASSERT(m3::m3_ptr != nullptr,
        "m3 handler is null");

    m3::RTLEventData ed;
    ed.event = m3::RTLEvent::KRecoveryRob;
    ed.hart_id = hart_id;
    ed.rob_head = rob_head;
    ed.rob_id = rob_id;
    ed.timestamp = global_counter;
    m3::m3_ptr->register_event(ed);
}


extern "C" void run_event_DPI()
{
    m3::m3_ptr->serve_registered_events();
}
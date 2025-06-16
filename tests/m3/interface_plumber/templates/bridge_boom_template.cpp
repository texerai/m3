/*
 * Copyright (c) 2021 Micro Architecture Santa Cruz, UCSC. All rights reserved.
 */

// C++ libraries.
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

// Local libraries.
#include "Gold_core.hpp"
#include "Gold_notify.hpp"
#include "riscv_utils.h"

extern uint8_t dromajo_get_byte_direct(uint64_t paddr);
static Gold_mem               mem(dromajo_get_byte_direct);

static std::vector<Gold_core> cores;

enum class MemopType
{
    kLoad,
    kStore,
    kAmo,
    kUndefined
};

struct MemopInfo
{
    Inst_id goldmem_id;
    uint64_t size = 0;
    uint64_t address = 0;
    uint64_t performed_data = 0;
    uint64_t store_data = 0;
    uint32_t stq_idx = 0;
    MemopType memop_type = MemopType::kUndefined;
    bool check_failed = false;
    bool is_performed = false;
    bool store_succeeded = false;
    bool is_amo = false;
};

struct CacheLineMetadatum
{
    CacheLineMetadatum(uint64_t tag, uint64_t coherence_state) : tag(tag), coherence_state(coherence_state) {}
    uint64_t tag = 0;
    uint64_t coherence_state = 0;
};

static std::map<uint64_t, MemopInfo> boom_to_goldmem_id;
static std::map<uint64_t, std::map<uint64_t, std::vector<MemopInfo>>> beyond_core_stores;
static std::map<uint64_t, std::vector<MemopInfo>> atomic_ops;
static std::map<uint64_t, std::vector<bool>> was_store_commited;

// Cache line write tracking.
static std::map<uint64_t, std::map<uint64_t, std::map<uint64_t, uint64_t>>> cache_line_metadata_tag;
static std::map<uint64_t, std::map<uint64_t, std::map<uint64_t, uint64_t>>> cache_line_metadata_coh;
static std::map<uint64_t, std::map<uint64_t, std::map<uint64_t, uint64_t>>> cache_line_data_written;
//                 ^                  ^                ^
//                 |                  |                |
//              hart_id            cache idx          way

static const uint64_t kOffset = 10000;
static const uint32_t kMesiModifiedState = 0b11;
static const uint32_t kMesiInvalidState = 0b00;
static const uint32_t kStqEntries = 8;

// Simple hashing function to make sure different
// ids created for different core's ROB ids.
uint64_t simple_hash(uint64_t hartid, uint64_t rob_id)
{
    // Returned value shouldn't alias
    // as long as kRobEntries is larger than number of ROB entries
    // in BOOM.
    return (hartid * kOffset) + rob_id;
}

void boom_goldmem_init(uint32_t ncores) 
{
    for (uint32_t i = 0; i < ncores; ++i) 
    {
        cores.emplace_back(mem, i);

        for (uint32_t i = 0; i < kStqEntries; ++i)
        {
            was_store_commited[i].push_back(false);
        }

        atomic_ops[i];
    }

    std::cout << "Goldmem Info: Goldmem initialized successfully." << std::endl;
}

// GENERATED AND PATCHED FUNCTIONS.
bool SignalMemopCommitHookBoomBridge
(
    int hart_id,
    int is_load,
    int is_amo,
    int rob_id,
    uint64_t clock_cycle,
    uint64_t* performed_data
)
{
    uint64_t id = simple_hash(hart_id, rob_id);

    const auto& id_iterator = boom_to_goldmem_id.find(id);
    if (id_iterator != boom_to_goldmem_id.end())
    {
        auto& memop_info = boom_to_goldmem_id[id];
        auto& rid = memop_info.goldmem_id;

        // We are commiting instruction, this should be point
        // of no return.
        cores[hart_id].set_safe(rid);

        // Sanity checks.
        std::string memop{"store"};
        if (is_load)
        {
            memop = "load";
            assert(memop_info.memop_type == MemopType::kLoad);
        }
        else if (is_amo)
        {
            memop = "amo";
            assert(memop_info.memop_type == MemopType::kAmo);
        }

        // Trace.
        std::cout << "GOLDMEM_INFO:COMMIT_MEMOP:";
        std::cout << "core=" << std::dec << hart_id << ": ";
        std::cout << "memop=" << memop << ": ";
        std::cout << "is_amo=" << is_amo << ": ";
        std::cout << "boom_rob_id=" << std::hex << id << ":";
        std::cout << "goldmem_id=" << std::hex << rid << ":";
        std::cout << "address=0x" << std::hex << memop_info.address << ":";
        std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;

        // Commit the performed data if check passed.
        if (!memop_info.check_failed)
        {
            // Output performed data out of this function to Dromajo world, 
            // so we can write the register with this value.
            *performed_data = memop_info.performed_data;
        }
        else
        {
            std::cout << "Loaded value does not match!" << std::endl;
            return false;
        }

        // If AMO is commiting it should be globally visible.
        // Stores go beyond the ROB to the cache. Save them at commit
        // to globally perform when they are in the dcache.
        // AMOs must have stored the value already by the time they commit.
        if (memop_info.memop_type == MemopType::kStore && memop_info.memop_type != MemopType::kAmo)
        {
            beyond_core_stores[hart_id][memop_info.address].push_back(memop_info);
        }

        // Recycle this entry.
        boom_to_goldmem_id.erase(id);
    }
    else
    {
        assert(false);
    }

    return true;
}

void SignalMemopInsertHookBoomBridge
(
    int hart_id,
    int rob_id,
    int raw_instruction,
    int is_load,
    uint64_t clock_cycle
)
{
    uint64_t id = simple_hash(hart_id, rob_id);

    MemopInfo memop_info;
    memop_info.goldmem_id = cores[hart_id].inorder();
    Inst_id& rid = memop_info.goldmem_id;

    std::string memop{"store"};
    memop_info.memop_type = MemopType::kStore;
    if (is_load)
    {
        memop = "load";
        memop_info.memop_type = MemopType::kLoad;
    }
    
    // Save memop info.
    boom_to_goldmem_id[id] = memop_info;

    // Trace.
    std::cout << "GOLDMEM_INFO:INSERT_MEM_OP:";
    std::cout << "core=" << std::dec << hart_id << ":";
    std::cout << "memop=" << memop << ":";
    std::cout << "boom_rob_id=" << std::hex << id << ":";
    std::cout << "goldmem_id=" << std::hex << rid << ":";
    std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;
}

void SignalAddMemopAddressHookBoomBridge
(
    int hart_id,
    int rob_id,
    int is_load,
    uint64_t address,
    int memop_size,
    uint64_t clock_cycle
)
{
    // Calculate id.
    uint64_t id = simple_hash(hart_id, rob_id);

    // Get the goldmem rob id.
    MemopInfo& memop_info = boom_to_goldmem_id[id];
    Inst_id& rid = memop_info.goldmem_id;

    // Get the reference.
    auto& d_load = cores[hart_id].ld_data_ref(rid);
    auto& d_store = cores[hart_id].st_data_ref(rid);

    // Determine memop string name.
    std::string memop{"store"};
    if (is_load)
    {
        memop = "load";
    }

    // Calculate memop size.
    uint64_t sz = 0;
    switch (memop_size) {
        case 0: sz = 1; break;
        case 1: sz = 2; break;
        case 2: sz = 4; break;
        case 3: sz = 8; break;
        default: sz = 0;
    } 
    assert(sz > 0);

    // Add address and size.
    if (is_load) 
    {
        d_load.add_addr(address, sz);
    }
    else
    {
        d_store.add_addr(address, sz);
    }
    memop_info.size = sz;
    memop_info.address = address;

    // Trace.
    std::cout << "GOLDMEM_INFO:ADD_ADDRESS:";
    std::cout << "core=" << std::dec << hart_id << ":";
    std::cout << "memop=" << memop << ":";
    std::cout << "address=0x" << std::hex << address << ":";
    std::cout << "size=" << std::dec << sz << ":";
    std::cout << "boom_rob_id=" << std::hex << id << ":";
    std::cout << "goldmem_id=" << std::hex << rid << ":";
    std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;
}

void SignalPerformLoadHookBoomBridge
(
    int hart_id,
    int rob_id,
    uint64_t loaded_data,
    uint64_t clock_cycle
)
{
    uint64_t id = simple_hash(hart_id, rob_id);

    // Check if this has ever been allocated.
    const auto& memop_info_iterator = boom_to_goldmem_id.find(id);
    if (memop_info_iterator != boom_to_goldmem_id.end())
    {
        MemopInfo& memop_info = boom_to_goldmem_id[id];
        // Debug.
        if (memop_info.memop_type == MemopType::kAmo)
        {
            std::cout << "X";
        }

        Inst_id& rid = memop_info.goldmem_id;
        uint64_t size = memop_info.size;
        uint64_t address = memop_info.address;

        // Sanity checks.
        assert(memop_info.memop_type == MemopType::kLoad
          || memop_info.memop_type == MemopType::kAmo);

        // Perform the load.
        auto &d = cores[hart_id].ld_data_ref(rid);
        cores[hart_id].ld_perform(rid);
        
        // Check if the loaded data matches what's in memory model.
        uint64_t model_data = d.get_data(address, size);
        int64_t model_data_signed = RVUtils::sign_extend(model_data, size);
        int64_t loaded_data_signed = RVUtils::sign_extend(loaded_data, size);
        if (model_data_signed == loaded_data_signed)
        {
            memop_info.performed_data = loaded_data;
        }
        // Bootram address access. Just give what it needs.
        else if (address == 0x4000) {
            memop_info.performed_data = 0x80000000;
        }
        // The loads could be done from memory mapped devices. 
        // These are unpredictable allow mismatches.
        else if (address < 0x80000000) {
            std::cout << "PERFORM_LOAD_MMAP_DEV(0x" << std::hex << address << ");" << std::endl;
        } 
        else
        {
            memop_info.check_failed = true;
            std::cout << "PERFORM_LOAD_FAIL(DUT: " << loaded_data << ", MODEL: " << model_data << ");" << std::endl;
        }

        std::cout << "GOLDMEM_INFO:PERFORM_LOAD:";
        std::cout << "core=" << std::dec << hart_id << ":";
        std::cout << "address=0x" << std::hex << memop_info.address << ":";
        std::cout << "load_data=0x" << std::hex << loaded_data << ":";
        std::cout << "model_data=0x" << std::hex << model_data << ":";
        std::cout << "boom_rob_id=" << std::hex << id << ":";
        std::cout << "goldmem_id=" << std::hex << rid << ":";
        std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;
    }
}

void SignalStoreNukeBoomBridge
(
    int hart_id,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
)
{
    if (!was_store_commited[hart_id][stq_idx])
    {
 
        uint64_t id = simple_hash(hart_id, rob_id);

        // Check if this has ever been allocated?
        const auto& memop_info_iterator = boom_to_goldmem_id.find(id);
        if (memop_info_iterator != boom_to_goldmem_id.end())
        {
            MemopInfo& memop_info = boom_to_goldmem_id[id];
            cores[hart_id].nuke(memop_info.goldmem_id);
        }
    }
}

void SignalStoreCommitBoomBridge
(
    int hart_id,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
)
{
    if (clock_cycle == 117911)
    {
      std::cout << "check";
    }
    was_store_commited[hart_id][stq_idx] = true;

    uint64_t id = simple_hash(hart_id, rob_id);
    const auto& memop_info_iterator = boom_to_goldmem_id.find(id);
    if (memop_info_iterator != boom_to_goldmem_id.end())
    {
        memop_info_iterator->second.stq_idx = stq_idx;
    }
    else
    {
        assert(false);
    }
}

void SignalStoreLocalPeformBoomBridge
(
    int hart_id,
    int rob_id,
    int is_amo,
    int stq_idx,
    uint64_t local_store_address,
    uint64_t local_store_data,
    uint64_t clock_cycle
)
{
    uint64_t id = simple_hash(hart_id, rob_id);

    // Check if this has ever been allocated?
    const auto& memop_info_iterator = boom_to_goldmem_id.find(id);
    if (memop_info_iterator != boom_to_goldmem_id.end())
    {
        MemopInfo& memop_info = boom_to_goldmem_id[id];
        if (is_amo)
        {
            memop_info.memop_type = MemopType::kAmo;
            atomic_ops[hart_id].push_back(memop_info);
        }
        else if (!memop_info.is_performed)
        {
            Inst_id& rid = memop_info.goldmem_id;
            uint64_t size = memop_info.size;
            uint64_t address = memop_info.address;

            // Sanity checks.
            assert(memop_info.memop_type == MemopType::kStore);
            assert(local_store_address == address);

            // Reset the committed bit.
            was_store_commited[hart_id][stq_idx] = false;

            // Trace.
            std::cout << "GOLDMEM_INFO:PERFORM_STORE_LOCAL:";
            std::cout << "core=" << std::dec << hart_id << ":";
            std::cout << "store_address_dut=0x" << std::hex << local_store_address << ":";
            std::cout << "store_address_saved=0x" << std::hex << address << ":";
            std::cout << "store_data_dut=0x" << std::hex << local_store_data << ":";
            std::cout << "boom_rob_id=" << std::hex << id << ":";
            std::cout << "goldmem_id=" << std::hex << rid << ":";
            std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;

            // Set as performed, so we don't perform twice.
            memop_info.is_performed = true;
            memop_info.store_data = local_store_data;

            // Locally perform the store. So that the loads could forward the values locally.
            auto &d = cores[hart_id].st_data_ref(rid);
            d.set_data(address, size, local_store_data);
            cores[hart_id].st_locally_perform(rid);
        }
    }
}

void TraceGlobalStorePerform(int hart_id, uint64_t address, uint64_t store_data, const Inst_id& goldmem_id, uint64_t clock_cycle)
{
    // Trace.
    std::cout << "GOLDMEM_INFO:PERFORM_STORE_GLOBAL:";
    std::cout << "core=" << std::dec << hart_id << ":";
    std::cout << "store_address=0x" << std::hex << address << ":";
    std::cout << "store_data=0x" << std::hex << store_data << ":";
    std::cout << "goldmem_id=" << std::hex << goldmem_id << ":";
    std::cout << "clock_cycle=" << std::dec << clock_cycle << std::endl;
}

bool MergeAndGetLatestCompletedStore(int hart_id, std::vector<MemopInfo>& store_q, MemopInfo& result)
{
    if (store_q.size() > 1)
    {
        std::cout <<"U";
    }
    auto store_info_r_it = store_q.rbegin();
    bool is_found = false;
    while (!is_found && store_info_r_it != store_q.rend())
    {
        if (store_info_r_it->store_succeeded)
        {
            // Copy the result.
            is_found = true;
        }
        
        // Move reverser iterator backwards.
        store_info_r_it++;
    }

    if (is_found)
    {
        // Clean up. Merge and remove completed stores.
        auto prev_store_r_it = store_info_r_it;
        while (prev_store_r_it != store_q.rend())
        {
            auto latest_store_r_it = prev_store_r_it - 1;
            assert(prev_store_r_it->store_succeeded);
            assert(latest_store_r_it->store_succeeded);

            // Store merge internally removes the entry.
            cores[hart_id].st_locally_merged(prev_store_r_it->goldmem_id, latest_store_r_it->goldmem_id);
            store_q.erase(latest_store_r_it.base() - 1);
            prev_store_r_it++;
        }
        
        // Save the result.
        auto latest_store_r_it = prev_store_r_it - 1;
        result = *latest_store_r_it;
        store_q.erase(latest_store_r_it.base() - 1);
    }

    return is_found;
}

void SignalStoreGlobalPerformFromMetaWriteArbiterBoomBridge
(
    int hart_id,
    int address,
    int tag,
    int idx,
    int coherence_state,
    int meta_way_en,
    int data_way_en,
    uint64_t data,
    int meta_write_valid,
    int data_write_valid,
    uint64_t clock_cycle
)
{
    if (clock_cycle == 120323)
    {
        std::cout << "check";
    }
    if (data_write_valid)
    {
        uint32_t cache_offset = address & 0x3f;
        uint32_t cache_index = (address >> 6) & 0x3f;
        assert(cache_offset < 64);
        cache_line_data_written[hart_id][cache_index][data_way_en];
        cache_line_data_written[hart_id][cache_index][data_way_en] |= (1 << (cache_offset));
    }

    if (meta_write_valid)
    {
        // Change the cache line.
        cache_line_metadata_tag[hart_id][idx][meta_way_en] = tag;
        cache_line_metadata_coh[hart_id][idx][meta_way_en] = coherence_state;

        if (coherence_state == kMesiModifiedState)
        {
            // Form the address range from tag and idx.
            uint64_t range_lower = (static_cast<uint64_t>(tag) << 12) | (static_cast<uint64_t>(idx) << 6);
            uint64_t range_upper = range_lower | 0x3f;

            std::vector<uint64_t> addresses_to_remove;
            addresses_to_remove.clear();
            for (auto& store_info : beyond_core_stores[hart_id])
            {
                uint64_t store_address = store_info.first;
                uint32_t cache_offset = store_address & 0x3f;
                uint32_t cache_index = (store_address >> 6) & 0x3f;
                bool was_written = (cache_line_data_written[hart_id][cache_index][meta_way_en] & (1 << cache_offset)) > 0;
                if (range_lower <= store_address && store_address < range_upper && was_written)
                {
                    // Get the correct memop information struct.
                    MemopInfo memop_info;
                    bool is_found = MergeAndGetLatestCompletedStore(hart_id, store_info.second, memop_info);

                    if (is_found)
                    {
                        // Globally perform.
                        uint64_t store_data = cores[hart_id].st_data_ref(memop_info.goldmem_id).get_data(memop_info.address, memop_info.size);
                        cores[hart_id].st_globally_perform(memop_info.goldmem_id);
                        addresses_to_remove.push_back(store_address);

                        // Trace.
                        TraceGlobalStorePerform(hart_id, memop_info.address, store_data, memop_info.goldmem_id, clock_cycle);
                    }
                }
            }

            // If we are changing to modified state, we must perform something.
            // assert(addresses_to_remove.size() > 0); 

            // Recycle the store addresses.
            for (auto address : addresses_to_remove)
            {
                if (beyond_core_stores[hart_id][address].empty())
                {
                    beyond_core_stores[hart_id].erase(address);
                }
            }
        }
        else if (coherence_state == kMesiInvalidState)
        {
            // Think: if we are invalidating a cache line, data_written should be invalidated based on the meta_way_en
            cache_line_data_written[hart_id][idx][meta_way_en] = 0;
        }
    }

    // When data write is valid and meta write is not valid because the cacheline is in M state.
    // Or meta write is valid, but it is changing coh state of the other cache line.
    uint64_t data_idx = (address >> 6) & 0x3f;
    bool is_change_on_modified = !meta_write_valid || (meta_write_valid && (idx != data_idx));
    if (data_write_valid && is_change_on_modified)
    { 
        bool is_tag_present = false;
        uint64_t tag_val = 0;
        uint64_t coh_val = 0;
        auto idx_it = cache_line_metadata_tag[hart_id].find(data_idx);
        if (idx_it != cache_line_metadata_tag[hart_id].end())
        {
            auto way_it = cache_line_metadata_tag[hart_id][data_idx].find(data_way_en);
            if (way_it != cache_line_metadata_tag[hart_id][data_idx].end())
            {
                is_tag_present = true;
                tag_val = way_it->second;
                coh_val = cache_line_metadata_coh[hart_id][data_idx][data_way_en];
            }
        }

        if (is_tag_present)
        {
            // If we are modifying the cache line.
            if (coh_val == kMesiModifiedState)
            {
                // Form the address range from tag and idx.
                uint64_t full_address = (tag_val << 12) | address;
                auto store_info_it = beyond_core_stores[hart_id].find(full_address);
                if (store_info_it != beyond_core_stores[hart_id].end())
                {
                    MemopInfo memop_info;
                    bool is_found = MergeAndGetLatestCompletedStore(hart_id, store_info_it->second, memop_info);
                    if (is_found)
                    {
                        uint64_t store_data = cores[hart_id].st_data_ref(memop_info.goldmem_id).get_data(memop_info.address, memop_info.size);
                        cores[hart_id].st_globally_perform(memop_info.goldmem_id);
                        TraceGlobalStorePerform(hart_id, memop_info.address, store_data, memop_info.goldmem_id, clock_cycle);

                        if (store_info_it->second.empty())
                        {
                            beyond_core_stores[hart_id].erase(full_address);
                        }
                    }
                }
                // This may be an atomic, which doesn't go beyond the core.
                else if (!atomic_ops[hart_id].empty())
                {
                    auto& atomic_op = atomic_ops[hart_id][0];
                    uint32_t found_count = 0;
                    for (const auto& memop_info : atomic_ops[hart_id])
                    {
                        if (memop_info.address == full_address)
                        {
                            atomic_op = memop_info;
                            found_count++;
                        }
                    }
                    assert(found_count <= 1);
                    if (found_count > 0)
                    {
                        // Align data. Based on default BOOMs cacheline size: 64 bytes/cacheline.
                        uint32_t dword_offset = address & 0x7;
                        uint64_t mask = (static_cast<uint64_t>(1) << (atomic_op.size * 8)) - 1;
                        uint64_t data_to_write = (data >> (dword_offset * 8)) & mask;

                        // Make locally visible the value that atomic op calculated
                        // and is writing to the cache.
                        auto &d = cores[hart_id].st_data_ref(atomic_op.goldmem_id);
                        d.set_data(atomic_op.address, atomic_op.size, data_to_write);
                        cores[hart_id].st_locally_perform(atomic_op.goldmem_id);

                        // Make globally visible.
                        cores[hart_id].st_globally_perform(atomic_op.goldmem_id);
                        TraceGlobalStorePerform(hart_id, atomic_op.address, data_to_write, atomic_op.goldmem_id, clock_cycle);

                        // Once performed, good to remove.
                        auto& ops = atomic_ops[hart_id];
                        ops.erase(std::remove_if(ops.begin(), ops.end(),
                            [&](MemopInfo memop_info) {
                                return memop_info.address == full_address;
                            }), ops.end());
                    }
                }
            }
        }
    }
}

void SignalStoreCompletedBoomBridge
(
    int hart_id,
    uint64_t store_address,
    uint64_t store_data,
    int stq_idx,
    uint64_t clock_cycle
)
{
    const auto& stores_iter = beyond_core_stores[hart_id].find(store_address);
    if (stores_iter != beyond_core_stores[hart_id].end())
    {
        auto& store_q = beyond_core_stores[hart_id][store_address];

        // Merging makes sense only if we have many pending stores.
        auto store_info_it = store_q.begin();

        while (store_info_it != store_q.end())
        {
            if (store_info_it->stq_idx == stq_idx)
            {
                store_info_it->store_succeeded = true;
            } 

            // Move iter forward.
            store_info_it++;
        }
    }
    else
    {
        //assert(false);
    }
}

void SignalAddStoreAddressBoomBridge
(
    int hart_id,
    uint64_t store_address,
    int store_size,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
)
{
    int is_load = 0;
    SignalAddMemopAddressHookBoomBridge(hart_id, rob_id, is_load, store_address, store_size, clock_cycle);
}

void SignalAddLoadAddressBoomBridge
(
    int hart_id,
    uint64_t load_address,
    int load_size,
    int rob_id,
    int ldq_idx,
    uint64_t clock_cycle
)
{
    int is_load = 1;
    SignalAddMemopAddressHookBoomBridge(hart_id, rob_id, is_load, load_address, load_size, clock_cycle);
}

// GENERATED FUNCTIONS.



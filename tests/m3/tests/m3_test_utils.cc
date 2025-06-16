#ifndef M3_TEST_UTILS_CC
#define M3_TEST_UTILS_CC

#include <string>
#include "core_m3_t.h"
#include "m3_test_utils.h"
#include "rvutils.h"
#include "rvamo_utils.h"

using namespace m3;

namespace m3_test_utils {

    std::string MemopTypeToString(MemopType type) {
        static const std::unordered_map<MemopType, std::string> memop_map = {
            {MemopType::kLoad, "load"},
            {MemopType::kStore, "store"},
            {MemopType::kAmo, "amo"}
        };

        auto it = memop_map.find(type);
        if (it != memop_map.end()) {
            return it->second;
        }

        return "unknown"; // Fallback case
    }

    char* getMemoryByte(uint64_t memop_address) {
        constexpr uint64_t M3PGSHIFT = 12;
        constexpr uint64_t M3PGSIZE  = 1 << M3PGSHIFT;

        uint64_t ppn   = memop_address >> M3PGSHIFT;
        uint64_t pgoff = memop_address % M3PGSIZE;

        auto search = state->m3cores[0].mem.mem_byte.find(ppn);

        if (search == state->m3cores[0].mem.mem_byte.end()) {
            std::cout << "Address: 0x" << std::hex << memop_address << std::dec << " not found." << std::endl;
            return nullptr;
        }

        return (char*) search->second + pgoff;
    }

    void setup(uint32_t ncores, debug::VerbosityLevel level, debug::ExecutionMode mode) {
        if (!m3_ptr) {
            m3_ptr = std::make_shared<core_m3_t>();
        }
        m3_ptr->init(ncores, level, mode);
    }

    void create_memop_inorder_test(int hart_id, int rob_id, MemopType memop, long long global_clock, AmoType amotype) {
        M3Cores& m3cores = state->m3cores;

        bool uses_ldq      = (memop == MemopType::kLoad);//RVUtils::is_load(static_cast<uint32_t>(insn));//(uses_q & 0b10) > 0;
        bool uses_stq      = (memop == MemopType::kStore);//RVUtils::is_store(static_cast<uint32_t>(insn)); //(uses_q & 0b01) > 0;
        bool is_amo        = (memop == MemopType::kAmo);//RVUtils::is_amo(static_cast<uint32_t>(insn));

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
            amo_type = amotype; 

            DEBUG_ASSERT(amo_type != m3::AmoType::kUndefined, "Undefined amo type");
        }
        //assert(memop_type != MemopType::kUndefined);

        // Get the entry.
        MemopInfo& memop_info = state->in_core_memops[hart_id][rob_id];

        // Drop the previous information.
        memop_info.Invalidate();

        // New entry in m3.
        memop_info.m3id = m3cores[hart_id].inorder(rob_id);
        memop_info.memop_type = memop_type;
        memop_info.amo_type = amo_type;
        memop_info.rob_id = rob_id;

        //m3cores[hart_id].add_rob_entry(rob_id);

        DEBUG_LOG(fmt::format(
            "Create operation, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, memop_type: {}",
            hart_id, rob_id, memop_info.m3id, MemopTypeToString(memop_info.memop_type)
        ), debug::VerbosityLevel::Low);
    }

    void add_memop_address_test(int hart_id, long long memop_address, int memop_size, int rob_id, long long global_clock) {
        M3Cores& m3cores = state->m3cores;

        // Should be created by this time.
        DEBUG_ASSERT(state->in_core_memops[hart_id].count(rob_id) > 0,
            fmt::format("in_core_memops entry not found, hart_id: {}, m3 rob_id: {}",
            hart_id, rob_id));
        MemopInfo& memop_info = state->in_core_memops[hart_id][rob_id];

        // Calculate memop size.
        //uint64_t byte_size = RVUtils::inst_size_to_byte_size(memop_size);

        // Get the reference.
        Inst_id m3id = Inst_id(memop_info.m3id);
        auto& d_load = m3cores[hart_id].ld_data_ref(m3id);
        auto& d_store = m3cores[hart_id].st_data_ref(m3id);

        memop_info.size = memop_size; //byte_size;
        memop_info.address = memop_address;
        memop_info.is_address_valid = true;

        switch(memop_info.memop_type) //switch (memop_type)
        {
            case MemopType::kLoad:
                DEBUG_LOG("Add memop load ", debug::VerbosityLevel::Low);
                d_load.add_addr(memop_address, memop_size);
                break;
            case MemopType::kStore:
                DEBUG_LOG("Add memop store ", debug::VerbosityLevel::Low);
                d_store.add_addr(memop_address, memop_size);
                break;
            case MemopType::kAmo:
                DEBUG_LOG("Add memop amo ", debug::VerbosityLevel::Low);
                d_store.add_addr(memop_address, memop_size);
                d_load.add_addr(memop_address, memop_size);
                break;
            default:
                DEBUG_ASSERT(false, fmt::format(
                    "Not memory instruction type, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}",
                    hart_id, rob_id, memop_info.m3id
                ));
                break;
        }
        DEBUG_LOG(fmt::format(
            "hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, addr: 0x{:x}, size: {}",
            hart_id, rob_id, memop_info.m3id, memop_address, memop_size
        ), debug::VerbosityLevel::Low);

        // Locally perform the store.
        // So that the loads could forward the values locally.
        if (memop_info.CanBePerformed())
        {
            auto &d_store = m3cores[hart_id].st_data_ref(m3id);
            d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
            m3cores[hart_id].st_locally_perform(m3id);
            DEBUG_LOG("Perform local store", debug::VerbosityLevel::Low);
        }
    }

    uint64_t i_perform_load_test(int hart_id, long long load_data, int rob_id, long long global_clock) {
        M3Cores& m3cores = state->m3cores;

        // Get memop info based on ROB I
        DEBUG_ASSERT(state->in_core_memops[hart_id].count(rob_id) > 0,
            fmt::format("in_core_memops entry not found, hart_id: {}, m3 rob_id: {}",
            hart_id, rob_id));
        MemopInfo& memop_info = state->in_core_memops[hart_id][rob_id];
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Perform the loa
        auto &d_load = m3cores[hart_id].ld_data_ref(m3id);
        m3cores[hart_id].ld_perform(m3id);

        uint64_t model_data = d_load.get_data(memop_info.address, memop_info.size);

        DEBUG_LOG(fmt::format("Perform load, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, data: 0x{:x}, addr: 0x{:x}, size: {}",
            hart_id, rob_id, m3id, model_data, memop_info.address, memop_info.size), debug::VerbosityLevel::Low);
        
        return model_data;
    }

    void add_store_data_test(int hart_id, long long store_data, int rob_id, long long global_clock) {
        M3Cores& m3cores = state->m3cores;

        // Get the M3 entry based on ROB I
        MemopInfo& memop_info = state->in_core_memops[hart_id][rob_id];
        Inst_id m3id = Inst_id(memop_info.m3id);

        // Skip if already set.
        if (!memop_info.is_data_valid)
        {
            // Save data.
            memop_info.store_data = store_data;
            memop_info.is_data_valid = true;
            DEBUG_LOG(fmt::format(
                "Add store data, data: 0x{:x}, addr: 0x{:x}, size: {}, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}",
                memop_info.store_data, memop_info.address, memop_info.size, hart_id, rob_id, m3id
            ), debug::VerbosityLevel::Low);

            // Locally perform the store.
            // So that the loads could forward the values locally.
            if (memop_info.CanBePerformed())
            {
                auto &d_store = m3cores[hart_id].st_data_ref(m3id);
                d_store.set_data(memop_info.address, memop_info.size, memop_info.store_data);
                m3cores[hart_id].st_locally_perform(m3id);
                DEBUG_LOG("Add store data, performing local store", debug::VerbosityLevel::Low);
            }
        }
    }

    void send_dcache_amo_test(int hart_id, int rob_id, long long global_clock) {
        /*M3Cores& m3cores = state->m3cores;

        // Should be created by this time.
        assert(state->in_core_memops[hart_id].count(rob_id) > 0);
        MemopInfo& memop_info = state->in_core_memops[hart_id][rob_id];

        assert(memop_info.memop_type == MemopType::kAmo);

        // Get the reference.
        Inst_id m3id = Inst_id(memop_info.m3id);
        auto& d_load = m3cores[hart_id].ld_data_ref(m3id);
        auto& d_store = m3cores[hart_id].st_data_ref(m3id);*/

        
    }

    void commit_memop_test(int hart_id, int rob_id, int store_buffer_id, long long global_clock, int xcpt) {
        bool should_abort = false;
        M3Cores& m3cores = state->m3cores;
        auto& in_core_memops = state->in_core_memops;

        // Get the M3 entry based on ROB I
        DEBUG_ASSERT(in_core_memops[hart_id].count(rob_id) > 0,
            "no incore_memops entry found");
        MemopInfo& memop_info = in_core_memops[hart_id][rob_id];
        //memop_info.store_buffer_id = store_buffer_id; //[TODO] is this correct? is this a pointer that updates the in_core_memop from state?
        //assert(memop_info.memop_type != MemopType::kUndefined);
        Inst_id m3id = Inst_id(memop_info.m3id);

        DEBUG_LOG(fmt::format(
            "Commit mem operation, hart_id: {}, rtl rob_id: {}, m3 rob_id: {}, xcpt: {}",
            hart_id, rob_id, m3id, xcpt
        ), debug::VerbosityLevel::Low);

        // exception case, keep entry but don't commit
        // No extra checking is necesary cause:
        // 1- in next cycle in spike, this entry will be flushed
        // 2- when there's an exception, it is the only instruction in the commit window
        if (xcpt) {
            return;
        }

        // Set this safe. Point of no return: instruction commite
        // Stores are set safe not at commit, but when they succeed
        // writing the cacheline. Memops besides stores can be
        // marked safe.
        memop_info.committed = true;
        if (memop_info.memop_type != MemopType::kStore)
        {
            m3cores[hart_id].set_safe(m3id);
            DEBUG_LOG(", set_safe ", debug::VerbosityLevel::Low);
        }

        // Load data check.
        // AMOs should get visible at commit. Commit of AMO
        // guarantees that the data in the cache.
        if (memop_info.memop_type == MemopType::kAmo)
        {
            // Perform amo operation locally
            // Sizes for load and store operation are the same
            Gold_data temp_data;
            // get gold_data
            temp_data.add_addr(memop_info.address, memop_info.size);
            temp_data.set_data(memop_info.address, memop_info.size, memop_info.store_data);
            auto &d_load = m3cores[hart_id].ld_data_ref(m3id);
            // obtain gol_data value
            uint64_t load_data = d_load.get_data(memop_info.address, memop_info.size);
            uint64_t store_data = temp_data.get_data(memop_info.address, memop_info.size);
            // calculate amo store value            
            DEBUG_ASSERT(memop_info.amo_type != AmoType::kUndefined,
            "Undefined amo type");
            DEBUG_ASSERT((memop_info.size == 4) || (memop_info.size == 8),
            "Wrong amo access size");

            DEBUG_LOG(fmt::format(
                "perform amo: type: {} size: {} load_data: 0x{:x} store_data: 0x{:x}",
                static_cast<int>(memop_info.amo_type), memop_info.size, load_data, store_data
            ), debug::VerbosityLevel::Low);

            uint64_t amo_store_data = RiscV_AMO::execute(memop_info.amo_type, memop_info.size, load_data, store_data);
            // Store amo operation result
            auto &d_store = m3cores[hart_id].st_data_ref(m3id);
            d_store.set_data(memop_info.address, memop_info.size, amo_store_data);
            m3cores[hart_id].st_locally_perform(m3id);
            m3cores[hart_id].st_globally_perform(m3id);
            // Display debug information
            uint64_t cache_line_tag = memop_info.address >> 6;

            DEBUG_LOG(fmt::format(
                ", perform amo op, tag: 0x{:x}, addr: 0x{:x} data: 0x{:x} size: {}",
                cache_line_tag, memop_info.address, amo_store_data, memop_info.size
            ), debug::VerbosityLevel::Low);
        }
        else if (memop_info.memop_type == MemopType::kStore)
        {
            //assert(memop_info.memop_type == MemopType::kStore);
            auto& beyond_core_stores = state->beyond_core_stores;

            // Set safe this store.
            m3cores[hart_id].set_safe(m3id);
            DEBUG_LOG(", set_safe", debug::VerbosityLevel::Low);

            // The store can be globally performed soon.
            // The stores that go to the same cache line can be merge
            uint64_t cache_line_tag = memop_info.address >> 6;
            bool is_first = beyond_core_stores[hart_id].find(cache_line_tag) == beyond_core_stores[hart_id].end();
            if (!is_first)
            {
                // Merge what was there previously with the new coming store.
                /*MemopInfo& prev_memop = beyond_core_stores[hart_id][cache_line_tag];
                m3cores[hart_id].st_locally_merged(prev_memop.m3id, m3id);
                // Update m3id so that the datas used to perform the store is the young one
                // st_locally_merged updates the young entry too, so that the load gets the most updated value
                prev_memop.m3id = m3id;*/
                beyond_core_stores[hart_id][cache_line_tag].push_back(memop_info.m3id);
                DEBUG_LOG(", Merge store buffer", debug::VerbosityLevel::Low);
            }
            else
            {
                memop_info.completed_time = global_clock;
                beyond_core_stores[hart_id][cache_line_tag].push_back(memop_info.m3id);
                DEBUG_LOG(", Set new value store buffer", debug::VerbosityLevel::Low);
            }
        }

    }

    void complete_store_test(int hart_id, int store_buffer_id, long long global_clock) {
        /*M3Cores& m3cores = state->m3cores;
        auto& beyond_core_stores = state->beyond_core_stores;
        MemopInfo* completing_memop;

        // Get the entry.
        for( int i = 0; i < (state->in_core_memops[hart_id]).size(); i++ ){ //[TODO]check if this works
            MemopInfo& aux = state->in_core_memops[hart_id][i];
            if( aux.store_buffer_id == store_buffer_id ){
                *completing_memop = aux;
                break;
            }
        }

        assert(completing_memop->memop_type == MemopType::kStore);
        std::cout << "Complete store, hart_id: " <<  hart_id << ", st_buffer_id:" << store_buffer_id;

        // Set safe this store.
        m3cores[hart_id].set_safe(completing_memop->m3id);
        std::cout << ", set_safe ";

        // The store can be globally performed soon.
        // The stores that go to the same cache line can be merge
        uint64_t cache_line_tag = completing_memop->address >> 6;
        bool is_first = beyond_core_stores[hart_id].find(cache_line_tag) == beyond_core_stores[hart_id].end();
        if (!is_first)
        {
            // Merge what was there previously with the new coming store.
            MemopInfo& prev_memop = beyond_core_stores[hart_id][cache_line_tag];
            m3cores[hart_id].st_locally_merged(prev_memop.m3id, completing_memop->m3id);
            prev_memop.m3id = completing_memop->m3id;
            std::cout << ", Merge store buffer";
        }
        else
        {
            completing_memop->completed_time = global_clock;
            beyond_core_stores[hart_id][cache_line_tag] = *completing_memop;
            std::cout << ", Set new value store buffer";
        }
        std::cout << std::endl;*/
    }

    void complete_amo_test(int hart_id, int rob_id, long long global_clock) {
        /*M3Cores& m3cores = state->m3cores;
        auto& beyond_core_stores = state->beyond_core_stores;
        MemopInfo* completing_memop;
        
        // Get the entry.
        for( int i = 0; i < (state->in_core_memops[hart_id]).size(); i++ ){ //[TODO]check if this works and if this amo is in_core_memops
            MemopInfo& aux = state->in_core_memops[hart_id][i];
            if( aux.rob_id == rob_id ){ //should this be aux.rob_id or aux.amo_rob_id?
                *completing_memop = aux;
                break;
            }
        }

        assert(completing_memop->memop_type == MemopType::kAmo);

        // Set safe this store.
        m3cores[hart_id].set_safe(completing_memop->m3id);

        //[TODO] Check: doing the same as for CompleteStore
        // The store can be globally performed soon.
        // The stores that go to the same cache line can be merge
        uint64_t cache_line_tag = completing_memop->address >> 6;
        bool is_first = beyond_core_stores[hart_id].find(cache_line_tag) == beyond_core_stores[hart_id].end();
        if (!is_first)
        {
            // Merge what was there previously with the new coming store.
            MemopInfo& prev_memop = beyond_core_stores[hart_id][cache_line_tag];
            m3cores[hart_id].st_locally_merged(prev_memop.m3id, completing_memop->m3id);
            prev_memop.m3id = completing_memop->m3id;
        }
        else
        {
            completing_memop->completed_time = global_clock;
            beyond_core_stores[hart_id][cache_line_tag] = *completing_memop;
        }*/
    }

    void update_cache_meta_test(int hart_id, int way_id, int cache_line_id, int new_coherence_state, int tag, long long global_counter) {
        M3Cores& m3cores = state->m3cores;
        Cache& cache = state->cache;
        auto& beyond_core_stores = state->beyond_core_stores;

        cache.UpdateMetaData(way_id, cache_line_id, new_coherence_state, tag);
        DEBUG_LOG(fmt::format(
            "Update cacheline state, state: {}, tag: 0x{:x}, count: {}",
            static_cast<int>(new_coherence_state),
            tag,
            beyond_core_stores[hart_id].count(tag)
        ), debug::VerbosityLevel::Low);

        // What if gets invalidated?
        // What if it was a store miss and the data never got
        // written to the cache because it was invalidated by
        // coherence mechanisms?
        if (new_coherence_state == kMesiMState)
        {
            if (beyond_core_stores[hart_id].count(tag) > 0)
            {
                //const auto& m3id = beyond_core_stores[hart_id][tag].m3id;
                //assert(m3id > 0);
                m3cores[hart_id].st_globally_perform(beyond_core_stores[hart_id][tag]);
                beyond_core_stores[hart_id].erase(tag);
                //was_globally_performed_ = true;
                //performed_id_ = static_cast<uint64_t>(m3id);
                /*std::cout << "Update cacheline from store buffer, hart_id: " <<
                hart_id << ", m3 rob_id: " << m3id << ", address tag: " << tag << std::endl;*/
            }
            else
            {
                DEBUG_LOG(
                    "Warning: changing cache line state, but M3 never seen it",
                    debug::VerbosityLevel::Warning);
            }
        }
    }

    void update_cache_data_test(int hart_id, int way_id, int address, long long global_counter) {
        /*M3Cores& m3cores = state->m3cores;
        Cache& cache = state->cache;
        auto& beyond_core_stores = state->beyond_core_stores;

        uint32_t cache_line_id = address >> 6;
        auto& cache_line = cache.banks[way_id][cache_line_id];
        if (cache_line.coh_state == kMesiMState)
        {
            if (beyond_core_stores[hart_id].count(cache_line.tag) > 0)
            {
                const auto& m3id = state->beyond_core_stores[hart_id][cache_line.tag].m3id;
                assert(m3id > 0);
                m3cores[hart_id].st_globally_perform(m3id);
                beyond_core_stores[hart_id].erase(cache_line.tag);
            }
            else
            {
                //assert(false);
                std::cout << "Warning: changing cache line state, but M3 never seen it" << std::endl;
            }
        }*/
    }

    int rob_recovery_test(int hart_id, int rob_head, int rob_id, long long global_counter) {
        return state->m3cores[hart_id].flush_entry(rob_id, rob_head);
    }

    void no_ins_store_spike(int hart_id, int rob_id, long long memop_address, long long stq_data, int len) {
        uint64_t PGSHIFT = 12;
        uint64_t PGSIZE  = 1 << PGSHIFT;
        uint64_t ppn = memop_address >> PGSHIFT, pgoff = memop_address % PGSIZE;
        char* r_addr;

        // set spike run variables for m3 hooks
        m3_ptr->core_id = m3_ptr->core_id;
        m3_ptr->rob_id  = rob_id;
        
        if (state->m3cores.empty()) {
            throw std::runtime_error("ERROR: state->m3cores is empty");
        }
        auto search = state->m3cores[m3_ptr->core_id].mem.mem_byte.find(ppn);
        if (search == state->m3cores[m3_ptr->core_id].mem.mem_byte.end()) {
            auto res = (uint8_t*)calloc(PGSIZE, 1);
            if (res == nullptr)
            throw std::bad_alloc();
            state->m3cores[m3_ptr->core_id].mem.mem_byte[ppn] = res;
            r_addr = (char*) res + pgoff;
        } else {
            r_addr = (char*) search->second + pgoff;
        }

        memcpy(r_addr, &stq_data, len);
    }

    uint64_t no_ins_load_spike(int hart_id, int rob_id, long long memop_address, int len) {
        uint64_t PGSHIFT = 12;
        uint64_t PGSIZE  = 1 << PGSHIFT;
        uint64_t result;
        char *buffer;
        assert(len<=8);

        // set spike run variables for m3 hooks
        m3_ptr->core_id = hart_id;
        m3_ptr->rob_id  = rob_id;

        uint64_t ppn = memop_address >> PGSHIFT, pgoff = memop_address % PGSIZE;
        //TODO: I think this is not necessary and we could access directly the gold_data structure in the rob buffer.
        Gold_data ld_data;
        ld_data = state->m3cores[m3_ptr->core_id].ld_perform_no_update(m3_ptr->rob_id, memop_address, len);
        uint64_t data = ld_data.get_data(memop_address, len);  // Get the 64-bit value
        // This solution is not robust for multithreading, as this memory will be rehused for each call.
        // That can be achieved using a vector so that memory is automatically managed without generating memory leaks.
        
        DEBUG_LOG(
            fmt::format("ppn: {} core_id: {} rob_id: {} addr: {} len: {}", ppn, m3_ptr->core_id, m3_ptr->rob_id, memop_address, len),
            debug::VerbosityLevel::Medium);

        return data;
        /*if (buffer == nullptr) {
            buffer = new char[sizeof(uint64_t)];
        }
        std::memcpy(buffer, &data, len);
        std::memcpy(&result, buffer, len);
        return result;*/
    }

};

#endif

#ifndef M3_TEST_UTILS_CC
#define M3_TEST_UTILS_CC

#include <string>
#include <cassert>

// Old dependency headers replaced by modern ones
#include "m3_test_utils.h"
#include "utils/rvutils.h"

// -----------------------------------------------------------------------------
// Stubs for legacy DEBUG macros (now no-ops) ----------------------------------
// -----------------------------------------------------------------------------
#define DEBUG_LOG(msg, lvl)   do { (void)(msg); } while (0)
#define DEBUG_ASSERT(cond, msg) assert(cond)

// -----------------------------------------------------------------------------
// Global singletons shared by all tests ---------------------------------------
// -----------------------------------------------------------------------------
namespace m3 {
    Memory       global_mem([](uint64_t) { return 0; });
    State*       state      = nullptr;
}

using namespace m3;
using namespace m3_test_utils;

// -----------------------------------------------------------------------------
// Legacy helpers rewritten on top of modern MemoryMarionette -------------------
// -----------------------------------------------------------------------------

namespace m3_test_utils {

//---------------------------------------------------------------------------
void setup(uint32_t ncores,
           debug::VerbosityLevel /*level*/, debug::ExecutionMode /*mode*/) {
    if (m3::state) return;  // already initialised

    m3::state = new State();
    for (uint32_t i = 0; i < ncores; ++i) {
        m3::state->m3cores.emplace_back(global_mem, static_cast<int>(i));
    }
}

//---------------------------------------------------------------------------
void create_memop_inorder_test(int hart_id, int rob_id, MemopType memop,
                               long long /*global_clock*/, AmoType /*amotype*/) {
    M3Cores &cores = state->m3cores;

    // Get / create entry in in-core table
    MemopInfo &info = state->in_core_memops[hart_id][rob_id];
    info.Invalidate();

    info.m3id        = cores[hart_id].inorder();
    info.memop_type  = memop;
    info.rob_id      = rob_id;
}

//---------------------------------------------------------------------------
void add_memop_address_test(int hart_id, long long addr, int sz, int rob_id,
                            long long /*global_clock*/) {
    M3Cores &cores = state->m3cores;

    DEBUG_ASSERT(state->in_core_memops[hart_id].count(rob_id) > 0,
                 "add_memop_address_test: entry missing");
    auto &info = state->in_core_memops[hart_id][rob_id];

    Inst_id iid = Inst_id(info.m3id);
    auto &d_ld  = cores[hart_id].ld_data_ref(iid);
    auto &d_st  = cores[hart_id].st_data_ref(iid);

    info.size             = sz;
    info.address          = addr;
    info.is_address_valid = true;

    switch (info.memop_type) {
        case MemopType::kLoad:  d_ld.add_addr(addr, sz);                 break;
        case MemopType::kStore: d_st.add_addr(addr, sz);                 break;
        case MemopType::kAmo:
            d_st.add_addr(addr, sz);
            d_ld.add_addr(addr, sz);
            break;
        default: DEBUG_ASSERT(false, "Unknown memop type");
    }

    if (info.CanBePerformed()) {
        d_st.set_data(info.address, info.size, info.store_data);
        cores[hart_id].st_locally_perform(iid);
    }
}

//---------------------------------------------------------------------------
uint64_t i_perform_load_test(int hart_id, long long /*load_data*/, int rob_id,
                             long long /*global_clock*/) {
    M3Cores &cores = state->m3cores;
    auto &info     = state->in_core_memops[hart_id][rob_id];
    Inst_id iid    = Inst_id(info.m3id);

    auto &d = cores[hart_id].ld_data_ref(iid);
    cores[hart_id].ld_perform(iid);
    return d.get_data(info.address, info.size);
}

//---------------------------------------------------------------------------
void add_store_data_test(int hart_id, long long data, int rob_id,
                         long long /*global_clock*/) {
    M3Cores &cores = state->m3cores;
    auto &info     = state->in_core_memops[hart_id][rob_id];
    Inst_id iid    = Inst_id(info.m3id);

    if (!info.is_data_valid) {
        info.store_data   = data;
        info.is_data_valid = true;

        if (info.CanBePerformed()) {
            auto &d_st = cores[hart_id].st_data_ref(iid);
            d_st.set_data(info.address, info.size, info.store_data);
            cores[hart_id].st_locally_perform(iid);
        }
    }
}

//---------------------------------------------------------------------------
void send_dcache_amo_test(int /*hart_id*/, int /*amo_rob_id*/, long long /*gc*/) {}

//---------------------------------------------------------------------------
void commit_memop_test(int hart_id, int rob_id, int /*store_buffer_id*/,
                       long long /*global_clock*/, int /*xcpt*/) {
    M3Cores &cores = state->m3cores;
    auto &info     = state->in_core_memops[hart_id][rob_id];
    Inst_id iid    = Inst_id(info.m3id);

    info.committed = true;
    if (info.memop_type != MemopType::kStore)
        cores[hart_id].set_safe(iid);

    if (info.memop_type == MemopType::kStore) {
        cores[hart_id].set_safe(iid);
        cores[hart_id].st_globally_perform(iid);
    }
}

//---------------------------------------------------------------------------
void complete_store_test(int, int, long long) {}
void complete_amo_test(int, int, long long)  {}

//---------------------------------------------------------------------------
void update_cache_meta_test(int hart_id, int way_id, int line_id,
                            int new_state, int tag,
                            long long /*gc*/) {
    auto &cache = state->cache;
    cache.UpdateMetaData(static_cast<uint32_t>(way_id),
                         static_cast<uint32_t>(line_id),
                         static_cast<uint32_t>(new_state),
                         static_cast<uint32_t>(tag));

    if (new_state == kMesiMState) {
        // In the simplified model the store is globally performed immediately
        // at commit, so nothing else to do here.
    }
}

void update_cache_data_test(int, int, int, long long) {}

//---------------------------------------------------------------------------
int rob_recovery_test(int /*hart_id*/, int /*rob_head*/, int rob_id,
                      long long /*gc*/) {
    // Simplified: return the recovered ROB entry id directly.
    return rob_id;
}

//---------------------------------------------------------------------------
void no_ins_store_spike(int /*hart_id*/, int /*rob_id*/, long long addr,
                        long long data, int len) {
    Data d;
    d.set_addr(static_cast<uint64_t>(addr), static_cast<uint8_t>(len));
    d.set_data(static_cast<uint64_t>(addr), static_cast<uint8_t>(len),
               static_cast<uint64_t>(data));
    global_mem.st_perform(d);
}

uint64_t no_ins_load_spike(int /*hart_id*/, int /*rob_id*/, long long addr,
                           int len) {
    Data d;
    d.set_addr(static_cast<uint64_t>(addr), static_cast<uint8_t>(len));
    global_mem.ld_perform(d);
    return d.get_data(static_cast<uint64_t>(addr), static_cast<uint8_t>(len));
}

} // namespace m3_test_utils

#endif // M3_TEST_UTILS_CC

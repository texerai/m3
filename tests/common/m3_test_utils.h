#ifndef M3_TEST_UTILS_H
#define M3_TEST_UTILS_H

// -----------------------------------------------------------------------------
// Compatibility layer for the new (2025) M3 implementation
// -----------------------------------------------------------------------------

#include <cstdint>

// Stub out the old 'debug' helpers that the legacy tests expect.
namespace debug {
    enum class VerbosityLevel { None, Error, Warning, Low, Medium, Debug };
    enum class ExecutionMode  { Testing, Simulation };
}

// Bring in the modern M3 headers.
#include "../src/bridges/bridge_boom/state.h"
#include "m3/memory_marionette.hpp"
#include "m3/memory.hpp"

// The old tests expect an AmoType enum that is no longer part of the public API.
namespace m3 {
    enum class AmoType {
        kUndefined,
        kAdd,
        kSwap,
        kXor,
        kAnd,
        kOr,
        kMin,
        kMax,
        kMinu,
        kMaxu
    };

    // Expose a global pointer to the state so that the legacy helpers can use
    // it exactly the same way they did before.
    extern State* state;
}

using namespace m3;

#define imm_4_0_mk  0b000000011111
#define imm_11_5_mk 0b111111100000

namespace m3_test_utils {
    static const uint32_t kMesiMState = 3;

    // Legacy helper prototypes ------------------------------------------------
    void setup(uint32_t ncores,
               debug::VerbosityLevel level,
               debug::ExecutionMode mode);

    // Test-personalised calls --------------------------------------------------
    void create_memop_inorder_test(int hart_id, int rob_id, MemopType memop,
                                   long long global_clock,
                                   m3::AmoType amotype = m3::AmoType::kUndefined);
    void add_memop_address_test(int hart_id, long long memop_address,
                                int memop_size, int rob_idx,
                                long long global_clock);
    uint64_t i_perform_load_test(int hart_id, long long load_data,
                                 int rob_idx, long long global_clock);
    void add_store_data_test(int hart_id, long long stq_data, int rob_id,
                             long long global_clock);
    void send_dcache_amo_test(int hart_id, int amo_rob_id,
                              long long global_clock);
    void commit_memop_test(int hart_id, int rob_id, int store_buffer_id,
                           long long global_clock, int xcpt = 0);
    void complete_store_test(int hart_id, int store_buffer_id,
                             long long global_clock);
    void complete_amo_test(int hart_id, int complete_rob_id,
                           long long global_clock);
    void update_cache_meta_test(int hart_id, int way_id, int cache_line_id,
                                int new_coh_state, int tag,
                                long long global_counter);
    void update_cache_data_test(int hart_id, int way_id,
                                int data_write_address,
                                long long global_counter);
    int rob_recovery_test(int hart_id, int rob_head, int rob_id,
                          long long global_counter);

    // Spike-style helpers ------------------------------------------------------
    void no_ins_store_spike(int hart_id, int rob_id, long long memop_address,
                            long long stq_data, int len);
    uint64_t no_ins_load_spike(int hart_id, int rob_id,
                               long long memop_address, int len);
};

#endif


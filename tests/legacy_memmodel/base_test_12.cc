#include "test_framework.h"
#include <cassert>
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_12() {
    /*ncores = 2;
    setup(ncores, debug::VerbosityLevel::Medium, debug::ExecutionMode::Testing);

    MemopType memop;
    int insn;
    int hart_id;
    int rob_id;
    int memop_size;
    long long memop_address;
    long long global_clock;
    long long load_data;
    long long stq_data;
    int store_buffer_id;

    int way_id;
    int cache_line_id;
    int new_coh_state;
    int tag;

    global_clock = 0;

    /////////////////////////////////////////////////////////////////////////////////////
    // CASE 1
    // store addr 0 (no commit)
    // store addr 0,1 (no commit)
    // load (right value)
    // spike load (wrong value)
    // commit
    // spike load (right value)
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x01;

    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 1;
    memop_size = 2;
    memop_address = 0x0; //32
    stq_data = 0x0202;

    way_id = 0;
    cache_line_id = 0;
    new_coh_state = 3; //M state in mesi protocol
    tag = memop_address >> 6;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    memop = MemopType::kLoad;
    rob_id = 2;
    memop_size = 2;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    uint64_t data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x0202) {
        throw std::runtime_error("failed");
    }
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // The load is not performed properlly because it is expected that the previous instructions are flushed or commited
    if(data != 0x0000) {
        throw std::runtime_error("failed");
    }

    commit_memop_test(hart_id, 2, store_buffer_id, global_clock+3);

    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // After the commits, the load is performed properlly
    if(data != 0x0202) {
        throw std::runtime_error("failed");
    }

    m3::m3_ptr->close();*/
}

// Register the test
REGISTER_TEST("To update", base_test_12);
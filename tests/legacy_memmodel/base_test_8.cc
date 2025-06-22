#include "test_framework.h"
#include <cassert>
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_8() {
    int ncores;
    MemopType memop;
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

    ncores = 2;
    setup(ncores, debug::VerbosityLevel::Medium, debug::ExecutionMode::Testing);

    //read without ins addr 0
    memop_address = 0;
    hart_id = 0;
    rob_id = 0;
    memop_size = 2;
    stq_data = 365;
    uint64_t data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    if(data != 0) {
        throw std::runtime_error("failed");
    }
    //write without ins addr 0, data 1
    stq_data      = 1;
    no_ins_store_spike(hart_id, rob_id, memop_address, stq_data, memop_size);
    //read without ins addr 0
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    if(data != 1) {
        throw std::runtime_error("failed");
    }

    //write ins addr 20
    //sb addr 20, data=0x32
    //sb x2, 20(x0)
    //x2 = 0x32
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 20; //32
    global_clock = 0;
    load_data = 0;
    stq_data = 0x32; //50
    store_buffer_id = 0;

    way_id = 0;
    cache_line_id = 0;
    new_coh_state = 3; //M state in mesi protocol
    tag = memop_address >> 6;

    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    //complete_store_test(hart_id, store_buffer_id, global_clock+4);
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);
    //read without ins addr 20
    data = no_ins_load_spike(hart_id, -1, memop_address, memop_size);
    if(data != 0x32) {
        throw std::runtime_error("failed");
    }


    //write without ins addr 20
    memop_address = 20; //32
    stq_data = 0x25;
    no_ins_store_spike(hart_id, -1, memop_address, stq_data, memop_size);
    //read ins addr 20
    //lb x5, 20(x0) 
    //20(x0) = 0x25
    memop = MemopType::kLoad;
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 20; //32
    global_clock = 1;
    load_data = 0;

    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x25) {
        throw std::runtime_error("failed");
    }
    commit_memop_test(hart_id, rob_id, 0, global_clock+9);
}

// Register the test
REGISTER_TEST("Load from reorder buffer in different harts and simulate loads and stores inside spike", base_test_8);
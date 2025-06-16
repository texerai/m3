#include "test_framework.h"
#include <cassert>
#include "core_m3_t.h"
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_9() {
    int ncores;
    ncores = 2;
    setup(ncores, debug::VerbosityLevel::Medium, debug::ExecutionMode::Testing);

    MemopType memop;
    int hart_id;
    int rob_id, rob_id_2;
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

    //sb x2, 65(x0)
    //x2 = 50
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    rob_id_2 = 1;
    memop_size = 1;
    memop_address = 0x20; //32
    global_clock = 0;
    load_data = 0;
    stq_data = 0x32; //50
    store_buffer_id = 0;

    way_id = 0;
    cache_line_id = 0;
    new_coh_state = 3; //M state in mesi protocol
    tag = memop_address >> 6;

    for (int i=0; i<10; ++i) {
        create_memop_inorder_test(hart_id, i, memop, global_clock);
        add_memop_address_test(hart_id, memop_address, memop_size, i, global_clock+1);
        add_store_data_test(hart_id, stq_data, i, global_clock+2);
    }

    for (int i=0; i<4; ++i) {
        commit_memop_test(hart_id, i, store_buffer_id, global_clock+3);
    }

    for (int i=0; i<4; ++i) {
        create_memop_inorder_test(hart_id, i, memop, global_clock);
    }

    int num = rob_recovery_test(hart_id, 4, 7, global_clock); //head: 4, recover: 7
    assert(num == 7);
    //Checking
    /*for (int i=0; i<10; ++i) {
        if (((i<=3) && (i>=0)) || ((i<=9) && (i>=7))) //0-3, 7-9
        create_memop_inorder_test(hart_id, i, memop, global_clock);
    }*/

    m3::m3_ptr->close();
}

// Register the test
REGISTER_TEST("Check recovery mechanism", base_test_9);
#include "test_framework.h"
#include <cassert>
#include "core_m3_t.h"
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_13() {
    int ncores;
    ncores = 2;
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

    uint64_t data;

    global_clock = 0;

    store_buffer_id = 0;

    /////////////////////////////////////////////////////////////////////////////////////
    // CASE 1
    // hart 0

    //a 0-store addr 0, data = 01 (no commit)
    //b 1-load addr 0, (no perform)
    //d 3-store addr 0, data = 02 (no commit)
    //e 4-load addr 0, (no perform)

    //g 1-perform load, data = 01
    //h 4-perform load, data = 02
    //i 4-commit
    //j 1-spike perform load, data = 01
    //k 4-spike perform load, data = 02

    ///////////////////////////////////////////

    //a 0-store addr 0, data = 01 (no commit)
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x01;

    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //b 1-load addr 0, (no commit)
    memop = MemopType::kLoad;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);

    //d 3-store addr 0, data = 02 (no commit)
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 3;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x02;

    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //e 4-load addr 0, (no commit)
    memop = MemopType::kLoad;
    rob_id = 4;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);

    //g 1-perform load, data = 01
    rob_id = 1;
    memop_address = 0x0;
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    if(data != 0x01) {
        throw std::runtime_error("failed");
    }

    //h 4-perform load, data = 02
    rob_id = 4;
    memop_address = 0x0;
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    if(data != 0x02) {
        throw std::runtime_error("failed");
    }

    //i 4-commit
    hart_id = 0;
    rob_id = 4;
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);

    //j 1-perform load, data = 01
    hart_id = 0;
    memop_size = 1;
    memop_address = 0x0;
    rob_id = 1;
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // After the commits, the load is performed properlly
    if(data != 0x01) {
        throw std::runtime_error("failed");
    }

    //k 4-perform load, data = 02
    hart_id = 0;
    memop_size = 1;
    memop_address = 0x0;
    rob_id = 4;
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // After the commits, the load is performed properlly
    if(data != 0x02) {
        throw std::runtime_error("failed");
    }

    m3::m3_ptr->close();
}

// Register the test
REGISTER_TEST("Disordered lds and sts to the same address and multiple commits same cycle", base_test_13);
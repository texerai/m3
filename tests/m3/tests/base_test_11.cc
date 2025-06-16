#include "test_framework.h"
#include <cassert>
#include "core_m3_t.h"
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_11() {
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

    /////////////////////////////////////////////////////////////////////////////////////
    //CASE 1

    //sb x2, 0(x0)
    //x2 = 0x1111
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 0x0; //32
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
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);

    //lb x1, 0(x0)
    memop = MemopType::kLoad;
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    uint64_t data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x32) {
        throw std::runtime_error("failed");
    }
    //commit_memop_test(hart_id, rob_id, 0, global_clock+9);

    int num = rob_recovery_test(hart_id, 0, 1, global_clock);
    assert(num == 1);

    //sb x2, 0(x0)
    //x2= 0x44
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x44;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);

    //lh x1, 0(x0)
    memop = MemopType::kLoad;
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x44) {
        throw std::runtime_error("failed");
    }
    commit_memop_test(hart_id, rob_id, 0, global_clock+9);


    /////////////////////////////////////////////////////////////////////////////////////
    //CASE 2

    //sb x2, 0(x0)
    //x2 = 0x1111
    memop = MemopType::kUndefined;
    hart_id = 0;
    rob_id = 0;
    memop_size = 1;
    memop_address = 0x0; //32
    global_clock = 0;
    load_data = 0;
    stq_data = 0x32; //50
    store_buffer_id = 0;

    way_id = 0;
    cache_line_id = 0;
    new_coh_state = 3; //M state in mesi protocol
    tag = memop_address >> 6;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);

    //lb x1, 0(x0)
    memop = MemopType::kUndefined;
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);

    num = rob_recovery_test(hart_id, 0, 1, global_clock); //head: 4, recover: 7
    assert(num == 1);

    //sb x2, 0(x0)
    //x2= 0x44
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x44;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);

    //lh x1, 0(x0)
    memop = MemopType::kLoad;
    hart_id = 0;
    rob_id = 2;
    memop_size = 1;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x44) {
        throw std::runtime_error("failed");
    }
    commit_memop_test(hart_id, rob_id, 0, global_clock+9);

    m3::m3_ptr->close();
}

// Register the test
REGISTER_TEST("Rehuse rbid test", base_test_11);
#include "test_framework.h"
#include <cassert>
#include "core_m3_t.h"
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_10() {
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

    //case 1

    //sh x2, 0(x0)
    //x2 = 0x1111
    memop = MemopType::kStore;
    hart_id = 0;
    rob_id = 0;
    memop_size = 2;
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

    //sb x2, 0(x0)
    //x2= 0x44
    memop = MemopType::kStore;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x44;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);

    //sb x2, 1(x0)
    //x2= 0x66
    memop = MemopType::kStore;
    rob_id = 2;
    memop_size = 1;
    memop_address = 0x1; //32
    stq_data = 0x66;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    //complete_store_test(hart_id, store_buffer_id, global_clock+4);
    update_cache_meta_test(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);

    //lh x1, 0(x0)
    memop = MemopType::kLoad;
    rob_id = 3;
    memop_size = 2;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    uint64_t data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x6644) {
        throw std::runtime_error("failed");
    }
    commit_memop_test(hart_id, rob_id, 0, global_clock+9);

    //case 2
    //addr 0 1 2 3 4 5 6 7
    //sh       x x
    //sh     x x
    //sh         x x
    //sb   x
    //sb             x
    //ld   x x x x x x x x

    //***WITH COMMITS

    //notice that data is inverted with respect to this graph in a double

    std::cout << "###############################################" << std::endl;
    std::cout << "TEST 10 CASE 2" << std::endl;

    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 0;
    memop_size = 2;
    memop_address = 0x2; //32
    stq_data = 0x0101;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);
    
    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 1;
    memop_size = 2;
    memop_address = 0x01; //32
    stq_data = 0x0202;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3); 

    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 2;
    memop_size = 2;
    memop_address = 0x3; //32
    stq_data = 0x0303;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);

    //sb addr 0
    memop = MemopType::kStore;
    rob_id = 3;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x04;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);

    //sb addr 5
    memop = MemopType::kStore;
    rob_id = 4;
    memop_size = 1;
    memop_address = 0x5; //32
    stq_data = 0x05;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);


    //ld x1, 0(x0)
    memop = MemopType::kLoad;
    rob_id = 5;
    memop_size = 8;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x50303020204) {
        throw std::runtime_error("failed test 10 :177");
    }
    commit_memop_test(hart_id, rob_id, 0, global_clock+9);

    //Check spike load
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    std::cout << "#############################################################" << std::endl;
    std::cout << "test_10, data: " << std::hex << data << std::dec << std::endl;
    // After the commits, the load is performed properlly
    if(data != 0x50303020204) {
        throw std::runtime_error("failed test 10 :185");
    }

    //std::cout << "#############################################################" << std::endl;
    //std::cout << "test_10, data: " << std::hex << data << std::dec << std::endl;

    memop = MemopType::kStore;
    rob_id = 6;
    memop_size = 2;
    memop_address = 0x2; //32
    stq_data = 0x0101;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //Check spike load after no commited store, that store shoudn't modify what this load sees
    rob_id = 5;
    memop_size = 8;
    memop_address = 0x0; //32
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // After the commits, the load is performed properlly
    if(data != 0x50303020204) {
        throw std::runtime_error("failed test 10 :207");
    }

    //ld x1, 0(x0)
    memop = MemopType::kLoad;
    rob_id = 7;
    memop_size = 8;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    // m3 load will see the modified data
    if(data != 0x50301010204) {
        throw std::runtime_error("failed");
    }

    commit_memop_test(hart_id, 7, 0, global_clock+9);

    //Check spike load
    // spike will see now the updated data, after the commit
    rob_id = 7;
    data = no_ins_load_spike(hart_id, rob_id, memop_address, memop_size);
    // After the commits, the load is performed properlly
    if(data != 0x50301010204) {
        throw std::runtime_error("failed");
    }

    //std::cout << "#############################################################" << std::endl;
    //std::cout << "test_10, data: " << std::hex << data << std::dec << std::endl;


    //case 3
    //addr 0 1 2 3 4 5 6 7
    //sh       x x
    //sh     x x
    //sh         x x
    //sb   x
    //sb             x
    //ld   x x x x x x x x

    //***WITH NO COMMITS

    //notice that data is inverted with respect to this graph in a double

    std::cout << "###############################################" << std::endl;
    std::cout << "TEST 10 CASE 3" << std::endl;

    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 0;
    memop_size = 2;
    memop_address = 0x2; //32
    stq_data = 0x0101;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    
    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 1;
    memop_size = 2;
    memop_address = 0x01; //32
    stq_data = 0x0202;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //sh addr 2,3
    memop = MemopType::kStore;
    rob_id = 2;
    memop_size = 2;
    memop_address = 0x3; //32
    stq_data = 0x0303;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //sb addr 0
    memop = MemopType::kStore;
    rob_id = 3;
    memop_size = 1;
    memop_address = 0x0; //32
    stq_data = 0x04;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);

    //sb addr 5
    memop = MemopType::kStore;
    rob_id = 4;
    memop_size = 1;
    memop_address = 0x5; //32
    stq_data = 0x05;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);


    //lh x1, 0(x0)
    memop = MemopType::kLoad;
    rob_id = 5;
    memop_size = 8;
    memop_address = 0x0; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x50303020204) {
        throw std::runtime_error("failed");
    }

    //std::cout << "#############################################################" << std::endl;
    //std::cout << "test_10, data: " << std::hex << data << std::dec << std::endl; 

    std::cout << "###############################################" << std::endl;
    std::cout << "TEST 10 CASE 4" << std::endl;

    //case 4
    //sd addr 0-7
    memop = MemopType::kStore;
    rob_id = 10;
    memop_size = 8;
    memop_address = 0x100; //32
    stq_data = 0x0;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);

    //sd addr 0-7
    memop = MemopType::kStore;
    rob_id = 11;
    memop_size = 8;
    memop_address = 0x100; //32
    stq_data = 0x1110;
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+1);
    add_store_data_test(hart_id, stq_data, rob_id, global_clock+2);
    commit_memop_test(hart_id, rob_id, store_buffer_id, global_clock+3);

    //lh 0,1
    memop = MemopType::kLoad;
    rob_id = 12;
    memop_size = 2;
    memop_address = 0x100; //32
    create_memop_inorder_test(hart_id, rob_id, memop, global_clock+6);
    add_memop_address_test(hart_id, memop_address, memop_size, rob_id, global_clock+7);
    data = i_perform_load_test(hart_id, load_data, rob_id, global_clock+8);
    //check test code
    if(data != 0x1110) {
        throw std::runtime_error("failed");
    }

    m3::m3_ptr->close();
}

// Register the test
REGISTER_TEST("Overlapp accesses", base_test_10);
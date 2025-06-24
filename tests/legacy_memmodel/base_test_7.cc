#include "test_framework.h"
#include <cassert>
#include "core_m3_t.h"
#include "m3_dpi.cpp"
#include "m3_test_utils.h"

using namespace m3_test_utils;

// Example test case
void base_test_7() {
    int ncores;
    ncores = 2;
    setup(ncores, debug::VerbosityLevel::Medium, debug::ExecutionMode::Testing);

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

    //sb x2, 65(x0)
    //x2 = 50
    insn =
        0 |                   //empty spaces
        (0b0100011) |         // Bits [6:0]   = Store opcode
        // Bits [11:7] = imm [4:0], Bits[31:25] = imm[11:5] 
        ((65 & imm_4_0_mk) << 7)  | (((65 & imm_11_5_mk) << (25-5))) |
        (0 << 12) |           // Bits [14:12] = 0 (funct3)
        (0 << 15) |           // Bits [19:15] = rs1 = 0
        (2 << 20);           // Bits [14:20]  = 2 (rs2)
    hart_id = 0;
    rob_id = 0;
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

    create_memop_inorder_DPI(hart_id, rob_id, insn, global_clock);
    run_event_DPI();
    add_memop_address_DPI(hart_id, memop_address, memop_size, rob_id, global_clock);
    run_event_DPI();
    add_store_data_DPI(hart_id, stq_data, rob_id, global_clock);
    run_event_DPI();
    commit_memop_DPI(hart_id, rob_id, store_buffer_id, 0, global_clock);
    run_event_DPI();
    //complete_store_DPI(hart_id, store_buffer_id, global_clock);
    //run_event_DPI();
    update_cache_meta_DPI(hart_id, way_id, cache_line_id, new_coh_state, tag, global_clock+4);


    //lb x5, 65(x0) 
    //20(x0) = 50
    insn =
        0 |                   //empty spaces
        (0b0000011) |         // Bits [6:0]   = Load opcode
        (5 << 7) |            // Bits [11:7]  = 5 (rd)
        (0 << 12) |           // Bits [14:12] = 0 (funct3)
        (0 << 15) |          // Bits [19:15] = rs1 = 0
        (65 << 20);           // Bits [31:20] = imm[11:0] = 20
    hart_id = 0;
    rob_id = 1;
    memop_size = 1;
    memop_address = 0x20; //32
    global_clock = 1;
    load_data = 0;

    create_memop_inorder_DPI(hart_id, rob_id, insn, global_clock);
    run_event_DPI();
    add_memop_address_DPI(hart_id, memop_address, memop_size, rob_id, global_clock);
    run_event_DPI();
    i_perform_load_DPI(hart_id, load_data, rob_id, global_clock);
    run_event_DPI();
    char* data = getMemoryByte(memop_address);
    //check test code
    if(*data != 0x32) {
        throw std::runtime_error("failed");
    }
    commit_memop_DPI(hart_id, rob_id, 0, 0, global_clock);
    run_event_DPI();

    m3::m3_ptr->close();
}

// Register the test
REGISTER_TEST("Check dpi calls", base_test_7);
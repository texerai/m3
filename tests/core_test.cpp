/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#include "m3/memory.hpp"
#include "m3/memory_marionette.hpp"

uint8_t get_byte(uint64_t addr) { return addr >> 4; }

int main() {
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto oldest_ld = core.inorder();

    core.set_type(oldest_ld, m3::Mem_op::Load);

    auto &oldest_ld_data = core.ld_data_ref(oldest_ld);
    oldest_ld_data.add_addr(0x135);

    auto oldest_st = core.inorder();

    core.set_type(oldest_st, m3::Mem_op::Load);

    auto bad_spec_ld = core.inorder();

    core.ld_perform(oldest_ld);
    assert(oldest_ld_data.get_byte(0x135) == 0x13);

    auto &bad_spec_data = core.ld_data_ref(bad_spec_ld);
    bad_spec_data.add_addr(0x134, 3);
    core.ld_perform(bad_spec_ld);
    assert(bad_spec_data.get_byte(0x134) == 0x13);
    assert(bad_spec_data.get_byte(0x135) == 0x13);
    assert(bad_spec_data.get_byte(0x136) == 0x13);

    core.dump();

    core.set_safe(oldest_st);
    auto &oldest_st_data = core.st_data_ref(oldest_st);
    oldest_st_data.add_addr(0x135, 2);
    oldest_st_data.set_byte(0x135, 0x55);
    oldest_st_data.set_byte(0x136, 0x66);

    core.st_locally_perform(oldest_st);

    assert(core.has_error(bad_spec_ld));

    assert(oldest_ld_data.get_byte(0x135) == 0x13);

    core.dump();

    core.ld_perform(bad_spec_ld);  // re-exec load
    core.dump();
    assert(bad_spec_data.get_byte(0x134) == 0x13);
    assert(bad_spec_data.get_byte(0x135) == 0x55);
    assert(bad_spec_data.get_byte(0x136) == 0x66);
    assert(!core.has_error(bad_spec_ld));
}

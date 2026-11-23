/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

#include "m3/memory.hpp"
#include "m3/memory_marionette.hpp"

// Deterministic "golden" memory: every byte equals its address >> 4.
static uint8_t get_byte(uint64_t addr) { return addr >> 4; }

TEST_CASE("a load performed before an older store is flagged and fixed on re-perform")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto oldest_ld = core.inorder();
    core.set_type(oldest_ld, m3::Mem_op::Load);
    auto &oldest_ld_data = core.ld_data_ref(oldest_ld);
    oldest_ld_data.add_addr(0x135);

    auto oldest_st = core.inorder();
    core.set_type(oldest_st, m3::Mem_op::Store);

    auto bad_spec_ld = core.inorder();
    core.set_type(bad_spec_ld, m3::Mem_op::Load);

    core.ld_perform(oldest_ld);
    CHECK(oldest_ld_data.get_byte(0x135) == 0x13);

    // The younger load performs before the older store has its data: it
    // reads memory, which is only correct if the store does not overlap.
    auto &bad_spec_data = core.ld_data_ref(bad_spec_ld);
    bad_spec_data.add_addr(0x134, 3);
    core.ld_perform(bad_spec_ld);
    CHECK(bad_spec_data.get_data(0x134, 3) == 0x131313);
    CHECK(!core.has_error(bad_spec_ld));

    // The store turns out to overlap: the younger load must be flagged.
    core.set_safe(oldest_st);
    auto &oldest_st_data = core.st_data_ref(oldest_st);
    oldest_st_data.add_addr(0x135, 2);
    oldest_st_data.set_data(0x135, 2, 0x6655);
    core.st_locally_perform(oldest_st);

    CHECK(core.has_error(bad_spec_ld));
    CHECK(!core.has_error(oldest_ld));
    CHECK(oldest_ld_data.get_byte(0x135) == 0x13);  // older load is unaffected

    // Re-performing the load (replay) picks up the forwarded bytes.
    core.ld_perform(bad_spec_ld);
    CHECK(bad_spec_data.get_byte(0x134) == 0x13);
    CHECK(bad_spec_data.get_byte(0x135) == 0x55);
    CHECK(bad_spec_data.get_byte(0x136) == 0x66);
    CHECK(!core.has_error(bad_spec_ld));
}

TEST_CASE("loads forward from the youngest older performed store")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto st1 = core.inorder();
    core.set_type(st1, m3::Mem_op::Store);
    core.st_data_ref(st1).add_addr(0x200, 4);
    core.st_data_ref(st1).set_data(0x200, 4, 0x11111111);
    core.st_locally_perform(st1);

    auto st2 = core.inorder();
    core.set_type(st2, m3::Mem_op::Store);
    core.st_data_ref(st2).add_addr(0x202, 2);
    core.st_data_ref(st2).set_data(0x202, 2, 0x2222);
    core.st_locally_perform(st2);

    auto ld = core.inorder();
    core.set_type(ld, m3::Mem_op::Load);
    core.ld_data_ref(ld).add_addr(0x200, 4);
    core.ld_perform(ld);

    CHECK(core.ld_data_ref(ld).get_data(0x200, 4) == 0x22221111);
    CHECK(!core.has_error(ld));
}

TEST_CASE("a load does not see younger stores")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto ld = core.inorder();
    core.set_type(ld, m3::Mem_op::Load);
    core.ld_data_ref(ld).add_addr(0x300, 1);

    auto st = core.inorder();
    core.set_type(st, m3::Mem_op::Store);
    core.st_data_ref(st).add_addr(0x300, 1);
    core.st_data_ref(st).set_data(0x300, 1, 0xaa);
    core.st_locally_perform(st);

    core.ld_perform(ld);
    CHECK(core.ld_data_ref(ld).get_byte(0x300) == 0x30);
    CHECK(!core.has_error(ld));
}

TEST_CASE("globally performed stores become visible to other cores")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core0(mem, 0);
    m3::MemoryMarionette core1(mem, 1);

    auto st = core0.inorder();
    core0.set_type(st, m3::Mem_op::Store);
    core0.st_data_ref(st).add_addr(0x400, 8);
    core0.st_data_ref(st).set_data(0x400, 8, 0x0123456789abcdefull);
    core0.st_locally_perform(st);

    // Not visible to core1 yet.
    auto ld_early = core1.inorder();
    core1.set_type(ld_early, m3::Mem_op::Load);
    core1.ld_data_ref(ld_early).add_addr(0x400, 8);
    core1.ld_perform(ld_early);
    CHECK(core1.ld_data_ref(ld_early).get_data(0x400, 8) == 0x4040404040404040ull);

    core0.set_safe(st);
    core0.st_globally_perform(st);

    auto ld_late = core1.inorder();
    core1.set_type(ld_late, m3::Mem_op::Load);
    core1.ld_data_ref(ld_late).add_addr(0x400, 8);
    core1.ld_perform(ld_late);
    CHECK(core1.ld_data_ref(ld_late).get_data(0x400, 8) == 0x0123456789abcdefull);
}

TEST_CASE("nuke removes the nuked instruction and everything younger")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto older = core.inorder();
    core.set_type(older, m3::Mem_op::Load);
    auto nuked = core.inorder();
    core.set_type(nuked, m3::Mem_op::Load);
    auto younger = core.inorder();
    core.set_type(younger, m3::Mem_op::Store);

    std::set<m3::Inst_id> removed;
    core.nuke(nuked, removed, false);

    CHECK(removed == std::set<m3::Inst_id>{nuked, younger});
    CHECK(!core.has_error(older));
    CHECK(core.has_error(nuked));    // gone from the ROB
    CHECK(core.has_error(younger));

    SUBCASE("a flush removes everything regardless of the id") {
        std::set<m3::Inst_id> flushed;
        core.nuke(0, flushed, true);
        CHECK(flushed == std::set<m3::Inst_id>{older});
    }
}

TEST_CASE("st_locally_merged folds a younger store into an older one")
{
    m3::Memory mem(get_byte);
    m3::MemoryMarionette core(mem, 0);

    auto st1 = core.inorder();
    core.set_type(st1, m3::Mem_op::Store);
    core.st_data_ref(st1).add_addr(0x500, 4);
    core.st_data_ref(st1).set_data(0x500, 4, 0x11111111);
    core.st_locally_perform(st1);
    core.set_safe(st1);

    auto st2 = core.inorder();
    core.set_type(st2, m3::Mem_op::Store);
    core.st_data_ref(st2).add_addr(0x504, 4);
    core.st_data_ref(st2).set_data(0x504, 4, 0x22222222);
    core.st_locally_perform(st2);
    core.set_safe(st2);

    core.st_locally_merged(st1, st2);
    CHECK(core.has_error(st2));  // no longer tracked on its own
    CHECK(core.st_data_ref(st1).get_data(0x500, 8) == 0x2222222211111111ull);

    core.st_globally_perform(st1);

    m3::MemoryMarionette other(mem, 1);
    auto ld = other.inorder();
    other.set_type(ld, m3::Mem_op::Load);
    other.ld_data_ref(ld).add_addr(0x500, 8);
    other.ld_perform(ld);
    CHECK(other.ld_data_ref(ld).get_data(0x500, 8) == 0x2222222211111111ull);
}

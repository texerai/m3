/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

#include "m3/data.hpp"

TEST_CASE("set_addr allocates exactly the requested byte range")
{
    m3::Data d1;
    d1.set_addr(33, 5);

    CHECK(!d1.has_byte(32));
    for (uint64_t a = 33; a <= 37; ++a) {
        CHECK(d1.has_byte(a));
    }
    CHECK(!d1.has_byte(38));
}

TEST_CASE("set_data and get_data are little endian and byte addressable")
{
    m3::Data d1;
    d1.set_addr(33, 5);

    d1.set_data(33, 1, 'h');
    uint64_t d = 'l';
    d = (d << 8) | 'l';
    d = (d << 8) | 'e';
    d1.set_data(34, 3, d);
    d1.set_byte(37, 'o');

    CHECK(d1.get_byte(33) == 'h');
    CHECK(d1.get_byte(34) == 'e');
    CHECK(d1.get_byte(35) == 'l');
    CHECK(d1.get_byte(36) == 'l');
    CHECK(d1.get_byte(37) == 'o');
    CHECK(d1.get_data(33, 5) == 0x6f6c6c6568ull);
    CHECK(d1.str() == " mem[21:25]=68656c6c6f");
}

TEST_CASE("add_newer overlays the newer bytes and keeps the rest")
{
    m3::Data d1;
    d1.set_addr(33, 5);
    d1.set_data(33, 5, 0x6f6c6c6568ull);  // "hello"

    m3::Data d2;
    d2.set_addr(33, 3);
    d2.set_data(33, 3, 0x657962ull);  // "bye"
    d2.add_addr(50, 3);
    d2.set_data(50, 3, 0x6c6c61ull);  // "all"
    CHECK(d2.str() == " mem[21:23]=627965 mem[32:34]=616c6c");

    m3::Data d3 = d1;
    d3.add_newer(d2);

    CHECK(d3.get_data(33, 5) == 0x6f6c657962ull);  // "byelo"
    CHECK(d3.get_data(50, 3) == 0x6c6c61ull);
    CHECK(d3.str() == " mem[21:25]=6279656c6f mem[32:34]=616c6c");
}

TEST_CASE("update_newer only touches bytes that already exist")
{
    m3::Data ld;
    ld.add_addr(0x100, 4);
    ld.set_data(0x100, 4, 0x44332211);

    m3::Data st;
    st.add_addr(0x102, 4);  // overlaps the two upper bytes of the load
    st.set_data(0x102, 4, 0xddccbbaa);

    CHECK(ld.update_newer(st));
    CHECK(ld.get_data(0x100, 4) == 0xbbaa2211);
    CHECK(!ld.has_byte(0x104));

    m3::Data far;
    far.add_addr(0x200, 4);
    CHECK(!ld.update_newer(far));
}

TEST_CASE("add_addr merges adjacent bytes into one chunk")
{
    m3::Data d;
    d.add_addr(0x101, 2);

    SUBCASE("appending") {
        d.add_addr(0x103);
        CHECK(d.has_byte(0x103));
        CHECK(d.str().rfind(" mem[101:103]=", 0) == 0);
    }

    SUBCASE("prepending to the first chunk") {
        d.add_addr(0x100);
        CHECK(d.has_byte(0x100));
        CHECK(d.str().rfind(" mem[100:102]=", 0) == 0);
    }

    SUBCASE("filling the gap between two chunks") {
        d.add_addr(0x104, 2);
        d.add_addr(0x103);
        CHECK(d.str().rfind(" mem[101:105]=", 0) == 0);
    }

    SUBCASE("an existing byte is a no-op") {
        d.set_data(0x101, 2, 0xbeef);
        d.add_addr(0x101);
        CHECK(d.get_data(0x101, 2) == 0xbeef);
    }
}

TEST_CASE("overlap queries")
{
    m3::Data d;
    d.set_addr(0x10, 8);

    CHECK(d.has_partial_overlap(0x0c, 8));   // straddles the start
    CHECK(d.has_partial_overlap(0x14, 8));   // straddles the end
    CHECK(d.has_full_overlap(0x12, 2));      // inside
    CHECK(d.has_full_overlap(0x08, 32));     // contains the chunk
    CHECK(!d.has_overlap(0x18, 4));          // adjacent after
    CHECK(!d.has_overlap(0x08, 8));          // adjacent before
}

TEST_CASE("equality compares address ranges and bytes")
{
    m3::Data a;
    a.set_addr(8, 2);
    a.set_data(8, 2, 0x1234);

    m3::Data b = a;
    CHECK(a == b);

    b.set_byte(9, 0xff);
    CHECK(a != b);

    m3::Data c;
    c.set_addr(8, 3);
    c.set_data(8, 2, 0x1234);
    CHECK(a != c);
}

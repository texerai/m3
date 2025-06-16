#ifndef GOLD_DATA_H
#define GOLD_DATA_H

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

#include "lrand.hpp"
#include "debug_utils.hpp"

#include "fmt/core.h"

class Gold_data {
  public:
    void clear() {
        device = false;
        chunks.clear();
    }

    void set_addr(const uint64_t addr, const uint32_t sz) {
        DEBUG_LOG(fmt::format("set_addr addr: 0x{:x} , size: {}", addr, static_cast<int>(sz)),
        debug::VerbosityLevel::Debug);

        DEBUG_ASSERT(!has_partial_overlap(addr, sz),
            "Address not overlapping valid ranges");  // No overlapping valid ranges
        chunks.emplace_back(Chunk(addr, sz));

        sort_chunks();
    }

    void add_addr(const uint64_t addr) {
        for (auto i = 0u; i < chunks.size(); ++i) {
            auto &c = chunks[i];

            if (c.is_hit(addr))
                return;  // done

            if (c.addr - 1 == addr) {
                --c.addr;
                DEBUG_ASSERT(i > 0 && !chunks[i - 1].is_hit(addr - 1), "Incorrect address");
                c.data.insert(c.data.begin(), 1, rand_data.any());
                return;
            }

            if ((c.addr + c.data.size()) == addr) {
                c.data.emplace_back(rand_data.any());

                // Merge with the next chunk.
                if (i < chunks.size() - 1 && chunks[i + 1].addr == addr + 1) {
                    c.data.insert(c.data.end(), chunks[i + 1].data.begin(), chunks[i + 1].data.end());
                    ++i;
                    chunks.erase(chunks.begin() + i);
                }
                return;
            }
        }
        set_addr(addr, 1);
    }

    void add_addr(const uint64_t addr, const uint32_t sz) {
        for (auto i = 0; i < sz; ++i) {
            add_addr(addr + i);
        }
    }

    void set_data(const uint64_t addr, const uint32_t sz, uint64_t d) {
        DEBUG_ASSERT(sz <= 8, "Incorrect data size");
        for (int i = 0; i < sz; ++i) {
            DEBUG_LOG(fmt::format("set_data addr 0x{:x} data 0x{:x}", addr + i, d & 0xFF),
            debug::VerbosityLevel::Debug);
            set_byte(addr + i, d & 0xFF);
            d = d >> 8;
        }
    }

    uint64_t get_data(const uint64_t addr, const uint32_t sz) const {
        uint64_t d = 0;
        DEBUG_ASSERT(sz <= 8, "Incorrect data size");
        for (int i = 0; i < sz; ++i) {
            uint64_t b = get_byte(addr + sz - i - 1);
            DEBUG_LOG(
                fmt::format("Value got after using get_byte function at address: {}", static_cast<int>(b)),
                debug::VerbosityLevel::Debug);
            d          = (d << 8) | b;
            DEBUG_LOG(
                fmt::format("Value got after using mmu function at address: {}", static_cast<int>(d)),
                debug::VerbosityLevel::Debug);
        }
        return d;
    }

    bool has_full_overlap(const uint64_t addr, const uint32_t sz) const {
        DEBUG_LOG(
            fmt::format("check full overlap, addr: 0x{:x}, size: {}", addr, static_cast<int>(sz)),
            debug::VerbosityLevel::Debug);        
        for (const auto &c : chunks) {
            DEBUG_LOG(
                fmt::format("check full overlap, chunk addr: 0x{:x}, size: {}", c.addr, static_cast<int>(c.data.size())),
                debug::VerbosityLevel::Debug);            
            if (c.addr <= addr && addr + sz <= c.addr + c.data.size()) {
                //     [c;              c+size]
                //          [addr; addr+sz]
                return true;
            }
            if (c.addr >= addr && addr + sz >= c.addr + c.data.size()) {
                //         [c;  c+size]
                //     [addr;          addr+sz]
                return true;
            }
        }
        return false;
    }

    bool has_partial_overlap(const uint64_t addr, const uint32_t sz) const {
        DEBUG_LOG(
            fmt::format("check partial overlap, addr: 0x{:x}, size: {}", addr, static_cast<int>(sz)),
            debug::VerbosityLevel::Debug);        
        for (const auto &c : chunks) {
            DEBUG_LOG(
                fmt::format("check partial overlap, chunk addr: 0x{:x}, size: {}", c.addr, static_cast<int>(c.data.size())),
                debug::VerbosityLevel::Debug);            
            if (c.addr <= addr && addr < c.addr + c.data.size()) {
                //     [c;        c+size]
                //          [addr;    addr+sz]
                return true;
            }
            if (c.addr < addr + sz && addr + sz <= c.addr + c.data.size()) {
                //        [c;        c+size]
                // [addr;    addr+sz]
                return true;
            }
        }
        return false;
    }

    bool has_overlap(const uint64_t addr, const uint32_t sz) const {
        DEBUG_LOG(
            fmt::format("check overlap, addr: 0x{:x}, size: {}", addr, static_cast<int>(sz)),
            debug::VerbosityLevel::Debug);        
        return has_full_overlap(addr, sz) || has_partial_overlap(addr, sz);
    }

    bool has_partial_overlap(const Gold_data &d2) const {
        for (auto const &c : d2.chunks) {
            if (has_partial_overlap(c.addr, c.data.size()))
                return true;
        }
        return false;
    }

    bool has_byte(const uint64_t addr) const { return has_full_overlap(addr, 1); }

    void set_byte(const uint64_t addr, const uint8_t b) {
        DEBUG_LOG(
            fmt::format("set_byte addr: 0x{:x}, data: 0x{:x}", addr, static_cast<int>(b)),
            debug::VerbosityLevel::Debug);        
        for (auto &c : chunks) {
            if (!c.is_hit(addr))
                continue;
            DEBUG_LOG(
                fmt::format("set_byte addr: 0x{:x} data 0x{:x}", addr, static_cast<int>(b)),
                debug::VerbosityLevel::Debug);                
            c.set_byte(addr, b);
            return;
        }
        DEBUG_ASSERT(false, "invalid address");
    }

    uint8_t get_byte(const uint64_t addr) const {
        for (auto &c : chunks) {
            if (!c.is_hit(addr))
                continue;
            DEBUG_LOG(
                fmt::format("set_byte addr: 0x{:x} data {}", addr, c.get_byte(addr)),
                debug::VerbosityLevel::Debug);                
            return c.get_byte(addr);
        }

        return rand_data.any();
    }

    bool has_data() const { return !chunks.empty(); }

    std::string str() const;

    void dump() const { std::cout << str(); }

    void add_newer(const Gold_data &d2) {
        for (const auto c : d2.chunks) {
            add_addr(c.addr, c.data.size());
            for (auto i = 0u; i < c.data.size(); ++i) {
                set_byte(c.addr + i, c.data[i]);
            }
        }
    }

    // Only add new data, do not update the existing one
    // It is intended to be used to update a young rob entry data with an old one
    void add_no_overlap_only(const Gold_data &d2) {
        int x = 0;
        for (const auto c : d2.chunks) {
            DEBUG_LOG(fmt::format("chunk {}", x), debug::VerbosityLevel::Debug);
            int n = c.data.size();
            for (int i = 0u; i < n; ++i) {
                DEBUG_LOG(fmt::format("before has_byte, addr: 0x{:x}", c.addr + i),
                debug::VerbosityLevel::Debug);
                if (!has_byte(c.addr + i)) {
                    DEBUG_LOG(fmt::format("add_newer addr: 0x{:x}, size: {}", c.addr + i, 1),
                    debug::VerbosityLevel::Debug);
                    add_addr(c.addr + i, 1);
                    set_byte(c.addr + i, c.data[i]);
                }
            }
            ++x;
        }
    }

    bool update_newer(const Gold_data &d2) {
        DEBUG_LOG("update newer", debug::VerbosityLevel::Debug);
        bool is_updated = false;
        int x = 0;
        for (const auto &c : chunks) {
            DEBUG_LOG(fmt::format("chunk self: {} addr: 0x{:x} size: {}", x, c.addr, c.data.size()),
            debug::VerbosityLevel::Debug);
            ++x;
        }
        x = 0;
        for (const auto c : d2.chunks) {
            DEBUG_LOG(fmt::format("chunk input: {} addr: 0x{:x} size: {}", x, c.addr, c.data.size()),
            debug::VerbosityLevel::Debug);
            if (!has_overlap(c.addr, c.data.size()))
                continue;
            DEBUG_LOG(fmt::format("update newer overlap addr: 0x{:x}, size: {}", c.addr, c.data.size()),
            debug::VerbosityLevel::Debug);
            for (auto i = 0u; i < c.data.size(); ++i) {
                if (!has_byte(c.addr + i))
                    continue;

                set_byte(c.addr + i, c.data[i]);
                is_updated = true;
            }
            ++x;
        }
        return is_updated;
    }

    void set_device() { device = true; }

    void cout_chunks(std::string a) {
        for (const auto c : chunks) {
            for (int i = 0; i < c.data.size(); ++i) {
                DEBUG_LOG(fmt::format("cout_chunks: {} chunk addr: 0x{:x}, data: 0x{:x}", a, c.addr + i, (int)c.data[i]),
                debug::VerbosityLevel::Debug);
            }
        }
    }

    void each_chunk(std::function<void(uint64_t, const std::vector<uint8_t> &)> fun) {
        for (const auto c : chunks) {
            DEBUG_LOG(fmt::format("chunk base addr 0x{:x}", c.addr),
            debug::VerbosityLevel::Debug);
            fun(c.addr, c.data);
        }
    }

    void each_chunk(std::function<void(uint64_t, const std::vector<uint8_t> &)> fun) const {
        for (const auto c : chunks) {
            DEBUG_LOG(fmt::format("chunk base addr 0x{:x}", c.addr),
            debug::VerbosityLevel::Debug);
            fun(c.addr, c.data);
        }
    }

    bool operator==(const Gold_data &d2) const {
        if (device && d2.device) {
            return false;  // do not flag diff/error of device
        }

        auto it1 = chunks.begin();
        auto it2 = d2.chunks.begin();

        for (; it1 != chunks.end() && it2 != d2.chunks.end(); ++it1, ++it2) {
            if (it1->addr != it2->addr)
                return false;
            if (it1->data != it2->data)
                return false;
        }
        if ((it1 == chunks.end() && it2 != d2.chunks.end()) || (it1 != chunks.end() && it2 == d2.chunks.end())) {
            return false;
        }

        return true;
    }

    bool operator!=(const Gold_data &d2) const {
        if (device && d2.device) {
            return false;  // do not flag diff/error of device
        }

        return !(*this == d2);
    }

    Gold_data() { device = false; }

  protected:

    static Lrand<uint8_t> rand_data;
    void sort_chunks() {
        std::sort(chunks.begin(), chunks.end(), [](const Chunk &a, const Chunk &b) -> bool { return a.addr < b.addr; });
#ifndef NDEBUG
        for (auto i = 1u; i < chunks.size(); ++i) {
            DEBUG_ASSERT(chunks[i - 1].addr + chunks[i - 1].data.size() < chunks[i].addr,
            "overlapp of chunks");  // no overlap, no even concatenatable chunks
        }
#endif
    }

    struct Chunk {
        static Lrand<uint8_t> rand_data;
        Chunk() {
            addr = 0;
            data.clear();
        }

        Chunk(uint64_t a, uint32_t s) : addr(a) { data.resize(s, rand_data.any()); }

        bool is_hit(uint64_t a) const { return addr <= a && (addr + data.size()) > a; }

        void set_byte(uint64_t a, uint8_t b) {
            DEBUG_ASSERT(is_hit(a), "not hit");
            DEBUG_ASSERT(data.size() > (a - addr), "out of chunk space");

            DEBUG_LOG(fmt::format("chunk set_byte base addr 0x{:x} offset addr 0x{:x} data 0x{:x}", addr, a - addr, static_cast<int>(b)), 
            debug::VerbosityLevel::Debug);
            data[a - addr] = b;
        }

        uint8_t get_byte(uint64_t a) const {
            DEBUG_ASSERT(is_hit(a), "not hit");
            DEBUG_ASSERT(data.size() > a - addr, "out of chunk space");

            DEBUG_LOG(fmt::format("chunk get_byte base addr 0x{:x} offset addr 0x{:x} data 0x{:x}",
                addr, a - addr, data[a - addr]), debug::VerbosityLevel::Debug);
            return data[a - addr];
        }

        uint64_t             addr;
        std::vector<uint8_t> data;
    };

    std::vector<Chunk> chunks;
    bool               device;
};

#endif

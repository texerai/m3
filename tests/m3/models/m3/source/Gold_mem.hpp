#pragma once

// TODO: the mem is byte (not efficient maybe line base?)

#include <algorithm>
#include <fmt/core.h>

#include "Gold_data.hpp"
#include "robin_hood.hpp"
#include "debug_utils.hpp"

class Gold_mem {
    public:
        Gold_mem() = default;
        Gold_mem(std::function<uint8_t(uint64_t)> gb) : get_byte(gb) {
            is_init = true;
        }

        void init(std::function<uint8_t(uint64_t)> gb) {
            get_byte = gb;
            is_init = true;
        }

        void st_perform(const Gold_data &st_data) {
            if (!is_init) {
                DEBUG_LOG("Gold_mem not initialized.", debug::VerbosityLevel::Error);
                return;
            }
            st_data.each_chunk([this](uint64_t addr, const std::vector<uint8_t> &data) {
                uint64_t ppn = addr >> M3PGSHIFT, pgoff = addr % M3PGSIZE;
                
                auto it = mem_byte.find(ppn);
                if (it == mem_byte.end()) {
                    // Allocate a whole page if not already allocated
                    uint8_t* res = (uint8_t*)calloc(M3PGSIZE, 1);
                    if (res == nullptr)
                        throw std::bad_alloc();
                    mem_byte[ppn] = res;
                    // Update iterator after insertion
                    it = mem_byte.find(ppn);
                }
                // Get the allocated page
                uint8_t* page = it->second;
                for (auto b_pos = 0u; b_pos < data.size(); ++b_pos) {
                    DEBUG_LOG(fmt::format("st_perform addr: 0x{:x}, data 0x{:x}", addr + b_pos, static_cast<uint32_t>(data[b_pos])),
                        debug::VerbosityLevel::Debug);
                    page[pgoff + b_pos] = data[b_pos];
                }
            });
        }

        void ld_perform(Gold_data &ld_data) {
            if (!is_init) {
                DEBUG_LOG("Gold_mem not initialized.", debug::VerbosityLevel::Error);
                return;
            }
            ld_data.each_chunk([this, &ld_data](uint64_t addr, const std::vector<uint8_t> &data) 
            {
                DEBUG_LOG(fmt::format("Address provided by the function is: 0x{:x}", addr),
                    debug::VerbosityLevel::Debug);
                for (auto b_pos = 0u; b_pos < data.size(); ++b_pos) {
                    auto a = addr + b_pos;
                    uint64_t ppn = a >> M3PGSHIFT, pgoff = a % M3PGSIZE;
                    auto it = mem_byte.find(ppn);
                    if (it == mem_byte.end()) {
                        // If the page is missing, return 0 instead of allocating
                        ld_data.set_byte(a, 0);
                    } else {
                        DEBUG_LOG(fmt::format("ld_perform addr: 0x{:x} data: 0x{:x}", a, data[b_pos]),
                            debug::VerbosityLevel::Debug);
                        // Get the allocated page
                        uint8_t* page = it->second;
                        ld_data.set_byte(a, (page[pgoff]));
                    }
                }
            });
        }
    
    u_int64_t size() { return sz; }
    robin_hood::unordered_map<uint64_t, uint8_t*> mem_byte;

    protected:
        std::function<uint8_t(uint64_t)> get_byte;
        uint64_t M3PGSHIFT = 12;
        uint64_t M3PGSIZE  = 1 << M3PGSHIFT;
    private:
        bool is_init = false;
        u_int64_t sz;
};

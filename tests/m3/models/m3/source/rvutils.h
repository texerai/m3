/*
 *  Copyright (c) 2023 Texer.ai. All rights reserved.
 */
#ifndef RISCV_UTILS_H_
#define RISCV_UTILS_H_

#include <stdint.h>
#include <algorithm>

class RVUtils
{
public: 
    static bool is_store(uint32_t insn);
    static bool is_compressed_store(uint32_t insn);
    static bool is_load(uint32_t insn);
    static bool is_fp_load(uint32_t insn);
    static bool is_compressed_load(uint32_t insn);
    static bool is_compressed_fp_load(uint32_t insn);
    static bool is_compressed_memop(uint32_t insn);
    static bool is_memop(uint32_t insn);
    static bool is_amo(uint32_t insn);
    static bool is_amo_add(uint32_t insn);
    static bool is_amo_swap(uint32_t insn);
    static bool is_amo_xor(uint32_t insn);
    static bool is_amo_and(uint32_t insn);
    static bool is_amo_or(uint32_t insn);
    static bool is_amo_min(uint32_t insn);
    static bool is_amo_max(uint32_t insn);
    static bool is_amo_minu(uint32_t insn);
    static bool is_amo_maxu(uint32_t insn);
    static uint32_t add_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t swap_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t xor_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t and_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t or_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t min_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t max_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t minu_w(uint32_t mem_value, uint32_t st_value);
    static uint32_t maxu_w(uint32_t mem_value, uint32_t st_value);
    static uint64_t add_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t swap_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t xor_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t and_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t or_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t min_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t max_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t minu_d(uint64_t mem_value, uint64_t st_value);
    static uint64_t maxu_d(uint64_t mem_value, uint64_t st_value);
    static int32_t get_source_from_store(uint32_t insn);
    static int32_t get_destination_from_load(uint32_t insn);
    static uint64_t simple_hash(uint64_t hartid, uint64_t rob_id);
    static uint64_t get_sized_mask(uint32_t size_in_bytes);
    static int64_t sign_extend(uint64_t data, uint32_t size);
    static uint32_t inst_size_to_byte_size(uint32_t inst_size);

    RVUtils() = delete;
    RVUtils(const RVUtils&) = delete;
    RVUtils& operator=(const RVUtils&) = delete;
};

#endif // RISCV_UTILS_H_

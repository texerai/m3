/*
 *  Copyright (c) 2023 Texer.ai. All rights reserved.
 */
#ifndef RISCV_UTILS_H_
#define RISCV_UTILS_H_

#include <stdint.h>

class RVUtils
{
public: 
    static bool is_store(uint32_t insn);
    static bool is_compressed_store(uint32_t insn);
    static bool  is_load(uint32_t insn);
    static bool is_fp_load(uint32_t insn);
    static bool is_compressed_load(uint32_t insn);
    static bool is_compressed_fp_load(uint32_t insn);
    static bool is_compressed_memop(uint32_t insn);
    static bool is_memop(uint32_t insn);
    static bool is_amo(uint32_t insn);
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

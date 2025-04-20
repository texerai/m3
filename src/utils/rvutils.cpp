/*
 *  Copyright (c) 2023 Texer.ai. All rights reserved.
 */
 #include "utils/rvutils.h"

// Constants.
static const uint64_t kOffset = 10000;
static const uint32_t kOneByteMask = 0xff;

bool RVUtils::is_store(uint32_t insn)
{
    bool is_i_store = (insn & 0x7f) == 0x23;
    bool is_f_store = (insn & 0x7f) == 0x27;
    return (is_i_store || is_f_store);
}

bool RVUtils::is_compressed_store(uint32_t insn)
{
    bool is_c_fsd = (insn & 0x3) == 0 && (insn & 0xe000) == 0xa000;
    bool is_c_sw = (insn & 0x3) == 0 && (insn & 0xe000) == 0xc000;
    bool is_c_fsw = (insn & 0x3) == 0 && (insn & 0xe000) == 0xe000;
    bool is_c_sd = (insn & 0x3) == 0 && (insn & 0xe000) == 0xe000;
    bool is_c_fsd_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0xa000;
    bool is_c_sw_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0xc000;
    bool is_c_fsw_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0xe000;
    bool is_c_sd_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0xe000;
    return (is_c_fsd || is_c_sw || is_c_fsw || is_c_sd
        || is_c_fsd_sp || is_c_sw_sp || is_c_fsw_sp || is_c_sd_sp);
}

bool RVUtils::is_load(uint32_t insn)
{
    bool is_i_load = (insn & 0x7F) == 0x03;
    bool is_f_load = (insn & 0x7F) == 0x07;
    return (is_i_load || is_f_load);
}

bool RVUtils::is_fp_load(uint32_t insn)
{
    bool is_f_load = (insn & 0x7F) == 0x07;
    return is_f_load;
}

bool RVUtils::is_compressed_load(uint32_t insn)
{
    bool is_c_fld = (insn & 0x3) == 0 && (insn & 0xe000) == 0x2000;
    bool is_c_lw = (insn & 0x3) == 0 && (insn & 0xe000) == 0x4000;
    bool is_c_flw = (insn & 0x3) == 0 && (insn & 0xe000) == 0x6000;
    bool is_c_ld = (insn & 0x3) == 0 && (insn & 0xe000) == 0x6000;
    bool is_c_fld_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x2000;
    bool is_c_lw_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x4000;
    bool is_c_flw_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x6000;
    bool is_c_ld_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x6000;
    return (is_c_fld || is_c_lw || is_c_flw || is_c_ld
        || is_c_fld_sp || is_c_lw_sp || is_c_flw_sp || is_c_ld_sp);
}

bool RVUtils::is_compressed_fp_load(uint32_t insn)
{
    bool is_c_fld = (insn & 0x3) == 0 && (insn & 0xe000) == 0x2000;
    bool is_c_flw = (insn & 0x3) == 0 && (insn & 0xe000) == 0x6000;
    bool is_c_fld_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x2000;
    bool is_c_flw_sp = (insn & 0x3) == 2 && (insn & 0xe000) == 0x6000;
    return (is_c_fld || is_c_flw || is_c_fld_sp || is_c_flw_sp);
}

bool RVUtils::is_compressed_memop(uint32_t insn)
{
    return is_compressed_load(insn) || is_compressed_store(insn);
}

bool RVUtils::is_memop(uint32_t insn)
{
    return is_compressed_memop(insn) || is_store(insn) || is_load(insn);
}

bool RVUtils::is_amo(uint32_t insn)
{
    int opcode = insn & 0x7f;
    if (opcode != 0x2f)
        return false;

    switch (insn >> 27) {
        case 1:    /* amiswap.w */
        case 2:    /* lr.w */
        case 0:    /* amoadd.w */
        case 4:    /* amoxor.w */
        case 0xc:  /* amoand.w */
        case 0x8:  /* amoor.w */
        case 0x10: /* amomin.w */
        case 0x14: /* amomax.w */
        case 0x18: /* amominu.w */
        case 0x1c: /* amomaxu.w */ return true;
        default: return false;
    }
}

int32_t RVUtils::get_source_from_store(uint32_t insn)
{
    int32_t source = -1;
    if (is_compressed_store(insn))
    {
        source = (insn >> 2) & 0x1f;
    }
    else if (is_store(insn))
    {
        source = (insn >> 20) & 0x1f;
    }

    return source;
}

int32_t RVUtils::get_destination_from_load(uint32_t insn)
{
    int32_t destination = -1;

    if (is_compressed_load(insn))
    {
        destination = ((insn >> 2) & 0x7) + 8;
    }
    else if (is_load(insn))
    {
        destination = (insn >> 7) & 0x1f;
    }

    // Shift by 32 if FP load.
    destination += static_cast<uint32_t>(is_compressed_fp_load(insn) || is_fp_load(insn))*32;
    return destination;
}

// Simple hashing function to make sure different
// ids created for different core's ROB ids.
uint64_t RVUtils::simple_hash(uint64_t hartid, uint64_t rob_id)
{
    // Returned value shouldn't alias
    // as long as kRobEntries is larger than number of ROB entries
    // in BOOM.
    return (hartid * kOffset) + rob_id;
}

uint64_t RVUtils::get_sized_mask(uint32_t size_in_bytes)
{
    uint64_t ret = 0;
    while (size_in_bytes > 0)
    {
        ret = (ret << 8) | kOneByteMask;
        --size_in_bytes;
    }
    return ret;
}

int64_t RVUtils::sign_extend(uint64_t data, uint32_t size)
{
    int64_t ret = 0;
    uint64_t mask = get_sized_mask(size);
    uint64_t inverted_mask = ~mask;
    data = data & mask;
    uint64_t is_signed = data >> (8 * size - 1);
    ret = static_cast<int64_t>(data | (inverted_mask * is_signed));
    return ret;
}

uint32_t RVUtils::inst_size_to_byte_size(uint32_t inst_size)
{
    uint32_t sz = 0;
    switch (inst_size)
    {
        case 0: sz = 1; break;
        case 1: sz = 2; break;
        case 2: sz = 4; break;
        case 3: sz = 8; break;
        default: sz = 0;
    }
    return sz;
}

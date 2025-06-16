#ifndef RISCV_AMO_UTILS_HPP
#define RISCV_AMO_UTILS_HPP

#include <stdint.h>
#include <algorithm>
#include "rvutils.h"
#include "memop_info.h"

class RiscV_AMO {
public:
    static uint64_t execute(m3::AmoType type, size_t size_in_bytes, uint64_t mem_value, uint64_t st_value);
};

#endif // RISCV_AMO_UTILS_HPP

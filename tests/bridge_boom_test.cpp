#include "bridge_boom.h"

#include <unordered_map>

static std::unordered_map<uint64_t, uint8_t> memory;
struct Core
{
    std::unordered_map<uint64_t, uint64_t> int_registers;
    std::unordered_map<uint64_t, uint64_t> fp_registers;
};
static std::unordered_map<uint32_t, Core> multi_core;

static uint8_t GetByte(uint64_t paddr)
{
    if (memory.count(paddr) == 0)
    {
        memory[paddr] = 0xAA;
    }

    return memory[paddr];
}

static void UpdateReg(uint32_t hart_id, uint32_t dest_reg, uint64_t write_data, bool is_fp)
{
    if (is_fp)
    {
        multi_core[hart_id].fp_registers[dest_reg] = write_data;
    }
    else
    {
        multi_core[hart_id].int_registers[dest_reg] = write_data;
    }
}

int main()
{
    m3::BridgeBoom bridge_boom;

    bridge_boom.Init(1);
    bridge_boom.SetCallbackGetByte(GetByte);
    bridge_boom.SetCallbackUpdateReg(UpdateReg);
}

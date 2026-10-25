/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#include "bridge_boom.h"
#include "rtl_hook.h"

// Local variables.
#include "test_trace_reader/test_trace_reader.h"

// C++ libraries.
#include <algorithm>
#include <iostream>
#include <unordered_map>

// Simple processor modeling for testing.
//
// The traces do not carry the initial memory image of the BOOM run they were
// recorded from, so the golden memory is seeded lazily: the first time the
// model touches a byte, it takes the value the RTL observed for the load that
// is touching it. A load that is the first reader of its bytes therefore
// trivially matches; every later load of those bytes, and all forwarding and
// ordering checks, are real checks.
static std::unordered_map<uint64_t, uint8_t> memory;

// Bytes the RTL has reported for loads that are about to be performed.
static std::unordered_map<uint64_t, uint8_t> rtl_load_bytes;

struct MemopAddress
{
    uint64_t address = 0;
    uint32_t size = 0;
};
// (hart_id, rob_id) -> address of the memop, from the kAddMemopAddress event.
static std::unordered_map<uint32_t, std::unordered_map<uint32_t, MemopAddress>> memop_addresses;

struct Core
{
    std::unordered_map<uint64_t, uint64_t> int_registers;
    std::unordered_map<uint64_t, uint64_t> fp_registers;
};
static std::unordered_map<uint32_t, Core> multi_core;

static uint32_t MemopSizeToBytes(uint32_t memop_size)
{
    return 1u << memop_size;
}

static uint8_t GetByte(uint64_t paddr)
{
    if (memory.count(paddr) == 0)
    {
        auto it = rtl_load_bytes.find(paddr);
        memory[paddr] = it != rtl_load_bytes.end() ? it->second : 0xAA;
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

// Remember addresses and expose the RTL load data to GetByte before the
// bridge serves the events of this cycle.
static void PrepareSeeding(const m3::RtlHookData& data)
{
    if (data.event == m3::RtlHook::kAddMemopAddress)
    {
        memop_addresses[data.hart_id][data.rob_id] = {data.address, MemopSizeToBytes(data.memop_size)};
    }
    else if (data.event == m3::RtlHook::kPerformLoad)
    {
        auto& per_hart = memop_addresses[data.hart_id];
        auto it = per_hart.find(data.rob_id);
        if (it == per_hart.end())
        {
            return;  // the bridge reports loads without an address itself
        }
        uint64_t value = data.load_rtl_data;
        for (uint32_t i = 0; i < it->second.size; ++i)
        {
            rtl_load_bytes[it->second.address + i] = value & 0xff;
            value >>= 8;
        }
    }
}

void print_help(const std::string& program_name)
{
    std::cout << "Usage: " << program_name << " <path-to-test-trace-file>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help      Show this help message and exit\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "  This program accepts a single argument: the path to a Test trace file.\n";
    std::cout << "  The traces are generated from the real simulation runs of BOOM.\n";
    std::cout << "  Exits with a non-zero status if the bridge reports any failure.\n";
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        print_help(argv[0]);
        return 1;
    }

    std::string arg = argv[1];

    if (arg == "-h" || arg == "--help")
    {
        print_help(argv[0]);
        return 0;
    }

    std::cout << "Trace file path: " << arg << "\n";

    std::vector<m3::RtlHookData> rtl_hook_data;
    std::string errorMessage;
    if (!m3::TestTraceReader::ParseFile(arg, rtl_hook_data, errorMessage))
    {
        std::cerr << "Error: " << errorMessage << "\n";
        return 1;
    }

    // The trace itself says how many cores were simulated.
    uint32_t ncores = 1;
    for (const auto& hook : rtl_hook_data)
    {
        ncores = std::max(ncores, hook.hart_id + 1);
    }

    m3::BridgeBoom bridge_boom;
    bridge_boom.Init(ncores);
    bridge_boom.SetCallbackGetByte(GetByte);
    bridge_boom.SetCallbackUpdateReg(UpdateReg);

    // Talk to the m3.
    uint64_t failed_cycles = 0;
    auto hook_data_iter = rtl_hook_data.begin();
    while (hook_data_iter != rtl_hook_data.end())
    {
        // Register all events happened at same clock cycle.
        uint64_t current_time = hook_data_iter->timestamp;
        while (hook_data_iter != rtl_hook_data.end() && hook_data_iter->timestamp == current_time)
        {
            PrepareSeeding(*hook_data_iter);
            bridge_boom.RegisterEvent(*hook_data_iter);
            ++hook_data_iter;
        }

        // Serve the events.
        if (!bridge_boom.ServeRegisteredEvents())
        {
            ++failed_cycles;
        }
        rtl_load_bytes.clear();
    }

    bridge_boom.DumpStatistics();
    std::cout << "Replayed " << rtl_hook_data.size() << " events, "
              << failed_cycles << " cycle(s) with failures.\n";
    return failed_cycles == 0 ? 0 : 1;
}

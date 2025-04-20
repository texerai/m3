#include "bridge_boom.h"
#include "rtl_hook.h"

// Local variables.
#include "test_trace_reader/test_trace_reader.h"

// C++ libraries.
#include <iostream>
#include <unordered_map>

// Simple processor modeling for testing.
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

void print_help(const std::string& program_name)
{
    std::cout << "Usage: " << program_name << " <path-to-test-trace-file>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help      Show this help message and exit\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "  This program accepts a single argument: the path to a Test trace file.\n";
    std::cout << "  The traces are generated from the real simulation runs of BOOM.\n";
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

    m3::BridgeBoom bridge_boom;
    bridge_boom.Init(2);
    bridge_boom.SetCallbackGetByte(GetByte);
    bridge_boom.SetCallbackUpdateReg(UpdateReg);

    std::vector<m3::RtlHookData> rtl_hook_data = m3::TestTraceReader::ParseFile(arg);

    // Talk to the m3.
    auto hook_data_iter = rtl_hook_data.begin();
    while (hook_data_iter != rtl_hook_data.end())
    {
        // Register all events happened at same clock cycle.
        uint64_t current_time = hook_data_iter->timestamp;
        while (hook_data_iter->timestamp == current_time)
        {
            bridge_boom.RegisterEvent(*hook_data_iter);
            ++hook_data_iter;
        }
        
        // Serve the events.
        bridge_boom.ServeRegisteredEvents();
    }
}

#include <iostream>
#include <vector>
#include <string>

#include "test_trace_reader.h"

// Helper function to print the raw data of an RtlHookData instance.
void printRawHookData(const m3::RtlHookData& data, const std::string& label)
{
    std::cout << "\n--- " << label << " ---" << std::endl;
    std::cout << "  event           : " << static_cast<int>(data.event)
              << " (Enum: RtlHook)" << std::endl;
    std::cout << "  hart_id         : " << data.hart_id << std::endl;
    std::cout << "  rob_id          : " << data.rob_id << std::endl;
    std::cout << "  rv_instruction  : " << data.rv_instruction << std::endl;
    std::cout << "  memop_size      : " << data.memop_size << std::endl;
    std::cout << "  address         : " << data.address << std::endl;
    std::cout << "  memop_id        : " << data.memop_id << std::endl;
    std::cout << "  load_rtl_data   : " << data.load_rtl_data << std::endl;
    std::cout << "  store_data      : " << data.store_data << std::endl;
    std::cout << "  timestamp       : " << data.timestamp << std::endl;
    std::cout << "  coherence_state : " << data.coherence_state << std::endl;
    std::cout << "  way_id          : " << data.way_id << std::endl;
    std::cout << "  cache_line_id   : " << data.cache_line_id << std::endl;
    std::cout << "  tag             : " << data.tag << std::endl;
    std::cout << "  is_load         : " << data.is_load << std::endl;
    std::cout << "  is_store        : " << data.is_store << std::endl;
    std::cout << "  is_amo          : " << data.is_amo << std::endl;
    std::cout << "---------------------" << std::endl;
}

int main()
{
    std::vector<m3::RtlHookData> parsedHooks;
    std::string errorMessage;

    bool success = m3::TestTraceReader::ParseFile("m3_events.txt", parsedHooks, errorMessage);

    if (!success)
    {
        std::cerr << "Error parsing file: " << errorMessage << std::endl;
        return 1;
    }

    std::cout << "Successfully parsed " << parsedHooks.size() << " hooks." << std::endl;

    size_t n = parsedHooks.size();

    if (n == 0)
    {
         std::cout << "\nNo hooks parsed to display." << std::endl;
    }
    else
    {
        std::cout << "\n--- Raw Hook Data (Last up to 3) ---" << std::endl;
        if (n >= 3)
        {
            printRawHookData(parsedHooks[n - 3], "Third-to-Last Hook (Index " + std::to_string(n - 3) + ")");
        }
        if (n >= 2)
        {
            printRawHookData(parsedHooks[n - 2], "Second-to-Last Hook (Index " + std::to_string(n - 2) + ")");
        }
        printRawHookData(parsedHooks[n - 1], "Last Hook (Index " + std::to_string(n - 1) + ")");

        if (n < 3)
        {
            std::cout << "\n(Displayed fewer than 3 hooks as only " << n << " were parsed)" << std::endl;
        }
    }

    return 0;
}

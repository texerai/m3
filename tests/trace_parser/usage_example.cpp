#include <iostream>
#include <vector>

#include "trace_parser.h"

// Helper function to print the raw data of an RTLEventData instance.
void printRawEventData(const m3::RTLEventData& data, const std::string& label) {
    std::cout << "\n--- " << label << " ---" << std::endl;
    std::cout << "  event           : " << static_cast<int>(data.event)
              << " (Enum: RTLEvent)" << std::endl;
    std::cout << "  memop_type      : " << static_cast<int>(data.memop_type)
              << " (Enum: MemopType)" << std::endl;
    std::cout << "  data_format     : " << static_cast<int>(data.data_format)
              << " (Enum: MemopFormat)" << std::endl;
    std::cout << "  hart_id         : " << data.hart_id << std::endl;
    std::cout << "  rob_id          : " << data.rob_id << std::endl;
    std::cout << "  load_dest_reg   : " << data.load_dest_reg << std::endl;
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
    std::cout << "---------------------" << std::endl;
}

int main() {
    try {
        std::vector<m3::RTLEventData> parsedEvents = m3::RTLEventParser::parseFile("m3_events.txt");
        std::cout << "Successfully parsed " << parsedEvents.size() << " events." << std::endl;

        size_t n = parsedEvents.size();

        // Output Raw Data for Last 3 Elements.
        if (n == 0) {
             std::cout << "\nNo events parsed to display." << std::endl;
        } else {
            std::cout << "\n--- Raw Event Data (Last up to 3) ---" << std::endl;
            if (n >= 3) {
                printRawEventData(parsedEvents[n - 3], "Third-to-Last Event (Index " + std::to_string(n - 3) + ")");
            }

            if (n >= 2) {
                printRawEventData(parsedEvents[n - 2], "Second-to-Last Event (Index " + std::to_string(n - 2) + ")");
            }

            printRawEventData(parsedEvents[n - 1], "Last Event (Index " + std::to_string(n - 1) + ")");

            if (n < 3) {
                std::cout << "\n(Displayed fewer than 3 events as only " << n << " were parsed)" << std::endl;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error parsing file: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

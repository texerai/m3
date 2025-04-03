#include "trace_parser.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace m3 {

// Static Helper Function Implementations.
RTLEvent RTLEventParser::stringToRTLEvent(const std::string& eventName) {
    static const std::map<std::string, RTLEvent> eventMap = {
        {"kCreateMemop", RTLEvent::kCreateMemop},
        {"kAddMemopAddress", RTLEvent::kAddMemopAddress},
        {"kPerformLoad", RTLEvent::kPerformLoad},
        {"kAddStoreData", RTLEvent::kAddStoreData},
        {"kCompleteStore", RTLEvent::kCompleteStore},
        {"kCommitMemop", RTLEvent::kCommitMemop},
        {"kUpdateCacheLineState", RTLEvent::kUpdateCacheLineState},
        {"kUpdateCacheLineData", RTLEvent::kUpdateCacheLineData}
    };

    auto it = eventMap.find(eventName);
    if (it != eventMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown event name: " + eventName);
}

MemopType RTLEventParser::stringToMemopType(const std::string& typeName) {
    static const std::map<std::string, MemopType> typeMap = {
        {"kLoad", MemopType::kLoad},
        {"kStore", MemopType::kStore},
        {"kAmo", MemopType::kAmo}
    };

    auto it = typeMap.find(typeName);
    if (it != typeMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown MemopType name: " + typeName);
}

MemopFormat RTLEventParser::stringToMemopFormat(const std::string& formatName) {
    static const std::map<std::string, MemopFormat> formatMap = {
        {"kInteger", MemopFormat::kInteger},
        {"kFloat", MemopFormat::kFloat}
    };

    auto it = formatMap.find(formatName);
    if (it != formatMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown MemopFormat name: " + formatName);
}

void RTLEventParser::parseLine(const std::string& key, const std::string& value,
                               RTLEventData& currentEventData, int lineNumber) {
    try {
        if (key == "event") {
            currentEventData.event = stringToRTLEvent(value);
        } else if (key == "hart_id") {
            currentEventData.hart_id = std::stoul(value);
        } else if (key == "rob_id") {
            currentEventData.rob_id = std::stoul(value);
        } else if (key == "timestamp") {
            currentEventData.timestamp = std::stoull(value);
        } else if (key == "memop_type") {
	    currentEventData.memop_type = stringToMemopType(value);
        } else if (key == "data_format") {
            currentEventData.data_format = stringToMemopFormat(value);
        } else if (key == "load_dest_reg") {
            currentEventData.load_dest_reg = std::stoul(value);
        } else if (key == "memop_size") {
            currentEventData.memop_size = std::stoul(value);
        } else if (key == "address") {
            currentEventData.address = std::stoull(value);
        } else if (key == "memop_id") {
            currentEventData.memop_id = std::stoul(value);
        } else if (key == "load_rtl_data") {
            currentEventData.load_rtl_data = std::stoull(value);
        } else if (key == "store_data") {
            currentEventData.store_data = std::stoull(value);
        } else if (key == "coherence_state") {
            currentEventData.coherence_state = std::stoul(value);
        } else if (key == "way_id") {
            currentEventData.way_id = std::stoul(value);
        } else if (key == "cache_line_id") {
            currentEventData.cache_line_id = std::stoul(value);
        } else if (key == "tag") {
            currentEventData.tag = std::stoull(value);
        } else {
            throw std::runtime_error("Unknown key '" + key + "' at line " +
                                     std::to_string(lineNumber));
        }
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid numeric value '" + value +
                                 "' for key '" + key + "' at line " +
                                 std::to_string(lineNumber));
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Numeric value '" + value + "' out of range " +
                                 " for key '" + key + "' at line " +
                                 std::to_string(lineNumber));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string(e.what()) + " at line " +
                                 std::to_string(lineNumber));
    }
}

// Public Static Method Implementation.
std::vector<RTLEventData> RTLEventParser::parseFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::vector<RTLEventData> events;
    std::string line;
    RTLEventData currentEventData{}; // Initialize with default values.
    bool inRecord = false;
    int lineNumber = 0;

    while (std::getline(inputFile, line)) {
        lineNumber++;
        // Trim leading/trailing whitespace.
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty()) {
            // Blank line signifies the end of a record
            if (inRecord) {
                if (currentEventData.event != RTLEvent::kUndefined) {
                    events.push_back(currentEventData);
                } else {
                    std::cerr << "Warning: Skipping potentially incomplete record ending before line " << lineNumber << std::endl;
                }
                currentEventData = RTLEventData{};
                inRecord = false;
            }
            continue;
        }

        // Parse key-value pair
        std::stringstream ss(line);
        std::string key, value;
        if (!(ss >> key >> value)) {
            throw std::runtime_error("Invalid line format at line " +
                                     std::to_string(lineNumber) + ": '" + line + "'");
        }

        // Check for extra content on the line
        std::string remaining;
        if (std::getline(ss, remaining) &&
            remaining.find_first_not_of(" \t") != std::string::npos) {
            throw std::runtime_error("Extra content after value at line " +
                                     std::to_string(lineNumber) + ": '" + line + "'");
        }

        if (key == "event") {
            if (inRecord) {
                // Started a new event before the previous one ended with a blank line?
                throw std::runtime_error("Unexpected 'event' key found within a record at line " +
                                     std::to_string(lineNumber) + ". Missing blank line?");
            }
            try {
                currentEventData.event = stringToRTLEvent(value);
                inRecord = true;
            } catch (const std::runtime_error& e) {
                 throw std::runtime_error(std::string(e.what()) + " at line " + std::to_string(lineNumber));
             }
        } else if (inRecord) {
            parseLine(key, value, currentEventData, lineNumber);
        } else {
            throw std::runtime_error("Orphan key-value pair found outside an event record at line " +
                                     std::to_string(lineNumber) + ": '" + line + "'");
        }
    }

    if (inRecord && currentEventData.event != RTLEvent::kUndefined) {
        events.push_back(currentEventData);
    }

    inputFile.close();
    return events;
}

}

#include "test_trace_reader.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace m3
{
    // Static helper functions to convert string names to appropriate types.
    static RtlHook StringToRtlHook(const std::string& hookName)
    {
        static const std::map<std::string, RtlHook> kHookMap =
        {
            {"kCreateMemop", RtlHook::kCreateMemop},
            {"kAddMemopAddress", RtlHook::kAddMemopAddress},
            {"kPerformLoad", RtlHook::kPerformLoad},
            {"kAddStoreData", RtlHook::kAddStoreData},
            {"kCompleteStore", RtlHook::kCompleteStore},
            {"kCommitMemop", RtlHook::kCommitMemop},
            {"kUpdateCacheLineState", RtlHook::kUpdateCacheLineState},
            {"kUpdateCacheLineData", RtlHook::kUpdateCacheLineData}
        };
        auto it = kHookMap.find(hookName);
        std::string errorMsg = "Unknown hook name: " + hookName;
        assert(it != kHookMap.end() && errorMsg.c_str());
        return it->second;
    }

    // Helper function to parse a single key-value line and update the hook data.
    static void ParseLine(const std::string& key, const std::string& value,
                                RtlHookData& currentHookData, int lineNumber)
    {
        try
        {
            if (key == "event")
            {
                currentHookData.event = StringToRtlHook(value);
            }
            else if (key == "hart_id")
            {
                currentHookData.hart_id = std::stoul(value);
            }
            else if (key == "rob_id")
            {
                currentHookData.rob_id = std::stoul(value);
            }
            else if (key == "rv_instruction")
            {
                currentHookData.rv_instruction = std::stoul(value);
            }
            else if (key == "memop_size")
            {
                currentHookData.memop_size = std::stoul(value);
            }
            else if (key == "address")
            {
                currentHookData.address = std::stoull(value);
            }
            else if (key == "memop_id")
            {
                currentHookData.memop_id = std::stoul(value);
            }
            else if (key == "load_rtl_data")
            {
                currentHookData.load_rtl_data = std::stoull(value);
            }
            else if (key == "store_data")
            {
                currentHookData.store_data = std::stoull(value);
            }
            else if (key == "timestamp")
            {
                currentHookData.timestamp = std::stoull(value);
            }
            else if (key == "coherence_state")
            {
                currentHookData.coherence_state = std::stoul(value);
            }
            else if (key == "way_id")
            {
                currentHookData.way_id = std::stoul(value);
            }
            else if (key == "cache_line_id")
            {
                currentHookData.cache_line_id = std::stoul(value);
            }
            else if (key == "tag")
            {
                currentHookData.tag = std::stoull(value);
            }
            else if (key == "is_load")
            {
       	    currentHookData.is_load = true;
            }
            else if (key == "is_store")
            {
       	    currentHookData.is_store = true;
            }
            else if (key == "is_amo")
            {
       	    currentHookData.is_amo = true;
            }
            else
            {
                std::string errorMsg = "Unknown key '" + key + "' at line " + std::to_string(lineNumber);
                assert(false && errorMsg.c_str());
            }
        }
        catch (const std::invalid_argument&)
        {
            std::string errorMsg = "Invalid numeric value '" + value + "' for key '" + key +
                                         "' at line " + std::to_string(lineNumber);
            assert(false && errorMsg.c_str());
        }
        catch (const std::out_of_range&)
        {
            std::string errorMsg ="Numeric value '" + value + "' out of range for key '" +
                                         key + "' at line " + std::to_string(lineNumber);
            assert(false && errorMsg.c_str());
        }
    }

    // Public Static Method Implementation.
    std::vector<RtlHookData> TestTraceReader::ParseFile(const std::string& filename)
    {
        std::ifstream inputFile(filename);
        std::string errorMsg = "Could not open trace file: " + filename;
        assert(inputFile.is_open() && errorMsg.c_str());

        std::vector<RtlHookData> hooks;
        std::string line;
        RtlHookData currentHookData{}; // Initialize with default values.
        bool inRecord = false;
        int lineNumber = 0;

        while (std::getline(inputFile, line))
        {
            lineNumber++;
            // Trim leading/trailing whitespace.
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

            if (line.empty())
            {
                // Blank line signifies the end of a record
                if (inRecord)
                {
                    if (currentHookData.event != RtlHook::kUndefined)
                    {
                        hooks.push_back(currentHookData);
                    }
                    else
                    {
                        std::cerr << "Warning: Skipping potentially incomplete record ending before line " << lineNumber << std::endl;
                    }
                    currentHookData = RtlHookData{};
                    inRecord = false;
                }
                continue;
            }

            // Parse key-value pair
            std::stringstream ss(line);
            std::string key, value;
            ss >> key >> value;
            bool validFormat = !ss.fail();
            std::string formatErrorMsg = "Invalid line format at line " + std::to_string(lineNumber) + ": '" + line + "'";
            assert(validFormat && formatErrorMsg.c_str());

            // Check for extra content on the line
            std::string remaining;
            if (std::getline(ss, remaining) && remaining.find_first_not_of(" \t") != std::string::npos)
            {
                std::string extraContentMsg = "Extra content after value in trace file at line " +
                                        std::to_string(lineNumber) + ": '" + line + "'";
                assert(false && extraContentMsg.c_str());
            }

            if (key == "event")
            {
                if (inRecord) {
                    std::string eventErrorMsg = "Unexpected 'event' key found within a record at line " +
                                        std::to_string(lineNumber) + ". Missing blank line?";
                    assert(false && eventErrorMsg.c_str());
                }
                currentHookData.event = StringToRtlHook(value);
                inRecord = true;
            }
            else if (inRecord)
            {
                ParseLine(key, value, currentHookData, lineNumber);
            }
            else
            {
                std::string orphanMsg = "Orphan key-value pair found outside an hook record at line " +
                                        std::to_string(lineNumber) + ": '" + line + "'";
                assert(false && orphanMsg.c_str());
            }
        }

        if (inRecord && currentHookData.event != RtlHook::kUndefined)
        {
            hooks.push_back(currentHookData);
        }

        inputFile.close();
        return hooks;
    }
}

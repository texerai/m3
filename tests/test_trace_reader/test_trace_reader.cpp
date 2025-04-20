#include "test_trace_reader.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace m3
{
    // Static helper function to trim leading/trailing whitespace.
    static inline std::string Trim(const std::string& s)
    {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){ return std::isspace(c); });
        if (wsfront == s.end()) return "";
        auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){ return std::isspace(c); }).base();
        return std::string(wsfront, wsback);
    }

    // Static helper function to convert string names to appropriate types.
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
        assert(it != kHookMap.end() && "Unknown hook name encountered");
        return it->second;
    }

    // Helper function to parse a single key-value line and update the hook data.
    static void ParseHookLine(const std::string& line, RtlHookData& hookData)
    {
        std::stringstream lineStream(line);
        std::string segment;

        while (std::getline(lineStream, segment, ';'))
        {
            segment = Trim(segment);
            if (segment.empty())
            {
                continue;
            }
            size_t spacePos = segment.find(' ');
            if (spacePos == std::string::npos || spacePos == 0 || spacePos == segment.length() - 1)
            {
                assert(false && "Malformed key-value segment found in trace line");
                continue;
            }
            std::string key = Trim(segment.substr(0, spacePos));
            std::string value = Trim(segment.substr(spacePos + 1));
            if (key.empty() || value.empty())
            {
                assert(false && "Empty key or value found in trace segment");
                continue;
            }
            bool keyProcessed = true;
            try
            {
                if (key == "event") { hookData.event = StringToRtlHook(value); }
                else if (key == "hart_id") { hookData.hart_id = std::stoul(value); }
                else if (key == "rob_id") { hookData.rob_id = std::stoul(value); }
                else if (key == "rv_instruction") { hookData.rv_instruction = std::stoll(value); }
                else if (key == "memop_size") { hookData.memop_size = std::stoul(value); }
                else if (key == "address") { hookData.address = std::stoull(value); }
                else if (key == "memop_id") { hookData.memop_id = std::stoul(value); }
                else if (key == "load_rtl_data") { hookData.load_rtl_data = std::stoull(value); }
                else if (key == "store_data") { hookData.store_data = std::stoull(value); }
                else if (key == "timestamp") { hookData.timestamp = std::stoull(value); }
                else if (key == "coherence_state") { hookData.coherence_state = std::stoul(value); }
                else if (key == "way_id") { hookData.way_id = std::stoul(value); }
                else if (key == "cache_line_id") { hookData.cache_line_id = std::stoul(value); }
                else if (key == "tag") { hookData.tag = std::stoull(value); }
                else if (key == "is_load")
                {
                    assert((value == "true" || value == "false") && "Invalid boolean value (expected 'true' or 'false')");
                    hookData.is_load = (value == "true");
                }
                else if (key == "is_store")
                {
                    assert((value == "true" || value == "false") && "Invalid boolean value (expected 'true' or 'false')");
                    hookData.is_store = (value == "true");
                }
                else if (key == "is_amo")
                {
                    assert((value == "true" || value == "false") && "Invalid boolean value (expected 'true' or 'false')");
                    hookData.is_amo = (value == "true");
                }
                else
                {
                    keyProcessed = false;
                }
            }
            catch (const std::invalid_argument&)
            {
                assert(false && "Invalid numeric or boolean format encountered");
                continue;
            }
            catch (const std::out_of_range&)
            {
                assert(false && "Numeric value out of range");
                continue;
            }
            catch (const std::runtime_error&)
            {
                assert(false && "Runtime error during string conversion helper");
                continue;
            }
            assert(keyProcessed && "Unknown key encountered in trace line");
        }
        assert(hookData.event != RtlHook::kUndefined && "Mandatory 'event' key missing or invalid on trace line");
    }

    // Public Static Method Implementation.
    std::vector<RtlHookData> TestTraceReader::ParseFile(const std::string& filename)
    {
        std::ifstream inputFile(filename);
        assert(inputFile.is_open() && "Failed to open trace file");

        std::vector<RtlHookData> hooks;
        std::string line;

        while (std::getline(inputFile, line))
        {
            line = Trim(line);
            if (line.empty())
            {
                continue;
            }
            RtlHookData currentHookData{};
            ParseHookLine(line, currentHookData);
            hooks.push_back(currentHookData);
        }

        inputFile.close();
        return hooks;
    }
}

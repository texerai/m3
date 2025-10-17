/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

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
    static bool StringToRtlHook(const std::string& hookName, RtlHook& outHook, std::string& error_message)
    {
        static const std::map<std::string, RtlHook> kHookMap =
        {
            {"kCreateMemop", RtlHook::kCreateMemop},
            {"kAddMemopAddress", RtlHook::kAddMemopAddress},
            {"kPerformLoad", RtlHook::kPerformLoad},
            {"kAddStoreData", RtlHook::kAddStoreData},
            {"kCompleteStore", RtlHook::kCompleteStore},
            {"kCommitMemop", RtlHook::kCommitMemop},
            {"kFlushRob", RtlHook::kFlushRob},
            {"kBranchMispredict", RtlHook::kBranchMispredict},
            {"kBranchResolve", RtlHook::kBranchResolve},
            {"kBranchPredictionStart", RtlHook::kBranchPredictionStart},
            {"kUpdateCacheLineState", RtlHook::kUpdateCacheLineState},
            {"kUpdateCacheLineData", RtlHook::kUpdateCacheLineData}
        };
        auto it = kHookMap.find(hookName);
        if (it == kHookMap.end())
        {
            error_message = "Unknown hook name encountered: '" + hookName + "'";
            return false;
        }
        outHook = it->second;
        return true;
    }

    // Helper function to parse a single key-value line and update the hook data.
    static bool ParseHookLine(const std::string& line, int lineNumber, RtlHookData& hookData, std::string& error_message)
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
            std::string key, value;

            if (spacePos == std::string::npos || spacePos == 0 || spacePos == segment.length() - 1)
            {
                error_message = "Malformed key-value segment '" + segment +
                                "' at line " + std::to_string(lineNumber);
                return false;
            }

            key = Trim(segment.substr(0, spacePos));
            value = Trim(segment.substr(spacePos + 1));

            if (key.empty() || value.empty())
            {
                error_message = "Empty key or value in segment '" + segment +
                                "' at line " + std::to_string(lineNumber);
                return false;
            }

            bool keyProcessed = true;
            try
            {
                if (key == "event")
                {
                    if (!StringToRtlHook(value, hookData.event, error_message))
                    {
                        error_message += " at line " + std::to_string(lineNumber);
                        return false;
                    }
                }
                else if (key == "hart_id") { hookData.hart_id = std::stoul(value); }
                else if (key == "rob_id") { hookData.rob_id = std::stoul(value); }
                else if (key == "rv_instruction") { hookData.rv_instruction = std::stoll(value); }
                else if (key == "branch_mask") { hookData.branch_mask = std::stoul(value); }
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
                    if (value == "true") hookData.is_load = true;
                    else if (value == "false") hookData.is_load = false;
                    else
                    {
                        error_message = "Invalid boolean value '" + value + "' for key 'is_load' at line " + std::to_string(lineNumber);
                        return false;
                    }
                }
                else if (key == "is_store")
                {
                    if (value == "true") hookData.is_store = true;
                    else if (value == "false") hookData.is_store = false;
                    else
                    {
                        error_message = "Invalid boolean value '" + value + "' for key 'is_store' at line " + std::to_string(lineNumber);
                        return false;
                    }
                }
                else if (key == "is_amo")
                {
                    if (value == "true") hookData.is_amo = true;
                    else if (value == "false") hookData.is_amo = false;
                    else
                    {
                        error_message = "Invalid boolean value '" + value + "' for key 'is_amo' at line " + std::to_string(lineNumber);
                        return false;
                    }
                }
                else
                {
                    keyProcessed = false;
                }
            }
            catch (const std::invalid_argument&)
            {
                error_message = "Invalid numeric format for value '" + value + "' with key '" + key +
                                "' at line " + std::to_string(lineNumber);
                return false;
            }
            catch (const std::out_of_range&)
            {
                error_message = "Numeric value '" + value + "' out of range for key '" + key +
                                "' at line " + std::to_string(lineNumber);
                return false;
            }
            catch (const std::runtime_error& e)
            {
                error_message = "Runtime error processing key '" + key + "' at line " +
                                std::to_string(lineNumber) + ": " + e.what();
                return false;
            }

            if (!keyProcessed)
            {
                error_message = "Unknown key '" + key + "' encountered in segment '" + segment +
                                "' at line " + std::to_string(lineNumber);
                return false;
            }
        }

        if (hookData.event == RtlHook::kUndefined)
        {
             error_message = "Mandatory 'event' key missing or invalid on line " + std::to_string(lineNumber);
             return false;
        }

        return true;
    }

    // Public Static Method Implementation.
    bool TestTraceReader::ParseFile(const std::string& filename, std::vector<RtlHookData>& hooks, std::string& error_message)
    {
        std::ifstream inputFile(filename);
        if (!inputFile.is_open())
        {
            error_message = "Failed to open trace file: " + filename;
            return false;
        }

        hooks.clear();
        std::string line;
        int lineNumber = 0;

        while (std::getline(inputFile, line))
        {
            lineNumber++;
            line = Trim(line);
            if (line.empty())
            {
                continue;
            }

            RtlHookData currentHookData{};

            if (!ParseHookLine(line, lineNumber, currentHookData, error_message))
            {
                inputFile.close();
                return false;
            }

            hooks.push_back(currentHookData);
        }

        inputFile.close();
        error_message = "";
        return true;
    }
} // namespace m3

/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef TEST_TRACE_READER_H_
#define TEST_TRACE_READER_H_

#include <string>
#include <vector>

#include "rtl_hook.h"

namespace m3
{
    class TestTraceReader
    {
    public:
        // Parses the given trace file and populates a vector of RtlHookData.
        static bool ParseFile(const std::string& filename, std::vector<RtlHookData>& hooks, std::string& error_message);
    };
}

#endif // TEST_TRACE_READER_H_

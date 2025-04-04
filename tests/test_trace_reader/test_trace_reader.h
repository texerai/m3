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
        // Parses the given file and returns a vector of RtlHookData objects.
        static std::vector<RtlHookData> ParseFile(const std::string& filename);
    };
}

#endif // TEST_TRACE_READER_H_

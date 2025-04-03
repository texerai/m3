#ifndef TEST_TRACE_READER_H_
#define TEST_TRACE_READER_H_

#include <stdexcept>
#include <string>
#include <vector>

#include "rtl_event.h"

namespace m3 {

class RTLEventParser {
public:
    // Parses the given file and returns a vector of RTLEventData objects.
    // Throws std::runtime_error on file open errors or parsing errors.
    static std::vector<RTLEventData> parseFile(const std::string& filename);

private:
    // Helper functions to convert string names to appropriate types.
    static RTLEvent stringToRTLEvent(const std::string& eventName);
    static MemopType stringToMemopType(const std::string& typeName);
    static MemopFormat stringToMemopFormat(const std::string& formatName);

    // Helper function to parse a single key-value line and update the event data.
    static void parseLine(const std::string& key, const std::string& value,
                          RTLEventData& currentEventData, int lineNumber);
};

}

#endif // TEST_TRACE_READER_H_

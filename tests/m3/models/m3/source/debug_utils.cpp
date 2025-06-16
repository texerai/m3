#include "debug_utils.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

namespace debug {

std::string to_hex(int value) {
    std::ostringstream oss;
    // Channel only used once, no need to recover state with std::dec
    oss << std::hex << value;
    return oss.str();
}

std::string getVerbosityPrefix(const VerbosityLevel level) {
    switch (level) {
        case VerbosityLevel::None:    return "[SILENT]";
        case VerbosityLevel::Error:   return "[ERROR]";
        case VerbosityLevel::Warning: return "[WARNING]";
        case VerbosityLevel::Low:     return "[LOW]";
        case VerbosityLevel::Medium:  return "[MEDIUM]";
        case VerbosityLevel::Debug:   return "[DEBUG]";
        default:                      return "[LOG]";
    }
}

Settings::Settings() : verbosity(VerbosityLevel::None), execMode(ExecutionMode::Production) {}

Settings& Settings::getInstance() {
    static Settings instance;
    return instance;
}

void Settings::setVerbosity(VerbosityLevel level) { verbosity = level; }
void Settings::setMode(ExecutionMode mode) { execMode = mode; }

VerbosityLevel Settings::getVerbosity() const { return verbosity; }
ExecutionMode Settings::getExecutionMode() const { return execMode; }

void Settings::log(const std::string& msg,
    VerbosityLevel level,
    const char* file,
    int line,
    const char* function) const {
    if (level <= this->verbosity) {
        std::cout << getVerbosityPrefix(level) << " " << file << ":" << line << " "
        << function << "() - " << msg << std::endl;
    }
}

void Settings::handleAssertion(bool condition,
                                const std::string& exprStr,
                                const std::string& msg,
                                const char* file,
                                int line,
                                const char* function) const {
    if (condition) return;

    std::ostringstream oss;
    oss << "[Assertion] "
        << file << ":" << line << " "
        << function << "() - "
        << " (" << exprStr << ") "
        << msg;

    switch (execMode) {
        case ExecutionMode::Testing:
            std::cerr << oss.str() << std::endl;
            std::abort();
            //assert(false); // Halt in testing
            break;

        case ExecutionMode::Production:
            std::cerr << oss.str() << std::endl;
            break;
    }
}

} // namespace debug
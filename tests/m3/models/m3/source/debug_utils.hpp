#pragma once

#include <string>

#include "fmt/core.h"

namespace debug {

// Seting a verbosity level will display the messages with a lower or equal level
enum class VerbosityLevel {
    None = 0,    // Quiet: No output at all
    Error = 1,   // Critical errors
    Warning = 2, // Warnings
    Low = 3,     // Basic messages
    Medium = 4,  // Medium verbosity
    Debug = 5    // Deep code state
};

enum class ExecutionMode {
    Production = 0, // 
    Testing = 2     // 
};

// Convert VerbosityLevel to string
inline std::string to_string(VerbosityLevel level) {
    switch (level) {
        case VerbosityLevel::None:    return "None";
        case VerbosityLevel::Error:   return "Error";
        case VerbosityLevel::Warning: return "Warning";
        case VerbosityLevel::Low:     return "Low";
        case VerbosityLevel::Medium:  return "Medium";
        case VerbosityLevel::Debug:   return "Debug";
        default:                      return "Unknown";
    }
}

// Convert ExecutionMode to string
inline std::string to_string(ExecutionMode mode) {
    switch (mode) {
        case ExecutionMode::Production: return "Production";
        case ExecutionMode::Testing:    return "Testing";
        default:                        return "Unknown";
    }
}

class Settings {
public:
    // Singleton access
    static Settings& getInstance();

    // Public constructor for per-instance use
    Settings();

    void setVerbosity(VerbosityLevel level);
    void setMode(ExecutionMode mode);

    VerbosityLevel getVerbosity() const;
    ExecutionMode getExecutionMode() const;

    void log(const std::string& msg,
        VerbosityLevel level,
        const char* file,
        int line,
        const char* function) const;

    void handleAssertion(bool condition,
        const std::string& exprStr,
        const std::string& message,
        const char* file,
        int line,
        const char* function) const;

private:
    VerbosityLevel verbosity;
    ExecutionMode execMode;
}; // class Settings

std::string getVerbosityPrefix(const VerbosityLevel level);

std::string to_hex(int value);

} // namespace debug

#define DEBUG_LOG(msg, level) \
    debug::Settings::getInstance().log(msg, level, __FILE__, __LINE__, __func__)

#define DEBUG_ASSERT(condition, msg) \
    debug::Settings::getInstance().handleAssertion((condition), #condition, (msg), __FILE__, __LINE__, __func__)


// fmt formatters
template <> class fmt::formatter<debug::VerbosityLevel> {
public:
    constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin(); // No custom format options
    }

    template <typename Context>
    constexpr auto format(debug::VerbosityLevel const& level, Context& ctx) const {
    return fmt::format_to(ctx.out(), "{}", debug::to_string(level));
    }
};

template <> class fmt::formatter<debug::ExecutionMode> {
public:
    constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
    }

    template <typename Context>
    constexpr auto format(debug::ExecutionMode const& mode, Context& ctx) const {
    return fmt::format_to(ctx.out(), "{}", debug::to_string(mode));
    }
};
    
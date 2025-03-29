#pragma once

#include <set>
#include <string>
#include <cstdio>
#include <cstdarg>

class Notifier {
  protected:
    static void trace_int(uint64_t iid, const std::string &text);
    static void fail_int(const std::string &text);
    static void warn_int(const std::string &text);
    static void info_int(const std::string &text);

    static std::set<uint64_t> tracing;

    static std::string format_string(const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        return std::string(buffer);
    }

  public:
    static void add_tracing(uint64_t iid) { tracing.insert(iid); }

    template <typename S, typename... Args>
    static void trace(uint64_t iid, const S &format, Args &&...args) {
        if (tracing.empty() || tracing.count(iid)) {
            trace_int(iid, format_string(format, args...));
        }
    }

    template <typename S, typename... Args>
    static void fail(const S &format, Args &&...args) {
        fail_int(format_string(format, args...));
    }

    template <typename S, typename... Args>
    static void warn(const S &format, Args &&...args) {
        warn_int(format_string(format, args...));
    }

    template <typename S, typename... Args>
    static void info(const S &format, Args &&...args) {
        info_int(format_string(format, args...));
    }
};

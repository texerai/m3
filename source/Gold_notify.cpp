#include "Gold_notify.hpp"
#include <cstdio>

std::set<uint64_t> Gold_nofity::tracing;

void Gold_nofity::trace_int(uint64_t iid, const std::string &msg) { printf("TRACE: iid:%lu %s\n", iid, msg.c_str()); }

void Gold_nofity::fail_int(const std::string &msg) { printf("FAIL:%s\n", msg.c_str()); }

void Gold_nofity::warn_int(const std::string &msg) { printf("WARN:%s\n", msg.c_str()); }

void Gold_nofity::info_int(const std::string &msg) { printf("INFO:%s\n", msg.c_str()); }

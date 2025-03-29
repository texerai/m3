#include "notifier.hpp"
#include <cstdio>

std::set<uint64_t> Notifier::tracing;

void Notifier::trace_int(uint64_t iid, const std::string &msg) { printf("TRACE: iid:%lu %s\n", iid, msg.c_str()); }

void Notifier::fail_int(const std::string &msg) { printf("FAIL:%s\n", msg.c_str()); }

void Notifier::warn_int(const std::string &msg) { printf("WARN:%s\n", msg.c_str()); }

void Notifier::info_int(const std::string &msg) { printf("INFO:%s\n", msg.c_str()); }

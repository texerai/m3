#include "rvamo_utils.h"

uint64_t RiscV_AMO::execute(m3::AmoType type, size_t size_in_bytes, uint64_t mem_value, uint64_t st_value) {
    bool is_32 = (size_in_bytes == 4);

    switch (type) {
        case m3::AmoType::kAdd:
            return is_32
                ? static_cast<uint32_t>(mem_value) + static_cast<uint32_t>(st_value)
                : mem_value + st_value;

        case m3::AmoType::kSwap:
            return is_32 ? static_cast<uint32_t>(st_value) : st_value;

        case m3::AmoType::kXor:
            return is_32
                ? static_cast<uint32_t>(mem_value) ^ static_cast<uint32_t>(st_value)
                : mem_value ^ st_value;

        case m3::AmoType::kAnd:
            return is_32
                ? static_cast<uint32_t>(mem_value) & static_cast<uint32_t>(st_value)
                : mem_value & st_value;

        case m3::AmoType::kOr:
            return is_32
                ? static_cast<uint32_t>(mem_value) | static_cast<uint32_t>(st_value)
                : mem_value | st_value;

        case m3::AmoType::kMin:
            return is_32
                ? static_cast<uint32_t>(std::min(
                      static_cast<int32_t>(mem_value), static_cast<int32_t>(st_value)))
                : static_cast<uint64_t>(std::min(
                      static_cast<int64_t>(mem_value), static_cast<int64_t>(st_value)));

        case m3::AmoType::kMax:
            return is_32
                ? static_cast<uint32_t>(std::max(
                      static_cast<int32_t>(mem_value), static_cast<int32_t>(st_value)))
                : static_cast<uint64_t>(std::max(
                      static_cast<int64_t>(mem_value), static_cast<int64_t>(st_value)));

        case m3::AmoType::kMinu:
            return is_32
                ? static_cast<uint32_t>(std::min(
                      static_cast<uint32_t>(mem_value), static_cast<uint32_t>(st_value)))
                : std::min(mem_value, st_value);

        case m3::AmoType::kMaxu:
            return is_32
                ? static_cast<uint32_t>(std::max(
                      static_cast<uint32_t>(mem_value), static_cast<uint32_t>(st_value)))
                : std::max(mem_value, st_value);

        case m3::AmoType::kUndefined:
        default:
            return 0;
    }
}
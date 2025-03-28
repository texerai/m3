/*
 * Copyright (c) 2023 micro architecture santa cruz
 * and texer.ai. all rights reserved.
 */
#ifndef MEMOP_INFO_H_
#define MEMOP_INFO_H_

// C++ libraries.
#include <stdint.h>

namespace m3
{
    enum class MemopFormat
    {
        kUndefined,
        kInteger,
        kFloat
    };

    enum class MemopType
    {
        kUndefined,
        kLoad,
        kStore,
        kAmo
    };

    // Structure to keep track of memory operation.
    struct MemopInfo
    {
        uint64_t m3id;
        uint64_t size = 0;
        uint64_t address = 0;
        uint64_t load_model_data = 0;
        uint64_t load_rtl_data = 0;
        uint32_t load_dest_reg = 0;
        uint64_t store_data = 0;
        uint32_t stq_id = 0;
        uint32_t rob_id = 0;
        uint32_t instruction = 0;
        uint64_t completed_time = 0;
        MemopType memop_type = MemopType::kUndefined;
        bool check_failed = false;
        bool is_performed = false;
        bool store_succeeded = false;
        bool committed = false;
        bool is_amo = false;
        bool is_address_valid = false;
        bool is_data_valid = false;
        bool is_just_created = true;

        bool CanBePerformed() const
        {
            return (is_address_valid && is_data_valid)
                && (memop_type == MemopType::kStore)
                && (memop_type != MemopType::kAmo);
        }

        void Invalidate()
        {
            is_amo = false;
            is_address_valid = false;
            is_data_valid = false;
            check_failed = false;
            is_performed = false;
            store_succeeded = false;
            committed = false;
            address = 0;
        }
    };
}
#endif
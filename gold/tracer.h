/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

// C++ libraries.
#include <stdint.h>
#include <string>

namespace m3
{
    enum class MemoryEvent
    {
        kUndefined,
        kMemopCreated,
        kLoadPerform,
        kLoadPerformFailed,
        kStoreLocalPerform,
        kStoreGlobalPerform,
        kMemopCommited,
        kMemopNuked
    };

    enum class TraceFileFormat
    {
        kUndefined,
        kText,
        kJson
    };

    class Tracer
    {
    public:
        void Init(uint32_t buffer_size);
        void RecordEvent(MemoryEvent memop_event, uint32_t hart_id,
            uint64_t address, uint64_t model_data, uint64_t rtl_data,
            uint64_t memop_id, uint32_t memop_type, uint64_t timestamp);
        void SaveTrace(const std::string& file_name, TraceFileFormat format);
        ~Tracer();

    private:
        struct TracerImpl;
        TracerImpl* pimpl_ = nullptr;
    };
}

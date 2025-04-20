/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#include "utils/tracer.h"

// C++ libraries.
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

namespace m3
{
    // Constants.
    static const std::map<MemoryEvent, const char*> kMapEventToString = {
        { MemoryEvent::kMemopCreated, "MEMOP_IN" },
        { MemoryEvent::kLoadPerform, "LOAD_PERFORM" },
        { MemoryEvent::kLoadPerformFailed, "LOAD_PERFORM_FAILED" },
        { MemoryEvent::kStoreLocalPerform, "STORE_LOCAL_PERFORM" },
        { MemoryEvent::kStoreGlobalPerform, "STORE_GLOBAL_PERFORM" },
        { MemoryEvent::kMemopCommited, "MEMOP_COMMIT" },
        { MemoryEvent::kMemopNuked, "MEMOP_NUKED" }
    };

    // Const error messages.
    static const char* kStringErrorTracerNoInit = "Error: Tracer is not initialized.";

    // Internally bind static helper functions go here.
    namespace aux
    {
        struct MemoryEventData
        {
            MemoryEvent memop_event = MemoryEvent::kUndefined;
            uint32_t hart_id    = 0;
            uint64_t address    = 0;
            uint64_t model_data = 0;
            uint64_t rtl_data   = 0;
            uint64_t memop_id   = 0;
            uint32_t memop_type = 0;
            uint64_t timestamp  = 0;
        };

        static std::string ToJson(const MemoryEventData& data, bool is_last)
        {
            std::string memop_type;
            switch (data.memop_type)
            {
                case 0:
                    memop_type = "undefined";
                    break;
                case 1:
                    memop_type = "load";
                    break;
                case 2:
                    memop_type = "store";
                    break;
                case 3:
                    memop_type = "amo";
                    break;
                default:
                    assert(false);
                    break;
            }

            std::stringstream r;
            r << "    {\n";
            r << "      \"event\" : \"" << kMapEventToString.at(data.memop_event) << "\",\n";
            r << "      \"hart_id\" : \"" << data.hart_id << "\",\n";
            r << "      \"address\" : \"" << std::hex << data.address << "\",\n";
            r << "      \"model_data\" : \"" << std::dec << data.model_data << "\",\n";
            r << "      \"rtl_data\" : \"" << data.rtl_data << "\",\n";
            r << "      \"memop_id\" : \"" << data.memop_id << "\",\n";
            r << "      \"memop_type\" : \"" << memop_type << "\",\n";
            r << "      \"timestamp\" : \"" << data.timestamp << "\"\n";
            r << "    }";
            if (!is_last)
            {
                r << ",";
            }
            r << "\n";
            return r.str();
        }
    }

    // Tracer pimpl.
    struct Tracer::TracerImpl
    {
        uint64_t time = 0;
        uint32_t buffer_size = 0;
        std::deque<aux::MemoryEventData> buffer;
    };

    void Tracer::Init(uint32_t buffer_size)
    {
        if (pimpl_ == nullptr)
        {
            pimpl_ = new TracerImpl();
        }
        pimpl_->buffer_size = buffer_size;
    }

    void Tracer::RecordEvent
    (
        MemoryEvent memop_event,
        uint32_t hart_id,
        uint64_t address,
        uint64_t model_data,
        uint64_t rtl_data,
        uint64_t memop_id,
        uint32_t memop_type,
        uint64_t timestamp
    )
    {
        // Check init.
        if (pimpl_ == nullptr)
        {
            std::cerr << kStringErrorTracerNoInit << std::endl;
            return;
        }

        // Pack record.
        aux::MemoryEventData d;
        d.memop_event = memop_event;
        d.hart_id     = hart_id;
        d.address     = address;
        d.model_data  = model_data;
        d.rtl_data    = rtl_data;
        d.memop_id    = memop_id;
        d.memop_type  = memop_type;
        d.timestamp   = timestamp;

        // Add record.
        auto& buffer = pimpl_->buffer;
        buffer.push_back(d);
        if (buffer.size() > pimpl_->buffer_size)
        {
            buffer.pop_front();
        }
    }

    void Tracer::SaveTrace(const std::string& file_name, TraceFileFormat format)
    {
        std::cout << "Saving trace to " << file_name << std::endl;
        std::ofstream out_file(file_name);
        if (format == TraceFileFormat::kJson)
        {
            uint32_t count = 0;
            out_file << "{\n";
            out_file << "  \"records\" : [\n";
            for (const auto& event_data : pimpl_->buffer)
            {
                ++count;
                bool is_last = (count == pimpl_->buffer.size());
                out_file << aux::ToJson(event_data, is_last);
            }
            out_file << "  ]\n";
            out_file << "}\n";
        }
        // Print text by default.
        else
        {
            for (const auto& event_data : pimpl_->buffer)
            {
                out_file << kMapEventToString.at(event_data.memop_event) << std::endl;
                out_file << event_data.timestamp << ":";
            }
        }
        out_file.close();
    }

    Tracer::~Tracer()
    {
        if (pimpl_ != nullptr)
        {
            delete pimpl_;
        }
    }
};

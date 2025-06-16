/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef BOOM_M3_T_H_
#define BOOM_M3_T_H_

// C++ libraries.
#include <memory>
#include <stdint.h>
#include <string>

// Forward declarations.
class dromajo_t;

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class boom_m3_t
    {
    public:
        void init(uint32_t ncores, std::shared_ptr<dromajo_t> core_model_ptr);
        void register_event(const RTLEventData& data);
        bool serve_registered_events();
        void close();

    private:
        struct boom_m3_impl;
        boom_m3_impl* pimpl_ = nullptr;
    };

}
#endif // BOOM_M3_T_H_
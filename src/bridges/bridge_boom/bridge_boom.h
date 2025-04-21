/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef BRIDGE_BOOM_T_H_
#define BRIDGE_BOOM_T_H_

// C++ libraries.
#include <functional>
#include <stdint.h>
#include <string>

namespace m3
{
    // Forward declarations.
    struct RtlHookData;

    class BridgeBoom
    {
    public:
        void Init(uint32_t ncores);
        void RegisterEvent(const RtlHookData& data);
        bool ServeRegisteredEvents();
        void Close();

        // Required callbacks.
        void SetCallbackGetByte(std::function<uint8_t(uint64_t)> get_byte_function);
        void SetCallbackUpdateReg(std::function<void(uint32_t, uint32_t, uint64_t, bool)> update_proc_register);

    private:
        struct BridgeBoomImpl;
        BridgeBoomImpl* pimpl_ = nullptr;
    };

}
#endif // BRIDGE_BOOM_T_H_

/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef BRIDGE_BOOM_T_H_
#define BRIDGE_BOOM_T_H_

// C++ libraries.
#include <functional>
#include <memory>
#include <stdint.h>
#include <string>

namespace m3
{
    // Forward declarations.
    struct RtlHookData;

    class BridgeBoom
    {
    public:
        BridgeBoom();
        ~BridgeBoom();

        void Init(uint32_t ncores);
        void RegisterEvent(const RtlHookData& data);

        // Declares [begin, end) as an uncached device region in addition to
        // the SoC's fixed ones: loads from it are not checked and stores to
        // it perform as soon as they complete. The harness uses it for the
        // HTIF tohost/fromhost words, which the host writes directly.
        void AddDeviceRegion(uint64_t begin, uint64_t end);

        // Serves every registered event. Returns false if any of them
        // detected a verification failure (e.g. a load committing with
        // data that does not match the model).
        bool ServeRegisteredEvents();
        void Close();

        // Prints how much the bridge and the model are tracking right now.
        void DumpStatistics() const;

        // Required callbacks.
        void SetCallbackGetByte(std::function<uint8_t(uint64_t)> get_byte_function);
        void SetCallbackUpdateReg(std::function<void(uint32_t, uint32_t, uint64_t, bool)> update_proc_register);

    private:
        struct BridgeBoomImpl;
        std::unique_ptr<BridgeBoomImpl> pimpl_;
    };

}
#endif // BRIDGE_BOOM_T_H_

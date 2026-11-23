/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#ifndef BRIDGE_XIANGSHAN_H_
#define BRIDGE_XIANGSHAN_H_

// C++ libraries.
#include <functional>
#include <memory>
#include <stdint.h>

namespace m3
{
    // Events reported by the XiangShan hooks (xiangshan.m3.M3Event). rob is
    // a robIdx packed as flag << 31 | value.
    enum class XsEvent : uint8_t
    {
        kUndefined   = 0,
        kLoadAlloc   = 1,   // idx = lqIdx, rob
        kStoreAlloc  = 2,   // idx = sqIdx, rob
        kRedirect    = 3,   // rob = redirect robIdx, aux = flush the redirecting op too
        kStoreAddr   = 5,   // idx = sqIdx, rob, addr = paddr, size = bytes, aux = device
        kStoreData   = 6,   // idx = sqIdx, data
        kLoadWb      = 7,   // idx = lqIdx, rob, addr = paddr, size = bytes, data, aux = device
        kRobCommit   = 8,   // rob = a committed load or store
        kToSbuffer   = 10,  // idx = sqIdx, addr = paddr
        kSbufferDone = 11,  // aux = sbuffer line, addr = line paddr, data = 1 if handed to the miss queue
        kAtomic      = 12,  // addr, size = bytes | 0x80 no write | 0x40 no read, data = written, aux = old value
        kMmioStore   = 13,  // idx = sqIdx, addr = paddr
        kSbufferEnq  = 14,  // aux = sbuffer line, addr = paddr
        kStoreRefill = 15   // addr = line paddr: the stores the miss queue holds for it reach the cache
    };

    struct XsEventData
    {
        XsEvent event = XsEvent::kUndefined;
        uint32_t hart_id = 0;
        uint32_t idx = 0;
        uint32_t rob = 0;
        uint64_t address = 0;
        uint32_t size = 0;
        uint64_t data = 0;
        uint64_t aux = 0;
        uint64_t timestamp = 0;
    };

    class BridgeXiangShan
    {
    public:
        BridgeXiangShan();
        ~BridgeXiangShan();

        void Init(uint32_t ncores);
        void RegisterEvent(const XsEventData& data);

        // Serves the events of one cycle in priority order. Returns false
        // if a check failed in this cycle.
        bool ServeRegisteredEvents();

        void AddDeviceRegion(uint64_t begin, uint64_t end);
        void SetCallbackGetByte(std::function<uint8_t(uint64_t)> get_byte_function);
        void DumpStatistics() const;
        uint64_t ErrorCount() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };
}

#endif // BRIDGE_XIANGSHAN_H_

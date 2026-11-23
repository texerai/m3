/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#include "bridge_xiangshan.h"

// C++ libraries.
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

// Local libraries.
#include "m3/memory.hpp"
#include "m3/memory_marionette.hpp"
#include "utils/rvutils.h"

namespace m3
{
    namespace
    {
        // Same-cycle ordering. A XiangShan load writes back two cycles after
        // it read the cache and queried the store queue, so nothing that
        // happened in the writeback cycle was visible to it: loads perform
        // first. Store addresses and data come before commits, a commit
        // before the store leaves the store queue, and the store queue before
        // the sbuffer. Redirects run after everything that can still name a
        // flushed op, and allocation runs last because a queue entry freed in
        // this cycle may be reused by it.
        const std::map<XsEvent, int> kPriority = {
            { XsEvent::kLoadWb,      9 },
            { XsEvent::kStoreAddr,   8 },
            { XsEvent::kStoreData,   8 },
            { XsEvent::kRobCommit,   7 },
            { XsEvent::kAtomic,      6 },
            { XsEvent::kToSbuffer,   5 },
            { XsEvent::kMmioStore,   5 },
            { XsEvent::kSbufferDone, 4 },
            { XsEvent::kStoreRefill, 4 },
            { XsEvent::kSbufferEnq,  3 },
            { XsEvent::kRedirect,    2 },
            { XsEvent::kLoadAlloc,   1 },
            { XsEvent::kStoreAlloc,  1 },
        };

        const bool kDebug = std::getenv("M3XS_DEBUG") != nullptr;
        constexpr uint64_t kLineMask = ~uint64_t(63);
        constexpr uint64_t kSbufferWordMask = ~uint64_t(15);

        // robIdx packed as flag << 31 | value; the ROB's circular order.
        bool RobAfter(uint32_t a, uint32_t b)
        {
            bool flag_differs = ((a ^ b) >> 31) != 0;
            return flag_differs ^ ((a & 0x7fffffffu) > (b & 0x7fffffffu));
        }

        // Events that a cycle must serve in ROB order.
        bool RobOrdered(XsEvent e)
        {
            return e == XsEvent::kLoadAlloc || e == XsEvent::kStoreAlloc || e == XsEvent::kRobCommit;
        }

        struct Op
        {
            uint64_t m3id = 0;
            uint32_t rob = 0;
            uint32_t idx = 0;
            bool is_store = false;
            uint64_t address = 0;
            uint64_t sbuffer_address = 0;  // 16B-aligned, as the store queue sends it
            uint32_t size = 0;
            uint64_t store_data = 0;
            uint64_t rtl_data = 0;
            uint64_t model_data = 0;
            bool addr_valid = false;
            bool data_valid = false;
            bool performed = false;
            bool check_failed = false;
            bool committed = false;
            bool is_device = false;
        };

        struct Hart
        {
            // Loads from allocation to commit, stores from allocation until
            // they leave the store queue; keyed (and so ordered) by m3 id.
            std::map<uint64_t, Op> live;
            // Committed stores on their way from the store queue into an
            // sbuffer line, in store queue order.
            std::deque<Op> to_sbuffer;
            std::unordered_map<uint32_t, std::vector<uint64_t>> sbuffer;      // line -> m3 ids
            std::unordered_map<uint64_t, std::vector<uint64_t>> miss_queue;   // line paddr -> m3 ids
            // Stores the store queue pre-committed (it may drain a store at
            // the ROB head before the ROB retires it): robIdx of each, until
            // the ROB commit arrives.
            std::multiset<uint32_t> precommitted;
            bool have_last_alloc = false;
            uint32_t last_alloc_rob = 0;
            uint64_t last_alloc_id = 0;
        };

        struct AddressRange
        {
            uint64_t begin;
            uint64_t end;
        };

        // Silences m3's own narration (nuke prints every removed entry).
        class Quiet
        {
        public:
            Quiet() : saved_(kDebug ? nullptr : std::cout.rdbuf(sink_.rdbuf())) {}
            ~Quiet()
            {
                if (saved_)
                    std::cout.rdbuf(saved_);
            }

        private:
            std::ostringstream sink_;
            std::streambuf* saved_;
        };
    }

    struct BridgeXiangShan::Impl
    {
        Memory mem;
        std::vector<MemoryMarionette> cores;
        std::vector<Hart> harts;
        std::vector<XsEventData> pending;
        std::vector<AddressRange> devices;
        uint64_t errors = 0;
        uint64_t loads_checked = 0;
        uint64_t stores_performed = 0;
        uint64_t atomics = 0;
        uint64_t perform_mismatches = 0;
        uint64_t redirects = 0;
        uint64_t flushed_ops = 0;
        uint64_t loads_without_wb = 0;
        uint64_t sbuffer_misses = 0;
        uint64_t store_precommits = 0;
        std::map<std::string, uint64_t> warnings;
        size_t max_tracked = 0;

        void Warn(const XsEventData& e, const std::string& what)
        {
            if (warnings[what]++ < 5)
                std::cout << "M3XS WARNING: " << what << " (hart " << e.hart_id << ", idx " << e.idx << ", rob 0x"
                          << std::hex << e.rob << ", addr 0x" << e.address << std::dec << ", cycle " << e.timestamp
                          << ")\n";
        }

        bool IsDevice(uint64_t address) const
        {
            for (const auto& r : devices)
                if (address >= r.begin && address < r.end)
                    return true;
            return false;
        }

        void Fail(const XsEventData& e, const Op& op, const std::string& what, const std::string& m3_error)
        {
            ++errors;
            std::cout << "M3XS ERROR: " << what << " (hart " << e.hart_id << ", iid " << op.m3id << ", rob 0x" << std::hex
                      << op.rob << ", idx 0x" << op.idx << ", cycle " << std::dec << e.timestamp << ")\n"
                      << "  DUT: 0x" << std::hex << op.rtl_data << ", Model: 0x" << op.model_data << ", Address: 0x"
                      << op.address << std::dec << ", size " << op.size << "\n";
            if (!m3_error.empty())
                std::cout << "  m3: " << m3_error << "\n";
        }

        Op* FindLive(uint32_t hart_id, bool is_store, uint32_t idx)
        {
            for (auto& [id, op] : harts[hart_id].live)
                if (op.is_store == is_store && op.idx == idx)
                    return &op;
            return nullptr;
        }

        void Alloc(const XsEventData& e, bool is_store)
        {
            auto& h = harts[e.hart_id];
            // robIdx order is defined within one ROB lap only, i.e. while the
            // previous allocation is still uncommitted (a long run of non-memory
            // instructions can wrap the ROB between two allocations).
            auto last = h.have_last_alloc ? h.live.find(h.last_alloc_id) : h.live.end();
            if (last != h.live.end() && !last->second.committed && RobAfter(h.last_alloc_rob, e.rob))
                Warn(e, "allocation older than the previous one");
            Op op;
            op.m3id = static_cast<uint64_t>(cores[e.hart_id].inorder());
            h.have_last_alloc = true;
            h.last_alloc_rob = e.rob;
            h.last_alloc_id = op.m3id;
            op.rob = e.rob;
            op.idx = e.idx;
            op.is_store = is_store;
            cores[e.hart_id].set_type(Inst_id(op.m3id), is_store ? Mem_op::Store : Mem_op::Load);
            if (kDebug)
                std::cout << "m3xs: hart " << e.hart_id << (is_store ? " sq " : " lq ") << e.idx << " alloc iid " << op.m3id
                          << " rob 0x" << std::hex << e.rob << std::dec << " @" << e.timestamp << "\n";
            h.live[op.m3id] = op;
            max_tracked = std::max(max_tracked, cores[e.hart_id].size());
        }

        // A redirect flushes every uncommitted op younger than the
        // redirecting one (and that one too if it flushes itself): nuke m3
        // from the oldest flushed op.
        void Redirect(const XsEventData& e)
        {
            auto& h = harts[e.hart_id];
            ++redirects;
            h.have_last_alloc = false;
            uint64_t oldest = 0;
            bool any = false;
            for (auto it = h.live.begin(); it != h.live.end();)
            {
                const Op& op = it->second;
                bool flush = !op.committed && (RobAfter(op.rob, e.rob) || (e.aux != 0 && op.rob == e.rob));
                if (!flush)
                {
                    ++it;
                    continue;
                }
                if (!any || op.m3id < oldest)
                    oldest = op.m3id;
                any = true;
                ++flushed_ops;
                it = h.live.erase(it);
            }
            if (!any)
                return;
            for (const auto& [id, op] : h.live)
                if (id > oldest)
                    Warn(e, "an op survives a redirect that flushed an older one");
            std::set<Inst_id> removed;
            Quiet quiet;
            cores[e.hart_id].nuke(Inst_id(oldest), removed, false);
        }

        void LocallyPerformIfReady(uint32_t hart_id, Op& op)
        {
            if (!op.addr_valid || !op.data_valid || op.performed)
                return;
            auto& d = cores[hart_id].st_data_ref(Inst_id(op.m3id));
            d.set_data(op.address, op.size, op.store_data);
            cores[hart_id].st_locally_perform(Inst_id(op.m3id));
            op.performed = true;
        }

        void StoreAddr(const XsEventData& e)
        {
            Op* op = FindLive(e.hart_id, true, e.idx);
            if (!op || op->rob != e.rob)
            {
                Warn(e, "store address of an unknown store");
                return;
            }
            if (op->addr_valid)
                return;  // a replayed address write
            op->address = e.address;
            op->size = e.size;
            op->addr_valid = true;
            op->is_device = e.aux != 0 || IsDevice(e.address);
            auto& d = cores[e.hart_id].st_data_ref(Inst_id(op->m3id));
            if (op->is_device)
                d.set_device();
            d.add_addr(op->address, op->size);
            LocallyPerformIfReady(e.hart_id, *op);
        }

        void StoreData(const XsEventData& e)
        {
            Op* op = FindLive(e.hart_id, true, e.idx);
            if (!op)
            {
                Warn(e, "store data of an unknown store");
                return;
            }
            if (op->data_valid)
                return;
            op->store_data = e.data;
            op->data_valid = true;
            LocallyPerformIfReady(e.hart_id, *op);
        }

        void LoadWb(const XsEventData& e)
        {
            Op* found = FindLive(e.hart_id, false, e.idx);
            if (!found || found->rob != e.rob)
            {
                Warn(e, "writeback of an unknown load");
                return;
            }
            Op& op = *found;
            Inst_id iid(op.m3id);
            op.address = e.address;
            op.size = e.size;
            op.is_device = e.aux != 0 || IsDevice(e.address);
            auto& d = cores[e.hart_id].ld_data_ref(iid);
            d.clear();
            if (op.is_device)
                d.set_device();
            d.add_addr(op.address, op.size);
            op.addr_valid = true;
            cores[e.hart_id].ld_perform(iid);
            op.performed = true;
            op.model_data = d.get_data(op.address, op.size);
            op.rtl_data = e.data;
            bool ok = op.is_device
                      || RVUtils::sign_extend(op.model_data, op.size) == RVUtils::sign_extend(op.rtl_data, op.size);
            // A mismatch is a failure only if the load commits: the core may
            // still squash it.
            op.check_failed = !ok;
            if (!ok)
            {
                ++perform_mismatches;
                if (kDebug)
                    std::cout << "m3xs: hart " << e.hart_id << " lq " << e.idx << " iid " << op.m3id
                              << " perform mismatch DUT 0x" << std::hex << op.rtl_data << " model 0x" << op.model_data
                              << " addr 0x" << op.address << std::dec << " @" << e.timestamp << "\n";
            }
        }

        bool CommitLoad(const XsEventData& e, Op op)
        {
            Inst_id iid(op.m3id);
            cores[e.hart_id].set_safe(iid);
            if (!op.performed)
            {
                // No writeback (a prefetch): perform a dummy device read so
                // that m3 can retire the entry.
                ++loads_without_wb;
                auto& d = cores[e.hart_id].ld_data_ref(iid);
                d.set_device();
                d.add_addr(0, 1);
                cores[e.hart_id].ld_perform(iid);
                return true;
            }
            ++loads_checked;
            std::string m3_error;
            if (cores[e.hart_id].is_tracked(iid) && cores[e.hart_id].has_error(iid))
            {
                op.check_failed = true;
                m3_error = cores[e.hart_id].error_text(iid);
            }
            if (op.check_failed)
            {
                Fail(e, op, "load committed a value the memory model does not allow", m3_error);
                return false;
            }
            return true;
        }

        bool RobCommit(const XsEventData& e)
        {
            auto& h = harts[e.hart_id];
            bool ok = true;
            bool any = false;
            for (auto it = h.live.begin(); it != h.live.end();)
            {
                Op& op = it->second;
                if (op.committed || op.rob != e.rob)
                {
                    ++it;
                    continue;
                }
                any = true;
                if (op.is_store)
                {
                    op.committed = true;
                    cores[e.hart_id].set_safe(Inst_id(op.m3id));
                    ++it;
                    continue;
                }
                ok &= CommitLoad(e, op);
                it = h.live.erase(it);
            }
            if (!any)
            {
                auto pre = h.precommitted.find(e.rob);
                if (pre != h.precommitted.end())
                    h.precommitted.erase(pre);
                else
                    Warn(e, "commit of a load/store the bridge does not hold");
            }
            return ok;
        }

        // Pops a committed store out of the store queue.
        bool TakeStore(const XsEventData& e, Op& out)
        {
            auto& h = harts[e.hart_id];
            for (auto it = h.live.begin(); it != h.live.end(); ++it)
                if (it->second.is_store && it->second.idx == e.idx)
                {
                    out = it->second;
                    h.live.erase(it);
                    if (!out.committed)
                    {
                        // Pre-committed at the ROB head (NewStoreQueue
                        // virtualStoreQueuePreCommitPtr): no older op can
                        // squash it any more.
                        ++store_precommits;
                        out.committed = true;
                        h.precommitted.insert(out.rob);
                        cores[e.hart_id].set_safe(Inst_id(out.m3id));
                    }
                    LocallyPerformIfReady(e.hart_id, out);
                    return true;
                }
            return false;
        }

        void GloballyPerform(uint32_t hart_id, uint64_t m3id)
        {
            cores[hart_id].st_globally_perform(Inst_id(m3id));
            ++stores_performed;
        }

        void ToSbuffer(const XsEventData& e)
        {
            Op op;
            if (!TakeStore(e, op))
            {
                Warn(e, "sbuffer write of an unknown store");
                return;
            }
            if ((op.address & kSbufferWordMask) != e.address)
                Warn(e, "sbuffer write address differs from the store address");
            op.sbuffer_address = e.address;
            harts[e.hart_id].to_sbuffer.push_back(op);
        }

        // The sbuffer wrote a store into a line. Writes of one cycle may
        // arrive in any order, so match on the address among the oldest few.
        void SbufferEnq(const XsEventData& e)
        {
            auto& q = harts[e.hart_id].to_sbuffer;
            if (q.empty())
            {
                Warn(e, "sbuffer write with no store on its way");
                return;
            }
            auto it = q.begin();
            for (auto j = q.begin(); j != q.end() && j - q.begin() < 4; ++j)
                if (j->sbuffer_address == e.address)
                {
                    it = j;
                    break;
                }
            if (it->sbuffer_address != e.address)
                Warn(e, "sbuffer write address matches no pending store");
            harts[e.hart_id].sbuffer[static_cast<uint32_t>(e.aux)].push_back(it->m3id);
            q.erase(it);
        }

        // An sbuffer line reached the cache (a hit) or the miss queue.
        void SbufferDone(const XsEventData& e)
        {
            auto& h = harts[e.hart_id];
            auto it = h.sbuffer.find(static_cast<uint32_t>(e.aux));
            if (it == h.sbuffer.end())
                return;
            std::vector<uint64_t> ids = std::move(it->second);
            h.sbuffer.erase(it);
            std::sort(ids.begin(), ids.end());
            if (e.data != 0)
            {
                ++sbuffer_misses;
                auto& held = h.miss_queue[e.address & kLineMask];
                held.insert(held.end(), ids.begin(), ids.end());
                return;
            }
            for (uint64_t id : ids)
                GloballyPerform(e.hart_id, id);
            if (kDebug)
                std::cout << "m3xs: hart " << e.hart_id << " sbuffer line " << e.aux << " written, " << ids.size()
                          << " stores performed @" << e.timestamp << "\n";
        }

        void StoreRefill(const XsEventData& e)
        {
            auto& h = harts[e.hart_id];
            auto it = h.miss_queue.find(e.address & kLineMask);
            if (it == h.miss_queue.end())
                return;
            std::vector<uint64_t> ids = std::move(it->second);
            h.miss_queue.erase(it);
            std::sort(ids.begin(), ids.end());
            for (uint64_t id : ids)
                GloballyPerform(e.hart_id, id);
        }

        void MmioStore(const XsEventData& e)
        {
            Op op;
            if (!TakeStore(e, op))
            {
                Warn(e, "uncached write of an unknown store");
                return;
            }
            GloballyPerform(e.hart_id, op.m3id);
        }

        // AMO, LR and SC execute alone at the head of the ROB (noSpec,
        // blockBack) with the sbuffer drained: read, check and write in one
        // step, as the youngest op of the hart.
        bool Atomic(const XsEventData& e)
        {
            auto& core = cores[e.hart_id];
            ++atomics;
            Op op;
            op.m3id = static_cast<uint64_t>(core.inorder());
            op.address = e.address;
            op.size = e.size & 0x3f;
            op.rtl_data = e.aux;
            Inst_id iid(op.m3id);
            const bool no_write = (e.size & 0x80) != 0;
            const bool no_read = (e.size & 0x40) != 0;
            core.set_type(iid, no_write ? Mem_op::Load : Mem_op::Store);
            core.set_safe(iid);
            bool ok = true;
            if (!no_read)
            {
                auto& dl = core.ld_data_ref(iid);
                dl.add_addr(op.address, op.size);
                core.ld_perform(iid);
                op.model_data = dl.get_data(op.address, op.size);
                ++loads_checked;
                ok = RVUtils::sign_extend(op.model_data, op.size) == RVUtils::sign_extend(op.rtl_data, op.size);
            }
            if (!no_write)
            {
                auto& ds = core.st_data_ref(iid);
                ds.add_addr(op.address, op.size);
                ds.set_data(op.address, op.size, e.data);
                core.st_locally_perform(iid);
                GloballyPerform(e.hart_id, op.m3id);
            }
            else if (no_read)
            {
                Warn(e, "atomic that neither reads nor writes");
            }
            if (!ok)
            {
                Fail(e, op, "atomic read a value the memory model does not allow", "");
                return false;
            }
            return true;
        }

        bool Serve(const XsEventData& e)
        {
            switch (e.event)
            {
                case XsEvent::kLoadAlloc:   Alloc(e, false); return true;
                case XsEvent::kStoreAlloc:  Alloc(e, true); return true;
                case XsEvent::kRedirect:    Redirect(e); return true;
                case XsEvent::kStoreAddr:   StoreAddr(e); return true;
                case XsEvent::kStoreData:   StoreData(e); return true;
                case XsEvent::kLoadWb:      LoadWb(e); return true;
                case XsEvent::kRobCommit:   return RobCommit(e);
                case XsEvent::kToSbuffer:   ToSbuffer(e); return true;
                case XsEvent::kSbufferEnq:  SbufferEnq(e); return true;
                case XsEvent::kSbufferDone: SbufferDone(e); return true;
                case XsEvent::kStoreRefill: StoreRefill(e); return true;
                case XsEvent::kAtomic:      return Atomic(e);
                case XsEvent::kMmioStore:   MmioStore(e); return true;
                default:
                    Warn(e, "unknown event " + std::to_string(static_cast<int>(e.event)));
                    return true;
            }
        }

        // Rank of a store still in the store queue, for draining one cycle's
        // stores oldest first.
        uint64_t StoreQueueOrder(const XsEventData& e) const
        {
            for (const auto& [id, op] : harts[e.hart_id].live)
                if (op.is_store && op.idx == e.idx)
                    return id;
            return 0;
        }
    };

    BridgeXiangShan::BridgeXiangShan() = default;
    BridgeXiangShan::~BridgeXiangShan() = default;

    void BridgeXiangShan::Init(uint32_t ncores)
    {
        pimpl_ = std::make_unique<Impl>();
        for (uint32_t i = 0; i < ncores; ++i)
            pimpl_->cores.emplace_back(pimpl_->mem, i);
        pimpl_->harts.resize(ncores);
    }

    void BridgeXiangShan::RegisterEvent(const XsEventData& data)
    {
        pimpl_->pending.push_back(data);
    }

    bool BridgeXiangShan::ServeRegisteredEvents()
    {
        auto& p = *pimpl_;
        if (p.pending.empty())
            return true;
        // Priority first. Allocations and commits of a cycle arrive in any
        // order: serve them in ROB order per hart. Stores leaving the store
        // queue in one cycle go oldest first, so that the sbuffer FIFO keeps
        // the store queue order.
        std::vector<std::pair<uint64_t, XsEventData>> batch;
        batch.reserve(p.pending.size());
        for (const auto& e : p.pending)
            batch.emplace_back((e.event == XsEvent::kToSbuffer || e.event == XsEvent::kMmioStore) ? p.StoreQueueOrder(e) : 0, e);
        p.pending.clear();
        std::stable_sort(batch.begin(), batch.end(), [](const auto& x, const auto& y) {
            const XsEventData& a = x.second;
            const XsEventData& b = y.second;
            int pa = kPriority.count(a.event) ? kPriority.at(a.event) : -1;
            int pb = kPriority.count(b.event) ? kPriority.at(b.event) : -1;
            if (pa != pb)
                return pa > pb;
            if (a.hart_id != b.hart_id)
                return a.hart_id < b.hart_id;
            if (RobOrdered(a.event) && RobOrdered(b.event))
                return RobAfter(b.rob, a.rob);
            return x.first < y.first;
        });
        bool ok = true;
        for (const auto& [order, e] : batch)
            ok &= p.Serve(e);
        return ok;
    }

    void BridgeXiangShan::AddDeviceRegion(uint64_t begin, uint64_t end)
    {
        pimpl_->devices.push_back({ begin, end });
    }

    void BridgeXiangShan::SetCallbackGetByte(std::function<uint8_t(uint64_t)> get_byte_function)
    {
        pimpl_->mem.init(get_byte_function);
    }

    uint64_t BridgeXiangShan::ErrorCount() const
    {
        return pimpl_->errors;
    }

    void BridgeXiangShan::DumpStatistics() const
    {
        const auto& p = *pimpl_;
        std::cout << "M3XS stats: loads checked " << p.loads_checked << ", stores globally performed " << p.stores_performed
                  << ", atomics " << p.atomics << ", perform mismatches (squashed or failed) " << p.perform_mismatches
                  << ", errors " << p.errors << "\n";
        std::cout << "M3XS stats: redirects " << p.redirects << ", flushed ops " << p.flushed_ops
                  << ", loads without writeback " << p.loads_without_wb << ", sbuffer lines to the miss queue "
                  << p.sbuffer_misses << ", stores pre-committed " << p.store_precommits << ", max tracked per hart "
                  << p.max_tracked << "\n";
        for (const auto& [what, n] : p.warnings)
            std::cout << "M3XS stats: warning x" << n << ": " << what << "\n";
        for (size_t h = 0; h < p.cores.size(); ++h)
        {
            const auto& hart = p.harts[h];
            size_t in_sbuffer = 0, in_miss_queue = 0;
            for (const auto& entry : hart.sbuffer)
                in_sbuffer += entry.second.size();
            for (const auto& entry : hart.miss_queue)
                in_miss_queue += entry.second.size();
            std::cout << "M3XS stats: hart " << h << " tracked " << p.cores[h].size() << ", live " << hart.live.size()
                      << ", to sbuffer " << hart.to_sbuffer.size() << ", in sbuffer " << in_sbuffer
                      << ", in miss queue " << in_miss_queue << "\n";
        }
    }
}

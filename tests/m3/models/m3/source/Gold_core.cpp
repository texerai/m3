
#include "Gold_core.hpp"

#include "Gold_notify.hpp"
#include "debug_utils.hpp"

#include "fmt/core.h"

Gold_core::Gold_core(Gold_mem &m, int id) : mem(m), cid(id), global_instid(0) {}

Inst_id Gold_core::inorder(Inst_id rtl_iid) {
    global_instid = global_instid + 1;
    auto i        = global_instid;

    // Check not active/not_commited entry with rtl_iid rid
    auto end_rid = rob.end();
    if (pnr != 0) {
        end_rid = std::find_if(rob.begin(), rob.end(), [this](const Rob_entry &x) { return x.rid == pnr; });
    }
    for (auto it = rob.begin(); it != end_rid; ++it) {
        DEBUG_ASSERT(it->rtl_rid != rtl_iid,
            fmt::format("rehusing non retired rtl_iid rtl_iid: {}", rtl_iid));        
    }

    rob.push_front({i});
    rob.front().rtl_rid = rtl_iid;

    DEBUG_ASSERT(rob.front().rid == i, "m3 rid not set properly");
    DEBUG_ASSERT(rob.front().rtl_rid == rtl_iid, "rtl rid not set properly");
    DEBUG_ASSERT(rob.front().op == Mem_op::Invalid, "op not initialized properly");

    return i;
}

/*void Gold_core::set_type(Inst_id iid, Mem_op op) {
    auto it = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });

    if (it == rob.end()) {
        return;  // may be nuked
    }

    it->op        = op;
    it->performed = false;
}*/

void Gold_core::set_safe(Inst_id iid) {
    DEBUG_LOG(fmt::format("iid: {}, pnr: {}", iid, pnr), debug::VerbosityLevel::Medium);
    //Check that the entry exists and is younger that the last commited instruction
    auto rob_it = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });
    DEBUG_ASSERT(rob_it != rob.end(), "safe entry");
    DEBUG_ASSERT(iid > pnr, "safe entry is older than commited one");
    pnr = iid;
}

/*void Gold_core::nuke(Inst_id iid) {
    if (iid < pnr) {
        dump();
        Gold_nofity::fail("nuke id:{} for already safe pnr:{}", iid, pnr);
        return;
    }

    if (rob.empty())
        return;

    while (rob.front().rid >= iid) {
        const auto &e = rob.front();
        std::cout << "nuke: rid:" << e.rid << " error:" << e.error << "\n";
        e.dump("rob");

        rob.pop_front();
        if (rob.empty())
            return;
    }
}*/

uint64_t Gold_core::flush_entry(Inst_id rtl_iid_recover, Inst_id rtl_iid_head) {
    uint64_t flush_count = 0;
    bool found = 0;

    for (auto it = rob.begin(); it != rob.end();) {
        int m3_rid = it->rid;
        int rtl_rid = it->rtl_rid;
        if(!found) {
            //it->dump("erase rob");
            DEBUG_LOG(fmt::format("m3_rid: {}, rtl_rid: {}, pnr: {}", m3_rid, rtl_rid, pnr),
            debug::VerbosityLevel::Medium);
            // Check that the flushed entry is not commited
            DEBUG_ASSERT(m3_rid > pnr, "Check flushed entry not commited");
            // Check that rob_head is not being erased in case it is not the flush rob id
            DEBUG_ASSERT(!((rtl_iid_head != rtl_iid_recover) && (rtl_rid == rtl_iid_head)),
                "Check that rob_head is not being erased in case it is not the flush rob id");
            // Erase ROB entry
            it = rob.erase(it);
            // Erase memop_info entry
            //in_core_memops.erase(rid);
            //in_core_memops[rid].Invalidate();
            ++flush_count;
            if (rtl_rid == rtl_iid_recover) {
                found = true;
            }
        } else {
            //it->dump("keep rob");
            ++it;
        }
    }
    
    DEBUG_ASSERT(found, "check rtl_iid_recover is found");
    /*if (rtl_iid_recover == rtl_iid_head) {
        assert(rob.size() == 0);
    }*/
    return flush_count;
}

// Rids are duplicated in the rob as all not flushed entries are saved.
// This method searches the youngest entry with iid rid.
// This is because only the last instruction window is active.
Gold_core::Rob_entry &Gold_core::find_entry(Inst_id iid) {
    auto rob_it = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });

    if (rob_it == rob.end()) {
        dump();
        DEBUG_LOG(fmt::format("iid: {} must be in ROB", iid), debug::VerbosityLevel::Warning);
        static Rob_entry inv(0);
        return inv;
    }

    return *rob_it;
}

Gold_data &Gold_core::ld_data_ref(Inst_id iid) { return find_entry(iid).ld_data; }

Gold_data &Gold_core::st_data_ref(Inst_id iid) { return find_entry(iid).st_data; }

const Gold_data &Gold_core::ld_perform(Inst_id iid) {
    // Get the entry.
    auto &ent = find_entry(iid);

    // Should have allocated data fields.
    // Before we perform, we should allocate address and size of the load instruction.
    if (!ent.ld_data.has_data()) {
        DEBUG_LOG(fmt::format("ld iid: {} must have address before perform", iid),
          debug::VerbosityLevel::Error);
        ent.dump("ld_perform");
        return ent.ld_data;
    }

    // Bring data from the memory.
    mem.ld_perform(ent.ld_data);
    DEBUG_LOG("LD perform, access memory", debug:: VerbosityLevel::Medium);

    // Check if there are older local store instructions that performed.
    // If yes, we must forward newest value to this load.
    if (!rob.empty() && rob.back().rid != iid) {
        // Get the pointer to this load.
        Rob_queue::reverse_iterator rob_end
            = std::find_if(rob.rbegin(), rob.rend(), [&iid](const Rob_entry &x) { return x.rid == iid; });
        DEBUG_ASSERT(rob_end != rob.rend(), "Entry not found");

        // Check stores older than this load that haven't stored globally.
        for (auto it = rob.rbegin(); it != rob_end; ++it) {
            DEBUG_ASSERT(it->rid < iid, "Loading from younger entry");
            // Check that data is ready, but not stored globally
            if (it->st_data.has_data() && it->performed && !it->st_global) {
                DEBUG_LOG(
                fmt::format("check reorder buffer: m3 rid: {}, rtl rid: {}, has_data: {}, performed: {}, st_global: {}",
                    it->rid, it->rtl_rid, it->st_data.has_data(), it->performed, it->st_global),
                    debug::VerbosityLevel::Medium);                
                bool is_updated = ent.ld_data.update_newer(it->st_data);
                if (is_updated) {
                    Gold_nofity::info("ld reorder buffer performed iid:{} fwd from st iid:{}", iid, it->rid);
                    DEBUG_LOG(fmt::format(", LD perform update, *ACCESS reorder/store buffer, m3 rid: {}", it->rid),
                        debug::VerbosityLevel::Medium);
                }
            }
        }
    }

    ent.performed = true;
    ent.error.clear();

    DEBUG_LOG(fmt::format(", gold data: {}", ent.ld_data.str()), debug::VerbosityLevel::Medium);
    Gold_nofity::trace(iid, "core:{} ld gp{}", cid, ent.ld_data.str());

    return ent.ld_data;
}

// This method is used in spike and requires:
// In case rtl_iid entry invalid
// 
// In case rtl_iid entry is valid
// -It belongs to a commited load
// -As it access the previously generated load data, it doesn't require more checkings
const Gold_data Gold_core::ld_perform_no_update(Inst_id rtl_iid, long long memop_address, int len) {
    Gold_data temp_data;
    temp_data.add_addr(memop_address, len);
    // Bring data from the memory.
    mem.ld_perform(temp_data);

    // Find youngest entry with the specified rtl rbid
    auto rob_it = std::find_if(rob.begin(), rob.end(), [&rtl_iid](const Rob_entry &x) { return x.rtl_rid == rtl_iid; });

    DEBUG_LOG(fmt::format("LD perform, no update. Read from rtl_rbid: {}, memop_address: 0x{:x}, len: {}", 
        rtl_iid, memop_address, len), debug::VerbosityLevel::Medium);

    // Checks if the entry exists
    // Checks if it has been commited
    // Checks if it has load data (load, amo)
    if(rob_it != rob.end()){
        Inst_id iid = rob_it->rid;
        DEBUG_ASSERT(iid <= pnr, "Check entry is commited");
        DEBUG_LOG(fmt::format("load m3 rbid {} fetch case or page access", iid),
          debug::VerbosityLevel::Medium);
        /*// load memory access case, we check it has load data, and it was called to load that data and not the instruction fetch*/
        // Extreme case 1: the cache in which a load data paddr is the load pc paddr, but that case shoudn't be executed
        // Extreme case 2: there's a commited ins that hasn't write to mem with an overlapp
        // In case the access is related to an instruction, older commited and not mem stored stores will be checked as well as memory accessed with the required size
        // -Accessing mem returns all bits in case some are missing in the rob/stb entries
        // -Checking if data has been stored to mem avoids using non valid data, access younger data always, in rob/stb or mem
        // -Accesing own entry covers store cacheline merge case
        /*if (rob_it->ld_data.has_data() && rob_it->performed && !rob_it->st_global && rob_it->ld_data.has_full_overlap(memop_address, len)) {
            // commited
            assert(iid <= pnr);
            // Previous performed data should be inside the structure without being modifyed
            temp_data = rob_it->ld_data;
        // fetch
        // page access
        } else {*/
        DEBUG_LOG("fetch case or page access", debug::VerbosityLevel::Medium);
            // Check if there are older local store instructions that performed.
            // If yes, we must forward newest value to this load.
            if (!rob.empty() && rob.back().rid != iid) {
                DEBUG_LOG("checkpoint 1", debug::VerbosityLevel::Debug);
                // Get the pointer to this load.
                Rob_queue::reverse_iterator rob_end
                    = std::find_if(rob.rbegin(), rob.rend(), [&iid](const Rob_entry &x) { return x.rid == iid; });
                //Rob_queue::reverse_iterator pnr_rob_end
                //    = std::find_if(rob.rbegin(), rob.rend(), [this](const Rob_entry &x) { return x.rid == pnr; });
                //check!! DEBUG_ASSERT(rob_end != rob.rend(),
                //    "");
                // Check stores older than this load that haven't stored globally.
                for (auto it = rob.rbegin(); it != rob_end; ++it) {
                    DEBUG_ASSERT(it->rid < iid,
                        "Loading from younger entry");
                    DEBUG_LOG(fmt::format("check entry rtl_rid: {} has st data: {} performed: {} st_global: {}",
                        it->rtl_rid, it->st_data.has_data(), it->performed, !it->st_global),
                        debug::VerbosityLevel::Debug);
                    // Check that data is ready, but not stored globally
                    if (it->st_data.has_data() && it->performed && !it->st_global) {
                        bool is_updated = temp_data.update_newer(it->st_data);
                        if (is_updated) {
                            Gold_nofity::info("ld iid:{} fwd from st iid:{}", iid, it->rid);
                            DEBUG_LOG(fmt::format(", LD perform no update, *ACCESS reorder/store buffer, m3 rid: {}", it->rid),
                                debug::VerbosityLevel::Debug);
                        }
                    }
                }
            }
        //}
        
    // other cases
    // 1) no instruction related to the access (read elf)
    // 2) m3 hooks disabled
    } else {
        DEBUG_LOG("Other cases, LD perform no update, access memory",
            debug::VerbosityLevel::Medium);
    }

    DEBUG_LOG(fmt::format(", gold data: {}", temp_data.str()), debug::VerbosityLevel::Medium);
    Gold_nofity::trace(-1, "core:{} ld gp{}", cid, temp_data.str());

    return temp_data;
}

void Gold_core::st_locally_perform(Inst_id iid) {
    auto &ent = find_entry(iid);

    if (!ent.st_data.has_data()) {
        dump();
        DEBUG_ASSERT(false, fmt::format("st iid:{} must have data before perform", iid));
        ent.dump("st_perform");
        return;
    }
    ent.performed = true;

    Rob_queue::reverse_iterator rob_it;
    rob_it = std::find_if(rob.rbegin(), rob.rend(), [&iid](const Rob_entry &x) { return x.rid == iid; });

    Gold_nofity::trace(iid, "core:{} st lp{}", cid, rob_it->st_data.str());

    ++rob_it;  // Skip itself

    for (; rob_it != rob.rend(); ++rob_it) {
        if (!rob_it->ld_data.has_data())
            continue;
        if (!rob_it->performed)
            continue;

        auto ld_data_copy = ld_perform(rob_it->rid);  // perform again
        if (ld_data_copy != rob_it->ld_data) {
            std::cout << "WARNING: ld id:" << rob_it->rid << " performed but data changed\n";
            rob_it->error = "store id:" + std::to_string(iid) + " changed value";
            ld_data_copy.dump();
            rob_it->dump("after");
            assert(false);
        }
    }
}

/*void Gold_core::st_locally_merged(Inst_id iid1, Inst_id iid2) {
    if (iid1 > iid2) {  // swap, so that iid1 is older
        auto tmp = iid1;
        iid1     = iid2;
        iid2     = tmp;
    }

    auto rob_it1 = std::find_if(rob.begin(), rob.end(), [&iid1](const Rob_entry &x) { return x.rid == iid1; });
    auto rob_it2 = std::find_if(rob.begin(), rob.end(), [&iid2](const Rob_entry &x) { return x.rid == iid2; });
    auto s_rob_it2 = std::find_if(rob.begin(), rob.end(), [&iid2](const Rob_entry &x) { return x.rid == iid2; });

    if (rob_it1 == rob.end() || rob_it2 == rob.end()) {
        dump();
        std::cout + "ERROR: locally merge has missing ids iid1:" + iid1 + " iid2:" + iid2 + "\n";
    }

    // Merge iid1 (older) to iid2 (younger)
    // No store between iid2 and iid1 sould overlap in address with iid2

    auto &d2 = find_entry(iid2);
    auto &d1 = find_entry(iid1);
    //d1.st_data.cout_chunks("d1");
    //d2.st_data.cout_chunks("d2");

    std::cout + "pnr: " + pnr + ", d1.rid: " + d1.rid + ", d2.rid: " + d2.rid + std::endl;
    std::cout + "d1.st_global: " + d1.st_global + ", d2.st_global: " + d2.st_global + std::endl;
    //older must be commited and not stored globally
    assert((d1.rid <= pnr) && (!d1.st_global));
    //younger must be the commited instruction
    assert((d2.rid == pnr) && (!d2.st_global));


    ++rob_it2;  // skip itself

    for (; rob_it2 != rob.end(); ++rob_it2) {
        if (rob_it2->rid < iid1)
            return;  // done

        if (rob_it2->rid == iid1) {
            assert(rob_it2 == rob_it1);
            std::cout + "INFO: merging st iid: " + iid1 + "to st iid:" + iid2 + std::endl;
            // Merge inside the youngest one, so that the entry doesn't have to be erased
            s_rob_it2->st_merge(d1);
            // The old entry must be set to st_global, so that the value is not read in a load
            rob_it1->st_global =  true;
            //s_rob_it2->st_data.cout_chunks("d1+d2");
            break;
        }

        if (rob_it2->ld_data.has_partial_overlap(d2.st_data) && !rob_it2->performed) {
            std::cout + "FAIL: between st id: " + iid2 + "and id:" + iid1
                      + " there is a still not performed with partial overlap ld id:" + rob_it2->rid + "\n";
            d2.dump("merged");
            rob_it2->dump("match");
            //assert(false);
        }

        if (!rob_it2->st_data.has_partial_overlap(d2.st_data))
            continue;

        std::cout + "WARNING: between st id:" + iid2 + "and id:" + iid1 + " there is a partial overlap st id:" + rob_it2->rid
                  + "\n";
        d2.dump("merged");
        rob_it2->dump("match");
        //assert(false);
    }
}*/

void Gold_core::st_globally_perform(Inst_id iid) {
    auto rob_it1 = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });

    if (rob_it1 == rob.end()) {
        //dump();
        //std::cout + "WARNING: globally perform st id:" + iid + " but id is gone. Nothing to do???\n";
        //assert(false);
        //return;
        DEBUG_ASSERT(false,
            "Entry not found");
    }

    if (iid > pnr) {
        //dump();
        //std::cout + "FAIL: globally perform st id:" + iid + " but NOT safe?? (doing it, but crazy)\n";
        //assert(false);
        DEBUG_ASSERT(false,
            "Trying to merge not commited entry");
    }

    // Check that there are no overlapping older stores

    auto it = rob_it1;
    ++it;  // skip itself

    bool only_reads = true;
    for (; it != rob.end(); ++it) {
        if (it->st_data.has_data() && it->performed)
            only_reads = false;

        if (it->st_data.has_partial_overlap(rob_it1->st_data)) {
            std::cout << "WARNING: older than st id:" << iid << " there is a partial overlap id:" << it->rid << "\n";
            rob_it1->dump("performed");
            it->dump("older");
            //assert(false);
        }
    }

    st_locally_perform(iid);  // even if performed, we can check again (just in
                              // case missing API call)

    Gold_nofity::trace(iid, "core:{} st gp{}", cid, rob_it1->st_data.str());
    DEBUG_LOG(fmt::format("Store globally perform, gold data: {}", rob_it1->st_data.str()),
        debug::VerbosityLevel::Medium);

    //Check not stored globally before
    DEBUG_ASSERT(!rob_it1->st_global, "store globally already stored rob entry");
    mem.st_perform(rob_it1->st_data);

    // Store performed globally
    rob_it1->st_global =  true;

    /*if (only_reads) {  // try to retire these
        bool notified = false;

        while (rob.back().rid != iid) {
            auto &oldest = rob.back();

            if (!notified && (!oldest.error.empty() || !oldest.performed)) {
                notified = true;
                std::cout << "FAIL: ld:" << oldest.rid << " is oldest with error:" << oldest.error << "\n";
                dump();
            }

            rob.pop_back();
        }
        assert(rob.back().rid == iid);
        rob.pop_back();
    } else {
        rob.erase(rob_it1);
    }*/

    // Check that itself got deleted
    //assert(std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; }) == rob.end());
}

void Gold_core::st_globally_perform(std::vector<Inst_id> &viid) {
    // check size
    DEBUG_ASSERT(viid.size() >= 1, "store buffer not empty");
    // In this case it is only required the data from the rob entries that conform the stb entry data
    // So memory will not be accessed
    Gold_data temp_data;
    // not used value, the first one should be 1
    Inst_id iid_old = 0;
    for (auto iid : viid) {
        DEBUG_LOG(fmt::format("st_globally_perform, merge m3 id: {}", iid),
          debug::VerbosityLevel::Medium);

        auto rob_it1 = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });

        // check found ins
        if (rob_it1 == rob.end()) {
            //dump();
            //std::cout + "WARNING: globally perform st id:" + iid + " but id is gone. Nothing to do???\n";
            //assert(false);
            //return;
            DEBUG_ASSERT(false,
                "Entry not found");
        }

        // check commited ins
        if (iid > pnr) {
            //dump();
            //std::cout + "FAIL: globally perform st id:" + iid + " but NOT safe?? (doing it, but crazy)\n";
            DEBUG_ASSERT(false,
                "Trying to merge not commited entry");
        }

        // check store data found
        DEBUG_ASSERT(rob_it1->st_data.has_data(),
            "No store data found");
        // check local store performed
        DEBUG_ASSERT(rob_it1->performed,
            "No local store performed");
        // Check not stored globally before
        DEBUG_ASSERT(!rob_it1->st_global,
            "Storing allready stored data");

        // perform the merge from smaller to bigger rid(older to younger)
        DEBUG_ASSERT(iid_old < iid,
            "Trying to merge younger to older entry");
        iid_old = iid;
        // add all new data
        temp_data.add_newer(rob_it1->st_data);
        // set data as stored to memory
        rob_it1->st_global =  true;

        // required?
        //st_locally_perform(iid);  // even if performed, we can check again (just in
        // case missing API call)

        Gold_nofity::trace(iid, "core:{} st gp{}", cid, rob_it1->st_data.str());
        DEBUG_LOG(fmt::format("Store globally perform, gold data: {}", rob_it1->st_data.str()),
          debug::VerbosityLevel::Medium);
    }

    mem.st_perform(temp_data);
}

bool Gold_core::has_error(Inst_id iid) const {
    auto it = std::find_if(rob.begin(), rob.end(), [&iid](const Rob_entry &x) { return x.rid == iid; });

    if (it == rob.end())
        return true;

    return !it->error.empty();
}

void Gold_core::dump() const {
    std::cout << "==================================================\n";
    std::cout << "core cid:" << cid << " pnr:" << pnr << "\n";

    for (auto it = rob.rbegin(); it != rob.rend(); ++it) {
        it->dump("rob");
    }
}

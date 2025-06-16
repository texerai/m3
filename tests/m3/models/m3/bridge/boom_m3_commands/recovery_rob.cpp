/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
 #include "recovery_rob.h"

 // Local libraries.
 #include "memop_info.h"
 #include "rtl_event.h"
 #include "state.h"
 #include "debug_utils.hpp"
 
 namespace m3
 {
    RecoveryRob::RecoveryRob(const RTLEventData& data)
    {
        data_ = data;
        priority_ = 2;
    }
 
    bool RecoveryRob::Execute(State& state)
    {
        RTLEventData& d = data_;
        M3Cores& m3cores = state.m3cores;

        // flush the rob
        DEBUG_LOG(
            fmt::format("Recovery rob: rob_id: {}, rob_head: {}", d.rob_id, d.rob_head),
            debug::VerbosityLevel::Low);        
        m3cores[d.hart_id].flush_entry(d.rob_id, d.rob_head);

        return true;
    }
 }
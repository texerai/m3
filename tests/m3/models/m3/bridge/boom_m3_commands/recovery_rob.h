/*
 * Copyright (c) 2023 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
 #ifndef RECOVERY_ROB_H_
 #define RECOVERY_ROB_H_
 
 #include "m3command.h"
 
 namespace m3
 {
    // Forward declarations.
    struct RTLEventData;

    class RecoveryRob : public IM3Command
    {
    public:
        RecoveryRob() = delete;
        RecoveryRob(const RTLEventData& data);
        bool Execute(State& state) override;
    };
 }
 
 #endif
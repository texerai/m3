// Copyright (c) 2023 Kabylkas Labs. All rights reserved.
#include "simulator.h"

// C++ libraries.
#include <memory>

// Verilator libraries.
#include "verilated.h"
#include "VTestHarness.h"

double sc_time_stamp() {
    return 0;
}

namespace marionette
{
    void Simulator::Init()
    {
        // Init DUT.
        dut_ = std::make_shared<VTestHarness>();
        dut_->reset = 0;
        dut_->clock = 0;
        dut_->eval();
        is_init_ = true;
    }
}

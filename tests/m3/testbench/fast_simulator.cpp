// Copyright 2023 Kabylkas Labs. All rights reserved.
#include "fast_simulator.h"

// C++ libraries.
#include <memory>
#include <sstream>

// Verilator libraries.
#include "verilated.h"
#include "VTestHarness.h"

// Local libraries.
#include "model_instances.h"

namespace marionette
{
    static const char* kStringErrorMsgCoreModelNotInit = "Error: core model is not initialized.";
    static const char* kStringErrorMsgMarionetteModelsNotInit = "Error: marionette models are not initialized.";
    static const uint32_t kResetCycles = 100;
    static const uint32_t kRunCycles = 10000;

    static void AdvanceClock(std::shared_ptr<VTestHarness>& dut)
    {
        dut->clock = 0;
        dut->eval();
        dut->clock = 1;
        dut->eval();
    }

    bool FastSimulator::Reset(std::string& err_message)
    {
        bool is_success = false;
        bool are_models_init = ModelInstances::get().IsInit();

        if (is_init_ && are_models_init)
        {
            dut_->reset = 1;
            for (uint32_t i = 0; i < kResetCycles; i++)
            {
                AdvanceClock(dut_);
            }
            is_success = true;
        }
        
        // Form error messages if failed.
        std::stringstream error_formatter;
        if (!is_init_)
        {
            error_formatter << kStringErrorMsgCoreModelNotInit << " ";
        }
        if (!are_models_init)
        {
            error_formatter << kStringErrorMsgMarionetteModelsNotInit;
        }
        err_message = error_formatter.str();

        return is_success;
    }

    CompletionCode FastSimulator::Run(std::string& err_message)
    {
        CompletionCode ret = CompletionCode::kUnknown;

        dut_->reset = 0;
        for (uint32_t cycles = 0; cycles < kRunCycles; cycles++)
        {
            AdvanceClock(dut_);
        }

        return ret;
    }
}

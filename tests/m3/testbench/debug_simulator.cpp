// Copyright 2023 Kabylkas Labs. All rights reserved.
#include "debug_simulator.h"

// C++ libraries.
#include <memory>
#include <sstream>

// Verilator libraries.
#include "verilated.h"
#include "verilated_fst_c.h"
#include "VTestHarness.h"

// Local libraries.
#include "model_instances.h"

namespace marionette
{
    static const char* kStringFileNameWaveform = "wave.fst";
    static const char* kStringErrorMsgCoreModelNotInit = "Error: core model is not initialized.";
    static const char* kStringErrorMsgMarionetteModelsNotInit = "Error: marionette models are not initialized.";
    static const uint32_t kResetCycles = 100;
    static const uint32_t kRunCycles = 50000;
    static const uint32_t kAfterFailureCycles = 5;
    static const uint32_t kHierarchyLevel = 99;

    static void AdvanceClock(VerilatedFstC* wave, std::shared_ptr<VTestHarness> dut, uint64_t& cycle_count)
    {
        dut->clock = 0;
        dut->eval();
        wave->dump(static_cast<vluint64_t>(cycle_count * 2));
        dut->clock = 1;
        dut->eval();
        wave->dump(static_cast<vluint64_t>(cycle_count * 2 + 1));
        cycle_count++;
    }

    bool DebugSimulator::Reset(std::string& err_message)
    {
        bool is_success = false;
        bool are_models_init = ModelInstances::get().IsInit();

        if (is_init_ && are_models_init)
        {
            // Setup waveform dumping.
            Verilated::traceEverOn(true);
            wave_ = new VerilatedFstC();
            dut_->trace(wave_, kHierarchyLevel);
            wave_->open(kStringFileNameWaveform);

            dut_->reset = 1;
            for (uint32_t i = 0; i < kResetCycles; i++)
            {
                AdvanceClock(wave_, dut_, cycle_count_);
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

    CompletionCode DebugSimulator::Run(std::string& err_message)
    {
        CompletionCode ret = CompletionCode::kUnknown;
        dut_->reset = 0;
        auto& models = ModelInstances::get();
        bool should_abort = false;
        for (uint32_t cycles = 0; !should_abort && cycles < kRunCycles; cycles++)
        {
            // Advance the clock.
            AdvanceClock(wave_, dut_, cycle_count_);

            // At the end of the cycle. Server registered events in memory model.
            models.PerformEndOfCycleActions();

            // Check for any errors.
            if (models.IsAborted(err_message))
            {
                ret = CompletionCode::kFail;
                should_abort = true;
            }
        }

        // Let the simulation run for a few cycles to see waveform activities
        // passed failure point.
        if (should_abort)
        {
            for (uint32_t cycles = 0; cycles < kAfterFailureCycles; cycles++)
            {
                AdvanceClock(wave_, dut_, cycle_count_);
            }
        }
        else
        {
            ret = CompletionCode::kSuccess;
        }
        wave_->close();
        models.CloseModels();

        return ret;
    }
}

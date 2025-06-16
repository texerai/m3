// Copyright (c) 2023 Kabylkas Labs. All rights reserved.

#ifndef DEBUG_SIMULATOR_H_
#define DEBUG_SIMULATOR_H_

// C++ libraries.
#include <memory>

// Parent class.
#include "simulator.h"

// Forward declaration.
class VerilatedFstC;

namespace marionette
{
    // FastSimulator does not dump any waveforms and minimizes the outputs.
    class DebugSimulator : public Simulator
    {
    public:
        bool Reset(std::string& err_message) override;
        CompletionCode Run(std::string& err_message) override;

    private:
        VerilatedFstC* wave_ = nullptr;
        uint64_t cycle_count_ = 0;
    };
}

#endif // DEBUG_SIMULATOR_H_

// Copyright (c) 2023 Kabylkas Labs. All rights reserved.

#ifndef FAST_SIMULATOR_H_
#define FAST_SIMULATOR_H_

// Parent class.
#include "simulator.h"

namespace marionette
{
    // FastSimulator does not dump any waveforms and minimizes the outputs.
    class FastSimulator : public Simulator
    {
        bool Reset(std::string& err_message) override;
        CompletionCode Run(std::string& err_message) override;
    };
}

#endif // FAST_SIMULATOR_H_

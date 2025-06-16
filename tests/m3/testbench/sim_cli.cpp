// Copyright (c) 2023 Kabylkas Labs. All rights reserved.

// Local libraries.
#include "debug_simulator.h"
#include "model_instances.h"
#include "simulator.h"

// C++ libraries.
#include <iostream>
#include <memory>

// Constant errors.
static const char* kStringErrorSimFailed = "Error: Simulation failed.";

static void SetupDramConfigs(marionette::Config& config)
{
    // DRAM configs.
    config.ini_dir_path = "/mnt/c/Users/kabylkas/marionette/models/dram/configs";
    config.image_path   = "/mnt/c/Users/kabylkas/marionette/models/dromajo/build/snaps/check.mainram";
    config.id_bits      = 4;
    config.mem_size     = 268435456;
    config.line_size    = 64;
    config.clock_hz     = 100000000;
    config.word_size    = 8;

    // Dromajo configs.
    config.dromajo_config_path = "/mnt/c/Users/kabylkas/marionette/models/dromajo/build/snaps/check.json";
}

int main()
{
    // Set up singleton model instances.
    marionette::Config config;
    SetupDramConfigs(config);
    marionette::ModelInstances& model_instances = marionette::ModelInstances::get();
    model_instances.InitializeModels(config);

    // Set up core simulator.
    std::shared_ptr<marionette::Simulator> sim = std::make_shared<marionette::DebugSimulator>();
    sim->Init();

    std::string err_message;
    bool is_reset = sim->Reset(err_message);
    marionette::CompletionCode code = marionette::CompletionCode::kUnknown;
    if (is_reset)
    {
        code = sim->Run(err_message);
    }

    if (code != marionette::CompletionCode::kSuccess)
    {
        std::cout << kStringErrorSimFailed << std::endl;
        std::cout << "--> " << err_message << std::endl;
    }
    return 0;
}

/*
 * Copyright (c) 2023-2024 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */
#ifndef COMMIT_MEMOP_H_
#define COMMIT_MEMOP_H_

#include "m3command.h"

// C++ libraries.
#include <memory>

// Dromajo model access needed to update register value
// with the one in M3.
#include "dromajo_t.h"

namespace m3
{
    // Forward declarations.
    struct RTLEventData;

    class CommitMemop : public IM3Command
    {
    public:
        CommitMemop() = delete;
        CommitMemop(const RTLEventData& data, std::shared_ptr<dromajo_t> core_model_ptr);
        bool Execute(State& state) override;
        bool Execute(State& state, Tracer& m3tracer) override;

    private:
        std::shared_ptr<dromajo_t> core_model_ptr_;
    };
}

#endif
// Copyright (c) 2023 Kabylkas Labs. All rights reserved.

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

// C++ libraries.
#include <memory>
#include <string>

namespace marionette
{
    enum class CompletionCode
    {
        kUnknown,
        kSuccess,
        kFail,
        kTimeout
    };

    class Simulator
    {
    public:
        void Init();
        virtual bool Reset(std::string& err_message) = 0;
        virtual CompletionCode Run(std::string& err_message) = 0;
        virtual ~Simulator() {}

    protected:
        std::shared_ptr<VTestHarness> dut_;
        bool is_init_ = false;
    };
}

#endif // SIMULATOR_H_

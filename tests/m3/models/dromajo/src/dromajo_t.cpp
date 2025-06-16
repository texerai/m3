// Copyright (c) 2016 Fabrice Bellard
// Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
// Copyright (C) 2023 texer.ai
#include "dromajo_t.h"

// C++ libraries.
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>

// Local libraries.
#include "dromajo_cosim.h"

// Constants used in implementation.
static const char* kStringErrorDromajoNotInit = "Error: Dromajo is not initialized.";
static const char* kStringWarningDromajoIsInit = "Warning: Dromajo is already initialized.";

struct dromajo_t::dromajo_t_impl
{
    dromajo_cosim_state_t* state_ptr = nullptr;
};

void dromajo_t::init(const std::vector<std::string>& dromajo_args)
{
    if (pimpl_ == nullptr)
    {
        pimpl_ = new dromajo_t_impl;

        // Convert to the form that dromajo lib understands.
        const uint32_t kArgC = dromajo_args.size();
        char* argv[kArgC];
        for (uint32_t i = 0; i < kArgC; i++)
        {
            argv[i] = new char[dromajo_args[i].size() - 1];
            std::strcpy(argv[i], dromajo_args[i].c_str());
        }

        pimpl_->state_ptr = dromajo_cosim_init(kArgC, argv);
        assert(pimpl_->state_ptr != nullptr);
    }
    else
    {
        std::cerr << kStringWarningDromajoIsInit << std::endl;
    }
}

void dromajo_t::finish()
{
    if (pimpl_ != nullptr)
    {
        if (pimpl_->state_ptr != nullptr)
        {
            dromajo_cosim_fini(pimpl_->state_ptr);
        }
        else
        {
            std::cerr << kStringErrorDromajoNotInit << std::endl;
        }
    }
    else
    {
        std::cerr << kStringErrorDromajoNotInit << std::endl;
    }
}

int dromajo_t::step(int hartid, uint64_t dut_pc, uint32_t dut_insn, uint64_t dut_wdata, uint64_t mstatus, bool check)
{
    int ret = -1;
    if (pimpl_ != nullptr)
    {
        if (pimpl_->state_ptr != nullptr)
        {
            ret = dromajo_cosim_step(pimpl_->state_ptr, hartid, dut_pc, dut_insn, dut_wdata, mstatus, check);
        }
        else
        {
            std::cerr << kStringErrorDromajoNotInit << std::endl;
        }
    }
    else
    {
        std::cerr << kStringErrorDromajoNotInit << std::endl;
    }
    return ret;
}

void dromajo_t::update_register(int hartid, uint32_t rd, uint64_t data, bool is_fp)
{
    if (pimpl_ != nullptr)
    {
        if (pimpl_->state_ptr != nullptr)
        {
            dromajo_cosim_update_register(pimpl_->state_ptr, hartid, rd, data, is_fp);
        }
        else
        {
            std::cerr << kStringErrorDromajoNotInit << std::endl;
        }
    }
    else
    {
        std::cerr << kStringErrorDromajoNotInit << std::endl;
    }
}

void dromajo_t::raise_trap(int hartid, int64_t cause)
{
    if (pimpl_ != nullptr)
    {
        if (pimpl_->state_ptr != nullptr)
        {
            dromajo_cosim_raise_trap(pimpl_->state_ptr, hartid, cause);
        }
        else
        {
            std::cerr << kStringErrorDromajoNotInit << std::endl;
        }
    }
    else
    {
        std::cerr << kStringErrorDromajoNotInit << std::endl;
    }
}

int dromajo_t::override_mem(int hartid, uint64_t dut_paddr, uint64_t dut_val, int size_log2)
{
    int ret = -1;
    if (pimpl_ != nullptr)
    {
        if (pimpl_->state_ptr != nullptr)
        {
            ret = dromajo_cosim_override_mem(pimpl_->state_ptr, hartid, dut_paddr, dut_val, size_log2);
        }
        else
        {
            std::cerr << kStringErrorDromajoNotInit << std::endl;
        }
    }
    else
    {
        std::cerr << kStringErrorDromajoNotInit << std::endl;
    }
    return ret;
}

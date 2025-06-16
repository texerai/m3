// Copyright (c) 2016 Fabrice Bellard
// Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
// Copyright (C) 2023 texer.ai
#ifndef DROMAJO_T_H_
#define DROMAJO_T_H_

// C++ libraries.
#include <string>
#include <vector>

// Thin dromajo C++ wrapper. Enables more natural usage within C++ projects.
class dromajo_t
{
public:
    void init(const std::vector<std::string>& dromajo_args);
    void finish();
    int  step(int hartid, uint64_t dut_pc, uint32_t dut_insn, uint64_t dut_wdata, uint64_t mstatus, bool check);
    void update_register(int hartid, uint32_t rd, uint64_t data, bool is_fp);
    void raise_trap(int hartid, int64_t cause);
    int  override_mem(int hartid, uint64_t dut_paddr, uint64_t dut_val, int size_log2);

private:
    struct dromajo_t_impl;
    dromajo_t_impl* pimpl_ = nullptr;
};

#endif // DROMAJO_T_H_

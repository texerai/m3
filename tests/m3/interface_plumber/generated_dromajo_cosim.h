/*
 * API for Dromajo-based cosimulation
 *
 * Copyright (C) 2018,2019, Esperanto Technologies Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _DROMAJO_COSIM_H
#define _DROMAJO_COSIM_H 1

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct dromajo_cosim_state_st dromajo_cosim_state_t;

// GENERATED FUNCTIONS.

void SignalMemopCommitHookAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int is_load,
    int rob_id,
    uint64_t clock_cycle
);

void DromajoStepHookAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    uint64_t pc,
    int raw_instruction,
    int rob_id,
    uint64_t clock_cycle,
    uint64_t write_data
);

void DromajoRaiseInterruptHookAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int is_interrupt,
    uint64_t cause_code
);

void SignalMemopInsertHookAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int rob_id,
    int raw_instruction,
    int is_load,
    uint64_t clock_cycle
);

void SignalAddStoreAddressAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    uint64_t store_address,
    int store_size,
    int is_amo,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
);

void SignalAddLoadAddressAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    uint64_t load_address,
    int load_size,
    int rob_id,
    int ldq_idx,
    uint64_t clock_cycle
);

void SignalPerformLoadHookAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int rob_id,
    uint64_t loaded_data,
    uint64_t clock_cycle
);

void SignalStoreLocalPeformAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int rob_id,
    int is_amo,
    int stq_idx,
    uint64_t local_store_address,
    uint64_t local_store_data,
    uint64_t clock_cycle
);

void SignalStoreNukeAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
);

void SignalStoreCommitAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int rob_id,
    int stq_idx,
    uint64_t clock_cycle
);

void SignalStoreCompletedAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    uint64_t store_address,
    uint64_t store_data,
    int stq_idx,
    uint64_t clock_cycle
);

void SignalStoreGlobalPerformFromMetaWriteArbiterAPI
(
    dromajo_cosim_state_t* state, 
    int hart_id,
    int address,
    int tag,
    int idx,
    int coherence_state,
    int meta_way_en,
    int data_way_en,
    uint64_t data,
    int meta_write_valid,
    int data_write_valid,
    uint64_t clock_cycle
);

// ORIGINAL FUNCTIONS.
/*
 * dromajo_cosim_init --
 *
 * Creates and initialize the state of the RISC-V ISA golden model
 * Returns NULL upon failure.
 */
dromajo_cosim_state_t *dromajo_cosim_init(int argc, char *argv[]);

/*
 * dromajo_cosim_fini --
 *
 * Destroys the states and releases the resources.
 */
void dromajo_cosim_fini(dromajo_cosim_state_t *state);

/*
 * dromajo_cosim_step --
 *
 * executes exactly one instruction in the golden model and returns
 * zero if the supplied expected values match and execution should
 * continue.  A non-zero value signals termination with the exit code
 * being the upper bits (ie., all but LSB).  Caveat: the DUT is
 * assumed to provide the instructions bit after expansion, so this is
 * only matched on non-compressed instruction.
 *
 * There are a number of situations where the model cannot match the
 * DUT, such as loads from IO devices, interrupts, and CSRs cycle,
 * time, and instret.  For all these cases the model will override
 * with the expected values.
 */
int dromajo_cosim_step(dromajo_cosim_state_t *state,
                       int                    hartid,
                       int                    rob_id,
                       uint64_t               clock_cycle,
                       uint64_t               dut_pc,
                       uint32_t               dut_insn,
                       uint64_t               dut_wdata,
                       uint64_t               mstatus,
                       bool                   check);

/*
 * dromajo_cosim_raise_trap --
 *
 * DUT raises a trap (exception or interrupt) and provides the cause.
 * MSB indicates an asynchronous interrupt, synchronous exception
 * otherwise.
 */
void dromajo_cosim_raise_trap(dromajo_cosim_state_t *state,
                              int                   hartid,
                              int64_t               cause);

/*
 * dromajo_cosim_override_mem --
 *
 * DUT sets Dromajo memory. Used so that other devices (i.e. block device, accelerators, can write to memory).
 */
int dromajo_cosim_override_mem(dromajo_cosim_state_t *state,
                               int hartid,
                               uint64_t dut_paddr,
                               uint64_t dut_val,
                               int size_log2);
#ifdef __cplusplus
} // extern C
#endif

#endif

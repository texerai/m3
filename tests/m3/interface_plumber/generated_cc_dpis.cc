/*
 * Copyright (c) 2021-2022 Kabylkas Labs.
 * The original source code is taken from Berkeley's BOOM repo.
 */
// DPI libraries.
#include <vpi_user.h>
#include <svdpi.h>

// C++ libraries.
#include <cstring>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Local libraries.
#include "dromajo_cosim.h"

// gotten from chipyard elaboration (originally <long_name>.dromajo_params.h)
#include "dromajo_params.h"

#define MAX_ARGS 24

static dromajo_cosim_state_t* state;

// GENERATED FUNCTIONS.

extern "C" void SignalMemopCommitHookDPI
(
    int hart_id,
    int is_load,
    int rob_id,
    long long clock_cycle
)
{
    SignalMemopCommitHookAPI(state, hart_id, is_load, rob_id, clock_cycle);
}

extern "C" void DromajoStepHookDPI
(
    int hart_id,
    long long pc,
    int raw_instruction,
    int rob_id,
    long long clock_cycle,
    long long write_data
)
{
    DromajoStepHookAPI(state, hart_id, pc, raw_instruction, rob_id, clock_cycle, write_data);
}

extern "C" void DromajoRaiseInterruptHookDPI
(
    int hart_id,
    int is_interrupt,
    long long cause_code
)
{
    DromajoRaiseInterruptHookAPI(state, hart_id, is_interrupt, cause_code);
}

extern "C" void SignalMemopInsertHookDPI
(
    int hart_id,
    int rob_id,
    int raw_instruction,
    int is_load,
    long long clock_cycle
)
{
    SignalMemopInsertHookAPI(state, hart_id, rob_id, raw_instruction, is_load, clock_cycle);
}

extern "C" void SignalAddStoreAddressDPI
(
    int hart_id,
    long long store_address,
    int store_size,
    int is_amo,
    int rob_id,
    int stq_idx,
    long long clock_cycle
)
{
    SignalAddStoreAddressAPI(state, hart_id, store_address, store_size, is_amo, rob_id, stq_idx, clock_cycle);
}

extern "C" void SignalAddLoadAddressDPI
(
    int hart_id,
    long long load_address,
    int load_size,
    int rob_id,
    int ldq_idx,
    long long clock_cycle
)
{
    SignalAddLoadAddressAPI(state, hart_id, load_address, load_size, rob_id, ldq_idx, clock_cycle);
}

extern "C" void SignalPerformLoadHookDPI
(
    int hart_id,
    int rob_id,
    long long loaded_data,
    long long clock_cycle
)
{
    SignalPerformLoadHookAPI(state, hart_id, rob_id, loaded_data, clock_cycle);
}

extern "C" void SignalStoreLocalPeformDPI
(
    int hart_id,
    int rob_id,
    int is_amo,
    int stq_idx,
    long long local_store_address,
    long long local_store_data,
    long long clock_cycle
)
{
    SignalStoreLocalPeformAPI(state, hart_id, rob_id, is_amo, stq_idx, local_store_address, local_store_data, clock_cycle);
}

extern "C" void SignalStoreNukeDPI
(
    int hart_id,
    int rob_id,
    int stq_idx,
    long long clock_cycle
)
{
    SignalStoreNukeAPI(state, hart_id, rob_id, stq_idx, clock_cycle);
}

extern "C" void SignalStoreCommitDPI
(
    int hart_id,
    int rob_id,
    int stq_idx,
    long long clock_cycle
)
{
    SignalStoreCommitAPI(state, hart_id, rob_id, stq_idx, clock_cycle);
}

extern "C" void SignalStoreCompletedDPI
(
    int hart_id,
    long long store_address,
    long long store_data,
    int stq_idx,
    long long clock_cycle
)
{
    SignalStoreCompletedAPI(state, hart_id, store_address, store_data, stq_idx, clock_cycle);
}

extern "C" void SignalStoreGlobalPerformFromMetaWriteArbiterDPI
(
    int hart_id,
    int address,
    int tag,
    int idx,
    int coherence_state,
    int meta_way_en,
    int data_way_en,
    long long data,
    int meta_write_valid,
    int data_write_valid,
    long long clock_cycle
)
{
    SignalStoreGlobalPerformFromMetaWriteArbiterAPI(state, hart_id, address, tag, idx, coherence_state, meta_way_en, data_way_en, data, meta_write_valid, data_write_valid, clock_cycle);
}

// MANUALLY CODED FUNCTIONS.
extern "C" int dromajo_init(
    const char* bootrom_file,
    const char* dtb_file,
    const char* binary_file)
{
    // setup arguments
    char local_argc = 18;
    char* local_argv[MAX_ARGS] = {
        "./dromajo",
        "--compact_bootrom",
        "--custom_extension",
        "--ncpus",
        "2",
        "--clear_ids",
        "--reset_vector",
        DROMAJO_RESET_VECTOR,
        "--bootrom",
        (char*)bootrom_file,
        "--mmio_range",
        DROMAJO_MMIO_START ":" DROMAJO_MMIO_END,
        "--plic",
        DROMAJO_PLIC_BASE ":" DROMAJO_PLIC_SIZE,
        "--clint",
        DROMAJO_CLINT_BASE ":" DROMAJO_CLINT_SIZE,
        "--memory_size",
        DROMAJO_MEM_SIZE,
        "--save",
        "dromajo_snap"
    };

    if (strlen(dtb_file) != 0) {
        local_argv[local_argc] = "--dtb";
        local_argv[local_argc+1] = (char*)dtb_file;
        local_argc += 2;
    }

    local_argv[local_argc] = (char*)binary_file;
    local_argc += 1;

    if (MAX_ARGS < local_argc) {
        printf("[DRJ_ERR] Too many arguments\n");
        exit(1);
    }

    printf("[DRJ_INFO] Dromajo command: ");
    for (char i = 0; i < local_argc; ++i)
        printf("%s ", local_argv[i]);
    printf("\n");

    state = dromajo_cosim_init(local_argc, local_argv);

    if (state == nullptr) {
        printf("[DRJ_ERR] Failed Dromajo initialization\n");
        return 1;
    }

    return 0;
}

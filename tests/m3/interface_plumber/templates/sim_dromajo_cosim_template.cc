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

// @generate:gen0

// @generate:gen1

// @generate:gen2

// @generate:gen_memop_insert

// @generate:gen_add_store_address

// @generate:gen_add_load_address

// @generate:gen_perform_load

// @generate:gen_store_local_perform

// @generate:gen_store_nuke

// @generate:gen_store_commit

// @generate:gen_store_completed

// @generate:gen_store_global_perform_arbiter

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

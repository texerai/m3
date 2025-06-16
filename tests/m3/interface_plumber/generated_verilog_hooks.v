// Auto generated file. Do not edit.
// Copyright (c) 2021-2022 Kabylkas Labs.
// Licensed under the Apache License, Version 2.0.

// Generated import declarations.
import "DPI-C" function void DromajoStepHookDPI(
    input int hart_id,
    input longint pc,
    input int raw_instruction,
    input int rob_id,
    input longint clock_cycle,
    input longint write_data
);

import "DPI-C" function void SignalStoreGlobalPerformFromMetaWriteArbiterDPI(
    input int hart_id,
    input int address,
    input int tag,
    input int idx,
    input int coherence_state,
    input int meta_way_en,
    input int data_way_en,
    input longint data,
    input int meta_write_valid,
    input int data_write_valid,
    input longint clock_cycle
);

import "DPI-C" function void SignalStoreCompletedDPI(
    input int hart_id,
    input longint store_address,
    input longint store_data,
    input int stq_idx,
    input longint clock_cycle
);

import "DPI-C" function void DromajoRaiseInterruptHookDPI(
    input int hart_id,
    input int is_interrupt,
    input longint cause_code
);

import "DPI-C" function void SignalMemopInsertHookDPI(
    input int hart_id,
    input int rob_id,
    input int raw_instruction,
    input int is_load,
    input longint clock_cycle
);

import "DPI-C" function void SignalPerformLoadHookDPI(
    input int hart_id,
    input int rob_id,
    input longint loaded_data,
    input longint clock_cycle
);

import "DPI-C" function void SignalMemopCommitHookDPI(
    input int hart_id,
    input int is_load,
    input int rob_id,
    input longint clock_cycle
);

import "DPI-C" function void SignalStoreLocalPeformDPI(
    input int hart_id,
    input int rob_id,
    input int is_amo,
    input int stq_idx,
    input longint local_store_address,
    input longint local_store_data,
    input longint clock_cycle
);

import "DPI-C" function void SignalAddStoreAddressDPI(
    input int hart_id,
    input longint store_address,
    input int store_size,
    input int is_amo,
    input int rob_id,
    input int stq_idx,
    input longint clock_cycle
);

import "DPI-C" function void SignalAddLoadAddressDPI(
    input int hart_id,
    input longint load_address,
    input int load_size,
    input int rob_id,
    input int ldq_idx,
    input longint clock_cycle
);

import "DPI-C" function void SignalStoreNukeDPI(
    input int hart_id,
    input int rob_id,
    input int stq_idx,
    input longint clock_cycle
);

import "DPI-C" function void SignalStoreCommitDPI(
    input int hart_id,
    input int rob_id,
    input int stq_idx,
    input longint clock_cycle
);

// Generated modules.
module DromajoStepHookHook (
    input clock,
    input reset,
    input instruction_commit_trigger,
    input hart_id,
    input [39:0] pc,
    input [31:0] raw_instruction,
    input [4:0] rob_id,
    input [63:0] clock_cycle,
    input [63:0] write_data
);
    always @(posedge clock) begin
        if (!reset) begin
            if (instruction_commit_trigger) begin
                DromajoStepHookDPI(hart_id, pc, raw_instruction, rob_id, clock_cycle, write_data);
            end
        end
    end
endmodule
module SignalStoreGlobalPerformFromMetaWriteArbiterHook (
    input clock,
    input reset,
    input store_global_perform_arbiter,
    input hart_id,
    input [11:0] address,
    input [19:0] tag,
    input [5:0] idx,
    input [1:0] coherence_state,
    input [3:0] meta_way_en,
    input [3:0] data_way_en,
    input [63:0] data,
    input meta_write_valid,
    input data_write_valid,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (store_global_perform_arbiter) begin
                SignalStoreGlobalPerformFromMetaWriteArbiterDPI(hart_id, address, tag, idx, coherence_state, meta_way_en, data_way_en, data, meta_write_valid, data_write_valid, clock_cycle);
            end
        end
    end
endmodule
module SignalStoreCompletedHook (
    input clock,
    input reset,
    input store_global_merge_trigger,
    input hart_id,
    input [39:0] store_address,
    input [63:0] store_data,
    input [2:0] stq_idx,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (store_global_merge_trigger) begin
                SignalStoreCompletedDPI(hart_id, store_address, store_data, stq_idx, clock_cycle);
            end
        end
    end
endmodule
module DromajoRaiseInterruptHookHook (
    input clock,
    input reset,
    input raise_interrupt_trigger,
    input hart_id,
    input is_interrupt,
    input [63:0] cause_code
);
    always @(posedge clock) begin
        if (!reset) begin
            if (raise_interrupt_trigger) begin
                DromajoRaiseInterruptHookDPI(hart_id, is_interrupt, cause_code);
            end
        end
    end
endmodule
module SignalMemopInsertHookHook (
    input clock,
    input reset,
    input hart_id,
    input insert_memop_trigger,
    input [4:0] rob_id,
    input [31:0] raw_instruction,
    input is_load,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (insert_memop_trigger) begin
                SignalMemopInsertHookDPI(hart_id, rob_id, raw_instruction, is_load, clock_cycle);
            end
        end
    end
endmodule
module SignalPerformLoadHookHook (
    input clock,
    input reset,
    input hart_id,
    input perform_load_trigger,
    input [4:0] rob_id,
    input [63:0] loaded_data,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (perform_load_trigger) begin
                SignalPerformLoadHookDPI(hart_id, rob_id, loaded_data, clock_cycle);
            end
        end
    end
endmodule
module SignalMemopCommitHookHook (
    input clock,
    input reset,
    input instruction_commit_trigger,
    input hart_id,
    input is_load,
    input [4:0] rob_id,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (instruction_commit_trigger) begin
                SignalMemopCommitHookDPI(hart_id, is_load, rob_id, clock_cycle);
            end
        end
    end
endmodule
module SignalStoreLocalPeformHook (
    input clock,
    input reset,
    input store_local_perform_trigger,
    input hart_id,
    input [4:0] rob_id,
    input is_amo,
    input [2:0] stq_idx,
    input [39:0] local_store_address,
    input [63:0] local_store_data,
    input [63:0] clock_cycle
);
    always @(posedge clock) begin
        if (!reset) begin
            if (store_local_perform_trigger) begin
                SignalStoreLocalPeformDPI(hart_id, rob_id, is_amo, stq_idx, local_store_address, local_store_data, clock_cycle);
            end
        end
    end
endmodule
module SignalAddStoreAddressHook (
    input clock,
    input reset,
    input store_address_add_trigger,
    input hart_id,
    input [39:0] store_address,
    input [1:0] store_size,
    input is_amo,
    input [4:0] rob_id,
    input [2:0] stq_idx,
    input [63:0] clock_cycle
);
    always @(posedge store_address_add_trigger) begin
        if (!reset) begin
            SignalAddStoreAddressDPI(hart_id, store_address, store_size, is_amo, rob_id, stq_idx, clock_cycle);
        end
    end
endmodule
module SignalAddLoadAddressHook (
    input clock,
    input reset,
    input load_address_add_trigger,
    input hart_id,
    input [39:0] load_address,
    input [1:0] load_size,
    input [4:0] rob_id,
    input [2:0] ldq_idx,
    input [63:0] clock_cycle
);
    always @(posedge load_address_add_trigger) begin
        if (!reset) begin
            SignalAddLoadAddressDPI(hart_id, load_address, load_size, rob_id, ldq_idx, clock_cycle);
        end
    end
endmodule
module SignalStoreNukeHook (
    input clock,
    input reset,
    input store_nuke_trigger,
    input hart_id,
    input [4:0] rob_id,
    input [2:0] stq_idx,
    input [63:0] clock_cycle
);
    always @(negedge store_nuke_trigger) begin
        if (!reset) begin
            SignalStoreNukeDPI(hart_id, rob_id, stq_idx, clock_cycle);
        end
    end
endmodule
module SignalStoreCommitHook (
    input clock,
    input reset,
    input store_commit_trigger,
    input hart_id,
    input [4:0] rob_id,
    input [2:0] stq_idx,
    input [63:0] clock_cycle
);
    always @(posedge store_commit_trigger) begin
        if (!reset) begin
            SignalStoreCommitDPI(hart_id, rob_id, stq_idx, clock_cycle);
        end
    end
endmodule

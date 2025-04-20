# Integrating the m3 Library into BOOM's Verification Infrastructure

## Overview

To enable the co-simulation of multi-core systems running shared programs, the **m3 library** offers a high-level abstraction of memory subsystem behavior. Rather than tightly coupling the memory model to RTL implementation details, m3 defines a set of **abstract semantic events** that represent key points in memory operation execution. These events serve as an interface between the design under test (DUT) and the m3.

Without loss of generality, we begin with the core APIs that M3 provides:


| Function                                    | Description        |
|---------------------------------------------|--------------------|
| `Inst_id inorder()`                         | Gets the in-order ID for instructions through the pipeline.                                                  |
| `const Data& ld_perform(Inst_id iid)`       | Invoked when a load is globally performed and the data is bound to the destination register.                 |
| `void st_locally_perform(Inst_id iid)`      | Marks the point when store address and data become available locally. Enables forwarding to dependent loads. |
| `void st_globally_perform(Inst_id iid)`     | Signals that a store has been globally committed to memory and is visible to other harts.                   |

> [!NOTE]
> The list of APIs described in this document is not exhaustive, but it should provide the reader with a solid starting point for the integration process. We believe that once the material in this document is understood, integrating the remaining API functions will become intuitive.

These calls define the **contract between RTL and m3**, abstracting away microarchitectural implementation details such as load queues, store queues, and cache hierarchy.

---

## Using the Bridge Design Pattern

In realistic RTL cores, the above abstract memory events do not correspond one-to-one with RTL signals. For example, for a single `ld_perform` to occur, the following might need to happen:
- Load memop is in the ROB.
- Load address is computed.
- Load data becomes available.

To decouple these microarchitectural steps from m3's abstract semantics, we apply a Bridge design pattern. In the Marionette model methodology, each of these RTL signals corresponds to a separate RTL hook — individual strings through which we "pull" memory operation events. The Bridge is responsible for tracking these triggers from the RTL, recording intermediate state, and defining the logic for when to invoke the core m3 library APIs. This pattern allows the DUT to send events to m3 without requiring m3 to understand the internal pipeline structure.

---

## Delivery of Core m3 API Calls via RTL Hooks

To integrate the m3 library into a dual-core Small BOOM configuration, we define several RTL hooks. The `BoomBridge` class is responsible for mapping these hooks to the appropriate m3 core API, maintaining internal state, and determining when an m3 API call should be triggered.

### `inorder()` API
This API assigns an in-order ID to a memory instruction so that it can be uniquely tracked by the verification infrastructure. The ID must reflect the program order, which is essential for ensuring memory consistency and detecting out-of-order violations. The correct place to invoke this API is at the moment when an instruction is allocated into the Reorder Buffer (ROB) in BOOM, provided that the instruction is a memory operation (load, store, or AMO). We named this RTL hook `CreateMemop` and it is responsible for detecting this condition. This marks the instruction's entry into m3's tracking system.

The exact RTL condition in BOOM corresponding to this hook:
```
tile_reset_domain_boom_tile.core.rob.(io_enq_valids_0&(io_enq_uops_0_uses_ldq|io_enq_uops_0_uses_stq))
```

When `CreateMemop` pulls the string the following additional information needs to be passed from RTL:

| Hierarchy                                                                 | Port Size |
|---------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                                | 1         |
| tile_reset_domain_boom_tile.core.rob.io_enq_uops_0_rob_idx                | 5         |
| tile_reset_domain_boom_tile.core.rob.io_enq_uops_0_debug_inst             | 32        |

The bridge then creates a memop in m3. BOOM has a debug wire going into the ROB for storing the opcode (`io_enq_uops_0_debug_inst`), which is handy — because by providing this information to the bridge, we can retrieve additional details such as whether the instruction is a store, load, or AMO, and mark the memop in m3 accordingly.

> [!IMPORTANT]
> Since the API can be called from different parts of the pipeline, we need a mechanism to map each memop event back to its corresponding m3 ID. The ROB ID is helpful here because it remains unique for an instruction while it's in flight. When a memop is created in m3, the bridge stores a mapping from ROB ID to m3 ID using a simple state object. Later, when another event occurs, the ROB ID is typically available, allowing us to retrieve the associated m3 ID. This mappings happen per core, hence we need the hart_id.

### `ld_perform(Inst_id iid)`
This call is made when the RTL pipeline reports that a load has successfully completed and returned data. The corresponding RTL hook is PerformLoad. However, for this event to be meaningful, it must be preceded by address computation. Therefore, prior to invoking ld_perform(), the address must be calculated and communicated via the AddAddress RTL hook. Only after this condition is met and the data is available can the Bridge safely invoke ld_perform(). At this point, the Bridge compares the RTL-produced data with the model's prediction to detect mismatches.

#### 1) AddAddress RTL Hook
The exact RTL condition for `AddAddress` hook:
```
tile_reset_domain_boom_tile.lsu.ldq_X_bits_addr_valid
```

The following additional information needs to be passed from RTL when address added:

| Hierarchy                                                              | Port Size |
|------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                             | 1         |
| tile_reset_domain_boom_tile.lsu.ldq_X_bits_addr_bits                   | 40        |
| tile_reset_domain_boom_tile.lsu.ldq_X_bits_uop_mem_size                | 2         |
| tile_reset_domain_boom_tile.lsu.ldq_X_bits_uop_rob_idx                 | 5         |
| tile_reset_domain_boom_tile.lsu.(X)                                    | 3         |

The `X` in the signal name represents the index of an entry in the Load Queue (LDQ). In a small BOOM configuration, there are 8 LDQ entries. The address-related signals are triggered when any of the corresponding `addr_valid` bits go high. The underlying logic is that when an entry's address becomes available in the RTL, we want to capture this moment in the m3 tracking system. At this point, we also provide additional metadata—such as the `hartid` and `rob_id`—which allows us to retrieve the corresponding m3 ID. Using this ID, we then update the memory operation in m3 with the address and memory operation size.

#### 2) PerformLoad RTL Hook
The exact RTL condition for `PerformLoad` hook:
```
tile_reset_domain_boom_tile.lsu.(io_core_exe_0_iresp_valid&(~io_core_exe_0_iresp_bits_uop_uses_stq))
```

The following additional information needs to be passed from RTL:

| Hierarchy                                                                      | Port Size |
|--------------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                                     | 1         |
| tile_reset_domain_boom_tile.lsu.io_core_exe_0_iresp_bits_data                  | 64        |
| tile_reset_domain_boom_tile.lsu.io_core_exe_0_iresp_bits_uop_rob_idx           | 5         |

As usual, the `hart_id` and `rob_id` are used to retrieve the corresponding m3 ID. Using this ID, the load operation is located in the M3 model, and the memory operation is executed—meaning the memory model returns the expected data. This data is then compared against the value produced by the RTL, available on the `io_core_exe_0_iresp_bits_data signal`.

> [!IMPORTANT]
> If the comparison fails, we do not halt the simulation immediately, unlike traditional single-core co-simulation scenarios. In this case, the load value is being checked while still in the pipeline, and it may belong to a mispredicted path. Therefore, we simply mark this memop in the Bridge's state object as having failed the comparison. The simulation is only halted if and when this load instruction commits, confirming that the incorrect value would have affected architecturally visible state.

### `st_locally_perform(Inst_id iid)` API
This function is used when both the store's address **AND** data are available in the RTL. It depends on two RTL hooks: `AddAddress` and `AddStoreData`. The Bridge waits until both conditions are satisfied before calling `st_locally_perform()`. Invoking this API indicates that the store is now locally visible — meaning that subsequent loads issued by the same core can see data from this store, even if it hasn't committed yet. 

This behavior is consistent with how BOOM implements its store-to-load forwarding mechanism: as long as the store queue contains a valid address and data for a pending store, subsequent loads can read from it. Therefore, reaching these two conditions (address and data availability) is sufficient to mark the store as locally performed and ready for potential forwarding.

#### 1) `AddAddress` RTL Hook
Very similar to `AddAddress` from loads, the exact RTL condition for `AddAddress` hook for stores:
```
tile_reset_domain_boom_tile.lsu.stq_X_bits_addr_valid
```

The following additional information needs to be passed from RTL:
```
| Hierarchy                                                              | Port Size |
|------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                             | 1         |
| tile_reset_domain_boom_tile.lsu.stq_X_bits_addr_bits                   | 40        |
| tile_reset_domain_boom_tile.lsu.stq_X_bits_uop_mem_size                | 2         |
| tile_reset_domain_boom_tile.lsu.stq_X_bits_uop_rob_idx                 | 5         |
| tile_reset_domain_boom_tile.lsu.(X)                                    | 3         |
```

The `X` in the signal name represents the index of an entry in the Store Queue (STQ). In a small BOOM configuration, there are 8 STQ entries. The address-related signals are triggered when any of the corresponding `addr_valid` bits go high. The underlying logic is that when an entry's address becomes available in the RTL, we want to capture this moment in the m3 tracking system. At this point, we also provide additional metadata—such as the `hartid` and `rob_id`—which allows us to retrieve the corresponding m3 ID. Using this ID, we then update the memory operation in m3 with the address and memory operation size.

#### 2) `AddStoreData` RTL Hook
The exact RTL condition for `AddStoreData` hook for stores:
```
tile_reset_domain_boom_tile.lsu.stq_X_bits_data_valid
```

The following additional information needs to be passed from RTL:
```
| Hierarchy                                                              | Port Size |
|------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                             | 1         |
| tile_reset_domain_boom_tile.lsu.stq_X_bits_data_bits                   | 64        |
| tile_reset_domain_boom_tile.lsu.stq_X_bits_uop_rob_idx                 | 5         |
```

As usual, the `hart_id` and `rob_id` are used to retrieve the corresponding m3 ID. After getting this ID, the store data is added to the memory operation in m3. When both the store's address **AND** data are available, we can invoke `st_locally_perform(Inst_id iid)`

### `st_globally_perform(Inst_id iid)` API
This function is triggered when the store becomes globally visible to other harts. Global visibility in this context means that the memory system has accepted the store, and all necessary coherence conditions are satisfied. In BOOM's case, cacheline state transitioned to `M` state in the context of MESI protocol. From the m3's perspective, if cacheline is in `M` state, the data must be seen by all the cores in the system. The RTL state transition tracking requires the definition of RTL hooks such as `UpdateCacheLineState` and `UpdateCacheLineData`.

In BOOM, the change of the cacheline mean that the store has already committed. Once an instruction commits, its ROB entry is deallocated, making it impossible to track the store further using only the ROB ID. To handle this, the Bridge must transfer tracking responsibility to a separate object that persists beyond ROB deallocation — typically by recording the store’s metadata in a `beyond_core_stores` structure. This requires the definition of additional RTL Hook: `CommitMemop`.

#### 1) `UpdateCacheLineState` RTL Hook

The exact RTL condition for `UpdateCacheLineState` hook:
```
tile_reset_domain_boom_tile.dcache.metaWriteArb.io_out_valid
```

The following additional information needs to be passed from the RTL:
```
| Hierarchy                                                                                  | Port Size |
|---------------------------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                                                  | 1         |
| tile_reset_domain_boom_tile.dcache.metaWriteArb.io_out_bits_way_en                          | 4         |
| tile_reset_domain_boom_tile.dcache.metaWriteArb.io_out_bits_idx                             | 6         |
| tile_reset_domain_boom_tile.dcache.metaWriteArb.io_out_bits_data_coh_state                  | 2         |
| tile_reset_domain_boom_tile.dcache.metaWriteArb.io_out_bits_data_tag                        | 20        |
```
Once the store leaves the core, the primary metadata used to retrieve the corresponding M3 ID is the store’s address. The signals listed above are used to reconstruct this address in order to perform the lookup.

#### 2) `UpdateCacheLineData` RTL Hook

The exact RTL condition for `UpdateCachelineData` hook:
```
tile_reset_domain_boom_tile.dcache.dataWriteArb.io_out_valid
```

The following additional information needs to be passed from the RTL:

```
| Hierarchy                                                                                  | Port Size |
|---------------------------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                                                  | 1         |
| tile_reset_domain_boom_tile.dcache.dataWriteArb.io_out_bits_way_en                          | 4         |
| tile_reset_domain_boom_tile.dcache.dataWriteArb.io_out_bits_addr                            | 12        |
```
Once the store leaves the core, the primary metadata used to retrieve the corresponding M3 ID is the store’s address. The signals listed above are used to reconstruct this address in order to perform the lookup.

#### 3) `CommitMemop` RTL Hook

The exact RTL condition for `CommitMemop` hook:
```
tile_reset_domain_boom_tile.core.rob.(io_commit_valids_0&(io_commit_uops_0_uses_ldq|io_commit_uops_0_uses_stq))
```

The following additional information needs to be passed from the RTL:
```
| Hierarchy                                                              | Port Size |
|------------------------------------------------------------------------|-----------|
| tile_reset_domain_boom_tile.core.io_hartid                             | 1         |
| tile_reset_domain_boom_tile.core.rob.com_idx                           | 5         |
```

These signals are used to retrieve the M3 ID. For store operations, the M3 ID is transferred from the ROB ID to the address once the store leaves the core.

> [!IMPORTANT]
> For load memops, we check whether the value comparison passed during the ld_perform stage. If the check is successful, we update the corresponding register in the "Processor" golden model using a callback function defined in the Bridge.

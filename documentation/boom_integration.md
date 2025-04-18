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

To integrate the M3 library into a dual-core Small BOOM configuration, we define RTL hooks that correspond to the semantic memory operations tracked by M3. The Bridge is responsible for mapping these hooks to the appropriate M3 core API, maintaining internal state, and determining when an m3 API call should be triggered.

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

### `st_locally_perform(Inst_id iid)`
This function is used when both the store's address and data are available in the RTL. It depends on two RTL hooks: `AddAddress` and `AddStoreData`. The Bridge waits until both conditions are satisfied before calling `st_locally_perform()`. This marks the store as ready for forwarding to subsequent loads.

### `st_globally_perform(Inst_id iid)`
This function is triggered when the store is globally visible to other harts. In practice, this means the memory system has accepted the store and coherence conditions (e.g., MESI states) are satisfied. The corresponding hooks are `UpdateCacheLineState` or `UpdateCacheLineData`, depending on which final transition occurs.

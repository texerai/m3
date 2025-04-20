## Integrating the M3 Library into a Verification Infrastructure

### Overview

To enable memory consistency verification in modern out-of-order cores, the **M3 library** offers a high-level abstraction of memory subsystem behavior. Rather than tightly coupling the memory model to RTL implementation details, M3 defines a set of **abstract semantic events** that represent key points in memory operation execution. These events serve as an interface between the design under test (DUT) and the M3 checker.

Without loss of generality, we begin with the core APIs that M3 provides.

```cpp
/** inorder
 * Gets the in-order ID for instructions through the pipeline
 */
Inst_id inorder();

/** ld_perform
 * Invoked when a load is globally performed and the data is bound to the destination register.
 */
const Data &ld_perform(Inst_id iid);

/** st_locally_perform
 * Marks the point when store address and data become available locally.
 * Enables forwarding to dependent loads.
 */
void st_locally_perform(Inst_id iid);

/** st_locally_merged
 * Indicates that two store instructions targeting the same cache line are merged locally.
 */
void st_locally_merged(Inst_id iid1, Inst_id iid2);

/** st_globally_perform
 * Signals that a store has been globally committed to memory and is visible to other harts.
 */
void st_globally_perform(Inst_id iid);
```

These calls define the **contract between RTL and M3**, abstracting away microarchitectural implementation details such as load queues, store queues, and cache hierarchy.

---

### Motivation for Using the Bridge Pattern

In realistic RTL cores, the above abstract memory events do not correspond one-to-one with RTL signals. For example, for a single `ld_perform` to occur, the following might need to happen:
- Load address is computed.
- Cache hit is confirmed.
- Data bypassed or fetched.
- Pipeline hazard cleared.
- Instruction retires.

To decouple these microarchitectural steps from M3's abstract semantics, we apply a **Bridge design pattern**. This pattern allows the DUT to **send events** to M3 when specific RTL hooks are triggered, without requiring M3 to understand the internal pipeline structure.

---

### Case Study: Dual-Core Small BOOM Integration

Let’s consider a small BOOM configuration with **two RISC-V cores** and a shared L2 cache. Integration involves the following steps:

#### 1. Instrument RTL Hooks
Instrument the BOOM core to emit RTL events relevant to memory operations. For example:
- `CreateMemop`: when a new load/store/AMO enters the pipeline.
- `AddAddress`: when address computation is complete.
- `AddStoreData`: when the store data becomes available.
- `PerformLoad`: when the core retrieves data for a load.
- `CompleteStore`: when the store is locally completed.
- `CommitMemop`: when the memory op commits in ROB.
- `UpdateCacheLineState/Data`: when L1 or L2 cache transitions occur.

Each of these hooks corresponds to a microarchitectural point in BOOM, and they are **registered as events** using:
```cpp
BridgeBoom::RegisterEvent(const RtlHookData& data);
```

#### 2. Use BridgeBoom to Translate Events
`BridgeBoom` implements logic to **map RTL events to M3 memory model invocations**. For instance:
- On `PerformLoad`, `ld_perform()` is called in M3.
- On `AddStoreData`, if address and data are ready, `st_locally_perform()` is triggered.
- On `CompleteStore`, the store is queued for `st_globally_perform()` after coherence is ensured.

This mapping is handled internally by:
```cpp
commands::kRtlHookCommands[data.event](data, state);
```

#### 3. Per-Core State Management
Each core maintains its own:
- In-flight memory operations (`in_core_memops`)
- Completed stores (`beyond_core_stores`)
- M3 core instance (`m3cores`)

The bridge ensures operations are tagged by `hart_id` and `rob_id`, maintaining correct isolation and consistency across cores.

#### 4. Register Processor Update Callbacks
M3 does not directly modify processor state. Instead, the verification infrastructure must provide a callback:
```cpp
BridgeBoom::SetCallbackUpdateReg(update_proc_register);
```
This function is invoked when a load is verified and needs to update architectural state in the DUT.

---

### Summary

By introducing a **bridge layer between RTL and M3**, this integration:
- Abstracts complex pipeline behavior into clean memory semantics.
- Allows reuse of the M3 library across multiple cores and microarchitectures.
- Enables accurate load/store/AMO checking without compromising RTL modularity.

In the next section, we will describe how to **extend this infrastructure to support cache coherence validation**, where `st_globally_perform()` is conditioned on MESI state transitions.


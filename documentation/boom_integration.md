## Integrating the M3 Library into a Verification Infrastructure

### Overview

To enable memory consistency verification in modern out-of-order cores, the **M3 library** offers a high-level abstraction of memory subsystem behavior. Rather than tightly coupling the memory model to RTL implementation details, M3 defines a set of **abstract semantic events** that represent key points in memory operation execution. These events serve as an interface between the design under test (DUT) and the M3 checker.

Without loss of generality, we begin with the assumption that M3 provides the following abstract memory behaviors:


| Function                                    | Description        |
|---------------------------------------------|--------------------|
| `Inst_id inorder()`                         | Gets the in-order ID for instructions through the pipeline.                                                  |
| `const Data& ld_perform(Inst_id iid)`       | Invoked when a load is globally performed and the data is bound to the destination register.                 |
| `void st_locally_perform(Inst_id iid)`      | Marks the point when store address and data become available locally. Enables forwarding to dependent loads. |
| `void st_locally_merged(Inst_id iid1, iid2)`| Indicates that two store instructions targeting the same cache line are merged locally.                      |
| `void st_globally_perform(Inst_id iid)`     | Signals that a store has been globally committed to memory and is visible to other harts.                   |

These calls define the **contract between RTL and M3**, abstracting away microarchitectural implementation details such as load queues, store queues, and cache hierarchy.


---

### Using the Bridge Design Pattern

In realistic RTL cores, the above abstract memory events do not correspond one-to-one with RTL signals. For example, for a single `ld_perform` to occur, the following might need to happen:
- Load memop is in the ROB.
- Load address is computed.
- Load data becomes available.

To decouple these microarchitectural steps from M3's abstract semantics, we apply a **Bridge design pattern**. This pattern allows the DUT to **send events** to M3 when specific RTL hooks are triggered, without requiring M3 to understand the internal pipeline structure.

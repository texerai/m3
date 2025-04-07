# Simple case for m3
This document walks you through a simple use case of how the Memory Marionette Model (m3) can be used. We will describe the main elements involved in the verification of memory models and explain how M3 ensures proper ordering of memory operations. 

## Overview
The figure below visually illustrates what an instance of the `m3` object and one of the main internal data structures, what we call the Memory Reorder Buffer. This data structure tracks memory operations in flight and enables dynamic checking of their ordering to ensure correctness.

![1](https://github.com/user-attachments/assets/f1a82220-712e-45f4-a4cd-280483287371)

This Reorder Buffer (ROB) mimics the ROB found in out-of-order processors, but it is dedicated exclusively to memory operations. Instructions are added to this data structure in program order, with the leftmost entry in the figure representing the oldest memory operation and the rightmost representing the youngest. However, additional information required for memory operations can be added out of order. Each memory operation is tracked with a unique ID, which is assigned when the operation is inserted into the buffer. Data forwarding from older stores to younger loads is handled automatically by m3.

## Simple case
Now, let's consider a simple but specific example to demonstrate how interactions with M3 occur. To guide this explanation, we introduce an X-axis representing time and mark key events along it. At each marked point, we will discuss what happens within M3. 

![2](https://github.com/user-attachments/assets/987c6a9a-581a-4039-b567-04a35ad3c0af)

The specific timing values are arbitrary and not important—what matters is that the events happen in increasing time order, reflecting the natural flow of operations through the system.

### (1) Add load ID 1
At time 1, in the RTL, a load instruction is enqueued into ROB. We are communicating this event to our model, registering, and assigning ID=1 to this memory operation.

![3](https://github.com/user-attachments/assets/2256d09c-eb44-4be3-84db-bc31f3e3e61c)

### (2) Add address to load ID 1
After several cycles, at time 4, the RTL calculated the address for the load with ID=1. This address was added to Load Queue Entry. This event is communicated to the model.

![4](https://github.com/user-attachments/assets/dd526487-b444-4b0f-bf8f-5fe354c5ed2b)

### (3) Add store ID 2
At time 7, in the RTL, a store instruction is enqueued into ROB. We are communicating this event to our model, registering, and assigning ID=2 to this memory operation.

![5](https://github.com/user-attachments/assets/550c3a72-d165-48b8-afc6-ca264e222365)

### (4) Add another load ID 3
At time 11, in the RTL, another load instruction is enqueued into ROB in-order. We are communicate this event to our model, registering, and assigning ID=3 to this memory operation.

![6](https://github.com/user-attachments/assets/bf1254b1-845f-4d63-8bdf-9ea30d7e015b)

### (5) Perform the load ID 1
At time 20, we received a signal from the RTL that the data for load ID 1 became available. The data was brought to pipeline. We are communicating this event to our model and registering the data. The load ID 1 can be now performed.

> [!NOTE]
> This is the moment where we call `MemoryMarionette::ld_perform(Inst_id iid)` function. Internally this function checks the value that was brought from RTL against what has been tracked by the model. If the data doesn't match, we flag this failure. 

![7](https://github.com/user-attachments/assets/e3b7a10e-cecd-4011-827c-7409f6934f8a)

### (6) Add address to load ID 3
At time 31, the RTL calculated the address and size for the load with ID 3. This address was added to Load Queue Entry. This event is communicated to the model.

![8](https://github.com/user-attachments/assets/ce9f87e8-eec7-46a1-9834-7f0f95beb221)

### (7) Perform the load ID 3
In a few cycles, at time 33, the data becomes available at the RTL. The data was brought to pipeline. We are communicating this event to our model and registering the data. The load ID 3 can be now performed.

> [!IMPORTANT]
> Note that something fishy is happening. We brought the data for load ID 3, but there is an older store to the same address region which has not been performed yet! This means we are bringing the data out-of-program order. This being the said, m3 doesn't check ordering at load_perform due to performance optimizations. This will be checked at store perform.

![9](https://github.com/user-attachments/assets/bda9343d-3ed0-444d-9b08-826fe89b4f9b)

### (8) Perform locally the store ID 2
At time 41, the store ID 2 information becomes available in the RTL. It get's communicated. If the processor implements the forwarding between load and stores, the data of this store should be visible locally. Therefore we call m3's `st_locally_perform` API. 

> [!IMPORTANT]
> `st_locally_perform` API checks the ordering restrictions. At time 41, the API will walk through younger loads and see if any of them has been performed. The performance of the younger load before older store is not permissive, so this triggers a warning message or could be configured to halt the simulation.

![10](https://github.com/user-attachments/assets/24869151-b8f2-40a6-975b-eda781cf4f39)



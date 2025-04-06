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

### Add load 1
At time 1, In the RTL, load instruction is enqueued into ROB. We are communicating this event to our model and registering this assigning ID=1 to this memory operation.

![3](https://github.com/user-attachments/assets/2256d09c-eb44-4be3-84db-bc31f3e3e61c)

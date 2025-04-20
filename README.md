# Memory Marionette Model (m3)

## Overview

The Memory Marionette Model (m3) is a powerful verification library designed to expose subtle bugs in multicore memory subsystems. By implementing the Marionette Model methodology, m3 brings unmatched precision to memory operation tracking and validation—making it an essential tool for anyone serious about building reliable multicore processors.

<img src="https://user-images.githubusercontent.com/8511359/168761545-07a34325-0b65-4f51-a6c4-99b379cc2194.png" width="150" style="float: left">

### Why use M3?
- Enable co-simulation of multicore systems running share memory programs
- Verify correct memory behavior in multicore systems
- Track and validate memory operation ordering
- Ensure memory consistency across cores
- Support both in-order and out-of-order execution models
- Provide comprehensive debugging and error detection capabilities

### Key Features
1. Marionette Model: Unlike traditional single-point "at commit" verification, m3 uses a marionette model that tracks operations throughout execution, enabling true co-simulation of multicore shared memory programs

2. Comprehensive Operation Tracking: Tracks loads, stores, and atomic operations from issue through completion, with support for store-to-load forwarding and store merging

3. Verification Mechanisms: Memory consistency checking, operation ordering validation, error detection and reporting, comprehensive state tracking

4. Flexible Integration: Support for multiple emulators (Dromajo, Spike), bridge-based architecture for easy extension, configurable verification parameters

## Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/texerai/m3.git
   cd m3
   ```

2. Install dependencies:
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake

   # macOS
   brew install cmake
   ```

3. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j
   ```

The library will be built as `libm3.a` in the `build` directory. Headers will be in `include/`.

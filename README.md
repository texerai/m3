# Memory Marionette Model (m3)

## Overview

The Memory Marionette Model (m3) is a powerful verification library designed to expose subtle bugs in multicore memory subsystems. By implementing the Marionette Model methodology, m3 brings unmatched precision to memory operation tracking and validation—making it an essential tool for anyone serious about building reliable multicore processors.

<img src="https://user-images.githubusercontent.com/8511359/168761545-07a34325-0b65-4f51-a6c4-99b379cc2194.png" width="150" style="float: left">

### Why use m3?
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
   cmake -S . -B build
   cmake --build build -j
   ```

The library will be built as `libm3.a` in the `build` directory. Headers will be in `include/`.

The default build type is `Debug`, which keeps the library's internal asserts enabled. Add `-DM3_SANITIZE=ON` to build with AddressSanitizer and UndefinedBehaviorSanitizer.

## Running the Tests

```bash
ctest --test-dir build --output-on-failure -j 4
```

The suite has three parts:

- `core_test` and `data_test`: unit tests of the library semantics (forwarding, ordering checks, nuking, merging, byte-level data handling), written with [doctest](https://github.com/doctest/doctest).
- `trace_<name>`: replays of event traces recorded from dual-core BOOM simulations (`tests/traces/`) through the BOOM bridge. A replay fails if the bridge reports any verification failure, e.g. a load committing with data that does not match the model.
- `trace_towers` is registered as an expected failure: that recording captured virtual instead of physical addresses for some memops and cannot pass until it is re-recorded.

The traces do not include the initial memory image of the simulation, so the replay seeds the golden memory lazily with the values the RTL observed on the first load of each byte. Every later load of those bytes, and all forwarding and ordering checks, are real checks.

CI (`.github/workflows/ci.yml`) runs the suite in Debug with sanitizers and in Release, both with `-Werror`.

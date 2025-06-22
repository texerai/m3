# Test environment

## Build and run

Executing run_tests.sh will build and run the tests simulation.
removing the build folder can be necessary.

## Debug mode

Enter run_tests.sh and uncomment cmake Debug build type and gdb execution.
Then, execute the script again.

## Check mechanism

The tests executed are verified through the checking of the loads using a gold reference, specific to each test.

## Test list

The tests executed by this environment aim to verify m3 code.
They simulate memory operations as they build be done in hardware, the operations event are executed in different patterns in order to verify the internal mechanisms.
Following descriptions show and overview of the tests which can be missing some details.

### base_test_1.cc

All mem operations are executed in the same hart.
This test performs a store and a load atomically.

### base_test_2.cc

All mem operations are executed in the same hart
This test performs a store until commit stage(buffer write) and a load which will access the store buffer structure.

### base_test_3.cc

All mem operations are executed in the same hart
This test performs a store until add data stage(data ready in reorder buffer) and a load which will access the reorder buffer structure.

### base_test_4.cc

The memory operations are executed in different harts.
This test performs a store and a load atomically.

### base_test_5.cc

The memory operations are executed in different harts.
This test performs a store until commit stage(buffer write) and a load which will access the store buffer structure.

### base_test_6.cc

The memory operations are executed in different harts.
This test performs a store until add data stage(data ready in reorder buffer) and a load which will access the reorder buffer structure.

### base_test_7.cc

This test checks all the DPI methods used by m3, as previous tests use custom wrapp methods only used in tests.

### base_test_8.cc

Load from reorder buffer in different harts and simulate loads and stores inside spike.

### base_test_9.cc

Check recovery mechanism.

### base_test_10.cc

Store overlapp accesses.

### base_test_11.cc

Rehuse rbids.

### base_test_12.cc

More overlapp stores.
Check spike load before and ater commiting the instruction.

## TODO

- Unit testing of smaller methods to cover more corner cases
- Structured list of cases
- Change assertion() to a mechanism that allows to test wrong cases

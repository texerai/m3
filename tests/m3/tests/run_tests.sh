#!/bin/bash

# Navigate to the script's directory (ensure we're in 'tests/')
cd "$(dirname "$0")"

# Create and navigate to the build directory
rm -rf build
mkdir -p build && cd build

# Run CMake
echo "Running CMake..."
#cmake .. || { echo "CMake failed!"; exit 1; }
cmake .. -DCMAKE_BUILD_TYPE=Debug || { echo "CMake failed!"; exit 1; }

# Compile the tests
echo "Building tests..."
make -j$(nproc) || { echo "Build failed!"; exit 1; }

# Run the tests
echo "Running tests..."
#gdb ./all_tests
./all_tests

# Capture test result
TEST_RESULT=$?

# Exit with the same status as the tests
exit $TEST_RESULT

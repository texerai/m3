# Marionette(M3) BSC version

## Ownership
Marionette or M3 is a copy and custom update of https://github.com/masc-ucsc/marionette/tree/dev-new-structure.

## Overview
It is a reference model of a multicore memory system. \
It realizes a simplified simulation of features such as a reorder buffer, a store buffer and a cache. \
It is integrated with spike (https://gitlab.bsc.es/hwdesign/verification/spike) substituing spike's memory model, which is a memory region reserved and accessed using a pointer, which doesn't comply with multicore requirements which are expected in cosimulation.

## Usage
### Run Spike+M3
It is a custom version, only a subset of the code will be used. To identify which files are used, check spike build mechanism, and models/m3 cmake files. \
The code is intended to work with spike. It is compiled as a library using CMAKE and is linked with the rest of spike code.\
Furthermore, in case of using spike in dpi mode, m3_dpi.cc is compiled separately from the CMAKE feature.\
The script is executed from spike repository, but is similar to the following one:

```
    cd models/m3
    cmake .
    make all
```
*CMakeFiles, output, cmake_install.cmake and CMakeCache.txt can be erased to re-build the tests binary.

### Test M3
Folder tests contains a unitary tests framework for M3.\
This folder includes a TESTS.md with tests information.\
In order to run the tests:\
```
    ./tests/run_tests.sh

```
*build folder can be erased to re-build the tests binary.

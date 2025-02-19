# M3: Memory Marionette Model

M3 is a set of libraries to integrate with functional emulators like
Dromajo and Spike to perform multicore verification.

The code was originaly at dromajo repository, and moved to a separate repo to
allow integration with other emulators and code bases.

* gold directory has the core functionality to perform m3
* bridges has code to help interface m3 functionality with different types of core/emulators
* test has several tests for bridges and/or gold functionality


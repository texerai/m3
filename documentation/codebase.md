## M3 Main Components

The library is built around three main components that work together to provide memory verification:

### 1. Data Class (`data.hpp/cpp`)
The foundation of memory operation tracking:

```cpp
class Data {
    // Core functionality
    void set_addr(const uint64_t addr, const uint8_t sz);  // Set memory range
    void set_data(const uint64_t addr, const uint8_t sz, uint64_t d);  // Set data
    uint64_t get_data(const uint64_t addr, const uint8_t sz) const;  // Get data
    
    // Memory range management
    bool has_full_overlap(const uint64_t addr, const uint8_t sz) const;
    bool has_partial_overlap(const uint64_t addr, const uint8_t sz) const;
    
    // Data comparison and merging
    void add_newer(const Data &d2);  // Merge newer data
    bool update_newer(const Data &d2);  // Update with newer data
};
```

Key features:
- Manages memory ranges as chunks of data
- Handles overlapping memory regions
- Provides data comparison and merging capabilities
- Supports random data generation for uninitialized memory
- Maintains byte-level granularity for memory operations

### 2. Memory Class (`memory.hpp`)
The physical memory model:

```cpp
class Memory {
    // Core operations
    void st_perform(const Data &st_data);  // Perform store operation
    void ld_perform(Data &ld_data);        // Perform load operation
    
    // Initialization
    void init(std::function<uint8_t(uint64_t)> gb);  // Initialize with byte reader
};
```

Key features:
- Provides byte-level memory access
- Handles memory initialization
- Manages memory state through a hash map

> [!IMPORTANT]
> The `Memory` class requires a callback function to be set. This callback connects to the allocated memory of the Processor Golden Model (e.g., Spike or Dromajo). On the first access to a memory location, the data is fetched from the Processor Golden Model's memory. Subsequent loads and stores access the unordered_map maintained by this class. This mechanism ensures that the memories are automatically synchronized at the beginning of the simulation.

To integrate M3 with a processor model (like Dromajo or Spike), you need to define a memory access function in your processor model that returns individual bytes:

```cpp
extern uint8_t dromajo_get_byte_direct(uint64_t paddr);
static Memory mem(dromajo_get_byte_direct);
```

### 3. Memory Marionette Class (`memory_marionette.hpp/cpp`)
The verification orchestrator:

```cpp
class MemoryMarionette {
    // Core state
    Inst_id global_instid;  // Global instruction counter
    Rob_queue rob;         // Reorder Buffer
    Inst_id pnr;           // Point of No Return
    
    // Memory operation management
    Data& st_data_ref(Inst_id iid);  // Get store data reference
    Data& ld_data_ref(Inst_id iid);  // Get load data reference
    const Data& ld_perform(Inst_id iid);  // Perform load
    void st_locally_perform(Inst_id iid);  // Perform local store
    void st_globally_perform(Inst_id iid);  // Perform global store
};
```

Key features:
- Manages instruction ordering
- Tracks memory operations
- Handles store-to-load forwarding
- Provides verification mechanisms
- Maintains operation safety

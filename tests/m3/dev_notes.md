[February 7, 2024]
- Just making note of the fact that when we commit memop, there is a
communication happening between m3 and dromajo models. This should be
implemented when the registered events get executed.

Code of the DPI from old approach:
extern "C" void commit_memop_DPI
(
    int hart_id,
    int rob_id,
    long long global_clock
)
{
    // Get the pointer M3 model.
    marionette::ModelInstances& model_instances = marionette::ModelInstances::get();
    std::shared_ptr<boom_m3_t> m3_ptr = model_instances.GetM3Ptr();
    std::shared_ptr<dromajo_t> dromajo_ptr = model_instances.GetDromajoPtr();
    assert(m3_ptr != nullptr);
    assert(dromajo_ptr != nullptr);

	bool is_good = m3_ptr->commit_memop(
        static_cast<uint32_t>(hart_id),
        static_cast<uint32_t>(rob_id),
        static_cast<uint64_t>(global_clock),
        dest_reg, load_data, memop_type,
        error_message
    );

    bool is_fp = (dest_reg > 31);
    dest_reg = dest_reg % 32;

    if (memop_type == MemopType::kLoad)
    {
        dromajo_ptr->update_register(hart_id, dest_reg, load_data, is_fp);
    }

    if (!is_good)
    {
        model_instances.Abort(error_message);
    }
}

[November 23, 2023]
- RTLEvent should not be merged with MemopInfo. The semantic difference
  is that RTLEventData is the information that stores information about
  the single event and gets packed at the DPI level to communicate to 
  the models. MemopInfo, on the other hand, has a life-time of the
  instruction. The RTLEventData is used to patch the MemopInfo as it
  goes through the pipeline.
- Finish adding events as a seperate classes.

[November 22, 2023]
- Refactoring contiues: implementing command processor pattern.
- Try merging concepts of RTLEvent and MemopInfo.

[November 17, 2023]
- In the methodology we are proposing, some events are interdependent, i.e.
  one event sets the state and the next event changes the model based on 
  the what the previous event did. For example, to globally perform the
  store I hook the string on the event when data is written to cache-line
  which is in M-state. The prerequsite, more precisely assumed prerequsite
  that I set in the code is that the store gets the "succeed" state set
  in STQ. The problem is that those two events can happen at same clock
  cycle and which of those will be called first is not deterministic.
  The update function can be called before the prereq function. So we 
  need to come up with a mechanism which could assign priorities to the
  functions if multiple occur at the same clock cycle.

  The problem with merging stores on commit is that some stores in the sequence
  that we are merging could not be written. For example, consider sequecnce of
  stores committing. They all go to the same cache-line so we decide to merge them
  at commit. The problem is that we assume that when

[November 16, 2023]
- The issue with some stores not globally performing correctly was identified.
  The chronology of store iid 362:
   * Commits @ timestamp 5187
   * Hits the cache @ 5190
   * Store completes @ 5191
   * Store updates the cacheline data @ 5191 as well
  The issue is that the function that updates the cache gets called before the store
  complete. The strong assumption I made for this approach to work is that store
  complete fuction will called before the update, which is not true in this case.

[November 15, 2034]
- The condition for forwarding seemed to be a bit off. The overlap function did
  not check for the fact that the load address of the load-to-perform instruction
  fully overlap with the stores that should be forwarded to the load.
- It seems the stores are not globally performing correctly. Yesterday's question
  on why store iid 362 merges so many stores is still valid. It could be that it
  never gets globally performed, hence not recycled from the memop queue.
  How can we check this? This is only possible if the cacheline for that tag
  never went to M state. Is it possible?

[November 14, 2023]
- Fixed FAIL cases for m3.
- Understood the forwarding mechanism from stores to loads in M3.
- Check has_partial_overlap() function: double check the conditions 
  and also think about if it is a complete overlap.
- Check why iid 362 merges so many stores? Why it is not being globally performed?
  Maybe it is indeed the case, as the data gets gathered.

[November 9, 2023]
- Got a good understanding of how stores are merged in M3 today.
- Next, disable the load check on dromjo side.
- Trace global stores.
- Add Dromjo DPI for interrupts/exceptions.

[October 26, 2023]
- Counter added, recording events in the tracer.
- Next, (1) in the tracer class implement trace_dump method.
        (2) write/extend existing python script to visualize traces.

[October 25, 2023]
- The script punch.py was extended to add modules, in particular to add global counter for now.
- Next, add counter ports to all hooks, test, and add events to tracer.

[October 22, 2023]
- Linking issue fixed.
- Confirm/validate visually through waveforms that DPIs work as intended.

[October 19, 2023]
- Fix linking issue.

[October 18, 2023]
- The current problem is that I am not reseting the stores. The mapped stq_idx->memop_info
  never get invalidated, so when the add_data or add_address is triggered, we retreive the 
  memop_info, but it has old information. This needs to be solved by triggering store
  complition, on which it should get reset.

[October 17, 2023]
- Implemented locally performing of the store. There is an assert happening. Look at the trace.

[August 22, 2023]
 - 
[May 24, 2023]
 - Figure out the correct ordering of the binary. How it should be laid out for BOOM to read it properly. +
 - Try running Dromajo generated bootram on BOOM. + 
 - Also load the main mem.
Notes:
???LINES MISSING

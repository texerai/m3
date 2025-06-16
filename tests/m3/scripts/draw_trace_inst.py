import pandas as pd
import plotly.express as px

# Constants.
kEventInsertMemop = 0
kEventAddAddress = 1
kEventPerformLocalStore = 2
kEventPerformLoad = 3
kEventCommitMemop = 4
kEventPerformGlobalStoreCache = 5
kEventPerformGlobalStoreMshr = 6

kEventIdToEventName = { kEventInsertMemop: "Insert Memop", \
    kEventAddAddress: "Add address", \
    kEventPerformLocalStore: "Perform Local Store", \
    kEventPerformLoad: "Perform Load", \
    kEventCommitMemop: "Commit Memop", \
    kEventPerformGlobalStoreCache: "Perform Global Store", \
    kEventPerformGlobalStoreMshr: "Global Store on Miss" }

kCoreMemopIdToName = { 0: "Core 0 Load", 1: "Core 0 Store", 2: "Core 1 Load", 3: "Core 1 Store" }
GoldmemIdToLocalId = { 0: {}, 1: {}}
def GetLocalId(core, goldmem_id):
    if goldmem_id not in GoldmemIdToLocalId[core]:
        size = len(GoldmemIdToLocalId[core])
        GoldmemIdToLocalId[core][goldmem_id] = size
    print("core {}: {} -> {}".format(core, goldmem_id, GoldmemIdToLocalId[core][goldmem_id]))
    return GoldmemIdToLocalId[core][goldmem_id]

def GetEntryFromList(data_list, id, is_int = True, is_decimal = False):
    entry = 0
    for data in data_list:
        if id in data:
            pair = data.split("=")
            if is_int:
                entry = int(pair[1], 16 - 6*int(is_decimal))
            else:
                entry = pair[1]
    return entry

def parse_file(file_name):
    plot_data = []

    # Parse ADD_ADDRESS.
    kEvent = "ADD_ADDRESS"
    kOffset = 12
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            if kEvent in line:
                data = line.split(":")
                clock_cycle = GetEntryFromList(data, "clock_cycle", is_decimal=True)
                memop = GetEntryFromList(data, "memop", is_int = False)
                core = GetEntryFromList(data, "core")
                goldmem_id = GetEntryFromList(data, "goldmem_id", is_int = False)
                memop_data = "N/A"

                # Calculate Y on the axis based on the memop and the core.
                local_inst_id = GetLocalId(core, goldmem_id)

                # Save plot data.
                name = kEventIdToEventName[kEventAddAddress]
                y_name = "core {} {} {}".format(core, memop, local_inst_id)
                plot_data.append([clock_cycle, y_name, kEventAddAddress, name, memop_data])

    # Parse PERFORM_STORE_LOCAL.
    kEvent = "PERFORM_STORE_LOCAL"
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            if kEvent in line:
                data = line.split(":")
                clock_cycle = GetEntryFromList(data, "clock_cycle", is_decimal=True)
                core = GetEntryFromList(data, "core")
                memop_data = GetEntryFromList(data, "store_data_dut")
                goldmem_id = GetEntryFromList(data, "goldmem_id", is_int = False)

                # Calculate Y on the axis based on the memop and the core.
                # This is a store, so hardcode the offset.
                local_inst_id = GetLocalId(core, goldmem_id)

                # Save plot data.
                name = kEventIdToEventName[kEventPerformLocalStore]
                y_name = "core {} {} {}".format(core, "store", local_inst_id)
                plot_data.append([clock_cycle, y_name, kEventPerformLocalStore, name, memop_data])

    # Parse PERFORM_LOAD.
    kEvent = "PERFORM_LOAD"
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            if kEvent in line:
                data = line.split(":")
                clock_cycle = GetEntryFromList(data, "clock_cycle", is_decimal=True)
                core = GetEntryFromList(data, "core")
                memop_data = GetEntryFromList(data, "load_data")
                goldmem_id = GetEntryFromList(data, "goldmem_id", is_int = False)

                # Calculate Y on the axis based on the memop and the core.
                # This is a load, so hardcode the offset.
                local_inst_id = GetLocalId(core, goldmem_id)

                # Save plot data.
                name = kEventIdToEventName[kEventPerformLoad]
                y_name = "core {} {} {}".format(core, "load", local_inst_id)
                plot_data.append([clock_cycle, y_name, kEventPerformLoad, name, memop_data])

    # Parse COMMIT_MEMOP.
    kEvent = "COMMIT_MEMOP"
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            if kEvent in line:
                data = line.split(":")
                clock_cycle = GetEntryFromList(data, "clock_cycle", is_decimal=True)
                memop = GetEntryFromList(data, "memop", is_int = False)
                core = GetEntryFromList(data, "core")
                memop_data = "N/A"
                goldmem_id = GetEntryFromList(data, "goldmem_id", is_int = False)

                # Calculate Y on the axis based on the memop and the core.
                local_inst_id = GetLocalId(core, goldmem_id)

                # Save plot data.
                name = kEventIdToEventName[kEventCommitMemop]
                y_name = "core {} {} {}".format(core, memop, local_inst_id)
                plot_data.append([clock_cycle, y_name, kEventCommitMemop, name, memop_data])

    # Parse COMMIT_MEMOP.
    kEvent = "PERFORM_STORE_GLOBAL:"
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            if kEvent in line:
                data = line.split(":")
                clock_cycle = GetEntryFromList(data, "clock_cycle", is_decimal=True)
                memop = GetEntryFromList(data, "memop", is_int = False)
                core = GetEntryFromList(data, "core")
                memop_data = GetEntryFromList(data, "store_data")
                goldmem_id = GetEntryFromList(data, "goldmem_id", is_int = False)

                # Calculate Y on the axis based on the memop and the core.
                local_inst_id = GetLocalId(core, goldmem_id)

                # Save plot data.
                name = kEventIdToEventName[kEventPerformGlobalStoreCache]
                y_name = "core {} {} {}".format(core, "store", local_inst_id)
                plot_data.append([clock_cycle, y_name, kEventPerformGlobalStoreCache, name, memop_data])

    return plot_data

def main():
    plot_data = parse_file("goldmem_trace")
    df = pd.DataFrame(plot_data, columns=["time", "core_op", "event", "name", "data"])
    print(df)

    fig = px.scatter(df, x="time", y="core_op", color="event", hover_data=["data"], \
        symbol="name", labels={"name": "Event names", "core_op": "Memory Operations", "time": "Clock Cycles", "data": "Memop Data"})
    #fig.update_traces(mode="markers", hovertemplate=None)
    #fig.update_layout(hovermode="x unified")
    fig.update_traces(marker_size=10)
    fig.update(layout_coloraxis_showscale=False)
    fig.update_yaxes(categoryorder='category descending')
    fig.show()

main()

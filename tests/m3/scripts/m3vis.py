import json
import pandas as pd
import plotly.express as px
import sys

# Constants.
EVENT_ID = {}
EVENT_ID["MEMOP_IN"] = 0
EVENT_ID["LOAD_PERFORM"] = 1
EVENT_ID["STORE_LOCAL_PERFORM"] = 2
EVENT_ID["STORE_GLOBAL_PERFORM"] = 3
EVENT_ID["LOAD_PERFORM_FAILED"] = 4
EVENT_ID["MEMOP_COMMIT"] = 5
id_to_address = {}

def get_event_id(event_name):
    ret = -1
    if event_name in EVENT_ID:
        ret = EVENT_ID[event_name]
    else:
        print("Warning: unrecognized event name in the trace file.")
    return ret

def parse_file(file_name, address, time):
    plot_data = []
    with open(file_name, "r") as infile:
        jdata = json.load(infile)
        for record in jdata["records"]:
            if get_event_id(record["event"]) in [1, 2, 4]:
                id_to_address[int(record["memop_id"])] = int(record["address"], base=16)

        for record in jdata["records"]:
            should_plot = False
            m_id = int(record["memop_id"])
            if m_id in id_to_address:
                if address == id_to_address[m_id]:
                    should_plot = True
                elif get_event_id(record["event"]) == 3 and (address >> 12) == int(record["address"]):
                    should_plot = True

            t = int(record["timestamp"])
            if should_plot and t > time:
                y = "core {} {} {}".format(record["hart_id"], record["memop_type"], record["memop_id"])
                event_name = record["event"]
                event_id = get_event_id(event_name)
                data = "M:{};".format(hex(int(record["model_data"])))
                if event_id == 1:
                    data = "M:{};RTL:{}".format(hex(int(record["model_data"])), hex(int(record["rtl_data"])))
                plot_data.append([t, y, event_id, event_name, data])

    return plot_data

def main():
    file_name = sys.argv[1]
    base = 10
    if "0x" in sys.argv[2]:
        base = 16
    time = int(sys.argv[3])
    address = int(sys.argv[2], base)
    plot_data = parse_file(file_name, address, time)
    df = pd.DataFrame(plot_data, columns=["time", "core_op", "event", "name", "data"])
    print(df)

    fig = px.scatter(df, x="time", y="core_op", color="event", hover_data=["data"], \
        symbol="name", labels={"name": "Event names", "core_op": "Memory Operations", "time": "Clock Cycles", "data": "Memop Data"})
    fig.update_traces(marker_size=10)
    fig.update(layout_coloraxis_showscale=False)
    fig.update_yaxes(categoryorder='category descending')
    fig.show()

main()

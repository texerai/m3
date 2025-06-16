import pandas as pd
import plotly.express as px
import sys

WINDOW_SIZE = int(sys.argv[2])

def main(file_name):
    events = []
    unique_seq = []
    with open(file_name, "r") as trace_file:
        for line in trace_file:
            data = line.split(":")
            event = "{}-{}".format(data[1], data[2])

            events.append(event)
            if len(events) >= WINDOW_SIZE:
                str_events = "|".join(events)
                if str_events not in unique_seq:
                    unique_seq.append(str_events)
                print(len(unique_seq))
                events.pop(0)

main(sys.argv[1])

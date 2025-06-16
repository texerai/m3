import sys
import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

parse_file = sys.argv[1]

kStringMarionetteTraceLineId = "MARIONETTE_TRACE:"
kStringUpdateBHTTraceLineId = "UpdateBHT()"
kStringBpredId = "BPRED_0016"

# Initialize plot data structure.
plot_data = {}
line_names = []
line_names.append("Actual")
line_names.append("BPRED_0016")
line_names.append("BPRED_0032")
line_names.append("BPRED_0064")
line_names.append("BPRED_0128")
line_names.append("BPRED_0256")
line_names.append("BPRED_0512")
line_names.append("BPRED_001K")
line_names.append("BPRED_002K")
line_names.append("BPRED_004K")
line_names.append("BPRED_008K")
line_names.append("BPRED_016K")
line_names.append("BPRED_032K")
line_names.append("BPRED_064K")
line_names.append("BPRED_128K")
line_names.append("BPRED_256K")
line_names.append("BPRED_512K")
count = 0
for line_name in line_names:
    plot_data[line_name] = {"count": [], "mispred": []}
# Parse data.
with open(parse_file) as in_file:
    for line in in_file:
        if kStringMarionetteTraceLineId in line and kStringUpdateBHTTraceLineId in line:
            data = line.split(";")

            if kStringBpredId in line:
                count += 1
                actual_mispred_count = int(data[3].split(",")[1].split(")")[0])
                plot_data["Actual"]["count"].append(count)
                plot_data["Actual"]["mispred"].append(actual_mispred_count)


            bpred_id = data[0].split(":")[1][1:]
            mispred_count = int(data[3].split("(")[1].split(",")[0])
            plot_data[bpred_id]["count"].append(count)
            plot_data[bpred_id]["mispred"].append(mispred_count)

# Chop some data.
cut_point = 220000
for line_name in line_names:
    plot_data[line_name] = {"count": plot_data[line_name]["count"][cut_point:], "mispred": plot_data[line_name]["mispred"][cut_point:]}

# Plot.
lines = []
legend_line_names = []
fig, axis = plt.subplots()
for line_name in line_names:
    line_color = 'grey'
    style = '-'
    is_regular = True
    line_width = 1
    if "Actual" in line_name:
        line_color = 'blue'
        is_regular = False
        line_width = 2
    elif "BPRED_0128" in line_name:
        line_color = 'red'
        style = '--'
        is_regular = False
        line_width = 2

    # Build data frame.
    df = pd.DataFrame({ 'branches': np.array(plot_data[line_name]["count"]), 'mispreds': np.array(plot_data[line_name]["mispred"]) })
    
    # Plot the line.
    df.plot.line(ax=axis, x='branches', y=["mispreds"], linestyle=style, linewidth=line_width, color=line_color)

# Annotate 16 entries.
font_size_annot = 'large'
x_annot = int(plot_data["BPRED_0016"]["count"][-1])+200
y_annot = plot_data["BPRED_0016"]["mispred"][-1]
axis.text(x=x_annot+20, y=y_annot, s="16 entr.", color='grey', fontsize=font_size_annot)

# Annotate 128 and Actual.
x_annot = plot_data["BPRED_0128"]["count"][-1]
y_annot = plot_data["BPRED_0128"]["mispred"][-1]
axis.text(x=x_annot, y=y_annot+40, s="Actual", color='blue', fontsize=font_size_annot)
axis.text(x=x_annot, y=y_annot-120, s="128 entr.", color='red', fontsize=font_size_annot)

# Annotate 512K.
x_annot = plot_data["BPRED_512K"]["count"][-1]
y_annot = plot_data["BPRED_512K"]["mispred"][-1]
axis.text(x=x_annot, y=y_annot, s="512K entr.", color='grey', fontsize=font_size_annot)

axis.set_xlabel("Branch occurance in program order", fontsize=16)
axis.set_ylabel("Misprediction count", fontsize=16)
axis.grid("on", linestyle='dotted', linewidth=0.5, color="#aaaaaa")

# Remove frames.
axis.spines['top'].set_visible(False)
axis.spines['right'].set_visible(False)

blue_patch = mpatches.Patch(color='blue', label='128 entry Actual CVA6 BPred')
red_patch = mpatches.Patch(color='red', label='128 entry M.M. BPred')
grey_patch = mpatches.Patch(color='grey', label='Various size M.M. BPreds (16->512K entries)')
plt.legend(handles=[blue_patch, red_patch, grey_patch], prop={ 'size': 11 })
plt.savefig("plot.pdf")
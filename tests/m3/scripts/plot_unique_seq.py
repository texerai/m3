import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter

g1 = []
with open("g3_w8", "r") as infile:
  for line in infile:
    g1.append(int(line))


g2 = []
with open("g4_w8", "r") as infile:
  for line in infile:
    g2.append(int(line))

g2 = g2[0:len(g1)]
x = []
for i in range(len(g1)):
  x.append(i)

print(len(g1), len(g2), len(x))

#plt.title("Occurance of unique memory operation sequences")
plt.xlabel("Cycles (in millions)")
plt.ylabel("# of unique event orderings")

plt.plot(x, g1, 'c', label='with LF')
plt.plot(x, g2, 'y', label='without LF')

plt.grid()
plt.subplots_adjust(left=0.2)
plt.legend()


plt.savefig("mm_with_lf.pdf")



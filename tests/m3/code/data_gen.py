num_zeros = 1024
num_cols = 32
num_rows = int(num_zeros/num_cols)

with open("data.h", "w") as out_file:
  out_file.write("#pragma once\n")
  out_file.write("volatile int data[{}] = ".format(num_zeros))
  out_file.write("{\n")
  for i in range(num_rows):
    out_file.write("    ")
    for j in range(num_cols - 1):
      out_file.write("0, ")

    if i < num_rows - 1:
      out_file.write("0,\n")
    else:
      out_file.write("0\n")
  out_file.write("};")

riscv64-unknown-elf-gcc \
    -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -fno-tree-loop-distribute-patterns \
    -static -nostdlib -nostartfiles -lgcc -T./test.ld \
    -o race.riscv \
    race_generator.cpp

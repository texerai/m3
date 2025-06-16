import os
import subprocess
import sys

CONFIG = sys.argv[1]
ID = sys.argv[2]
TOP_MODULE = 'TestHarness'

# Constant file and dir names.
#SIM_FILES_LIST = 'sim_files.common.f'
SIM_FILES_LIST = 'sim_files.common.f.patched.f'
VERI_GEN = 'veri_generated'

# Paths configurations.
CURRENT_PATH = os.path.dirname(os.path.realpath(sys.argv[0]))
RISCV = '/home/kabylkas/riscv'
DRAMSIM_PATH = '{}/models/dram'.format(CURRENT_PATH)
DROMAJO_PATH = '{}/models/dromajo'.format(CURRENT_PATH)
M3_PATH = '{}/models/m3'.format(CURRENT_PATH)
CHIPYARD = '/home/kabylkas/chipyard'
RTL_ORIG = '{}/sims/verilator/generated-src/chipyard.TestHarness.{}'.format(CHIPYARD, CONFIG)
RTL_LOCAL = '{}/core_rtl/{}_{}'.format(CURRENT_PATH, CONFIG, ID)
SIM_PATH = '{}/sim-{}-{}'.format(CURRENT_PATH, CONFIG, ID)
TESTBENCH_PATH = '{}/testbench'.format(CURRENT_PATH)
MODEL_PATH = '{}/models'.format(CURRENT_PATH)

def get_time_from_output(output):
    data = output.split()
    for datum in data:
        if 'elapsed' in datum:
            time = datum.split("elapsed")[0]
            return time
    return "0:00.00"

def run_command(msg, cmd, should_print=True, should_log=False):
    if should_print:
        print(msg)

    res = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    if should_log or not res.returncode == 0:
        with open("fail.log", "w") as fail_file:
            fail_file.write("Std outpupt:\n")
            fail_file.write(res.stdout)
            fail_file.write("Std error:\n")
            fail_file.write(res.stderr)

    if should_print:
        time = get_time_from_output(res.stderr)
        if res.returncode == 0:
            print(" - Success [{}]".format(time))
        else:
            print(" - Fail [{}]".format(time))
            exit(1)

def gen_verilate_command():
    c_flags = []
    c_flags.append('-O2')
    c_flags.append('-std=c++17')
    # c_flags.append('-g') # Slows down the compilation significantly.
    c_flags.append('-I{}/include'.format(RISCV))
    c_flags.append('-I{}/lib'.format(DRAMSIM_PATH))
    c_flags.append('-I{}/src'.format(DRAMSIM_PATH))
    c_flags.append('-I{}/uart/src'.format(MODEL_PATH))
    c_flags.append('-I{}/dromajo/include'.format(MODEL_PATH))
    c_flags.append('-I{}/m3/source'.format(MODEL_PATH))
    c_flags.append('-I{}/m3/bridge'.format(MODEL_PATH))
    c_flags.append('-I{}'.format(TESTBENCH_PATH))
    c_flags.append('-I{}/rtl'.format(RTL_LOCAL))
    #c_flags.append('-D__STDC_FORMAT_MACROS')
    c_flags.append('-DVERILATOR')
    c_flags.append('-include {}/boom.plusArgs'.format(RTL_LOCAL))
    c_flags.append('-include {}/{}/V{}.h'.format(SIM_PATH, VERI_GEN, TOP_MODULE))
    c_flags.append('-include {}/rtl/verilator.h'.format(RTL_LOCAL))
    all_c_flags = " ".join(c_flags)

    ld_flags = []
    ld_flags.append('-L{}/lib'.format(RISCV))
    ld_flags.append('-L{}/build'.format(DROMAJO_PATH))
    ld_flags.append('-L{}/build/output'.format(M3_PATH))
    ld_flags.append('-Wl,-rpath,{}/lib,-rpath,{}/lib,-rpath,{}/build,-rpath,{}/build/output'.format(RISCV, DRAMSIM_PATH, DROMAJO_PATH, M3_PATH))
    ld_flags.append('-L{}'.format(CURRENT_PATH))
    ld_flags.append('-L{}/lib'.format(DRAMSIM_PATH))
    ld_flags.append('-lfesvr')
    ld_flags.append('-ldramsim')
    ld_flags.append('-ldromajo_cosim')
    ld_flags.append('-lm3')
    all_ld_flags = " ".join(ld_flags)

    veri_flags = []
    veri_flags.append('--cc')
    veri_flags.append('--exe')
    veri_flags.append('--threads 1')
    veri_flags.append('--threads-dpi all')
    veri_flags.append('--timescale 1ns/1ps')
    veri_flags.append('-O3')
    veri_flags.append('--sv')
    veri_flags.append('--x-assign fast')
    veri_flags.append('--x-initial fast')
    veri_flags.append('--output-split 10000')
    veri_flags.append('--output-split-cfuncs 100')
    veri_flags.append('--assert -Wno-fatal')
    veri_flags.append('--max-num-width 1048576')
    veri_flags.append('--top-module {}'.format(TOP_MODULE))
    veri_flags.append('-f {}/{}'.format(RTL_LOCAL, SIM_FILES_LIST))
    veri_flags.append('-o {}/sim'.format(SIM_PATH))
    #veri_flags.append('--vpi')
    veri_flags.append('--trace-fst')
    veri_flags.append('-Mdir {}/{}'.format(SIM_PATH, VERI_GEN))
    all_veri_flags = " ".join(veri_flags)

    command = 'time verilator -CFLAGS " {}" -LDFLAGS " {}" {}'.format(all_c_flags, all_ld_flags, all_veri_flags)

    return command

def create_local_dir():
    command = 'time mkdir -p {}/rtl'.format(RTL_LOCAL)
    msg = 'Creating local directory {}'.format(RTL_LOCAL)
    run_command(msg, command)

def build_dependencies():
    # Build DRAMSim.
    command = 'cd {}/lib&&time make libdramsim.a'.format(DRAMSIM_PATH)
    msg = 'Building dependencies: DRAMSim...'
    run_command(msg, command)
    command = 'cd {}/build&&time make'.format(DROMAJO_PATH)
    msg = 'Building dependencies: Dromajo...'
    run_command(msg, command)
    command = 'cd {}/build&&time make'.format(M3_PATH)
    msg = 'Building dependencies: M3...'
    run_command(msg, command)

def copy_files():
    # Copy RTL.
    command = 'time cp -r {}/gen-collateral/. {}/rtl/.'.format(RTL_ORIG, RTL_LOCAL)
    msg = 'Copying RTL...'
    run_command(msg, command)

    # Copy plusargs.
    command = 'time cp {}/chipyard.TestHarness.{}.plusArgs {}/boom.plusArgs'.format(RTL_ORIG, CONFIG, RTL_LOCAL)
    msg = 'Copying plus arguments file...'
    run_command(msg, command)

    # Copy file list.
    command = 'time cp {}/{} {}/.'.format(RTL_ORIG, SIM_FILES_LIST, RTL_LOCAL)
    msg = 'Copying design file list...'
    run_command(msg, command)

def clean_up():
    full_path = RTL_LOCAL + "/rtl"
    paths_to_del = []
    paths_to_del.append("{}/mm.cc".format(full_path))
    paths_to_del.append("{}/mm.h".format(full_path))
    paths_to_del.append("{}/mm_dramsim2.cc".format(full_path))
    paths_to_del.append("{}/mm_dramsim2.h".format(full_path))
    paths_to_del.append("{}/uart.cc".format(full_path))
    paths_to_del.append("{}/uart.h".format(full_path))

    command = "rm -rf {}".format(" ".join(paths_to_del))
    msg = "Removing unnecessary files..."
    run_command(msg, command)


def edit_file_list():
    # Generate new sim files list in temporary file.
    full_path = RTL_LOCAL + "/rtl"
    file_list_path = "{}/{}".format(RTL_LOCAL, SIM_FILES_LIST)
    with open(file_list_path, "r") as infile, open("temp.f", "w") as outfile:
        for line in infile:
            path = line.strip()
            if "emulator.cc" in path:
                outfile.write("{}/sim_cli.cpp\n".format(TESTBENCH_PATH))
                outfile.write("{}/simulator.cpp\n".format(TESTBENCH_PATH))
                outfile.write("{}/fast_simulator.cpp\n".format(TESTBENCH_PATH))
                outfile.write("{}/debug_simulator.cpp\n".format(TESTBENCH_PATH))
                outfile.write("{}/model_instances.cpp\n".format(TESTBENCH_PATH))
            elif "plusarg_reader.v" in path:
                outfile.write("{}/dummy/plusarg_reader.v\n".format(MODEL_PATH))
            elif "uart" in path.lower():
                if "SimUART.v" in path:
                    outfile.write("{}/uart/dpi/uart_wrapper.v\n".format(MODEL_PATH))
                elif "SimUART.cc" in path:
                    outfile.write("{}/uart/dpi/uart_dpi.cpp\n".format(MODEL_PATH))
                elif "uart.cc" in path:
                    outfile.write("{}/uart/src/uart.cpp\n".format(MODEL_PATH))
                elif "uart.h" in path:
                    outfile.write("{}/uart/src/uart.h\n".format(MODEL_PATH))
                else:
                    outfile.write("{}/{}\n".format(full_path, os.path.basename(path)))
            elif "SimDTM" in path:
                pass
            elif "dram" in path.lower() or "mm" in path.lower():
                if "SimDRAM.cc" in path:
                    outfile.write("{}/dram/dpi/dram_dpi.cpp\n".format(MODEL_PATH))
                elif "SimDRAM.v" in path:
                    outfile.write("{}/dram/dpi/dram_wrapper.v\n".format(MODEL_PATH))
                elif "mm_dramsim2.cc" in path:
                    outfile.write("{}/dram/src/mm_dramsim2.cpp\n".format(MODEL_PATH))
                elif "mm.cc" in path:
                    outfile.write("{}/dram/src/mm.cpp\n".format(MODEL_PATH))
                elif "mm.h" in path:
                    outfile.write("{}/dram/src/mm.h\n".format(MODEL_PATH))
                else:
                    outfile.write("{}/{}\n".format(full_path, os.path.basename(path)))
            elif "serial" in path.lower():
                if "SimSerial.v" in path:
                    outfile.write("{}/dummy/serial_wrapper.v\n".format(MODEL_PATH))
                elif "SimSerial.cc" in path:
                    pass
                else:
                    outfile.write("{}/{}\n".format(full_path, os.path.basename(path)))    
            else:
                outfile.write("{}/{}\n".format(full_path, os.path.basename(path)))

    # Replace files.
    command = 'time cp temp.f {}'.format(file_list_path)
    msg = 'Chanding path to design files in file list...'
    run_command(msg, command)

    # Silently remove temp file.
    run_command("", "rm -rf temp.f", should_print=False)

def create_sim_dir():
    command = 'time mkdir -p {}'.format(SIM_PATH)
    msg = 'Creating simulator directory {}'.format(SIM_PATH)
    run_command(msg, command)

def generate_verilator_model():
    command = gen_verilate_command()
    msg = 'Generating verilator model in {}/{}'.format(SIM_PATH, VERI_GEN)
    #print(command)
    run_command(msg, command)

def build_verilator_model():
    command = 'time make -j10 -C {}/{} -fV{}.mk'.format(SIM_PATH, VERI_GEN, TOP_MODULE)
    msg = 'Building verilator model for config {}'.format(CONFIG)
    #print(command)
    run_command(msg, command)

def main():
    if len(sys.argv) < 4:
        print("Provide a Chipyard generator configuration name, ID and command.")
        print("Example to copy, generate model and build the model:")
        print("  python3 config.py SmallBoomConfig a cgb")
        exit(1)

    if not os.path.isdir(RTL_ORIG):
        print("{} is not a directory".format(RTL_ORIG))
        exit(1)

    args = sys.argv[3]

    if "c" in args:
        create_local_dir()
        build_dependencies()
        copy_files()
        edit_file_list()
        clean_up()

    if "g" in args:
        create_sim_dir()
        generate_verilator_model()

    if "b" in args:
        build_dependencies()
        build_verilator_model()

main()

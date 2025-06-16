# Copyright (c) 2023 Kabylkas Lab. All rights reserved.
import os
import re
import sys
import subprocess

was_patched = []
generated_files = []

class FileWriter:
    def __init__(self, file):
        self.file = file

    def line(self, line_contents, indentation_size = 0):
        indentation = ""

        # Add indentation.
        for i in range(indentation_size):
            indentation += "    "

        self.file.write("{}{}\n".format(indentation, line_contents))

    def append(self, contents):
        self.file.write(contents)

class Data:
    def __init__(self, action):
        self.action = action
        self.clock_signal = ""
        self.reset_signal = ""
        self.trigger_signal = ""
        self.edged_trigger = False
        self.pos_edged = False
        self.neg_edged = False
        self.assign_port_name = ""
        self.signal_to_search = ""
        self.dpi_signals = []
        self.punch_name = ""
        self.inst_module_lines = []

def read_verilog_files_list(file_path):
    sv_files = []

    try:
        with open(file_path, 'r') as file:
            for line in file:
                line = line.strip()
                if line.endswith(".sv") or line.endswith(".v"):
                    sv_files.append(line)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return None

    return sv_files

def get_module_to_file_map(sv_files_list):
    module_to_file_map = {}

    module_pattern = re.compile(r'\s*module\s+(\w+)\s*\(')

    for sv_file in sv_files_list:
        try:
            with open(sv_file, 'r') as file:
                for line in file:
                    match = module_pattern.search(line)
                    if match:
                        module_name = match.group(1)
                        module_to_file_map[module_name] = sv_file
        except FileNotFoundError:
            print(f"File not found: {sv_file}")
            continue

    return module_to_file_map

def get_module_to_submodules_map(sv_files_list):
    module_to_submodules_map = {}

    module_pattern = re.compile(r'\s*module\s+(\w+)\s*\(')
    submodule_pattern = re.compile(r'\s*(\w+)\s+(\w+)\s*\(')

    for sv_file in sv_files_list:
        current_module = None

        try:
            with open(sv_file, 'r') as file:
                for line in file:
                    if "$" in line or "else" in line:
                        continue
                    module_match = module_pattern.search(line)
                    if module_match:
                        current_module = module_match.group(1)
                        if current_module not in module_to_submodules_map:
                            module_to_submodules_map[current_module] = []
                    else:
                        submodule_match = submodule_pattern.search(line)
                        if submodule_match and current_module is not None:
                            submodule_type = submodule_match.group(1)
                            submodule_instance = submodule_match.group(2)
                            module_to_submodules_map[current_module].append({ "module": submodule_type, "instance": submodule_instance })

        except FileNotFoundError:
            print(f"File not found: {sv_file}")
            continue

    return module_to_submodules_map

def gen_wrapper(output_file_path, data):
    punch_name = data.punch_name
    dpi_signals = data.dpi_signals
    clock_port_name = data.clock_signal
    reset_port_name = data.reset_signal
    trigger_port_name = data.trigger_signal
    edged_trigger = data.edged_trigger
    generated_files.append(output_file_path)
    with open(output_file_path, "w") as out_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        # Header.
        writer.line("// Auto generated file. Do not edit.")
        writer.line("// Copyright (c) 2021-2023 Kabylkas Labs. All rights reserved.")
        writer.line("")

        # Declare imports.
        writer.line("// Generated import declaration.")
        writer.line("import \"DPI-C\" function void {}_DPI(".format(punch_name))
        for signal in dpi_signals:
            type = "int"
            if signal["port_size"] > 32:
                type = "longint"

            if not signal["port_name"] in trigger_port_name:
                line = "input {} {}".format(type, signal["port_name"])
                if not signal["port_name"] == dpi_signals[-1]["port_name"]:
                    line += ","
                writer.line(line, 1)
        writer.line(");")
        writer.line("")

        # Generate module defintion and ports.
        writer.line("// Generated wrapper module.")
        writer.line("module {}_wrapper (".format(punch_name))
        arguments = ""
        writer.line("input clock,", 1)
        writer.line("input reset,", 1)
        for signal in dpi_signals:
            port_line = "input "
            if signal["port_size"] > 1:
                port_line += "[{}:0] ".format(signal["port_size"] - 1)

            port_line += signal["port_name"]
            if not signal["port_name"] == data.trigger_signal:
                arguments += signal["port_name"]
            if not signal["port_name"] == dpi_signals[-1]["port_name"]:
                port_line += ","
                if len(arguments) > 0:
                    if not arguments[-1] == ',':
                        arguments += ","

            writer.line(port_line, 1)

        writer.line(");")

        # Generate sequential access to DPI.
        if not edged_trigger:
            writer.line("always @(posedge clock) begin", 1)
            writer.line("if (!reset) begin", 2)
            writer.line("if ({}) begin".format(trigger_port_name), 3)
            writer.line("{}_DPI({});".format(punch_name, arguments), 4)
            writer.line("end", 3)
            writer.line("end", 2)
            writer.line("end", 1)
        else:
            writer.line("reg prev;", 1)
            writer.line("reg trigger_once;", 1)
            writer.line("always @(posedge clock)", 1)
            writer.line("begin", 1)
            writer.line("if (reset) begin", 2)
            writer.line("trigger_once <= 1'b0;", 3)
            writer.line("prev <= 1'b0;", 3)
            writer.line("end else begin", 2)
            writer.line("if (!prev && {}) begin".format(trigger_port_name), 3)
            writer.line("trigger_once <= 1'b1;", 4)
            writer.line("end else begin", 3)
            writer.line("trigger_once <= 1'b0;", 4)
            writer.line("end", 3)
            writer.line("prev <= {};".format(trigger_port_name), 3)
            writer.line("end", 2)
            writer.line("end", 1)

            writer.line("always @(negedge clock) begin", 1)
            writer.line("if (!reset) begin", 2)
            writer.line("if (trigger_once) begin", 3)
            writer.line("{}_DPI({});".format(punch_name, arguments), 4)
            writer.line("end", 3)
            writer.line("end", 2)
            writer.line("end", 1)


        # Generate end the module definition.
        writer.line("endmodule")

def gen_cpp_dpi_signature(dpi_cpp_path, data):
    signature = []
    signature.append("extern \"C\" void {}_DPI".format(data.punch_name))
    signature.append("(")
    if not dpi_cpp_path in generated_files:
        generated_files.append(dpi_cpp_path)
    for signal in data.dpi_signals:
        if signal["port_name"] == data.trigger_signal:
            continue

        type = "int"
        if signal["port_size"] > 32:
            type = "long long"

        comment = "    // {}:{}".format(signal["signal_name"], signal["port_size"])
        line = "    {} {}".format(type, signal["port_name"])
        if not signal["port_name"] == data.dpi_signals[-1]["port_name"]:
            line += ",\n"

        signature.append(comment)
        signature.append(line)
    signature.append(")")
    signature.append("{")
    signature.append("}")

    print("Suggested DPI function signature for {}:".format(data.punch_name))
    if not os.path.exists(dpi_cpp_path):
        print("DPI file does not exist... creating: {}".format(dpi_cpp_path))
        with open(dpi_cpp_path, "w") as out_file:
            for line in signature:
                out_file.write("{}\n".format(line))
    for line in signature:
        print(line)
    print("")

def add_code_after_port_def(file_name, module_name, data):
    module_pattern = re.compile(r'\s*module\s+(\w+)\s*\(')
    end_of_port_definition_pattern = re.compile(r'\s*\);')

    found_module = False
    should_add_assignment = False
    assignment_added = False

    content_to_write = []
    patched_file_name = "{}.patched.sv".format(file_name)
    if file_name in was_patched:
        file_name = patched_file_name
    else:
        was_patched.append(file_name)
    with open(file_name, "r") as infile:
        for line in infile:
            if "$" in line or "else" in line:
                content_to_write.append(line)
                continue

            if assignment_added:
                pass
            elif not found_module:
                module_match = module_pattern.search(line)
                if module_match:
                    current_module = module_match.group(1)
                    if current_module == module_name:
                        found_module = True
            else:
                end_of_port_def_match = end_of_port_definition_pattern.search(line)
                if end_of_port_def_match:
                    should_add_assignment = True

            content_to_write.append(line)
            if should_add_assignment:
                assignment_added = True
                should_add_assignment = False;
                if data.action == "assignment":
                    content_to_write.append("assign {} = {};\n".format(data.assign_port_name, data.signal_to_search))
                elif data.action == "inst_wrapper":
                    punch_name = data.punch_name
                    content_to_write.append("{0}_wrapper {0}_wrapper_inst (\n".format(punch_name))
                    content_to_write.append(".clock({}),\n".format(data.clock_signal))
                    content_to_write.append(".reset({}),\n".format(data.reset_signal))
                    for s in data.dpi_signals:
                        content_to_write.append(".{0}({0})".format(s["port_name"]))
                        if not s["port_name"] == data.dpi_signals[-1]["port_name"]:
                            content_to_write[-1] += ',\n'
                        else:
                            content_to_write[-1] += '\n'
                    content_to_write.append(");\n")
                elif data.action == "dpi_signals":
                    for s in data.dpi_signals:
                        if s["port_size"] > 1:
                            content_to_write.append("wire [{}:0] {};\n".format(s["port_size"]-1, s["port_name"]))
                        else:
                            content_to_write.append("wire {};\n".format(s["port_name"]))
                elif data.action == "inst_module":
                    for inst_line in data.inst_module_lines:
                        content_to_write.append(inst_line)
                else:
                    print("Error: unknown action for code writing after port defintions.")
                    exit(1)

    with open(patched_file_name, "w") as outfile:
        for line in content_to_write:
            outfile.write(line)

def alter_file(file_name, top_module, module_name, instance_in_module, port_name, port_size):
    module_pattern = re.compile(r'\s*module\s+(\w+)\s*\(')
    submodule_pattern = re.compile(r'\s*(\w+)\s+(\w+)\s*\(')

    should_add_module_port = False
    should_add_instance_port = False
    current_module = None

    content_to_write = []
    patched_file_name = "{}.patched.sv".format(file_name)
    if file_name in was_patched:
        file_name = patched_file_name
    else:
        was_patched.append(file_name)
    with open(file_name, 'r') as infile:
        for line in infile:
            if "$" in line or "else" in line:
                content_to_write.append(line)
                continue

            module_match = module_pattern.search(line)
            if module_match:
                current_module = module_match.group(1)
                if current_module == module_name and not current_module == top_module:
                    should_add_module_port = True
            else:
                submodule_match = submodule_pattern.search(line)
                if submodule_match and current_module is not None:
                    submodule_instance = submodule_match.group(2)
                    if submodule_instance == instance_in_module:
                        should_add_instance_port = True

            content_to_write.append(line)
            if should_add_module_port:
                should_add_module_port = False
                if port_size > 1:
                    content_to_write.append("    output [{}:0] {},\n".format(port_size-1, port_name))
                else:
                    content_to_write.append("    output {},\n".format(port_name))

            if should_add_instance_port:
                should_add_instance_port = False
                content_to_write.append(".{0}({0}),\n".format(port_name))

    with open(patched_file_name, "w") as outfile:
        for line in content_to_write:
            outfile.write(line)

def punch_out(hierarchy, top_module, module_to_file_map, module_to_submodule_map, port_name, port_size):
    instances = hierarchy.split(".")
    module_to_search = top_module
    signal_to_search = instances[-1]

    # 1. Punch through the hierarchy.
    for instance in instances[:-1]:
        if module_to_search in module_to_submodule_map:
            found = False
            for pair in module_to_submodule_map[module_to_search]:
                if pair["instance"] == instance:
                    print(" - Punching through instance {} in module {}".format(instance, module_to_search))
                    alter_file(module_to_file_map[module_to_search], top_module, module_to_search, instance, port_name, port_size)
                    found = True
                    module_to_search = pair["module"]
                    break
            if not found:
                print("Instance {} was not found in Module {}".format(instance, module_to_search))
                exit(1)
        else:
            print("Module was not found: {}".format(module_to_search))
            exit(1)

    # 2. Do an assignment at the last level.
    alter_file(module_to_file_map[module_to_search], top_module, module_to_search, "", port_name, port_size)
    data = Data("assignment")
    data.assign_port_name = port_name
    data.signal_to_search = signal_to_search
    add_code_after_port_def(module_to_file_map[module_to_search], module_to_search, data)

def instantiate_module(data, top_module, module_to_file_map, instance_name, module_path):
    # Get the module name we are instantiating.
    module_name = ""
    module_pattern = re.compile(r'\s*module\s+(\w+)\s*\(')
    was_found = False
    with open(module_path, "r") as infile:
        for line in infile:
            module_match = module_pattern.search(line)
            if module_match:
                module_name = module_match.group(1)
                was_found = True
                break
    if not was_found:
        print("Error: No module definition was found in {}.".format(module_path))
        exit(1)

    # Copy the module to RTL folder.
    was_copied = False
    top_file_path = ""
    top_file_dir = ""
    if was_found:
        top_file_path = module_to_file_map[top_module]
        top_file_dir = os.path.dirname(top_file_path)
        cmd = "cp {} {}/.".format(module_path, top_file_dir)
        result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        was_copied = (result.returncode == 0)
    if not was_copied:
        print("Error: File {} was not copied.".format(module_path))
        exit(1)

    # Form module data.
    # TODO: for the purposes of faster forward progress
    # let's make this specific for counter now. We should make it
    # generic: identify all output ports of the module and create wires for
    # all output ports.
    if was_copied:
        generated_files.append('{}/{}'.format(top_file_dir, os.path.basename(module_path)))
        inst_data = []
        inst_data.append('wire [63:0] global_counter;\n')
        inst_data.append('{} {} (\n'.format(module_name, instance_name))
        inst_data.append('.clock({}),\n'.format(data.clock_signal))
        inst_data.append('.reset({}),\n'.format(data.reset_signal))
        inst_data.append('.count_o(global_counter));\n\n')
        data.inst_module_lines = inst_data
        add_code_after_port_def(top_file_path, top_module, data)


def put_wrapper(top_module_file_path, top_module, data):
    data.action = "inst_wrapper"
    add_code_after_port_def(top_module_file_path, top_module, data)

    data.action = "dpi_signals"
    add_code_after_port_def(top_module_file_path, top_module, data)

def hook_dpi(file_path, module_to_file_map, module_to_submodule_map):
    state = "search_begin"
    top_module = ""
    punch_name = ""
    punch_id = 0
    dpi_signals = []
    should_gen_wrapper = False
    clock_signal = ""
    reset_signal = ""
    dpi_group = ""
    trigger_signal = ""
    edged_trigger = False
    with open(file_path, "r") as infile:
        for line in infile:
            if "#" in line or len(line.strip()) == 0:
                continue

            if state == "search_begin":
                if "@begin" in line:
                    if "instantiate" in line:
                        data = line.split()
                        top_module = data[2].split(':')[1]
                        clock_signal = data[3].split(':')[1]
                        reset_signal = data[4].split(':')[1]
                        state = "inst_module"
                        print("Instantiating modules at: {}...".format(top_module))
                    else:
                        should_gen_wrapper = False
                        data = line.split()
                        punch_name = data[1].split(':')[1]
                        top_module = data[2].split(':')[1]
                        clock_signal = data[3].split(':')[1]
                        reset_signal = data[4].split(':')[1]
                        dpi_group = data[5].split(':')[1]
                        trigger_signal = ""
                        edged_trigger = False
                        dpi_signals = []
                        state = "punch_signals"
                        print("Punching signals for DPI: {}...".format(punch_name))
                        print("Bringing signals from deeper hierarchy to: {}.".format(top_module))
            elif state == "punch_signals":
                if not "@end" in line:
                    data = line.strip().split()
                    full_hierarchy = data[0].split(':')[1]
                    port_size = int(data[1].split(':')[1])
                    signal_name = full_hierarchy.split('.')[-1]
                    hierarchy = full_hierarchy.split('.')[:-1]
                    port_name = "punch_{}_{}".format(punch_name, punch_id)
                    if len(data) > 2:
                        t = data[2].split(':')
                        if t[0] == "type":
                            trigger_signal = port_name
                            edged_trigger = ("edged" in t[1])
                            pos_edged = ("pos" in t[1])
                            neg_edged = ("neg" in t[1])

                    punch_id += 1
                    dpi_signals.append({"port_size": port_size, "port_name": port_name, "signal_name": signal_name})
                    print("Punching out: {}".format(full_hierarchy))
                    punch_out(full_hierarchy, top_module, module_to_file_map, module_to_submodule_map, port_name, port_size)
                else:
                    should_gen_wrapper = True
                    state = "search_begin"
            elif state == "inst_module":
                if not "@end" in line:
                    data = line.strip().split()
                    instance_name = data[0].split(':')[1]
                    module_path = data[1].split(':')[1]
                    inst_data = Data("inst_module")
                    inst_data.clock_signal = clock_signal
                    inst_data.reset_signal = reset_signal
                    instantiate_module(inst_data, top_module, module_to_file_map, instance_name, module_path)
                else:
                    state = "search_begin"

            if should_gen_wrapper:
                # Pack data.
                data = Data("none")
                data.punch_name = punch_name
                data.clock_signal = clock_signal
                data.reset_signal = reset_signal
                data.trigger_signal = trigger_signal
                data.edged_trigger = edged_trigger
                data.pos_edged = pos_edged
                data.neg_edged = neg_edged
                data.dpi_signals = dpi_signals

                # Put wrapper instance on top level.
                put_wrapper(module_to_file_map[top_module], top_module, data)

                # Generate wrapper verilog file.
                wrapper_path = "{}/{}_wrapper.v".format(os.path.dirname(module_to_file_map[top_module]), punch_name)
                gen_wrapper(wrapper_path, data)

                # Suggest DPI signatures.
                dpi_cpp_path = "{}/{}_dpi.cpp".format(os.path.dirname(module_to_file_map[top_module]), dpi_group)
                gen_cpp_dpi_signature(dpi_cpp_path, data)

def gen_patched_file_list(file_list_path):
    new_file_list_path = "{}.patched.f".format(file_list_path)
    with open(file_list_path, "r") as infile, open(new_file_list_path, "w") as outfile:
        for line in infile:
            if line.strip() in was_patched:
                outfile.write("{}.patched.sv\n".format(line.strip()))
            else:
                outfile.write(line)

        for generated_file in generated_files:
            outfile.write("{}\n".format(generated_file))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python punch.py <file_with_sv_list> <file_with_punch_config>")
        sys.exit(1)

    file_list_path = sys.argv[1]
    punch_config_file_path = sys.argv[2]
    v_files_list = read_verilog_files_list(file_list_path)
    module_to_file_map = get_module_to_file_map(v_files_list)
    module_to_submodule_map = get_module_to_submodules_map(v_files_list)
    hook_dpi(punch_config_file_path, module_to_file_map, module_to_submodule_map)
    gen_patched_file_list(file_list_path)

# Copyright (c) 2021-2022 Kabylkas Lab.
# Licensed under the Apache License, Version 2.0.
import json
import os

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

def GenerateVerilogModuleHooks(json_data, output_file_path):
    with open(output_file_path, "w") as out_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        # Header.
        writer.line("// Auto generated file. Do not edit.")
        writer.line("// Copyright (c) 2021-2022 Kabylkas Labs.")
        writer.line("// Licensed under the Apache License, Version 2.0.")
        writer.line("")

        # Declare imports.
        writer.line("// Generated import declarations.")
        for interface in json_data["interfaces"]:
            writer.line("import \"DPI-C\" function void {}DPI(".format(interface["module_name"]))
            num_ports = 0
            for port in interface["module_ports"]:
                type = "int"
                if port["width"] > 32:
                    type = "longint"

                if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                    line = "input {} {}".format(type, port["name"])
                    num_ports += 1
                    if num_ports < len(interface["module_ports"]) - 3:
                        line += ","
                    writer.line(line, 1)
            writer.line(");")
            writer.line("")


        # Define modules.
        writer.line("// Generated modules.")
        for interface in json_data["interfaces"]:
            # Generate module defintion and ports.
            writer.line("module {}Hook (".format(interface["module_name"]))
            num_ports = 0
            clock_port_name = ""
            reset_port_name = ""
            trigger_port_name = ""
            arguments = ""
            trigger_edge = "posedge"
            always_block = "regular"
            for port in interface["module_ports"]:
                port_line = ""

                if port["direction"] == "in":
                    port_line = "input "
                elif port["direction"] == "out":
                    port_line = "output "

                if port["width"] > 1:
                    port_line += "[{}:0] ".format(port["width"] - 1)

                port_line += port["name"]

                num_ports += 1

                # Is the port should be DPIed
                should_be_dpied = False
                if port["is_clock"]:
                    clock_port_name = port["name"]
                elif port["is_reset"]:
                    reset_port_name = port["name"]
                elif port["is_trigger"]:
                    trigger_port_name = port["name"]
                    if "negedge" in port:
                        always_block = "edged"
                        if port["negedge"]:
                            trigger_edge = "negedge"
                else:
                    should_be_dpied = True

                if should_be_dpied:
                    arguments += port["name"]
                    if num_ports < len(interface["module_ports"]):
                        arguments += ", "

                if num_ports < len(interface["module_ports"]):
                    port_line += ","

                writer.line(port_line, 1)


            writer.line(");")

            # Generate the sequential access to the DPI.
            if always_block == "regular":
                writer.line("always @(posedge {}) begin".format(clock_port_name), 1)
                writer.line("if (!{}) begin".format(reset_port_name), 2)
                writer.line("if ({}) begin".format(trigger_port_name), 3)
                writer.line("{}DPI({});".format(interface["module_name"], arguments), 4)
                writer.line("end", 3)
                writer.line("end", 2)
                writer.line("end", 1)
            elif always_block == "edged":
                writer.line("always @({} {}) begin".format(trigger_edge, trigger_port_name), 1)
                writer.line("if (!{}) begin".format(reset_port_name), 2)
                writer.line("{}DPI({});".format(interface["module_name"], arguments), 3)
                writer.line("end", 2)
                writer.line("end", 1)

            # Complete the module.
            writer.line("endmodule")

def GenerateVerilogModuleHookInstances(json_data, template_file_path, output_file_path):
    with open(output_file_path, "w") as out_file, open(template_file_path, "r") as template_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        for line in template_file:
            if "@generate" in line:
                gen_id = line.split(":")[1][:-1]
                processed = False
                for interface in json_data["interfaces"]:
                    if interface["generator_id"] == gen_id:
                        module_name = interface["module_name"]
                        processed = True
                        for i in range(interface["instances"]):
                            writer.line("{0}Hook {0}_inst_{1} (".format(module_name, i), 1)
                            num_ports = 0
                            for port in interface["module_ports"]:
                                line = ".{}({})".format(port["name"], port["connections"][i])
                                num_ports += 1
                                if (num_ports < len(interface["module_ports"])):
                                    line += ","
                                writer.line(line, 2)
                            writer.line(");", 1)

                            if (i+1 < interface["instances"]):
                                writer.line("")

            else:
                writer.append(line)

def GenerateCcDPIs(json_data, template_file_path, output_file_path):
    with open(output_file_path, "w") as out_file, open(template_file_path, "r") as template_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        for line in template_file:
            if "@generate" in line:
                gen_id = line.split(":")[1][:-1]
                processed = False
                for interface in json_data["interfaces"]:
                    if interface["generator_id"] == gen_id:
                        module_name = interface["module_name"]
                        processed = True
                        writer.line("extern \"C\" void {}DPI".format(interface["module_name"]))
                        writer.line("(")
                        num_ports = 0
                        arguments = "state, "
                        for port in interface["module_ports"]:
                            type = "int"
                            if port["width"] > 32:
                                type = "long long"

                            if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                                line = "{} {}".format(type, port["name"])
                                arguments += port["name"]
                                num_ports += 1
                                if num_ports < len(interface["module_ports"]) - 3:
                                    line += ","
                                    arguments += ", "
                                writer.line(line, 1)
                        writer.line(")")
                        writer.line("{")
                        writer.line("{}API({});".format(module_name, arguments), 1)
                        writer.line("}")

                if not processed:
                    print("Warning: Could not find module name '{}' annotation in template file {}.".format(module_name, template_file_path))
            else:
                writer.append(line)

def GenerateDromajoHeaderAPIs(json_data, template_file_path, output_file_path):
    with open(output_file_path, "w") as out_file, open(template_file_path, "r") as template_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        for line in template_file:
            if "@generate" in line:
                gen_id = line.split(":")[1][:-1]
                processed = False
                for interface in json_data["interfaces"]:
                    if interface["generator_id"] == gen_id:
                        module_name = interface["module_name"]
                        processed = True
                        writer.line("void {}API".format(interface["module_name"]))
                        writer.line("(")
                        num_ports = 0
                        arguments = "dromajo_cosim_state_t* state, "
                        writer.line(arguments, 1)
                        for port in interface["module_ports"]:
                            type = "int"
                            if port["width"] > 32:
                                type = "uint64_t"

                            if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                                line = "{} {}".format(type, port["name"])
                                arguments += port["name"]
                                num_ports += 1
                                if num_ports < len(interface["module_ports"]) - 3:
                                    line += ","
                                    arguments += ", "
                                writer.line(line, 1)
                        writer.line(");")

                if not processed:
                    print("Warning: Could not find module name '{}' annotation in template file {}.".format(module_name, template_file_path))
            else:
                writer.append(line)

def GenerateDromajoImplAPIs(json_data, template_file_path, output_file_path):
    with open(output_file_path, "w") as out_file, open(template_file_path, "r") as template_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        for line in template_file:
            if "@generate" in line:
                gen_id = line.split(":")[1][:-1]
                processed = False
                for interface in json_data["interfaces"]:
                    if interface["generator_id"] == gen_id:
                        module_name = interface["module_name"]
                        processed = True
                        # Forwared declare BOOM bridge.
                        writer.line("void {}BoomBridge".format(module_name))
                        writer.line("(")
                        num_ports = 0
                        for port in interface["module_ports"]:
                            type = "int"
                            if port["width"] > 32:
                                type = "uint64_t"

                            if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                                line = "{} {}".format(type, port["name"])
                                num_ports += 1
                                if num_ports < len(interface["module_ports"]) - 3:
                                    line += ","
                                writer.line(line, 1)
                        writer.line(");")
                        
                        # Function definition.
                        writer.line("void {}API".format(interface["module_name"]))
                        writer.line("(")
                        num_ports = 0
                        arguments = ""
                        writer.line("dromajo_cosim_state_t* state," , 1)
                        for port in interface["module_ports"]:
                            type = "int"
                            if port["width"] > 32:
                                type = "uint64_t"

                            if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                                line = "{} {}".format(type, port["name"])
                                arguments += port["name"]
                                num_ports += 1
                                if num_ports < len(interface["module_ports"]) - 3:
                                    line += ","
                                    arguments += ", "
                                writer.line(line, 1)
                        writer.line(")")
                        writer.line("{")
                        writer.line("{}BoomBridge({});".format(module_name, arguments), 1)
                        writer.line("}")

                if not processed:
                    print("Warning: Could not find module name '{}' annotation in template file {}.".format(module_name, template_file_path))
            else:
                writer.append(line)

def GenerateBoomBridgeFunctions(json_data, template_file_path, output_file_path):
    with open(output_file_path, "w") as out_file, open(template_file_path, "r") as template_file:
        # Instantiate FileWriter.
        writer = FileWriter(out_file)

        for line in template_file:
            if "@generate" in line:
                gen_id = line.split(":")[1][:-1]
                processed = False
                for interface in json_data["interfaces"]:
                    if interface["generator_id"] == gen_id:
                        module_name = interface["module_name"]
                        processed = True
                        # Forwared declare BOOM bridge.
                        writer.line("void {}BoomBridge".format(module_name))
                        writer.line("(")
                        num_ports = 0
                        for port in interface["module_ports"]:
                            type = "int"
                            if port["width"] > 32:
                                type = "uint64_t"

                            if not port["is_clock"] and not port["is_reset"] and not port["is_trigger"]:
                                line = "{} {}".format(type, port["name"])
                                num_ports += 1
                                if num_ports < len(interface["module_ports"]) - 3:
                                    line += ","
                                writer.line(line, 1)
                        writer.line(")")
                        writer.line("{")
                        writer.line("")
                        writer.line("}")
                if not processed:
                    print("Warning: Could not find module name '{}' annotation in template file {}.".format(module_name, template_file_path))
            else:
                writer.append(line)

def main(interface_definition_file_path):
    # Open and get the JSON data.
    with open(interface_definition_file_path, "r") as interface_definition_file:
        json_data = json.load(interface_definition_file)

        # File names.
        kBoomTopTemplate = "./templates/top_template.v"
        kSimDpiTemplate = "./templates/sim_dromajo_cosim_template.cc"
        kDromajoCosimHeaderTemplate = "./templates/dromajo_cosim_template.h"
        kDromajoCosimImplTemplate = "./templates/dromajo_cosim_template.cpp"
        kBoomBridgeFunctionsTemplate = "./templates/bridge_boom_template.cpp"

        kGeneratedVerilogHooks = "generated_verilog_hooks.v"
        kGeneratedBoomTop = "generated_top.v"
        kGeneratedSimDpis = "generated_cc_dpis.cc"
        kGeneratedDromajoCosimHeader = "generated_dromajo_cosim.h"
        kGeneratedDromajoCosimImpl = "generated_dromajo_cosim.cpp"
        kGeneratedDromajoBoomBridge = "generated_boom_bridge.cpp"

        # Generate files.
        GenerateVerilogModuleHooks(json_data, kGeneratedVerilogHooks)
        GenerateVerilogModuleHookInstances(json_data, kBoomTopTemplate, kGeneratedBoomTop)
        GenerateCcDPIs(json_data, kSimDpiTemplate, kGeneratedSimDpis)
        GenerateDromajoHeaderAPIs(json_data, kDromajoCosimHeaderTemplate, kGeneratedDromajoCosimHeader)
        GenerateDromajoImplAPIs(json_data, kDromajoCosimImplTemplate, kGeneratedDromajoCosimImpl)
        GenerateBoomBridgeFunctions(json_data, kBoomBridgeFunctionsTemplate, kGeneratedDromajoBoomBridge)

        # Copy generated files to the right places.
        cmd = "cp {} {}"
        os.system(cmd.format(kGeneratedVerilogHooks, "../generated_verilog_hooks.v"))
        os.system(cmd.format(kGeneratedBoomTop, "../core_rtl/boom/top.v"))
        os.system(cmd.format(kGeneratedSimDpis, "../tools/dromajo/dromajo-dpi/dromajo_dpi.cc"))
        os.system(cmd.format(kGeneratedDromajoCosimHeader, "../tools/dromajo/dromajo-src/include/dromajo_cosim.h"))
        os.system(cmd.format(kGeneratedDromajoCosimImpl, "../tools/dromajo/dromajo-src/src/dromajo_cosim.cpp"))
        os.system(cmd.format(kGeneratedDromajoBoomBridge, "../tools/dromajo/dromajo-src/gold/bridge_boom.cpp"))

main("interfaces.json")

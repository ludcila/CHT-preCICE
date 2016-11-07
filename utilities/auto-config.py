#!/usr/bin/python

import os
import logging
import yaml
import pprint
import argparse
from preciceautoconf.participant import *
from preciceautoconf.rules import *
from preciceautoconf.schemes import *

log_level = getattr(logging, "INFO", None)
logging.basicConfig(level=log_level)

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--input-config", default="config.yml.org", help="Name of the input YML file")
parser.add_argument("--output-xml-config", default="precice-config.xml", help="Name of the output XML file")
parser.add_argument("--output-yml-config", default="config.yml", help="Name of the output YML file")
parser.add_argument("--output-comm-config", default="config.comm", help="Name of the output .comm file")
parser.add_argument("--output-sh", default="run.sh", help="Name of the output .sh file")
args = parser.parse_args()
input_file_name = args.input_config
output_xml_file_name = args.output_xml_config
output_yml_file_name = args.output_yml_config
output_comm_file_name = args.output_comm_config
output_sh_file_name = args.output_sh

# Create participants and couplings from YAML file

config_file = open(input_file_name)
config = yaml.load(config_file.read())
logging.info("Input YML file:\t" + input_file_name)

if "base-path" not in config:
    config["base-path"] = os.getcwd()

participants = []
interfaces_map = {}
participants_list = config["participants"]
for participant_name in participants_list:
    participant_data = participants_list[participant_name]
    participant = ParticipantFactory.get_participant(name=participant_name,
                                                     data=participant_data,
                                                     steady_state=config["simulation"]["steady-state"])
    participants.append(participant)
    for interface in participant_data["interfaces"]:
        interface_name = interface["name"]
        interface = participant.add_interface(interface_name)
        interfaces_map[interface_name] = interface

couplings_list = config["couplings"]
for coupling in couplings_list:
    interface1 = interfaces_map[coupling[0]]
    interface2 = interfaces_map[coupling[1]]
    interface1.set_partner_interface(interface2)

couplings = []
for i in range(len(participants)):
    for j in range(i, len(participants)):
        if participants[i].has_interfaces_with(participants[j]):
            couplings.append([participants[i], participants[j]])

# Configure coupling

coupling_config = CouplingConfiguration(
    time_step=config["simulation"]["time-step"],
    max_time=config["simulation"]["max-time"],
    max_iterations=config["simulation"]["max-coupling-iterations"],
    couplings=couplings,
    steady_state=config["simulation"]["steady-state"],
    force_explicit=config["simulation"]["force-explicit"],
    force_parallel=config["simulation"]["force-parallel"]
)

# --------------------------------------------------------------------------------
#   Create XML file
# --------------------------------------------------------------------------------

nsmap = {
    "data": "data",
    "mapping": "mapping",
    "coupling-scheme": "coupling-scheme",
    "post-processing": "post-processing",
    "m2n": "m2n",
    "master": "master"
}

precice_configuration_tag = etree.Element("precice-configuration", nsmap=nsmap)
log_tag = etree.SubElement(precice_configuration_tag, "log")
sink_tag = etree.SubElement(log_tag, "sink", type="stream", output="stdout", enabled="true")

solver_interface_tag = etree.SubElement(precice_configuration_tag, "solver-interface", dimensions="3")

for participant in participants:
    participant.add_data_tags_to(solver_interface_tag)

for participant in participants:
    participant.add_mesh_tags_to(solver_interface_tag)

for participant in participants:
    participant.add_participant_tag_to(solver_interface_tag)

if coupling_config.is_multi():

    # If multi, all couplings are treated together
    couplingScheme = MultiCouplingScheme(coupling_config, couplings)

    # Add tags
    couplingScheme.add_m2n_tag_to(solver_interface_tag,
                                  exchange_directory="." if "base-path" not in config else config["base-path"])
    couplingScheme.add_coupling_scheme_tag_to(solver_interface_tag)

else:

    # If not multi, couplings are treated per pair
    for participants_pair in couplings:

        # Create implicit or explicit coupling scheme
        if coupling_config.is_implicit():
            couplingScheme = ImplicitCouplingScheme(coupling_config, participants_pair)
        else:
            couplingScheme = CouplingScheme(coupling_config, participants_pair)

        # Add tags
        couplingScheme.add_m2n_tag_to(solver_interface_tag,
                                      exchange_directory="." if "base-path" not in config else config["base-path"])
        couplingScheme.add_coupling_scheme_tag_to(solver_interface_tag)

xml_string = etree.tostring(precice_configuration_tag, pretty_print=True, xml_declaration=True, encoding="UTF-8")

# Remove xmlns:* attributes which are not recognized by preCICE
from_index = xml_string.find("<precice-configuration")
to_index = xml_string.find(">", from_index)
xml_string = xml_string[0:from_index] + "<precice-configuration>" + xml_string[to_index+1:]

output_xml_file = open(output_xml_file_name, "w")
output_xml_file.write(xml_string)
output_xml_file.close()
logging.info("Output XML file:\t" + output_xml_file_name)

# --------------------------------------------------------------------------------
#   Create YAML file
# --------------------------------------------------------------------------------

# To the original config.yml file, we append the mesh and data names
config["precice-config-file"] = output_xml_file_name
for participant in config["participants"]:
    for interface_config in config["participants"][participant]["interfaces"]:
        interface = interfaces_map[interface_config["name"]]
        if interface.read_mesh == interface.write_mesh:
            interface_config["mesh"] = interface.read_mesh
        else:
            interface_config["read-mesh"] = interface.read_mesh
            interface_config["write-mesh"] = interface.write_mesh
        interface_config["write-data"] = [interface.participant.data_name_T,
                                          interface.participant.data_name_HTC]
        interface_config["read-data"] = [interface.partner_interface.participant.data_name_T,
                                         interface.partner_interface.participant.data_name_HTC]

output_yml_file = open(output_yml_file_name, "w")
output_yml_file.write(yaml.dump(config))
output_yml_file.close()
logging.info("Output YML file:\t" + output_yml_file_name)

# --------------------------------------------------------------------------------
#   Create .comm file(s) for Code_Aster participant(s)
# --------------------------------------------------------------------------------

for participant in participants:
    if participant.solver_type == "Code_Aster":
        output_comm_file = open(output_comm_file_name, "w")
        output_comm_file.write("settings = \\\n")
        if "output-frequency" not in config["simulation"]:
            config["simulation"]["output-frequency"] = 1
        pprint.pprint(config, output_comm_file)
        output_comm_file.close()
        logging.info("Output .comm file:\t" + output_comm_file_name)
        break

# --------------------------------------------------------------------------------
#   Create run.sh
# --------------------------------------------------------------------------------

output_sh_file = open(output_sh_file_name, "w")
i = 0
for participant in participants:
    output_sh_file.write(participant.get_run_command() + "\n")
    output_sh_file.write("pid" + str(i) + "=$!\n\n")
    i += 1
for j in range(i):
    output_sh_file.write("wait $pid" + str(j) + "\n")
output_sh_file.close()
logging.info("Output .sh file:\t" + output_sh_file_name)

import yaml
from preciceautoconf.participant import *
from preciceautoconf.rules import *
from preciceautoconf.schemes import *

# Create participants and couplings from YAML file

config_file = open("config.yml")
config = yaml.load(config_file.read())

participants = []
interfaces_map = {}
participants_list = config["participants"]
for participant_name in participants_list:
    participant_data = participants_list[participant_name]
    participant = ParticipantFactory.get_participant(participant_data["solver"], participant_name)
    participants.append(participant)
    for interface_name in participant_data["interfaces"]:
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

# Simulation parameters

time_step = config["simulation"]["time_step"]
max_time = config["simulation"]["max_time"]
max_iterations = config["simulation"]["max_coupling_iterations"]

# Determine type of coupling configuration to be used

coupling_config = CouplingConfiguration(
    couplings=couplings,
    steady_state=config["simulation"]["steady_state"],
    force_explicit=config["simulation"]["force_explicit"],
    force_parallel=config["simulation"]["force_parallel"]
)

# --------------------------------------------------------------------------------
#   Create XML tree
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

for participant in participants:
    participant.add_data_tags_to(precice_configuration_tag)

for participant in participants:
    participant.add_mesh_tags_to(precice_configuration_tag)

for participant in participants:
    participant.add_participant_tag_to(precice_configuration_tag)

if coupling_config.multi():

    # If multi, all couplings are treated together
    couplingScheme = MultiCouplingScheme(time_step, max_time, max_iterations, couplings)

    # Add tags
    couplingScheme.add_m2n_tag_to(precice_configuration_tag)
    couplingScheme.add_coupling_scheme_tag_to(precice_configuration_tag)

else:

    # If not multi, couplings are treated per pair
    for participants_pair in couplings:

        # Determine first and second participant
        coupling_config.sort_participants(participants_pair)

        # Create implicit or explicit coupling scheme
        if coupling_config.implicit():
            couplingScheme = ImplicitCouplingScheme(time_step, max_time, max_iterations, participants_pair,
                                                    serial=coupling_config.serial())
        else:
            couplingScheme = CouplingScheme(time_step, max_time, participants_pair, serial=coupling_config.serial())

        # Add tags
        couplingScheme.add_m2n_tag_to(precice_configuration_tag)
        couplingScheme.add_coupling_scheme_tag_to(precice_configuration_tag)

print etree.tostring(precice_configuration_tag, pretty_print=True)

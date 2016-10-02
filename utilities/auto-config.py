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
    participant = ParticipantFactory.get_participant(solver_type=participant_data["solver"],
                                                     name=participant_name,
                                                     domain_decomposed=participant_data["domain-decomposed"])
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

if coupling_config.is_multi():

    # If multi, all couplings are treated together
    couplingScheme = MultiCouplingScheme(coupling_config, couplings)

    # Add tags
    couplingScheme.add_m2n_tag_to(precice_configuration_tag)
    couplingScheme.add_coupling_scheme_tag_to(precice_configuration_tag)

else:

    # If not multi, couplings are treated per pair
    for participants_pair in couplings:

        # Create implicit or explicit coupling scheme
        if coupling_config.is_implicit():
            couplingScheme = ImplicitCouplingScheme(coupling_config, participants_pair)
        else:
            couplingScheme = CouplingScheme(coupling_config, participants_pair)

        # Add tags
        couplingScheme.add_m2n_tag_to(precice_configuration_tag)
        couplingScheme.add_coupling_scheme_tag_to(precice_configuration_tag)

print etree.tostring(precice_configuration_tag, pretty_print=True)

for participant in config["participants"]:
    for interface in config["participants"][participant]["interfaces"]:
        interface["read-mesh"] = interfaces_map[interface["name"]].read_mesh
        interface["write-mesh"] = interfaces_map[interface["name"]].write_mesh

print yaml.dump(config["participants"])

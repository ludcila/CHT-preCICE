# Tutorial: Coupling OpenFOAM and CalculiX

## Setting up a Dirichlet-Neumann coupled simulation

This document explains how to setup a Dirichlet-Neumann coupled simulation (Dirichlet for the fluid and Neumann for the solid).

### The preCICE configuration file

We define `Temperature` and `Heat-Flux` as our coupling data.

    <data:scalar name="Temperature"/>
    <data:scalar name="Heat-Flux"/>

We define the meshes.  Note that the in CalculiX we extract the temperature values from the nodes, while the heat flux is applied to the element faces.  Therefore, we need to define two interface meshes.  In OpenFOAM, all data is specified at the faces.

    <!-- CalculiX interface meshes -->
    <mesh name="CCX-Nodes">
        <use-data name="Temperature"/>
    </mesh>
    <mesh name="CCX-Face-Centers">
        <use-data name="Heat-Flux"/>
    </mesh>

    <!-- OpenFOAM interface meshes -->
    <mesh name="OF-Face-Centers">
        <use-data name="Temperature"/>
        <use-data name="Heat-Flux"/>
    </mesh>

Next, we have to specify the participants.  


    <participant name="OpenFOAM">
        <use-mesh name="OF-Face-Centers" provide="yes"/>
        <write-data name="Heat-Flux" mesh="OF-Face-Centers"/>
        <read-data  name="Temperature" mesh="OF-Face-Centers"/>
    </participant>

    <participant name="CCX">
        <use-mesh name="CCX-Nodes" provide="yes"/>
        <use-mesh name="CCX-Face-Centers" provide="yes"/>
        <use-mesh name="OF-Face-Centers" from="OpenFOAM"/>
        <write-data name="Temperature" mesh="CCX-Nodes"/>
        <read-data  name="Heat-Flux" mesh="CCX-Face-Centers"/>
        <mapping:nearest-projection direction="write" from="CCX-Nodes" to="OF-Face-Centers" constraint="consistent" timing="initial"/>
        <mapping:nearest-neighbor direction="read"  from="OF-Face-Centers" to="CCX-Face-Centers" constraint="consistent" timing="initial"/>
    </participant>

The complete `precice-config.xml` can be found in the example case `flow-over-plate/buoyantPimpleFoam-ccx/`

### config.yml

    precice-config-file:		precice-config.xml

    OpenFOAM:
        coupled-surfaces:
        - mesh-name: OF-Face-Centers
          patch-names:
          - interface
          write-data:
          - Heat-Flux
          read-data:
          - Temperature

    CCX:
        coupled-surfaces:
        - faces-mesh-name: CCX-Face-Centers
          nodes-mesh-name: CCX-Nodes
          patch-names:
          - INTERFACE
          write-data:
          - Temperature
          read-data:
          - Heat-Flux

### OpenFOAM Boundary Conditions

It is enough to set the temperature of the interface patch to `fixedValue`.

For example, in the `0/T` file we set:

    interface
    {
      type fixedValue;
      value uniform 293;
    }

### CalculiX Boundary Conditions

In the CalculiX input file (`.inp` file), we need to include a `.dfl` file that contains the initial flux values for the coupled surface elements.

    *INCLUDE, INPUT=solid/interface.dfl

This file can be generated using `cgx` with the send command.  For example, assuming that the interface set is called `interface`, we write:

    send abq interface dflux 0

## Setting up a Robin-Robin coupled simulation

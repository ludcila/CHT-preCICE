

# Overview

## Contents


<!-- toc orderedList:0 -->

- [Overview](#overview)
	- [Contents](#contents)
- [Running a CHT simulation with preCICE](#running-a-cht-simulation-with-precice)
	- [Directory structure of the case](#directory-structure-of-the-case)
- [Setting up a CHT simulation with preCICE](#setting-up-a-cht-simulation-with-precice)
	- [Overview](#overview-1)
	- [Coupling boundary conditions](#coupling-boundary-conditions)
	- [The interfaces](#the-interfaces)
		- [How to specify an interface](#how-to-specify-an-interface)
			- [OpenFOAM](#openfoam)
			- [CalculiX](#calculix)
			- [Code_Aster](#code_aster)
		- [Multiple interfaces](#multiple-interfaces)
	- [The preCICE XML configuration file](#the-precice-xml-configuration-file)
	- [Transient vs Steady-State Simulation](#transient-vs-steady-state-simulation)

<!-- tocstop -->


---

# Running a CHT simulation with preCICE

## Directory structure of the case

Inside the root directory of the CHT case, we will find the configuration files `precice-config.xml` and `config.yml`, and typically, a subdirectory for each participant.  In the example below, we have an OpenFOAM fluid participant, an a CalculiX solid participant.

	cht-case
		precice-config.xml
		config.yml
		fluid
			0
			constant
			system
		solid
			all.mesh
			solid.inp
			interface.nam
			interface.sur
			interface.flm

The solvers are executed from the root directory.  That means, for example, that for OpenFOAM we need to use the `-case` parameter, and in CalculiX we need to include the `solid/` folder in the paths.

	buoyantPimpleFoam_preCICE -case fluid -precice-participant Fluid
	ccx -i solid/solid -precice-participant Solid

The output from preCICE (e.g. regarding the coupling convergence and iterations) is stored in the case's root directory.  (This can be configured by changing the `exchange-directory` value in the `precice-config.xml` file.)

# Setting up a CHT simulation with preCICE

## Overview
The input files for the case are prepared for each solver the usual way, as if they were running independently.  However, special care must be taken so that the boundary conditions are appropriate for the coupling.  Additional configuration files are required for the coupling and extra command line arguments must be passed to the coupled solvers:

 1. **Boundary conditions at the interfaces**: the appropriate type of boundary condition must be set according to the type of coupling to be used (e.g. Dirichlet-Neumann, Robin-Robin, etc.).  It is important to use the correct type of boundary condition, such that afterwards the values can be overriden during the coupling.
 2. **Configuration files**:
	 3. For preCICE: `precice-config.xml` (read by all participants)
	 4. For the adapter: `config.yml` for OpenFOAM and CalculiX, `config.comm` for Code_Aster)
 5.  **Command line arguments**: OpenFOAM and CalculiX recognize two additional command line arguments:
	 6. `-precice-participant` (required) [name]
	 7. `-precice-config` (optional) [YAML config file]

(To be added: what parameters need to match)

## Coupling boundary conditions

Below is a list of the boundary conditions used for the coupling.  The terminology is specific to each solver.

| Boundary condition | OpenFOAM | CalculiX | Code_Aster |
|---|---|---|---|
| Temperature (Dirichlet) | fixedValue | BOUNDARY | - |
| Heat Flux (Neumann) | zeroGradient | DFLUX | - |
| Convection (Robin) | mixed | FILM | ECHANGE

For instructions on how to setup a particular solver, please refer to the section dedicated to that solver.

> Note that Code_Aster only supports Robin-Robin coupling at the moment.


## The interfaces

### How to specify an interface

Each interface consists of nodes and/or faces.  We need to tell the adapter which nodes/faces belong to which interface.  The way we associate these nodes and faces with an interface, depends on the particular solver we are using.

#### OpenFOAM
In OpenFOAM, an interface can be made up from several patches.  To associate multiple patches with one interface, it is enough to specify the list of patches in the `config.yml` file.  No changes are required in the OpenFOAM case files:

	Fluid:
	- solver: OpenFOAM
	  interfaces:
	    - mesh-name: OF-Face-Centers
		  patches:
		 	- top
		 	- left
			- right
	  ...


#### CalculiX

In CalculiX, we do not have the concept of multiple patches making up an interface, but instead, we create a new set that contains all interface nodes and elements.  This is easy to do in CalculiX, since nodes and elements can belong to multiple sets.

For example, if the original case contains three node/surface sets that we want to declare as our interface, we just have to create a larger set from the union of these three sets.

	Solid:
	- solver: CalculiX
	  interfaces:
	    - nodes-mesh-name: CCX-Nodes
		  faces-mesh-name: CCX-Face-Centers
		  patch: interface
	  ...

#### Code_Aster

Similar to CalculiX, in Code_Aster we have to create a "group" that contains all interface elements.  The element group must exist in the input mesh file (`.mmed` file).

	Solid:
	- solver: Code_Aster
	  interfaces:
	    - nodes-mesh-name: Aster-Nodes
		  faces-mesh-name: Aster-Face-Centers
		  patch: interface
	  ...



### Multiple interfaces

A participant may have multiple interfaces.  A typical situation is when a participant is coupled to two (or more) other participants.  An example of this is the heat exchanger case, where the solid is coupled with two fluids:

```{mermaid}
graph LR
Inner-Fluid---Solid
Solid---Outer-Fluid
```
The solid participant has an interface with each of the two fluid participants.

More than one interface may also be defined between the same pair of coupled participants.  If we simulate the heat exchanger with only one fluid participant and a solid participant, then we may decide to treat it as only one fluid-solid interface, or we may treat the interfaces with the inner fluid and the outer fluid separately.

## The preCICE XML configuration file

- Participants of the coupling
- Coupling data: what data is exchanged
- Coupling scheme (parallel/serial, implicit/explicit)
- Data mapping scheme

(...)


## Transient vs Steady-State Simulation

| | Transient | Steady-State |
| --- | --- | --- |
| OpenFOAM (different solvers) | buoyantPimpleFoam | buoyantSimpleFoam |
| CalculiX (change in .inp file) | *HEAT TRANSFER, DIRECT | *HEAT TRANSFER, DIRECT, STEADY STATE|
| Code_Aster (different adapter in .export file) | adapter.comm | adapter-steady-state.comm** |
| Coupling schemes (in precice-config.xml) | Implicit / Explicit | Explicit only |
| Coupling boundary conditions | Dirichlet-Neumann, Robin-Robin | Robin-Robin only |
| Timestep | any, but no subcycling | 1 |

**Will be merged with adapter.comm

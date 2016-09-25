

# Contents

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Contents](#contents)
- [Setting up a CHT simulation with preCICE](#setting-up-a-cht-simulation-with-precice)
	- [Overview](#overview)
	- [Steps](#steps)
	- [The preCICE XML configuration file](#the-precice-xml-configuration-file)
	- [Boundary conditions for thermal coupling](#boundary-conditions-for-thermal-coupling)
	- [Transient vs Steady-State Simulation](#transient-vs-steady-state-simulation)

<!-- /TOC -->

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
Initially, the input files for the case are prepared for each solver the usual way, as if they were running independently.  Afterwards, some changes (or files) are required in order for the coupling to work.  These include:

 1. **Boundary conditions at the interfaces**: the appropriate type of boundary condition must be set according to the type of coupling to be used (e.g. Dirichlet-Neumann, Robin-Robin, etc.).  It is important to use the correct type of boundary condition, such that afterwards the values can be overriden during the coupling.
 2. **Configuration files**:
	 3. For preCICE: `precice-config.xml` (read by all participants)
	 4. For the adapter: `config.yml` for OpenFOAM and CalculiX, `config.comm` for Code_Aster)
 5.  **Command line arguments**: OpenFOAM and CalculiX must be run with additional command line arguments:
	 6. `-precice-participant` [name]
	 7. `-precice-config` [YAML config file]

## The interfaces

### What is an interface



### How to specify an interface

Each interface consists of nodes and faces.  The way we associate these nodes and faces with an interface, depends on the particular solver we are using.

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



### How many interfaces to use

A simulation coupled with preCICE can contain multiple interfaces.  At least one interface must exist between every pair of coupled participants.  More than one interface can exist between the same pair of coupled participants, but this is rarely necessary, and therefore we only consider the case of one interface per pair of coupled participants.

#### Multi-coupling

A typical case where multiple interfaces are necessary, is when we are coupling more than two participants.  An example of this, is the heat exchanger case, where we have a coupling as depicted in the picture below:

```{mermaid}
graph LR
Inner-Fluid---Solid
Solid---Outer-Fluid
```

The solid participant has an interface with eahc of the two fluid participants.


## The preCICE XML configuration file

- Participants of the coupling
- Coupling data: what data is exchanged
- Coupling scheme (parallel/serial, implicit/explicit)
- Data mapping scheme

## Boundary conditions for thermal coupling

| BC | OpenFOAM | CalculiX | Code_Aster |
|---|---|---|---|
| Temperature (Dirichlet) | fixedValue | BOUNDARY | - |
| Heat Flux (Neumann) | zeroGradient | DFLUX | - |
| Convection (Robin) | mixed | FILM | ECHANGE


## Summary

| - | OpenFOAM | CalculiX | Code_Aster |
|---|---|---|---|
| Transient | YES | YES | YES |
| Steady-State | YES | YES | YES |
| Dirichlet-Neumann coupling implemented | YES | YES | NO |
| Robin-Robin coupling implemented | YES | YES | YES |
| Configuration file | config.yml | config.yml | config.comm |

## Steps

1. Is it a transient or steady-state simulation?
	- Transient
		- Implicit and explicit coupling available
		-
	- Steady-state
		- Only explicit coupling
		- Timestep size always 1
2. What type of coupling will be used?
	- Dirichlet-Neumann
	- Robin-Robin
3.




---

## Transient vs Steady-State Simulation

1. OpenFOAM: Different solvers are executed (e.g. buoyantPimpleFoam for transient, buoyantSimpleFoam for steady-state)
2. CalculiX: Keyword STEADY STATE must be added to the *HEAT TRANSFER card
3. Code_Aster: Different adapters are included in the .export file

> If multiple participants are used, residual control must be disabled for the SIMPLE algorithm, otherwise one of the participants may terminate earlier, causing the others to terminate as well

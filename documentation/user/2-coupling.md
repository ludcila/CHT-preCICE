# Coupling


<!-- toc orderedList:0 -->

- [Coupling](#coupling)
	- [Dirichlet-Neumann vs Robin-Robin coupling](#dirichlet-neumann-vs-robin-robin-coupling)
	- [The coupling data](#the-coupling-data)
	- [Coupling multiple regions](#coupling-multiple-regions)
	- [Location of the coupling data](#location-of-the-coupling-data)
		- [Location of "read" data](#location-of-read-data)
		- [Location of "write" data](#location-of-write-data)

<!-- tocstop -->



## Dirichlet-Neumann vs Robin-Robin coupling

In most applications, it is recommended to use Robin-Robin (RR) type of coupling, as it shows better stability and convergence properties.

Robin-Robin coupling is stable enough that the simulation does not blow up with explicit coupling, but with implicit coupling we can make sure that the coupling converges.  A trade-off can also be achieved by using implicit coupling with a low number of `max-iterations`.

## The coupling data

At the moment of setting up the coupled simulation, we must decide what data will be exchanged for the coupling.  Available options are:
- `Temperature` (for Dirichlet-Neumann coupling)
- `Heat-Flux` (for Dirichlet-Neumann coupling)
- `Sink-Temperature` & `Heat-Transfer-Coefficient` (for Robin-Robin coupling)

## Coupling multiple regions

In every simulation, we will have at least two regions (typically a fluid and a solid region).  There are some cases where more than two physical regions need to be simulated.  We have different possibilities on how to assign the regions to different participants:

- How many fluid participants to use:
    - One participant can be used for all fluid regions
        - If they all have the same properties (e.g. thermophysical properties)
        - If there are no fluid-fluid interfaces
    - Multiple participants must be used
        - If the regions have different properties (e.g. thermophysical properties)
        - If there are fluid-fluid interfaces
- How many solid participants to use:
    - One participant may be used; if there are solid-solid interfaces, they can be dealt with by defining contacts
    - Multiple participants may be used; the interfaces are coupled with preCICE (only perfect contact can be modeled in this case)

Another way to summarize these guidelines, is in terms of the interfaces:
- Fluid-solid interfaces: must be coupled through preCICE
- Fluid-fluid interfaces*: must be coupled through preCICE
- Solid-solid interfaces: can be coupled either through preCICE or by using contacts

*The current implementation couples the buoyantPimpleFoam solver, which does not support regions.  In future developments, one might couple chtMultiRegionFoam in order to be able to simulate fluid-fluid interfaces within one solver, instead of using an external coupling.

## Location of the coupling data

Depending on the solver, it is more "natural" or straightforward to extract or apply the coupling data either at the faces or at the nodes.  This section describes the location of the data in the current implementation.

### Location of "read" data
Data that is read from the coupling partner is applied as BC at the following locations:

| Data | OpenFOAM | CalculiX | Code_Aster |
|---|---|---|---|
| Temperature BC | Face center | Nodes | - |
| Heat Flux BC | Face center | Face center (element face) | - |
| Convection BC | Face center | Face center (element face) | Nodes

### Location of "write" data
Data that is written or sent to the coupling partner is extracted from the following locations:

| Data | OpenFOAM | CalculiX | Code_Aster |
|---|---|---|---|
| Temperature | Face center | Nodes | - |
| Heat Flux | Face center | Face center (element face) | - |
| Sink Temperature | Face center | Face center (element face) | Face center (element face)

# Coupling




## Dirichlet-Neumann vs Robin-Robin coupling

In most applications, it is recommended to use Robin-Robin (RR) type of coupling, as it shows better stability and convergence properties.

Robin-Robin coupling is stable enough that the simulation does not blow up with explicit coupling; but using implicit coupling we can make sure that the coupling has converged.  A trade-off can also be achieved by using implicit coupling with a small number of `max-iterations`.

## The coupling data

At the moment of setting up the coupled simulation, we must decide what data will be exchanged for the coupling.  Available options are:
- `Temperature` (for Dirichlet-Neumann coupling)
- `Heat-Flux` (for Dirichlet-Neumann coupling)
- `Sink-Temperature` & `Heat-Transfer-Coefficient` (for Robin-Robin coupling)

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

# Coupling



## Types of coupling

When talking about the type of coupling, this might be referring to different categorizations:

| Type of coupling | Description |
| --- | --- |
| Implicit / Explicit | Implicit: coupling iterations are performed until convergence.  Explicit: Only one coupling iteration per timestep |
| Serial / Parallel | Serial: Gauss-Seidel type of update; participants are executed in turn, using the most recent data from the other participant. Parallel: Jacobi type of update.   |
| Dirichlet-Neumann / Robin-Robin | Depends on what coupling data is used (or what kind of boundary conditions). |

Combinations of implicit/explicit, serial/parallel couplings are referred to as `coupling-scheme` and need to be specified in the `precice-config.xml` file.

Available options are:
- `coupling-scheme:serial-explicit`
- `coupling-scheme:serial-implicit`
- `coupling-scheme:parallel-explicit`
- `coupling-scheme:parallel-implicit`
- `coupling-scheme:multi`

The first four coupling schemes are defined between two participants, and can be defined as implicit or explicit.  The last coupling scheme can be configured for multiple participants coupled implicitly.

Whether Dirichlet-Neumann or Robin-Robin type of coupling is used, depends on what coupling data is exchanged and what boundary conditions are set on the solvers.  


## The coupling data

At the moment of setting up the coupled simulation, we must decide what data will be exchanged for the coupling.  Available options are:
- `Temperature` (for Dirichlet-Neumann coupling)
- `Heat-Flux` (for Dirichlet-Neumann coupling)
- `Sink-Temperature` & `Heat-Transfer-Coefficient` (for Robin-Robin coupling)

## Location of the coupling data

Depending on the solver, it might be more "natural" or easy to extract or apply the coupling data either at the faces or at the nodes.

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

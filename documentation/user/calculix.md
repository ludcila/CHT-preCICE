# CalculiX



## How to setup the interface

There are two files that are required for each interface:
- `interface.nam`: contains the set of nodes
- `interface.sur`: contains the set of element faces

Depending on the type of boundary conditions used for the coupling, _one_ of the following files might be required:
- `interface.dfl`: initializes the flux values at the interface elements; required if Neumann boundary condition is to be used
- `interface.flm`: initializes the sink temperature $T_{\infty}$ and the heat transfer coefficient $h$; required if Robin boundary condition is to be used

## Boundary conditions in CalculiX

### Dirichlet

To use Dirichlet boundary condition in CalculiX, we add a line that specifies the temperature of the interface.

    *BOUNDARY
    Ninterface,11,11,300

### Neumann

To use Neumann boundary condition in CalculiX, we specify a distributed heat flux (DFL):

    *INCLUDE, INPUT=solid/interface.dfl

A sample of the `.dfl` file is provided bellow.  Each line contains the element ID, the face ID and the value of the heat flux.

    ** DFlux based on interface
    ** DFlux based on interface
    7118, S1, 0.000000e+00
    2781, S4, 0.000000e+00
    6014, S1, 0.000000e+00
    6178, S1, 0.000000e+00
    3818, S2, 0.000000e+00
    ...

### Robin

To use Robin boundary condition in CalculiX, we specify a film heat transfer at the coupled surface.

    *INCLUDE, INPUT=solid/interface.flm


## Location of the coupling data

| Data | Location |
| --- | --- |
| Temperature | Nodes |
| Heat-Flux | Gauss points |
| Sink temperature and heat transfer coefficient | Gauss points |

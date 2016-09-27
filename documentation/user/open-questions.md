# Open questions

## General

- Non-perfect contacts: how to specify something similar to the kappaLayers in chtMultiRegionFoam or GAP CONDUCTANCE in CalculiX
- How to deal with the coupling between two participants of the same type (e.g. two coupled CalculiX participants: use contact or use preCICE?)
- How to include radiation?
    - With Dirichlet-Neumann coupling this might be straightforward: simply exchange the total flux (convective plus radiative) and the temperature.
    - Is it possible to use Robin-Robin coupling? The way the Robin coupling is implemented in CalculiX and Code_Aster, is actually by using a convective type of boundary condition.

## preCICE
- In which cases is it problematic to use radial basis functions (RBF) for the data mapping? (e.g. complex geometries, where the interface consists of several distinct physical surfaces: the RBF interpolant might be constructed with points that belong to another surface.)

## OpenFOAM

## CalculiX

## Code_Aster

- Initialization time
- Dealing with interfaces that have multiple materials assigned
- How does having the write data at the nodes and the read data at the face centers affect accuracy, stability, etc.?
- Is domain decomposition possible? If yes, then each process has to set its own part of the mesh in preCICE.


# Future work / possible improvements

Some ideas that might be implemented in the future:
- In preCICE, add a new type of data-mapping scheme, like the nearestPatchFace from OpenFOAM

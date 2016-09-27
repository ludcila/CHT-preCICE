# Overview of preCICE

This section summarizes the main components of preCICE and the terms that are commonly used. The purpose of this section is to get the user familiar with the terminology used in the rest of the documentation. It does not intend to be a complete documentation of the coupling library.  For more detailed information about preCICE, please refer to the following resources:

- http://www5.in.tum.de/pub/Gatzhammer2014_preCICE.pdf
- http://www.precice.org/publications/
- https://github.com/precice/precice/wiki

## Contents
<!-- toc orderedList:0 -->

- [Overview of preCICE](#overview-of-precice)
	- [Contents](#contents)
	- [Participants](#participants)
	- [Adapter](#adapter)
	- [Coupling data](#coupling-data)
	- [Meshes](#meshes)
	- [Data-mapping schemes](#data-mapping-schemes)
	- [Coupling schemes](#coupling-schemes)

<!-- tocstop -->

## Participants

A participant is a solver instance that participates in the coupling.  There may be more than two participants in the coupling, and more than one instance from the same solver type is possible (e.g. two OpenFOAM participants and one CalculiX participant).

## Adapter

The adapter connects the solver with the coupling library.  It is in charge of steering the solver and making calls to the coupling library.

## Coupling data

This is the data exchanged by the participants.  In the implemented CHT solution, possible choices of data are listed below (the naming that is recognized by the adapted solvers is shown in parenthesis):

- Temperature (`Temperature`)
- Heat flux (`Heat-Flux`)
- Sink temperature and heat transfer coefficient (`Sink-Temperature-*`, `Heat-Transfer-Coefficient-*`, where * could be `Fluid`, `Solid`, `A` or `B`)

## Meshes

Within preCICE, a mesh refers specifically to a surface mesh that is coupled (the interface).  

A participant may have more than one surface mesh:
- If different meshes are required to describe different data on the same physical interface (e.g. CalculiX uses a nodes mesh for the temperature and a face centers mesh for the heat-flux);
- If it is coupled with more than one participant
- More than one interface between the same two participants is also possible, but should be avoided if possible, in order to avoid increasing the complexity of the setup.

## Data-mapping schemes

This refers to the method that is used to map the data from the surface mesh of one participant to the surface mesh of the other participant.

- Nearest neighbor (cloud of points)
- Nearest projection (requires specifying the vertices that constitute the faces, but the data must be located at the nodes!)
- Radial basis functions (several different types available)

Furthermore, each of these mapping schemes can be either consistent or conservative:

- Conservative mapping: sum of the data values is equal on both sides of the mapping
- Consistent mapping: constants must be interpolated exactly

> In this CHT implementation, only consistent mapping is used!  Therefore it is not mentioned anymore in this document.  Consistent mapping is always assumed.

## Coupling schemes

In preCICE, the coupling scheme refers to whether the coupling is implicit or explicit, serial or parallel.  In the literature, it may also refer to the type of partitioning (boundary conditions) used.

| Type of coupling | Description |
| --- | --- |
| Implicit / Explicit | Implicit: coupling iterations are performed until convergence.  Explicit: Only one coupling iteration per timestep |
| Serial / Parallel | Serial: Gauss-Seidel type of update; participants are executed in turn, using the most recent data from the other participant. Parallel: Jacobi type of update  (interfield parallelism).   |
| Dirichlet-Neumann / Robin-Robin | Depends on what coupling data is used (or what kind of boundary conditions). |

Note that when talking about the coupling scheme, parallelism refers to interfield parallelism (e.g. solving the fluid equations and the solid equations in parallel).

Combinations of implicit/explicit, serial/parallel couplings are referred to as `coupling-scheme` and need to be specified in the `precice-config.xml` file.

Available options are:
- `coupling-scheme:serial-explicit`
- `coupling-scheme:serial-implicit`
- `coupling-scheme:parallel-explicit`
- `coupling-scheme:parallel-implicit`
- `coupling-scheme:multi`

The first four coupling schemes are defined between two participants, and can be defined as implicit or explicit.  The last coupling scheme can be configured for multiple participants coupled implicitly.

Whether Dirichlet-Neumann or Robin-Robin type of coupling is used, depends on what coupling data is exchanged and what boundary conditions are set on the solvers.  This is transparent to preCICE, who only knows that data is exchanged.

# Overview

Let's assume that the solver has this structure:

    t = 0
    while t < tFinal:
        t = t + dt
        solve

## Explicit coupling:

Explicit coupling requires only:
- Reading the coupling data and updating the boundary conditions
- Writing the coupling data after the solver solves
- Exchanging the data by calling the `advance()` function of preCICE



    t = 0
    while t < tFinal:
        t = t + dt
        read coupling data
        solve
        write coupling data
        advance coupling

- Read coupling data: `precice.readBlockScalarData(...)` or `precice.readBlockVectorData(...)`
- Write coupling data: `precice.writeBlockScalarData(...)` or `precice.writeBlockVectorData(...)`
- Advance: `precice.advance(dt)`

For an example of explicit coupling, have a look at the adapted `buoyantSimpleFoam` solver.

## Implicit coupling:

In the case of implicit coupling, one time step may be solved multiple times.  This means that the solver must be able to return to its state at the beginning of the step, and therefore checkpointing is necessary.

    t = 0
    while t < tFinal:
        t = t + dt
        if beginning of new coupling iteration:
            write checkpoint
        read coupling data
        solve
        write coupling data
        advance coupling
        if coupling did not converge:
            read checkpoint


Sometimes, if the solver involves solving the fields iteratively in a loop, the solver iterations can be "fused" with the coupling iterations.  The time is not advanced until the coupling converges, and checkpointing is avoided:

    while coupling is ongoing:
        read coupling data and update boundary conditions
        adjust time step
        for solver iterations 1..n:
            solve
        write coupling data
        advance coupling
        if coupling converged:
            t = t + dt

However, if subcycling is to be used, checkpointing will be always necessary.

## Official example from preCICE

https://github.com/precice/precice/wiki/Adapter%20Example

## How to couple OpenFOAM, CalculiX, Code_Aster

- Where is the solver loop?
    - in OpenFOAM, it is in the `main` function of each solver (e.g. solvers/heatTransfer/buoyantPimpleFoam/buoyantPimpleFoam.C)
    - in CalculiX, it is in the `nonlingeo` function (the solver for all non-linear problems, including heat transfer)
    - in Code_Aster, instead of modifying the solver loop, a loop is created outside of the thermal solver.  Usually, one would provide the solver with a list of time steps to be solved.  However, the approach taken here, is to create a loop outside the solver, and call the solver to solve only one step at a time
- Where is the boundary data located? At the nodes? At the face centers? Are boundary conditions assigned to the nodes or to the element faces?
- How to access to the boundary mesh
- How to update the boundary conditions
    - Boundaries need to be properly initialized so that the values can be changed afterwards

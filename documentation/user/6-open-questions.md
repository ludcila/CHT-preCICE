# TODO

## Open points of the current work

- Open questions
- Issues that could not be solved
- Things that could be done in a better way
- Aspects that have not been fully tested (require closer inspection)

### OpenFOAM
- There are several unanswered questions regarding the **checkpointing**:
    - Checkpointing is always necessary if sub-cycling is used.  However, when the solver's step size matches the coupling step size, it _should_ be possible to avoid checkpointing, by combining the solver iterations with the coupling iterations.  However, this approach was attempted, but it resulted in bad convergence of the coupling.  Further investigation is necessary to determine why it does not work as expected
    - The current checkpointing is done in a very naive way.  Plain copy assignment is done to the fields that need checkpointing.  Is this the proper way to do it?
- It is not clear whether **nOuterCorrectors** for PIMPLE can be set to be > 1.  In the cavity case (with turbulence), setting nOuterCorrectors = 5 led to bad results (still have to test whether it also happens without turbulence)
    - At the end of the PIMPLE loop, relaxation is set to 1 -- what happens if the PIMPLE loop is repeated for the same time step (i.e. implicit coupling).  Are the relaxation factors reset to something < 1?
- **Subcycling** is possible for the coupled buoyantPimpleFoam, but
    - Need to test whether it works well for nOuterCorrectors > 1
    - If a fixed time step is used (`adjustTimeStep no`), the coupling time step must be devisible by the sub-cycle step size (e.g. coupling time step of 1.0 and sub-cycle of 0.25).  Otherwise, for example if the coupling time step is 1.0 and the solver step size is set to 0.3 (`deltaT 0.3`), at the fourth time step, preCICE will allow a maximum step size of 0.1, in order to synchronize the solvers and exchange the data.  If the time step is not adjustable, then it will not be set to 0.3 again, and the rest of the subsequent solves will be performed with a step size of 0.1.
    - The writing of the results does not work well when the time step is automatically adjusted, unless `writeControl timeStep` and `writeInterval 1` is used.

### CalculiX
- **Contact** together with preCICE coupling has not been tested

### Code_Aster

- **Initialization** time
- Dealing with interfaces that have **multiple materials** assigned: for the coupling, it is necessary to read the value of the conductivity
- How does having the write data at the nodes and the read data at the face centers affect accuracy, stability, etc.?
    - In the flat plate validation case (transient) a very fine solid mesh is required to obtain a smooth solution
    - In the steady-state case, RBF functions had to be used to obtain a smooth solution
- Robin coupling boundary condition requires obtaining a temperature value at an interior point of the solid near to the interface.  The **distance** has been hard-coded to 1e-5.  This value can be found in `adapter.py`, as a member `delta` of the `Interface` class.  This value should be adjusted depending on the mesh size.
- Is **domain decomposition** possible? If yes, then each process has to set its own part of the mesh in preCICE.

### preCICE
- In which cases is it problematic to use radial basis functions (RBF) for the data mapping? (e.g. complex geometries, where the interface consists of several distinct physical surfaces: the RBF interpolant might be constructed with points that belong to another surface.)
- How to deal with the coupling between two participants of the same type (e.g. two coupled CalculiX participants: use contact or use preCICE?)


## Future work

- **Radiation** heat transfer at the conjugate interface
    - With Dirichlet-Neumann coupling this might be straightforward: simply exchange the total flux (convective plus radiative) and the temperature.
    - Is it possible to use Robin-Robin coupling? The way the Robin coupling is implemented in CalculiX and Code_Aster, is actually by using a convective type of boundary condition.
- **Non-perfect contacts**: how to specify something similar to the kappaLayers in chtMultiRegionFoam or GAP CONDUCTANCE in CalculiX
- **Multiple fluid regions**: the adapter was implemented in such a way that it could be reused by multiple OpenFOAM solvers; multiple regions are currently not supported by the adapter, because only one OpenFOAM solver supports it (chtMultiRegionFoam).
- More **validation** is required for the following cases:
    - Turbulent flows --> how turbulence affects heat transfer (the effective fluid conductivity depends on the turbulence variable alphat)
    - Natural convection (buoyancy effects)
    - Non-linear cases (temperature-dependent materials)

Some ideas that might be implemented in the future:

For preCICE:
- Add a new type of data-mapping scheme, like the nearestPatchFace from OpenFOAM
- Allow using the same data names: for example, when doing Robin-Robin coupling, it is currently necessary to use different data names to distinguish the data that is sent/received from each side of the coupling, even though they have the same physical meaning (e.g. `Sink-Temperature-A`, `Sink-Temperature-B`) (explanation to be improved...)

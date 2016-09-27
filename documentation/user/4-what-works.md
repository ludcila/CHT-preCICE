# What works

This section describes configurations that have been tested to work.

### Coupling boundary conditions

- Robin-Robin (RR): the preferred option, superior in stability and convergence.
- Dirichlet-Neumann (DN): Fluid Dirichlet - Solid Neumann (FDSN) worked well in the flat plate validation case.  For the rest of the document, DN actually refers to FDSN.

## preCICE setup

### Coupling-scheme

- Explicit coupling scheme: safe to use only when using Robin-Robin coupling.  Robin-Robin explicit coupling must be used for steady-state setups.
- Implicit coupling scheme

Below are some suggestions regarding the parameters for implicit coupling:

- `exchange`
    - `initialize="yes"`: always initialize the data
- `extrapolation-order`:
- `max-iterations`:
    - Robin-Robin:
        - Large value (e.g. 200): to ensure convergence
        - Small value (e.g. 5) as a trade-off between accuracy and computational cost
- `relative-convergence-measure`
    - Dirichlet-Neumann: Require convergence of both temperature and heat-flux (`suffices="0"`)
    - Robin-Robin: For laminar flow and constant material properties is usually enough to require convergence of the sink temperature

### Multi-coupling (`coupling-scheme:multi`)

Multi-coupling can/should be used when:

- There is more than one interface between the same pair of participants.
- There are more than two participants

### Data-mapping

- Nearest neighbor was the most used in the thesis
- Nearest projection cannot be used in most of the cases, because the data is located at the faces and not at the nodes (for nearest projection to work, we need to specify the vertices of the faces, and the data must be located at these vertices)
- Radial basis functions: there are some challenges when using RBFs:
    - If they have global support, they are not efficient if the interface meshes are large
    - If they have local support, we have to choose a suitable support radius, which depends on the mesh, and might be problematic for complex geometries or if the mesh has both very coarse and very fine elements (?)

### Post-processing (??)

(needs more testing... especially for turbulent flows)

#### Laminar flow

When using implicit coupling, IQN-ILS data post-processing is recommended.  Below are some comments on the possible parameter values for implicit coupling with IQN-ILS post-processing:

- `initial-relaxation`
    - Robin-Robin: `value="1.0"`
    - Dirichlet-Neumann: `value="0.1"`
- `max-used-iterations`:
    - Depends on the size of the interface (PENDING)
- `timesteps-reused`:
    - `value="10"`

#### Turbulent flow

In order for IQN-ILS to work properly, correct checkpointing of the variables is important.  The checkpointing of the turbulence variables is not fully implemented, and therefore Aitken is suggested for turbulent simulations.

# Documentation

## Tags

- `data`:
    - Generate a `Heat-Transfer-Coefficient-*` and `Sink-Temperature-*` for each participant
- `mesh`:
    - For each OpenFOAM participant:
        - add a `[self]-to-[partner]` mesh
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[partner]`
    - For each CalculiX participant:
        - add a `[self]-to-[partner]` mesh
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[partner]`
    - For each Code_Aster participant:
        - add a `[self]-to-[partner]-Nodes` mesh
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
        - add a `[self]-to-[partner]-Faces` mesh
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[partner]`
- `participant`:
    - For each OpenFOAM/CalculiX participant:
        - add `<use-mesh name="[self]-to-[partner]" provide="yes"/>`
        - for each coupled partner:
            - if partner is OpenFOAM:
                - add `<use-mesh name="[partner]-to-[self]" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="[partner]-to-[self]" to="[self]-to-[partner]" constraint="consistent" timing="initial"/>`
            - if partner is CalculiX:
                - add `<use-mesh name="[partner]-to-[self]" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="[partner]-to-[self]" to="[self]-to-[partner]" constraint="consistent" timing="initial"/>`
            - if partner is Code_Aster:
                - add `<use-mesh name="[partner]-to-[self]-Nodes" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="[partner]-to-[self]-Nodes" to="[self]-to-[partner]" constraint="consistent" timing="initial"/>`
            - any:
                - add `<write-data name="Heat-Transfer-Coefficient-[self]" mesh="[self]-to-[partner]/>"`
                - add `<write-data name="Sink-Temperature-[self]" mesh="[self]-to-[partner]/>`
                - add `<read-data name="Heat-Transfer-Coefficient-[partner]" mesh="[self]-to-[partner]/>`
                - add `<read-data name="Sink-Temperature-[partner]" mesh="[self]-to-[partner]/>`
    - For each Code_Aster participant:
        - add `<use-mesh name="[self]-Nodes-Mesh" provide="yes"/>`
        - add `<use-mesh name="[self]-Faces-Mesh" provide="yes"/>`
        - for each coupled partner:
            - if partner is OpenFOAM/CalculiX:
                - add `<use-mesh name="[partner]-to-[self]" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="[partner]-to-[self]" to="[self]-to-[partner]-Faces" constraint="consistent" timing="initial"/>`

- `coupling-scheme`:
    - ?


## Naming conventions

- Data names
    - Heat-Transfer-Coefficient-[participant]
    - Sink-Temperature-[participant]
- Mesh names
    - [self]-to-[partner]
    - [self]-to-[partner]-Nodes and [self]-to-[partner]-Faces (for Code_Aster)

## Settings

- Always read mapping `<mapping:... direction="read" .../>`

### Steady-State
- Forces explicit coupling

### Transient

#### Implicit

- IQN-ILS
    - `filter` 1e-6 : if convergence is initially good but starts to go bad after some time

### Coupling (rules)

These are the rules for deciding what type of coupling to use (serial/parallel x implicit/explicit or multi)

Input:
- steady-state
- (auto-detected:has-cycles)
- option:force-explicit
- option:force-parallel

Output:
- explicit = steady-state OR force-explicit
- implicit = NOT explicit
- parallel-multi = has-cycles OR force-parallel
    - multi = parallel-multi AND implicit AND num-participants > 2
    - parallel = parallel-multi AND NOT multi
- serial = NOT parallel AND NOT multi

Note: multi-coupling is temporarily disabled, because it is not clear in all cases how to define a controller for the multi-coupling.  Multiple parallel-implicit couplings are used instead.

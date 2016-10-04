# Documentation

## Tags

- `data`:
    - Generate a `Heat-Transfer-Coefficient-*` and `Sink-Temperature-*` for each participant
- `mesh`:
    - For each OpenFOAM participant:
        - add a `Faces-Mesh-[self]`
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[neighbor]`
    - For each CalculiX participant:
        - add a `Faces-Mesh-[self]`
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[neighbor]`
    - For each Code_Aster participant:
        - add a `Nodes-Mesh-[self]`
            - add `use-data` tag for its own data: `Heat-Transfer-Coefficient-[self]` and `Sink-Temperature-[self]`
        - add a `Faces-Mesh-[self]`
            - add `use-data` tag for partner's data: `Heat-Transfer-Coefficient-[partner]` and `Sink-Temperature-[neighbor]`
- `participant`:
    - For each OpenFOAM/CalculiX participant:
        - add `<use-mesh name="[self]-Faces-Mesh" provide="yes"/>`
        - for each coupled partner:
            - if partner is OpenFOAM:
                - add `<use-mesh name="[partner]-Faces-Mesh" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="Faces-Mesh-[partner]" to="Faces-Mesh-[self]" constraint="consistent" timing="initial"/>`
            - if partner is CalculiX:
                - add `<use-mesh name="[partner]-Faces-Mesh" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="Faces-Mesh-[partner]" to="Faces-Mesh-[self]" constraint="consistent" timing="initial"/>`
            - if partner is Code_Aster: `<use-mesh name="[partner]-Nodes-Mesh" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="Nodes-Mesh-[partner]" to="Faces-Mesh-[self]" constraint="consistent" timing="initial"/>`
            - any:
                - add `<write-data name="Heat-Transfer-Coefficient-[self]" mesh="Faces-Mesh-[self]/>"`
                - add `<write-data name="Sink-Temperature-[self]" mesh="Faces-Mesh-[self]/>`
                - add `<read-data name="Heat-Transfer-Coefficient-[partner]" mesh="Faces-Mesh-[self]/>`
                - add `<read-data name="Sink-Temperature-[partner]" mesh="Faces-Mesh-[self]/>`
    - For each Code_Aster participant:
        - add `<use-mesh name="[self]-Nodes-Mesh" provide="yes"/>`
        - add `<use-mesh name="[self]-Faces-Mesh" provide="yes"/>`
        - for each coupled partner:
            - if partner is OpenFOAM:
                - add `<use-mesh name="[partner]-Faces-Mesh" from="[partner]"/>`
                - add `<mapping:nearest-neighbor direction="read" from="Faces-Mesh-[partner]" to="Faces-Mesh-[self]" constraint="consistent" timing="initial"/>`
            - if partner is CalculiX
- `coupling-scheme`:
    -


## Naming conventions

- Data names
    - Heat-Transfer-Coefficient-[participant]
    - Sink-Temperature-[participant]
- Mesh names
    - [interface]
    - [interface]-Faces or [interface]-Nodes for Code_Aster

## Settings

- Always read mapping `<mapping:... direction="read" .../>`

### Steady-State
- Timestep 1
- Serial/Parallel explicit coupling

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

# Automatic configuration utilities

## Contents

<!-- toc orderedList:0 -->

- [Automatic configuration utilities](#automatic-configuration-utilities)
	- [Contents](#contents)
	- [Configuration file generation utility](#configuration-file-generation-utility)
		- [Input and output files](#input-and-output-files)
		- [Simulation parameters](#simulation-parameters)
		- [Participant parameters](#participant-parameters)
	- [Case setup utility](#case-setup-utility)
	- [Post-processing utility](#post-processing-utility)

<!-- tocstop -->

## Configuration file generation utility

### Input and output files

```{mermaid}
graph TB
config.yml.org --> A(auto-config utility)
A --> precice-config.xml
A --> config.yml
A --> config.comm
A --> run.sh
style A fill: #fff
```

	python auto-config.py
		[--input-file default:config.yml.org]
		[--output-xml-file default:precice-config.xml]
		[--output-yml-file default:config.yml]
		[--output-comm-file default:config.comm]
		[--output-sh-file default:run.sh]

Input files:
- `config.yml.org`

Output files
- `precice-config.xml`
- `config.yml`
- `config.comm` (only generated if there is a Code_Aster participant)
- `run.sh`

### Simulation parameters

General parameters (in `config.yml.org`):
- `time-step`
- `max-time`
- `max-coupling-iterations` (only applies if implicit coupling is used)
- `output-frequency`

Parameters that affect the type of coupling (in `config.yml.org`):
- `steady-state`: if set to True, explicit coupling will be used
- `force-explicit`: if set to False, it will use implicit coupling
- `force-parallel`: if set to False, it will try to use serial coupling.  Parallel coupling will be used if `force-parallel` is True, or, if there are cycles in the coupling dependency graph

Fixed parameters (at the moment cannot be changed):
- `IQN-ILS` post-processing for implicit coupling
- `nearest-neighbor` data mapping

### Participant parameters

General:
- `domain-decomposed`: whether the participant is run in parallel (domain decomposition - currently only supported by OpenFOAM!)
- `directory`: case directory of the participant

Code_Aster specific:
- `non-linear`
- `material-id` must be specified for each interface


## Case setup utility


```{mermaid}
graph TB
config.yml.org --> A(case-setup utility)
A --> controlDict
A --> *.inp
A --> *.export
style A fill: #fff
```

(To be implemented)

This utility ensures consistency between the input parameters of the different participants.  This include:

- Matching timestep size
- Matching output frequency
- Boundary conditions

## Post-processing utility

(To be implemented)

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Coupling Code_Aster](#coupling-codeaster)
	- [Location of the coupling data](#location-of-the-coupling-data)
- [Code_Aster files](#codeaster-files)
	- [Code_Aster `.export` file](#codeaster-export-file)
	- [Case definition file (`setup.comm`)](#case-definition-file-setupcomm)
	- [Other configuration parameters (`config.comm`)](#other-configuration-parameters-configcomm)
	- [The adapter (`adapter.comm`)](#the-adapter-adaptercomm)
	- [Execution](#execution)
	- [Important: Full paths](#important-full-paths)

<!-- /TOC -->

# Coupling Code_Aster

> Important: Only Robin-Robin coupling has been implemented for Code_Aster!

## Location of the coupling data

| Data | Location |
| --- | --- |
| (Input) Read from another solver and applied as boundary condition | Face centers |
| (Output) Sent to other coupled solvers | Nodes |

# Code_Aster files

Unlike OpenFOAM and CalculiX,  it is not necessary to compile and execute a separate binary of Code_Aster in order to do the coupling.  It is enough to include additional files to the case.  These are described in the following table.  All these files are in written in `Python`.

| File | Description | Unit |
| --- | --- | --- |
| `setup.comm` | Contains the case definition as in a normal Code_Aster simulation (i.e., creation of the mesh, definition and assignment of materials, initial and boundary conditions, etc.).  The difference is that the solver is not called (e.g. no calls to `THER_LINEAIRE`), as this is called from the adapter | 91 |
| `config.comm` | Contains additional configuration parameters | 90 |
| `adapter.comm` | Contains the timestepping / coupling loop; makes calls to the solver | 1 |

> Please note that the names of the files are not important, but the number of the logical unit is!

## Code_Aster `.export` file

To include these additional files to the case, we add specify them in the `.export` file.

Below is a snippet of the `.export` file.  Note that there are three `.comm` files, as opposed to one, as is common in most Code_Aster cases.  The main `.comm` file is the adapter.  The adapter can be reused accross different simulations.  The other two `.comm` files are case specific and need to be created for each simulation.

    ...
    F comm /home/lcheung/Thesis/CHT-preCICE/solvers/Code_Aster/adapter.comm D  1
    F comm /home/lcheung/Thesis/precice-cases/flow-over-plate-steady-state/buoyantSimpleFoam-aster/solid/setup.comm D  91
    F comm /home/lcheung/Thesis/precice-cases/flow-over-plate-steady-state/buoyantSimpleFoam-aster/solid/config.comm D  90
    F mmed /home/lcheung/Thesis/precice-cases/flow-over-plate-steady-state/buoyantSimpleFoam-aster/solid/solid.mmed D  20
    ...

## Case definition file (`setup.comm`)

The variables that are defined in this file are:

    MESH
    MODEL
    MAT
    BC
    INIT_T

> The names of the variables are the same as those generated by the SimScale platform.

## Other configuration parameters (`config.comm`)


    settings = dict(

    	# Solver
    	isNonLinear = [True/False],

    	# preCICE
    	preciceConfigFile = [string],      # Use full path!
    	participantName = [string],
    	interfaces = [{
            "materialID": [int],            # e.g. 0 if MAT[0] was assigned
            "groupName": [string],          # GROUP_MA name
            "nodesMeshName": [string],      # preCICE mesh name
            "faceCentersMeshName": [string] # preCICE mesh name
        }],

    	# Output
    	outputFrequency = [int],
    	outputDirectory = [string]

    )


## The adapter (`adapter.comm`)

Finally, the complete case is run by the adapter.  The adapter will include the two previous files.

The `adapter.comm` file has two main responsabilities:
- Declaring the boundary condition of the coupled interface(s)
- Controlling the timestepping and the exchange of coupling data

Below is a snippet of the adapter code.

    ...

    adapter = Adapter(precice, settings["interfaces"], MESH, MODEL, MAT, isNonLinear=settings["isNonLinear"])
    BCs = [{'CHARGE': bc} for bc in BC]
    LOADS = BCs + adapter.LOADS

    preciceDt = precice.initialize()

    ...

    while precice.isCouplingOngoing():

    	adapter.receiveCouplingData()

        if settings["isNonLinear"]:
    		TEMP = THER_NON_LINE(
    			MODELE=MODEL,
    			CHAM_MATER=MATS,
    			EXCIT=LOADS,
    			ETAT_INIT=_F(STATIONNAIRE='OUI'),
    		)
    	else:
    		TEMP = THER_LINEAIRE(
    			MODELE=MODEL,
    			CHAM_MATER=MATS,
    			EXCIT=LOADS,
    			ETAT_INIT=_F(STATIONNAIRE='OUI'),
    		)
    	T = CREA_CHAMP(
    		RESULTAT=TEMP,
    		NOM_CHAM='TEMP',
    		TYPE_CHAM='NOEU_TEMP_R',
    		OPERATION='EXTR'
    	)

    	adapter.sendCouplingData(T, preciceDt)

    	preciceDt = precice.advance(preciceDt)

    	time = time + preciceDt

        ...


## Execution

    as_run --run solid/solid.export

## Important: Full paths

When running a Code_Aster case, all the input files are copied into a temporary folder, where the case is executed.  Therefore, it is important to use full paths:

- The full path to the `precice-config.xml` file must be specified in `config.comm`.
- In the `precice-config.xml` file, the exchange directory must be explicitly specified and the full path must be used:

    &lt;m2n:sockets from="OpenFOAM" to="Aster" exchange-directory="/full/path/to/coupled-case"/&gt;
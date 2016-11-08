# CalculiX Adapter

## Contents


<!-- toc orderedList:0 -->

- [CalculiX Adapter](#calculix-adapter)
	- [Contents](#contents)
	- [Downloading and Installing CalculiX 2.10](#downloading-and-installing-calculix-210)
	- [Adapter Files](#adapter-files)
		- [Main files](#main-files)
			- [CalculiX Documentation](#calculix-documentation)
	- [Running the Adapted CalculiX](#running-the-adapted-calculix)
	- [Parallelization](#parallelization)

<!-- tocstop -->


## Downloading and Installing CalculiX 2.10
Download CalculiX 2.10 from http://www.dhondt.de/ccx_2.10.src.tar.bz2.  Follow the instructions in the README.INSTALL file included in the package.

The graphical user interface for version 2.10 can be downloaded from http://dhondt.de/cgx_2.10.all.tar.bz2.  Follow the instructions included in the package.

PDF documentation: http://www.dhondt.de/ccx_2.10.pdf

## Adapter Files

The `Makefile` was modified so that it uses the original source code from CalculiX, but overriding the files that have been changed for coupling with preCICE and including the additional files present in this directory.

Modified files:

    ccx_2.10.c
    CalculiX.h
	Makefile

Additional files:

    nonlingeo_precice.c
    getflux.f
    getkdeltatemp.f
    adapter/ConfigReader.cpp
    adapter/ConfigReader.hpp
    adapter/CCXHelpers.c
    adapter/CCXHelpers.h
    adapter/PreciceInterface.c
    adapter/PreciceInterface.h

Variables that may need to be changed in the Makefile:

 1. `CCX`: location of the original CalculiX source code (version 2.10)
 2. `SPOOLES`: location of SPOOLES
 3. `ARPACK`: location of ARPACK
 4. `PRECICE_C_BUILD`: location of preCICE, built with `mpicc`

To compile, run `make`.


### Main files

- `nonlingeo.c` is the solver for non-linear problems, it was copied and renamed as `nonlingeo_precice.c`.  Calls to the adapter were added to this file.
- `ccx_2.10.c` contains the entry point of the program.  It checks whether preCICE is used, and calls `nonlingeo` or `nonlingeo_precice` accordingly:
    - if the command line argument `-precice-participant` is used, it will run the adapted version for CHT;
    - otherwise, it will execute the original CalculiX solvers.
- `PreciceInterface.c` is the adapter.
- `CCXHelpers.c` contains some helper functions to handle CalculiX's data.


#### CalculiX Documentation
These information is particularly useful for understanding / developing the adapter:
- Program structure of the non-linear solver: page 503 of ccx_2.10.pdf
- List of variables and their meaning: page 518 of ccx_2.10.pdf

## Running the Adapted CalculiX

The command to run a CalculiX participant is the following:

	ccx_preCICE -i [CalculiX input file] -precice-participant [participant name]

For example, if the CalculiX input file is `solid/solid.inp` and the participant name is CCX, we use:

    ccx_preCICE -i solid/solid -precice-participant CCX

## Parallelization

CalculiX supports multithreaded computations.  However, from preCICE's point of view, it is just a serial participant.  Please have a look at page 9 of the documentation ccx_2.10.pdf for more information on performing calculations in parallel.

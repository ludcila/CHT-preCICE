# CalculiX

## Contents


<!-- toc orderedList:0 -->

- [CalculiX](#calculix)
	- [Contents](#contents)
	- [Installing CalculiX 2.10](#installing-calculix-210)
	- [Adapting CalculiX for CHT using preCICE](#adapting-calculix-for-cht-using-precice)
	- [Main files](#main-files)
	- [Running the adapted CalculiX](#running-the-adapted-calculix)

<!-- tocstop -->


## Installing CalculiX 2.10
Follow the instructions in the website.

## Adapting CalculiX for CHT using preCICE

The `Makefile` was modified such that it uses the original source code from CalculiX, but overriding the files that have been changed for preCICE and including additional files present in this directory.

Modified files:

    ccx_2.10.c
    CalculiX.h

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
    adapter/Helpers.c
    adapter/Helpers.h

Variables that may need to be changed in the Makefile:

 1. `CCX`: location of the original CalculiX source code (version 2.10)
 2. `SPOOLES`: location of SPOOLES
 3. `ARPACK`: location of ARPACK

To compile, simply run `make`.


## Main files

- `ccx_2.10.c` contains the entry point of the program.  It checks whether preCICE is used, and calls `nonlingeo` or `nonlingeo_precice` accordingly
    - If the command line argument `-precice-participant` is used, it will run the adapted version for CHT
    - Otherwise, it will execute the original CalculiX solvers
- `nonlingeo.c` was copied to `nonlingeo_precice.c`, and coupling code was added

## Running the adapted CalculiX

    ccx_preCICE -i solid/solid -precice-participant CCX

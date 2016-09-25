# OpenFOAM adapted solvers #

This document describes how to compile and run the adapted OpenFOAM solvers.  For more details on how to compile a new OpenFOAM solver with preCICE, please refer to this other document.



<!-- toc orderedList:0 -->

- [OpenFOAM adapted solvers](#openfoam-adapted-solvers)
	- [Installing OpenFOAM 3.0](#installing-openfoam-30)
		- [Create a new compilation rule for wmake](#create-a-new-compilation-rule-for-wmake)
	- [Compiling the adapted OpenFOAM solvers](#compiling-the-adapted-openfoam-solvers)
	- [Running the adapted OpenFOAM solvers](#running-the-adapted-openfoam-solvers)

<!-- tocstop -->



## Installing OpenFOAM 3.0
    sudo apt-get install openfoam30

### Create a new compilation rule for wmake

We first have to create a new compilation rule for our adapted FOAM solvers.  We do so by copying an existing rule:

    cd $WM_DIR/rules
    sudo cp -r ${WM_ARCH}${WM_COMPILER} ${WM_ARCH}Mpicc
    cd ${WM_ARCH}Mpicc

We open the file named `c++` inside the `${WM_ARCH}Mpicc` directory.  Next, we change the compiler to `mpic++`, and we add the flag `-std=c++11`.  After these changes, line 8 of this file should look like this:

    CC          = mpic++ -m64 -std=c++11

To use these new rules, we have to change the environment variable `WM_COMPILER`:

    export WM_COMPILER=Mpicc

## Compiling the adapted OpenFOAM solvers
Several adapted solvers for CHT are provided in the `solvers` directory.  In order to compile them, we first need to compile the adapter library in the `adapter` folder:

    cd adapter
    wmake lib
    mv libNULL.a libFoamPreciceAdapter.a
Now we are ready to compile the solvers, by entering to each solver directory and running `wmake`.  For example:

    cd laplacianFoam
    wmake

> Important: make sure that `WM_COMPILER` is set to `Mpicc`.

## Running the adapted OpenFOAM solvers

For example, outside of an OpenFOAM case directory named `Fluid`, we run the adapted laplacianFoam solver with:

    laplacianFoam_preCICE -case Fluid -precice-participant [participant name] -precice-config [YAML config file]

# Installing OpenFOAM 3.0 #
    sudo apt-get install openfoam30

### Create a new compilation rule for wmake ###

We first have to create a new compilation rule for our adapted FOAM solvers.  We do so by copying an existing rule:

    cd $WM_DIR/rules
    sudo cp -r ${WM_ARCH}${WM_COMPILER} ${WM_ARCH}Mpicc
    cd ${WM_ARCH}Mpicc

We change the compiler to `mpic++`, and we add the flag `-std=c++11`, in the file named `c++` inside the `${WM_ARCH}Mpicc` directory.  Line 8 of this file should look like this:

    CC          = mpic++ -m64 -std=c++11

To use these new rules, we have to change the environment variable `WM_COMPILER`:

    export WM_COMPILER=Mpicc

# Compiling the adapted OpenFOAM solvers #
Several adapted solvers for CHT are provided in the `solvers` directory.  In order to compile them, we first need to compile the adapter library in the `adapter` folder:

    cd adapter
    wmake lib

Now we are ready to compile the solvers, by entering to each solver directory and running `wmake`.  For example:

    cd laplacianFoam
    wmake

Alternatively, the `Allwmake` script can be used, which will take care of compiling the adapter and the solvers:

    ./Allwmake

# Running the adapted OpenFOAM solvers #

For example, outside of an OpenFOAM case directory named `Fluid`, we run the adapted laplacianFoam solver with:

    laplacianFoam_preCICE -case Fluid -precice-participant [participant name] -precice-config [YAML config file]

# Compiling and linking OpenFOAM with preCICE #
This section describes how to compile a new OpenFOAM solver with preCICE.  If you don't want to adapt your own OpenFOAM solver, you may skip this section.

A tutorial on how to create a new OpenFOAM solver based on a standard OpenFOAM solver can be found here:
https://openfoamwiki.net/index.php/How_to_add_temperature_to_icoFoam

### Make a copy of the FOAM solver to be adapted ###
Copy the original solver files from `$FOAM_SOLVERS`, which need to be modified.  These include the Make directory, the solver .C file and possibly also .H files.
For example:

    cd solvers
    mkdir laplacianFoam
    cd laplacianFoam
    cp -r $FOAM_SOLVERS/basic/laplacianFoam/Make .
    cp $FOAM_SOLVERS/basic/laplacianFoam/laplacianFoam.C .
    cp $FOAM_SOLVERS/basic/laplacianFoam/createFields.H .

### Edit the files in the Make directory ###
Change the name and the directory of the binary that will be created.
For example, the file `Make/files` will look like this:

    laplacianFoam.C

    EXE = $(FOAM_USER_APPBIN)/laplacianFoam_preCICE

Add the directory of the original solver to the include paths in `Make/options`, such that the unmodified files can be located:

    EXE_INC = \
    -I$(FOAM_SOLVERS)/basic/laplacianFoam \
    -I$(LIB_SRC)/finiteVolume/lnInclude \
    -I$(LIB_SRC)/meshTools/lnInclude

Compile by typing `wmake`.

### Including and linking with preCICE ###
If the compilation works, we can now proceed by including the preCICE library.  We continue modifying the `Make/options` file:

    EXE_INC = \
    -I$(FOAM_SOLVERS)/basic/laplacianFoam \
    -I$(LIB_SRC)/finiteVolume/lnInclude \
    -I$(LIB_SRC)/meshTools/lnInclude \
    -I$(PRECICE_ROOT)/src

    EXE_LIBS = \
    -lfiniteVolume \
    -lmeshTools \
    -L$(PRECICE_ROOT)/build/last \
    -L$(PETSC_DIR)/$(PETSC_ARCH)/lib \
    -lprecice \
    -lpetsc \
    -lboost_system \
    -lboost_filesystem \
    -lpython2.7

### Adding the preCICE header ###

We add the following line to the adapted solver (for example in laplacianFoam.C):

    #include "precice/SolverInterface.hpp"

Now we test whether the compilation works by typing `wmake`.   Make sure to have followed the instructions to add the new make rule for wmake and that the environment variable `WM_COMPILER` is set to `Mpicc`.

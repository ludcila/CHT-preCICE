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

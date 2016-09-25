# OpenFOAM

## Overview

In order to couple OpenFOAM, the solver to be coupled is copied and preCICE coupling code is added to the timestepping loop.

- buoyantSimpleFoam
- buoyantPimpleFoam

## Location of the coupling data

All the coupling data of OpenFOAM solvers is located at the cell face centers (i.e. no data is located at the cell vertices).

## Boundary conditions in OpenFOAM

### Dirichlet

    interface
    {
	    type fixedValue;
	    value uniform 0;
    }

### Neumann

    interface
    {
	    type fixedGradient;
	    value uniform 0;
    }

### Robin

    interface
    {
	    type mixed;
	    value uniform 293;
	    refValue uniform 310;
	    refGradient 0; // must be 0!
	    valueFraction 0.5;
    }

## Robin-Robin coupling

Robin-Robin coupling is achieved in OpenFOAM through the use of `mixed` type boundary conditions for the temperature.  Here, it is important to set `refGradient` to 0.

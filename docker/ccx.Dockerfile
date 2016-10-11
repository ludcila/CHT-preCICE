FROM precice
MAINTAINER Lucía Cheung <lcheung@simscale.com>

RUN rm /bin/sh && ln -s /bin/bash /bin/sh

RUN apt-get update \
    && apt-get install -y build-essential \
    git \
    python \
    python-numpy \
    wget \
    gfortran \
    libopenmpi-dev \
    cmake

# Download and extract ccx 2.10
RUN \
    wget http://www.dhondt.de/ccx_2.10.src.tar.bz2 \
    && bzip2 -d ccx_2.10.src.tar.bz2 \
    && tar xvf ccx_2.10.src.tar

# Download and extract cgx 2.10
RUN \
    wget http://www.dhondt.de/cgx_2.10.all.tar.bz2 \
    && bzip2 -d cgx_2.10.all.tar.bz2 \
    && tar xvf cgx_2.10.all.tar

# Move CalculiX to /usr/local
RUN \
    mv CalculiX /usr/local/CalculiX

# Download and install SPOOLES
RUN \
    wget http://www.netlib.org/linalg/spooles/spooles.2.2.tgz \
    && mkdir /usr/local/SPOOLES.2.2 \
    && mv spooles.2.2.tgz /usr/local/SPOOLES.2.2 \
    && cd /usr/local/SPOOLES.2.2 \
    && tar xvzf spooles.2.2.tgz \
    && sed -i 's/drawTree.c/draw.c/g' Tree/src/makeGlobalLib \
    && make CC=gcc lib \
    && make CC=gcc global \
    && cd /usr/local/SPOOLES.2.2/MT && make CC=gcc lib \
    && cd /usr/local/SPOOLES.2.2/MPI && make CC=mpicc lib

# Download and install ARPACK
RUN \
    wget http://www.caam.rice.edu/software/ARPACK/SRC/arpack96.tar.gz \
    && tar xvzf arpack96.tar.gz \
    && mv ARPACK /usr/local/ARPACK \
    && cd /usr/local/ARPACK \
    && sed -i 's/$(HOME)/\/usr\/local/g' ARmake.inc \
    && sed -i 's/\/bin\/make/make/g' ARmake.inc \
    && sed -i 's/f77/gfortran/g' ARmake.inc \
    && sed -i 's/SUN4/INTEL/g' ARmake.inc \
    && sed -i 's/-cg89//g' ARmake.inc \
    && sed -i 's/      EXTERNAL           ETIME/*     EXTERNAL           ETIME/g' UTIL/second.f \
    && make all

# Make original ccx
RUN \
    cd /usr/local/CalculiX/ccx_2.10/src \
    && make

ENV PRECICE_ROOT=/precice

RUN \
    mkdir -p CHT-preCICE/solvers/

COPY \
    solvers/CalculiX /CHT-preCICE/solvers/CalculiX

RUN \
    cd /CHT-preCICE/solvers/CalculiX \
    && rm bin/* \
    && sed -i 's/_org//g' Makefile \
    && sed -i 's/extern "C" {//g' /precice/src/precice/adapters/c/SolverInterfaceC.h \
    && tac /precice/src/precice/adapters/c/SolverInterfaceC.h | sed '0,/}/{s/}//}' | tac > /precice/src/precice/adapters/c/SolverInterfaceC.h \
    && make

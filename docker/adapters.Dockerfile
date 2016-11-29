FROM solvers
MAINTAINER Lucía Cheung <lcheung@simscale.com>

# Auto config

USER \
    root

RUN \
    apt-get update \
    && apt-get install -y python-yaml \
    && apt-get install -y python-lxml \
    && pip install networkx==1.11 --allow-unverified networkx

USER \
    precice

COPY \
    /utilities/ /home/precice/CHT-preCICE/utilities/


# Adapters
# ------------------------------------------------------------------------------

USER \
    root

RUN \
	useradd -m -s /bin/bash precice

RUN \
    mkdir -p /home/precice/CHT-preCICE/solvers/OpenFOAM  \
    && mkdir -p /home/precice/CHT-preCICE/solvers/CalculiX \
    && mkdir -p /home/precice/CHT-preCICE/solvers/Code_Aster \
    && chown -R precice:precice /home/precice/CHT-preCICE

COPY \
    /solvers/Code_Aster /home/precice/CHT-preCICE/solvers/Code_Aster

RUN \
    chown -R precice:precice /home/precice/CHT-preCICE

USER \
    precice


ENV PETSC_DIR=/petsc
ENV PETSC_ARCH=arch-linux2-c-opt

# OpenFOAM

COPY \
    /solvers/OpenFOAM /home/precice/CHT-preCICE/solvers/OpenFOAM

USER \
    root

RUN \
    chown -R precice:precice /home/precice

USER \
    precice

RUN \
    export PRECICE_ROOT=/precice \
    && source /opt/openfoam30/etc/bashrc \
    && export WM_COMPILER=Mpicc \
    && cd ~/CHT-preCICE/solvers/OpenFOAM/adapter \
    && wclean && wmake lib \
    && mv libNULL.a libOpenFoamAdapter.a

RUN \
    export PRECICE_ROOT=/precice \
    && source /opt/openfoam30/etc/bashrc \
    && export WM_COMPILER=Mpicc \
    && cd ~/CHT-preCICE/solvers/OpenFOAM/solvers/buoyantPimpleFoam \
    && wclean && wmake

RUN \
    export PRECICE_ROOT=/precice \
    && source /opt/openfoam30/etc/bashrc \
    && export WM_COMPILER=Mpicc \
    && cd ~/CHT-preCICE/solvers/OpenFOAM/solvers/buoyantSimpleFoam \
    && wclean && wmake

# CalculiX

COPY \
    /solvers/CalculiX /home/precice/CHT-preCICE/solvers/CalculiX

USER \
    root

RUN \
    cp /precice/src/precice/adapters/c/SolverInterfaceC.h /precice/src/precice/adapters/c/SolverInterfaceC.h.bk

RUN \
    chmod 777 /usr/local/CalculiX/ccx_2.10/src/*

RUN \
    sed -i 's/extern "C" {//g' /precice/src/precice/adapters/c/SolverInterfaceC.h \
    && tac /precice/src/precice/adapters/c/SolverInterfaceC.h | sed '0,/}/{s/}//}' | tac > /precice/src/precice/adapters/c/SolverInterfaceC.h

USER \
    root

RUN \
    chown -R precice:precice /home/precice

USER \
    precice

RUN \
    cd ~/CHT-preCICE/solvers/CalculiX \
    && sed -i 's/_org//g' Makefile \
    && make clean && make

USER \
    root

RUN \
    mv /precice/src/precice/adapters/c/SolverInterfaceC.h.bk /precice/src/precice/adapters/c/SolverInterfaceC.h

# Environment variables
USER \
    precice

ENV ASTER_ROOT=/srvtc/cloud/aster-12.6.0/build/

RUN \
    echo 'source /opt/openfoam30/etc/bashrc' >> ~/.bashrc \
    echo 'source $ASTER_ROOT/12.6/share/aster/profile.sh' >> ~/.bashrc \
    && echo 'source $ASTER_ROOT/etc/codeaster/profile.sh' >> ~/.bashrc  \
    && echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PRECICE_ROOT/build/last:$PETSC_DIR/$PETSC_ARCH/lib' >> ~/.bashrc \
    && echo 'export PATH=$PATH:~/CHT-preCICE/solvers/CalculiX/bin:~/CHT-preCICE/utilities' >> ~/.bashrc

ENV ASTER_ADAPTER_ROOT=/home/precice/CHT-preCICE/solvers/Code_Aster

USER \
    codeaster
ENV ASTER_ADAPTER_ROOT=/home/precice/CHT-preCICE/solvers/Code_Aster



USER root

RUN \
    apt-get update && apt-get install -y gdb

USER precice
WORKDIR /home/precice

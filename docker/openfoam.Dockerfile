FROM precice
MAINTAINER Lucía Cheung <lcheung@simscale.com>

# Install OpenFOAM
RUN \
    apt-get update \
    && apt-get install -y software-properties-common

RUN \
    add-apt-repository http://dl.openfoam.org/ubuntu \
    && sh -c "wget -O - http://dl.openfoam.org/gpg.key | apt-key add -" \
    && apt-get update \
    && apt-get -y install openfoam30 \
    && apt-get -y install paraviewopenfoam44

RUN \
    source /opt/openfoam30/etc/bashrc \
    && cd $WM_DIR/rules \
    && cp -r ${WM_ARCH}${WM_COMPILER} ${WM_ARCH}Mpicc \
    && cd ${WM_ARCH}Mpicc \
    && sed -i 's@CC          = g++ -m64@CC          = mpic++ -m64 -std=c++11@g' c++

RUN \
    mkdir -p /CHT-preCICE/solvers

COPY \
    /solvers/OpenFOAM/ /CHT-preCICE/solvers/OpenFOAM/

RUN \
    export PRECICE_ROOT=/precice \
    && source /opt/openfoam30/etc/bashrc \
    && export WM_COMPILER=Mpicc \
    && cd /CHT-preCICE/solvers/OpenFOAM/adapter \
    && wclean && wmake lib \
    && mv libNULL.a libOpenFoamAdapter.a

RUN \
    export PRECICE_ROOT=/precice \
    && source /opt/openfoam30/etc/bashrc \
    && export WM_COMPILER=Mpicc \
    && cd /CHT-preCICE/solvers/OpenFOAM/solvers/buoyantPimpleFoam \
    && wclean && wmake

FROM precice
MAINTAINER Lucía Cheung <lcheung@simscale.com>

RUN \
    apt-get update \
    && apt-get install -y \
    software-properties-common \
    build-essential \
    git \
    python \
    python-numpy \
    wget \
    gfortran \
    libopenmpi-dev \
    cmake

# Install OpenFOAM
# ------------------------------------------------------------------------------

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


# Install CalculiX
# ------------------------------------------------------------------------------

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


# Code_Aster
# ------------------------------------------------------------------------------

#
# Install prerequisites
#

# Choose German mirror for Ubuntu
RUN sed -e 's@//archive.ubuntu.com@//de.archive.ubuntu.com@' \
	-i /etc/apt/sources.list

RUN apt-get update \
    && apt-get install -y \
	wget \
	unzip \
	gfortran \
	g++ \
	gcc \
	python-dev \
	python-qt4 \
	python-numpy \
	tcl \
	tk \
	zlib1g-dev \
	bison \
	flex \
	checkinstall \
	openmpi-bin \
	libopenmpi-dev \
	grace \
	libx11-dev \
	cmake \
    ssh \
	gettext \
	rsync \
    && apt-get clean

#
# Build
#

# We set up a non-root user for compilation and building.
# NB: although it may seem unorthodox, the user must have a home folder. Don't change to useradd -M!
RUN \
	useradd -m -s /bin/bash codeaster

RUN \
    mkdir -p /srvtc/cloud \
    && chown -R codeaster:codeaster /srvtc

USER codeaster

WORKDIR \
    /srvtc/cloud

ENV \
    CA_VERSION=12.6.0 \
    CABT_VERSION=12.6.8 \
    CA_TAG=12.6 \
    CA_REVISION=4 \
    OpBL_VERSION=0.2.18 \
    ScTch_VERSION=5.1.11 \
    MUMPS_VERSION=4.10.0 \
    MUMPS_ARCHIVE=${MUMPS_VERSION}-aster3-2 \
    HDF5_VERSION=1.8.14 \
    MED_VERSION=3.2.0 \
    METIS_VERSION=4.0.3 \
    PETSC_VERSION=3.4.5 \
    SCLPK_VERSION=1.0.2 \
    VERSION_COMMENT=V12

RUN \
    wget -nv http://www.web-code-aster.org/FICHIERS/aster-full-src-$CA_VERSION-$CA_REVISION.noarch.tar.gz \
    && tar -xzf aster-full-src-$CA_VERSION-$CA_REVISION.noarch.tar.gz \
    && mv aster-full-src-$CA_VERSION/ aster-$CA_VERSION/ \
    && rm aster-full-src-$CA_VERSION-$CA_REVISION.noarch.tar.gz \
    && mkdir -p /srvtc/cloud/aster-$CA_VERSION/build \
    && mkdir -p /srvtc/cloud/aster-$CA_VERSION/contrib

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib

RUN \
    wget -nv https://github.com/xianyi/OpenBLAS/archive/v$OpBL_VERSION.tar.gz \
    && tar -xzf v$OpBL_VERSION.tar.gz \
    && rm v$OpBL_VERSION.tar.gz \
    && mkdir -p /srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION/build

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION

ENV OMP_NUM_THREADS=1 OPENBLAS_BUILD=/srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION/build

# NB: TARGET=CORE2 sets the build target to the specific architecture
# otherwise, OpenBLAS will detect the build architecture itself and
# set it explicitly to the architecture of the build host.
# Setting a build architecture (with rather low requirements to the
# instruction set) increases portability of Docker containers
# or makes the successful execution of Code_Aster inside the Docker container
# at all possible.
RUN \
    make -j 4 NO_AFFINITY=1 USE_OPENMP=1 TARGET=CORE2 \
    && make PREFIX=$OPENBLAS_BUILD install

USER \
    root

RUN \
    echo "/srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION" > /etc/ld.so.conf.d/ca.conf

RUN \
    echo "/srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION/lib" > /etc/ld.so.conf.d/openblas.conf

RUN \
    ldconfig

USER \
    codeaster

RUN \
	export LD_LIBRARY_PATH=/srvtc/cloud/aster-$CA_VERSION/contrib/OpenBLAS-$OpBL_VERSION/lib

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION

# If this is not set, error when compiling aster V13 due to: no program using openmp
#V13commentoutENV OMP_NUM_THREADS=2

RUN \
    sed -i "s:ASTER_ROOT='/opt/aster':ASTER_ROOT='/srvtc/cloud/aster-$CA_VERSION/build':g" setup.cfg \
    && sed -i "s:PREFER_COMPILER\ =\ 'GNU':PREFER_COMPILER\ =\'GNU_without_MATH'\nMATHLIB=\ '-L$OPENBLAS_BUILD/lib/ -lopenblas':g" setup.cfg \
    && yes | python setup.py install \
    && echo localhost cpu=`nproc` >> /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/mpi_hostfile

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib

RUN \
    wget -nv http://www.netlib.org/scalapack/scalapack_installer.tgz \
    && tar -xzf scalapack_installer.tgz \
    && rm scalapack_installer.tgz \
    && mv scalapack_installer scalapack-$SCLPK_VERSION

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib/scalapack-$SCLPK_VERSION

RUN \
    mkdir -p /srvtc/cloud/aster-$CA_VERSION/contrib/scalapack-$SCLPK_VERSION/build \
    && ./setup.py --lapacklib=$OPENBLAS_BUILD/lib/libopenblas.a --mpicc=mpicc --mpif90=mpif90 --mpiincdir=/usr/lib/openmpi/include --ldflags_c=-fopenmp --ldflags_fc=-fopenmp --prefix=/srvtc/cloud/aster-$CA_VERSION/contrib/scalapack-$SCLPK_VERSION/build

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib

# Todo: fix above
ENV \
    MUMPS_ARCHIVE=4.10.0-aster3-2

RUN \
    tar -xzf /srvtc/cloud/aster-$CA_VERSION/SRC/mumps-$MUMPS_ARCHIVE.tar.gz -C /srvtc/cloud/aster-$CA_VERSION/contrib \
    && mv mumps-$MUMPS_VERSION mumps-${MUMPS_VERSION}_mpi

ADD build-utils-tmp /srvtc/cloud/build-utils

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib/mumps-${MUMPS_VERSION}_mpi

ENV MUMPS_BUILD=/srvtc/cloud/aster-$CA_VERSION/contrib/mumps-${MUMPS_VERSION}_mpi

RUN \
    cp /srvtc/cloud/build-utils/MUMPS_Makefile.inc Makefile.inc \
    && make -j 4 all

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib

RUN \
    wget -nv http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-$PETSC_VERSION.tar.gz \
    && tar -xzf petsc-$PETSC_VERSION.tar.gz \
    && rm petsc-$PETSC_VERSION.tar.gz

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/contrib/petsc-$PETSC_VERSION

ENV PETSC_BUILD=/srvtc/cloud/aster-$CA_VERSION/contrib/petsc-$PETSC_VERSION

RUN \
    ./config/configure.py --with-mpi-dir=/usr/lib/openmpi/ --with-blas-lapack-lib=$OPENBLAS_BUILD/lib/libopenblas.a --download-hypre=yes --download-ml=yes --with-debugging=0 COPTFLAGS=-O3 FOPTFLAGS=-O3 --configModules=PETSc.Configure --optionsModule=PETSc.compilerOptions --with-x=0 --with-shared-libraries=0 \
    && make PETSC_DIR=$PETSC_BUILD PETSC_ARCH=arch-linux2-c-opt all \
    && make PETSC_DIR=$PETSC_BUILD PETSC_ARCH=arch-linux2-c-opt test

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/SRC

RUN \
    wget -nv https://www.bitbucket.org/code_aster/codeaster-src/get/$CABT_VERSION.tar.gz \
    && mkdir aster-bitbucket-$CABT_VERSION \
    && tar xf $CABT_VERSION.tar.gz -C aster-bitbucket-$CABT_VERSION --strip-components 1 \
	&& rm $CABT_VERSION.tar.gz \
	&& mv aster-bitbucket-$CABT_VERSION aster-$CA_VERSION \
	&& cp /srvtc/cloud/aster-$CA_VERSION/build/$CA_TAG/lib/aster/Accas/pkginfo.py aster-$CA_VERSION/bibpyt/Accas/

# blacs is only required for mumps, not code_aster itself, so it can be ecluded, otherwise error
#V13commentoutRUN\
#V13commentout    sed -i "s/call blacs_pinfo (iam, nprocs)/!call blacs_pinfo (iam, nprocs)/g" /srvtc/cloud/aster-13.2.0/SRC/aster-13.2.0/waftools/mathematics.py \
#V13commentout    && sed -i "s/print \*,iam/!print *,iam/g" /srvtc/cloud/aster-13.2.0/SRC/aster-13.2.0/waftools/mathematics.py \
#V13commentout    && sed -i "s/print \*,nprocs/!print *,nprocs/g" /srvtc/cloud/aster-13.2.0/SRC/aster-13.2.0/waftools/mathematics.py

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/SRC/aster-$CA_VERSION/

RUN \
    ./waf configure --use-config-dir=/srvtc/cloud/build-utils --use-config=Ubuntu_gnu_mpi --prefix=/srvtc/cloud/aster-$CA_VERSION/build/PAR$CA_TAG --install-tests \
    && ./waf install -p

WORKDIR \
    /srvtc/cloud/aster-$CA_VERSION/build

RUN \
    sed -i "s/PMI_RANK/OMPI_COMM_WORLD_RANK/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/asrun \
    && echo vers : PAR$CA_TAG:/srvtc/cloud/aster-$CA_VERSION/build/PAR$CA_TAG/share/aster >> /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/aster

RUN \
    sed -i "s/#batch_memmax : 12000/batch_memmax : 64000/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/asrun \
    && sed -i "s/#interactif_memmax : 2048/interactif_memmax : 64000/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/asrun \
    && sed -i "s/#batch_nbpmax : 16/batch_nbpmax : 32/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/asrun \
    && sed -i "s/#interactif_nbpmax : 16/interactif_nbpmax : 32/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/asrun

RUN \
    sed -i "s/export ASTER_TEMPDIR:\/tmp/export ASTER_TEMPDIR:\/srvtc\/tmp/g" /srvtc/cloud/aster-$CA_VERSION/build/etc/codeaster/profile.sh

WORKDIR \
	/srvtc/cloud/aster-$CA_VERSION/contrib

RUN \
    mv /srvtc/cloud/aster-$CA_VERSION/build/share/codeaster/asrun/data/mpirun_template /srvtc/cloud/aster-$CA_VERSION/build/share/codeaster/asrun/data/mpirun_template_backup \
    && cp /srvtc/cloud/build-utils/mpirun_template /srvtc/cloud/aster-$CA_VERSION/build/share/codeaster/asrun/data/.

RUN \
	wget -nv https://crowdin.com/download/project/code-aster.zip \
	&& unzip code-aster.zip -d code-aster-locales \
	&& rm code-aster.zip \
	&& cd code-aster-locales/code_aster/en-US \
	&& msgfmt aster-messages.po \
	&& mv messages.mo /srvtc/cloud/aster-$CA_VERSION/build/PAR$CA_TAG/share/locale/aster/en-US/LC_MESSAGES/aster_messages.mo

## Clean up for reduced container size

#cleanUPWORKDIR \
#cleanUP    /srvtc/cloud/aster-$CA_VERSION

#cleanUPRUN \
#cleanUP    rm -r SRC/ build/$CA_TAG/share/aster/tests/ build/PAR$CA_TAG/share/aster/tests/


# Adapters
# ------------------------------------------------------------------------------

USER \
    root

RUN \
	useradd -m -s /bin/bash precice

RUN \
    mkdir -p /home/precice/CHT-preCICE/solvers

COPY \
    /solvers/ /home/precice/CHT-preCICE/solvers/

RUN \
    chown -R precice:precice /home/precice/CHT-preCICE

USER \
    precice

# OpenFOAM

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
    precice

RUN \
    cd ~/CHT-preCICE/solvers/CalculiX \
    && sed -i 's/_org//g' Makefile \
    && make clean && make

USER \
    root

RUN \
    mv /precice/src/precice/adapters/c/SolverInterfaceC.h.bk /precice/src/precice/adapters/c/SolverInterfaceC.h

# Python interface for preCICE: TODO: move to preCICE Dockerfile
RUN \
    cd $PRECICE_ROOT \
    && wget https://github.com/cython/cython/archive/0.23.4.tar.gz \
    && tar xvzf 0.23.4.tar.gz \
    && cd cython-0.23.4 \
    && python setup.py install

RUN \
    cd $PRECICE_ROOT \
    &&cd src/precice/adapters/python \
    && python setup.py build_ext --inplace

RUN \
    apt-get install -y python-pip \
    && pip install mpi4py

# Environment variables
USER \
    precice

ENV ASTER_ROOT=/srvtc/cloud/aster-12.6.0/build/

RUN \
    echo 'source /opt/openfoam30/etc/bashrc' >> ~/.bashrc \
    echo 'source $ASTER_ROOT/12.6/share/aster/profile.sh' >> ~/.bashrc \
    && echo 'source $ASTER_ROOT/etc/codeaster/profile.sh' >> ~/.bashrc  \
    && echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PRECICE_ROOT/build/last' >> ~/.bashrc

ENV ASTER_ADAPTER_ROOT=/home/precice/CHT-preCICE/solvers/Code_Aster

USER \
    codeaster
ENV ASTER_ADAPTER_ROOT=/home/precice/CHT-preCICE/solvers/Code_Aster

USER \
    precice

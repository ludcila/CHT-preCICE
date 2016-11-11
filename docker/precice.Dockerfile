FROM ubuntu:trusty
MAINTAINER LucíaCheung <lcheung@simscale.com>

RUN rm /bin/sh && ln -s /bin/bash /bin/sh

RUN apt-get update && apt-get install -y build-essential
RUN apt-get update && apt-get install -y git

RUN git clone https://github.com/precice/precice.git

# Install python
RUN apt-get install -y python python-numpy

# Install scons
RUN apt-get install -y scons

RUN apt-get install -y wget

# Install Eigen
RUN wget http://bitbucket.org/eigen/eigen/get/3.2.10.tar.gz \
    && tar -xvzf 3.2.10.tar.gz \
    && mv eigen-eigen-b9cd8366d4e8/Eigen precice/src/Eigen

# Install boost
RUN wget http://downloads.sourceforge.net/project/boost/boost/1.62.0/boost_1_62_0.tar.gz
RUN tar -xvzf boost_1_62_0.tar.gz

RUN apt-get install -y libboost-all-dev
RUN cd boost_1_62_0 && ./bootstrap.sh && ./b2 install --prefix=/usr

RUN \
    apt-get install libblas-dev liblapack-dev

RUN \
    git clone -b maint https://bitbucket.org/petsc/petsc petsc \
    && cd petsc \
    && ./configure --with-debugging=0 \
    && make

ENV PETSC_DIR=/petsc
ENV PETSC_ARCH=arch-linux2-c-opt

RUN cd precice && scons -j4 petsc=yes compiler=mpic++ build=release

RUN cd precice \
    && sed -i '/env.Replace(CC = env\["compiler"\])/aenv.Append(LIBS = ['\''-lm'\'', '\''-lstdc++'\'', '\''-lmpi_cxx'\''])' SConstruct \
    && scons -j4 petsc=yes compiler=mpicc build=release builddir=build/release-c

RUN cd precice \
    && scons -j4 petsc=yes compiler=mpicc build=debug builddir=build/debug-c


RUN \
    apt-get update && apt-get install -y cmake
RUN \
    git clone https://github.com/jbeder/yaml-cpp \
    && mv yaml-cpp /usr/local/yaml-cpp \
    && cd /usr/local/yaml-cpp \
    && mkdir build && cd build \
    && cmake -G "Unix Makefiles" .. \
    && make

ENV PRECICE_ROOT=/precice


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

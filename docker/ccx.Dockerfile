FROM ubuntu:trusty
MAINTAINER LucíaCheung <lcheung@simscale.com>

RUN rm /bin/sh && ln -s /bin/bash /bin/sh

RUN apt-get update \
    && apt-get install -y build-essential \
    git \
    python \
    python-numpy \
    wget \
    gfortran \
    libopenmpi-dev

RUN \
    wget http://www.dhondt.de/ccx_2.10.src.tar.bz2 \
    && bzip2 -d ccx_2.10.src.tar.bz2 \
    && tar xvf ccx_2.10.src.tar

RUN \
    wget http://www.dhondt.de/cgx_2.10.all.tar.bz2 \
    && bzip2 -d cgx_2.10.all.tar.bz2 \
    && tar xvf cgx_2.10.all.tar

RUN \
    mv CalculiX /usr/local/CalculiX


RUN \
    wget http://www.netlib.org/linalg/spooles/spooles.2.2.tgz \
    && mkdir /usr/local/SPOOLES.2.2 \
    && mv spooles.2.2.tgz /usr/local/SPOOLES.2.2 \
    && cd /usr/local/SPOOLES.2.2 \
    && tar xvzf spooles.2.2.tgz \
    && sed -i 's/drawTree.c/draw.c/g' Tree/src/makeGlobalLib \
    && make CC=gcc lib \
    && make CC=gcc global

RUN \
    cd /usr/local/SPOOLES.2.2/MT && make CC=gcc lib \
    && cd /usr/local/SPOOLES.2.2/MPI && make CC=mpicc lib

RUN \
    apt-get install -y fort77

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

RUN \
    cd /usr/local/CalculiX/ccx_2.10/src \
    && make

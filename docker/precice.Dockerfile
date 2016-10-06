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

RUN cd precice && scons petsc=no compiler=mpic++ build=release

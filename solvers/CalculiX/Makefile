
CFLAGS = -g -Wall -O0 -fopenmp -I ../../../SPOOLES.2.2 -DARCH="Linux" -DSPOOLES -DARPACK -DMATRIXSTORAGE
FFLAGS = -g -Wall -O0 -fopenmp

CC=cc
FC=gfortran

.c.o :
	$(CC) $(CFLAGS) -c $<
.f.o :
	$(FC) $(FFLAGS) -c $<

include Makefile.inc

SCCXMAIN = ccx_2.10.c

OCCXF = $(SCCXF:.f=.o)
OCCXC = $(SCCXC:.c=.o)
OCCXMAIN = $(SCCXMAIN:.c=.o)

DIR=../../../SPOOLES.2.2

LIBS = \
       $(DIR)/spooles.a \
	../../../ARPACK/libarpack_INTEL.a \
       -lpthread -lm -lc

ccx_2.10: $(OCCXMAIN) ccx_2.10.a  $(LIBS)
	./date.pl; $(CC) $(CFLAGS) -c ccx_2.10.c; $(FC) -fopenmp -Wall -O3 -o $@ $(OCCXMAIN) ccx_2.10.a $(LIBS)

ccx_2.10.a: $(OCCXF) $(OCCXC)
	ar vr $@ $?
                                                                               

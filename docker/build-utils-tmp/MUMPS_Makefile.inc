#
#  This file is part of MUMPS 4.10.0, built on Tue May 10 12:56:32 UTC 2011
# modified for Code_Aster setup (see fields like ?CC?)
#
#Begin orderings
ASTERDIR=/srvtc/cloud/aster-${CA_VERSION}
# NOTE that PORD is distributed within MUMPS by default. If you would like to
# use other orderings, you need to obtain the corresponding package and modify
# the variables below accordingly.
# For example, to have Metis available within MUMPS:
#          1/ download Metis and compile it
#          2/ uncomment (suppress # in first column) lines
#             starting with LMETISDIR,  LMETIS
#          3/ add -Dmetis in line ORDERINGSF
#             ORDERINGSF  = -Dpord -Dmetis
#          4/ Compile and install MUMPS
#             make clean; make   (to clean up previous installation)
#
#          Metis/ParMetis and SCOTCH/PT-SCOTCH (ver 5.1 and later) orderings are now available for MUMPS.
#

#SCOTCHDIR  = ?HOME_SCOTCH?
SCOTCHDIR  = $(ASTERDIR)/build/public/scotch-${ScTch_VERSION}
ISCOTCH    = -I$(SCOTCHDIR)/include

# You have to choose one among the following two lines depending on
# the type of analysis you want to perform. If you want to perform only
# sequential analysis choose the first (remember to add -Dscotch in the ORDERINGSF
# variable below); for both parallel and sequential analysis choose the second
# line (remember to add -Dptscotch in the ORDERINGSF variable below)

LSCOTCH    = -L$(SCOTCHDIR)/lib -lesmumps -lscotch -lscotcherr
#LSCOTCH    = -L$(SCOTCHDIR)/lib -lptesmumps -lptscotch -lptscotcherr


LPORDDIR = $(topdir)/PORD/lib/
IPORD    = -I$(topdir)/PORD/include/
LPORD    = -L$(LPORDDIR) -lpord

#LMETISDIR = Directory containing Metis library
#LMETISDIR = ?HOME_METIS?/lib
LMETISDIR = $(ASTERDIR)/build/public/metis-${METIS_VERSION}/lib

#IMETIS    = # Metis doesn't need include files (Fortran interface avail.)

# You have to choose one among the following two lines depending on
# the type of analysis you want to perform. If you want to perform only
# sequential analysis choose the first (remember to add -Dmetis in the ORDERINGSF
# variable below); for both parallel and sequential analysis choose the second
# line (remember to add -Dparmetis in the ORDERINGSF variable below)

LMETIS    = -L$(LMETISDIR) -lmetis
#LMETIS    = -L$(LMETISDIR) -lparmetis -lmetis

# The following variables will be used in the compilation process.
# Please note that -Dptscotch and -Dparmetis imply -Dscotch and -Dmetis respectively.
#ORDERINGSF = -Dscotch -Dmetis -Dpord -Dptscotch -Dparmetis
#ORDERINGSF = -Dscotch -Dmetis -Dpord -Dparmetis
#ORDERINGSF  = -Dpord -Dmetis
#ORDERINGSF  = -Dpord -Dparmetis -Dmetis
ORDERINGSF  = -Dscotch -Dpord -Dmetis
ORDERINGSC  = $(ORDERINGSF)

LORDERINGS = $(LMETIS) $(LPORD) $(LSCOTCH)
IORDERINGSF = $(ISCOTCH)
IORDERINGSC = $(IMETIS) $(IPORD) $(ISCOTCH)

#End orderings
########################################################################
################################################################################

PLAT    =
LIBEXT  = .a
OUTC    = -o
OUTF    = -o
RM      = /bin/rm -f
CC      = mpicc
FC      = mpif90
FL      = mpif90
# WARNING: AR must ends with a blank space!
AR      = ar vr 
RANLIB = ranlib
#RANLIB  = echo
# See point 17 in the FAQ to have more details on the compilation of mpich with gfortran
SCALAP  = $(ASTERDIR)/contrib/scalapack-${SCLPK_VERSION}/build/lib/libscalapack.a \
        $(ASTERDIR)/contrib/OpenBLAS-${OpBL_VERSION}/build/lib/libopenblas.a

INCPAR = -I/usr/include/openmpi $(IPORD)
LIBPAR = $(SCALAP)  -L/usr/lib -lmpi -lmpi_f77

#LIBPAR = $(SCALAP)  -L/usr/local/lib/ -llamf77mpi -lmpi -llam
#LIBPAR = $(SCALAP) -lutil -ldl -lpthread
#LIBPAR = -lmpi++ -lmpi -ltstdio -ltrillium -largs -lt
INCSEQ = -I$(topdir)/libseq
LIBSEQ  =  -L$(topdir)/libseq -lmpiseq
#LIBBLAS = -L/usr/lib/xmm/ -lf77blas -latlas
LIBBLAS = $(ASTERDIR)/contrib/OpenBLAS-${OpBL_VERSION}/build/lib/libopenblas.a
LIBOTHERS = -lpthread
#Preprocessor defs for calling Fortran from C (-DAdd_ or -DAdd__ or -DUPPER)
CDEFS   = -DAdd_

#Begin Optimized options
#OPTF    = -O -Dintel_ -DALLOW_NON_INIT ?F90FLAGS?
#OPTL    = -O ?LDFLAGS?
#OPTC    = -O ?CFLAGS?
OPTF    = -O -Dintel_ -DALLOW_NON_INIT -O2 -ffixed-line-length-0 -x f77-cpp-input -fPIC  -fopenmp
OPTL    = -O -fopenmp
OPTC    = -O -O2 -fno-stack-protector -fPIC -fopenmp
#End Optimized options
#INCS = $(INCSEQ)
#LIBS = $(LIBSEQ)
#LIBSEQNEEDED = libseqneeded
INCS = $(INCPAR)
LIBS = $(LIBPAR)
LIBSEQNEEDED =

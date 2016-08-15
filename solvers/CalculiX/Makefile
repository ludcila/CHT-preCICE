# Specify the locations of: the original CCX source, SPOOLES and ARPACK
CCX 		= /usr/local/CalculiX/ccx_2.10_org/src
SPOOLES 	= /usr/local/SPOOLES.2.2
ARPACK 		= /usr/local/ARPACK

# Specify where to store the generated .o files
OBJDIR 		= bin

# Includes and libs
INCLUDES = \
	-I./ \
	-I./adapter \
	-I$(CCX) \
	-I$(SPOOLES) \
	-I$(PRECICE_ROOT)/src

LIBS = \
		$(SPOOLES)/spooles.a \
		$(ARPACK)/libarpack_INTEL.a \
       -lpthread -lm -lc \
       -L$(PRECICE_ROOT)/build/release-c/release-nopetsc \
       -lprecice \
       -lboost_system \
       -lboost_filesystem \
       -lpython2.7 \
       -lstdc++ \
       -lmpi_cxx \
       -lm \
       -lmpi \
       -lyaml \
       -L/usr/local/yaml-cpp/build \
       -lyaml-cpp       
       

# Compilers and flags
CFLAGS = -g -Wall -std=c++11 -O0 -fopenmp $(INCLUDES) -DARCH="Linux" -DSPOOLES -DARPACK -DMATRIXSTORAGE
FFLAGS = -g -Wall -O0 -fopenmp $(INCLUDES)
CC = mpicc
FC = gfortran
	
# Include a list of all the source files
include $(CCX)/Makefile.inc
SCCXMAIN = ccx_2.10.c

# Append additional sources
SCCXC += Helpers.c CCXHelpers.c PreciceInterface.c
SCCXF += getflux.f getkdeltatemp.f



# Source files in this folder and in the adapter directory
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o : %.f
	$(FC) $(FFLAGS) -c $< -o $@
$(OBJDIR)/%.o : adapter/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o : adapter/%.cpp
	g++ -std=c++11 -I/usr/local/yaml-cpp/include -c $< -o $@ $(LIBS)
	
# Source files in the $(CCX) folder
$(OBJDIR)/%.o : $(CCX)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o : $(CCX)/%.f
	$(FC) $(FFLAGS) -c $< -o $@

# Generate list of object files from the source files, prepend $(OBJDIR)
OCCXF = $(SCCXF:%.f=$(OBJDIR)/%.o)
OCCXC = $(SCCXC:%.c=$(OBJDIR)/%.o)
OCCXMAIN = $(SCCXMAIN:%.c=$(OBJDIR)/%.o)
OCCXC += $(OBJDIR)/ConfigReader.o



$(OBJDIR)/ccx_preCICE: $(OBJDIR) $(OCCXMAIN) $(OBJDIR)/ccx_2.10.a
	$(FC) -fopenmp -Wall -O3 -o $@ $(OCCXMAIN) $(OBJDIR)/ccx_2.10.a $(LIBS)

$(OBJDIR)/ccx_2.10.a: $(OCCXF) $(OCCXC)
	ar vr $@ $?
	
$(OBJDIR):
	mkdir -p $(OBJDIR)
                                                                               
clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/ccx_2.10.a $(OBJDIR)/ccx_preCICE

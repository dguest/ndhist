# makefile for JETNET ntuple preprocessor 
# Author: Dan Guest (dguest@cern.ch)
# Created: Sat Jan 28 13:09:53 CET 2012

# --- set dirs
BIN          := bin
SRC          := src
INC          := include
PYTHON       := python
LIB          := lib

#  set search path
vpath %.o    $(BIN)
vpath %.cxx  $(SRC) 
vpath %.hh   $(INC) 

# --- set the version of python-config
PY_CONFIG := python2.7-config

# --- load in root config
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTLDFLAGS   := $(shell root-config --ldflags)

PY_FLAGS :=   $(shell $(PY_CONFIG) --includes)
PY_LIB   := -L$(shell $(PY_CONFIG) --prefix)/lib
PY_LIB   +=   $(shell $(PY_CONFIG) --libs)

# --- set COMMON_LIBS if you want to include another library 
COMMON_LIBS  :=

# --- set compiler and flags (roll c options and include paths together)
CXX          := g++
CXXFLAGS     := -O2 -Wall -fPIC -I$(INC) -g
LDFLAGS      := -Wl,-no-undefined 
LIBS         := -L$(COMMON_LIBS) -Wl,-rpath,$(COMMON_LIBS) 

COPT += -D HDF5
LIBS += -lhdf5_cpp -lhdf5 

# rootstuff 
CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS         += $(ROOTLIBS)

# pystuff (roll the linking options and libraries together)
PY_LDFLAGS := $(LDFLAGS)
PY_LDFLAGS += $(PY_LIB)
PY_LDFLAGS += -shared

# ---- define objects
# - not-python 
GEN_OBJ     := Histogram.o Binners.o HdfFromHist.o

# - python interface
PY_OBJ       := 

# - command line interface
EXE_OBJ      := test.o

ALLOBJ       := $(GEN_OBJ) $(PY_OBJ) $(EXE_OBJ)
ALLOUTPUT    := test $(LIB)/libndhist.so

all: $(ALLOUTPUT) 

test: $(GEN_OBJ:%=$(BIN)/%) $(EXE_OBJ:%=$(BIN)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) 

$(LIB)/libndhist.so: $(GEN_OBJ:%=$(BIN)/%)
	@mkdir -p $(LIB)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS) -shared

$(PYTHON)/_ndhist.so: $(GEN_OBJ:%=$(BIN)/%) $(PY_OBJ:%=$(BIN)/%)
	@mkdir -p $(PYTHON)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(PY_LDFLAGS)

# --------------------------------------------------

# python object compile
$(BIN)/_%.o: _%.cxx 
	@echo compiling python object $@
	@mkdir -p $(BIN)
	@$(CXX) -c $(CXXFLAGS) $(PY_FLAGS) $< -o $@ 

# compile rule
$(BIN)/%.o: %.cxx
	@echo compiling $<
	@mkdir -p $(BIN)
	@$(CXX) -c $(CXXFLAGS) $< -o $@

# use auto dependency generation
DEP = $(BIN)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),rmdep)
include  $(ALLOBJ:%.o=$(DEP)/%.d)
endif
endif

DEPTARGSTR = -MT $(BIN)/$*.o -MT $(DEP)/$*.d
$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -MP $(DEPTARGSTR) $(CXXFLAGS) $< -o $@ 

# clean
.PHONY : clean rmdep
CLEANLIST     = *~ *.o *.o~ *.d core 
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BIN)/%) $(CLEANLIST:%=$(DEP)/%)
	rm -fr $(BIN) $(ALLOUTPUT)

rmdep: 
	rm -f $(DEP)/*.d
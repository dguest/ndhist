# makefile for ndhist histogram library
# Author: Dan Guest (dguest@cern.ch)
# Created: Sat Jan 28 13:09:53 CET 2012

# --- set dirs
BUILD        := build
BIN          := bin
SRC          := src
INC          := include/ndhist
PYTHON       := python
LIBDIR       := $(CURDIR)/lib
LIBNAME      := libndhist.so

#  set search path
vpath %.o    $(BUILD)
vpath %.cxx  $(SRC)
vpath %.hh   $(INC)

# --- set compiler and flags (roll c options and include paths together)
CXX          ?= g++
CXXFLAGS     := -O2 -Wall -fPIC -I$(INC) -g -std=c++11
# LDFLAGS      := -Wl,--no-undefined

# fix for ubuntu (that doesn't use bash for /bin/sh)
SHELL         := bash

# --- external dirs
# (sometimes hdf is in a werid place, this will work as long as h5cc works)
HDF_PATH      := $(dir $(shell type -p h5cc | xargs dirname))
ifndef HDF_PATH
$(error "couldn't find HDF5 `h5cc` command, HDF5 probably not installed...")
endif
COMMON_LIBS   := $(HDF_PATH)/lib
LIBS          := -L$(COMMON_LIBS) -Wl,-rpath,$(COMMON_LIBS)
CXXFLAGS      += -I$(HDF_PATH)/include

LIBS += -lhdf5_cpp -lhdf5

# ---- define objects
# - not-python
GEN_OBJ     := Histogram.o Binners.o

# - command line interface
EXE_OBJ      := test.o

ALLOBJ       := $(GEN_OBJ) $(PY_OBJ) $(EXE_OBJ)
ALLOUTPUT    := test $(LIBDIR)/$(LIBNAME)

all: $(ALLOUTPUT)

test: $(GEN_OBJ:%=$(BUILD)/%) $(EXE_OBJ:%=$(BUILD)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS)

$(LIBDIR)/$(LIBNAME): $(GEN_OBJ:%=$(BUILD)/%)
	@mkdir -p $(LIBDIR)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS) -shared

# --------------------------------------------------

# compile rule
$(BUILD)/%.o: %.cxx
	@echo compiling $<
	@mkdir -p $(BUILD)
	@$(CXX) -c $(CXXFLAGS) $< -o $@

# use auto dependency generation
DEP = $(BUILD)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),rmdep)
include  $(ALLOBJ:%.o=$(DEP)/%.d)
endif
endif

DEPTARGSTR = -MT $(BUILD)/$*.o -MT $(DEP)/$*.d
$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -MP $(DEPTARGSTR) $(CXXFLAGS) $< -o $@

# clean
.PHONY : clean rmdep
CLEANLIST     = *~ *.o *.o~ *.d core
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BUILD)/%) $(CLEANLIST:%=$(DEP)/%)
	rm -fr $(BUILD) $(ALLOUTPUT)

rmdep:
	rm -f $(DEP)/*.d

# ----------------------------------------------------
# install

PREFIX?=/usr/local
.PHONY: install remove
install: all
	cp $(LIBDIR)/$(LIBNAME) $(PREFIX)/lib
	mkdir -p $(PREFIX)/include/ndhist
	cp $(INC)/* $(PREFIX)/include/ndhist
	cp $(BIN)/ndhist-config $(PREFIX)/bin

remove:
	rm $(PREFIX)/lib/$(LIBNAME)
	rm -r $(PREFIX)/include/ndhist
	rm $(PREFIX)/bin/ndhist-config

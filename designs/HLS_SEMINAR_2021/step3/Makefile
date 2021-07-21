
CXXFLAGS += -O3 -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-label

# =====================================================================
# ENVIRONMENT VARIABLES
#
# The following environment variables will specify paths
# to open-source repositories that are also included in
# a Catapult install tree.
# If you are using Catapult (i.e. if CATAPULT_HOME or MGC_HOME is set)
# then you do not need to define these environment variables.
# If, however, you wish to point to your own github clone
# of any of these repositories, then define the appropriate
# environment variable.

# If CATAPULT_HOME not set, use value of MGC_HOME for backward compatibility.
CATAPULT_HOME ?= $(MGC_HOME)

ifneq "$(CATAPULT_HOME)" ""

# Pick up SystemC via "SYSTEMC_HOME"
ifneq "$(SYSTEMC_HOME)" ""
$(warning - Warning: SYSTEMC_HOME and MGC_HOME/CATAPULT_HOME are both set. Using SystemC from MGC_HOME/CATAPULT_HOME)
endif
SYSTEMC_HOME := $(CATAPULT_HOME)/shared

# Pick up Connections via "CONNECTIONS_HOME"
CONNECTIONS_HOME ?= $(CATAPULT_HOME)/shared

# Pick up MatchLib via "MATCHLIB_HOME"
MATCHLIB_HOME ?= $(CATAPULT_HOME)/shared/pkgs/matchlib

# Pick up AC Datatypes via "AC_TYPES"
AC_TYPES ?= $(CATAPULT_HOME)/shared

# Pick up AC Simutils via "AC_SIMUTILS"
AC_SIMUTILS ?= $(CATAPULT_HOME)/shared

# Pick up C++ compiler
CXX := $(CATAPULT_HOME)/bin/g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(CATAPULT_HOME)/lib:$(CATAPULT_HOME)/shared/lib,$(CATAPULT_HOME)/lib:$(CATAPULT_HOME)/shared/lib)
export LD_LIBRARY_PATH
LIBDIRS += -L$(CATAPULT_HOME)/lib -L$(CATAPULT_HOME)/shared/lib

else

# CATAPULT_HOME appears to not be set. Make sure required variables are defined

ifndef SYSTEMC_HOME
$(error - Environment variable SYSTEMC_HOME must be defined)
endif
ifndef CONNECTIONS_HOME
$(error - Environment variable CONNECTIONS_HOME must be defined)
endif
ifndef MATCHLIB_HOME
$(error - Environment variable MATCHLIB_HOME must be defined)
endif
ifndef AC_TYPES
$(error - Environment variable AC_TYPES must be defined)
endif
ifndef AC_SIMUTILS
$(error - Environment variable AC_SIMUTILS must be defined)
endif

# Default to the compiler installed on the machine
CXX ?= g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64,$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64)
export LD_LIBRARY_PATH
LIBDIRS += -L$(SYSTEMC_HOME)/lib -L$(SYSTEMC_HOME)/lib-linux64

endif

# ---------------------------------------------------------------------

# Check: $(SYSTEMC_HOME)/include/systemc.h must exist
checkvar_SYSTEMC_HOME: $(SYSTEMC_HOME)/include/systemc.h

# Check: $(CONNECTIONS_HOME)/include/connections/connections.h must exist
checkvar_CONNECTIONS_HOME: $(CONNECTIONS_HOME)/include/connections/connections.h

# Check: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h
checkvar_MATCHLIB_HOME: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h

# Check: $(AC_TYPES)/include/ac_int.h
checkvar_AC_TYPES: $(AC_TYPES)/include/ac_int.h

# Check: $(AC_SIMUTILS)/include/mc_scverify.h
checkvar_AC_SIMUTILS: $(AC_SIMUTILS)/include/mc_scverify.h

# Rule to check that environment variables are set correctly
checkvars: checkvar_SYSTEMC_HOME checkvar_CONNECTIONS_HOME checkvar_MATCHLIB_HOME checkvar_AC_TYPES checkvar_AC_SIMUTILS
# =====================================================================

# Export variables to any sub-invoked Make
export CATAPULT_HOME
export SYSTEMC_HOME
export CONNECTIONS_HOME
export MATCHLIB_HOME
export AC_TYPES
export AC_SIMUTILS
export LD_LIBRARY_PATH

# Determine the directory containing the source files from the path to this Makefile (var has trailing slash)
PWD := $(shell pwd)
SOURCE_DIR1 = $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
SOURCE_DIR = $(if $(subst ./,,$(SOURCE_DIR1)),$(SOURCE_DIR1),$(PWD)/)

INCDIRS := -I$(SOURCE_DIR)
INCDIRS += -I$(SYSTEMC_HOME)/include -I$(SYSTEMC_HOME)/src
INCDIRS += -I$(CONNECTIONS_HOME)/include
INCDIRS += -I$(MATCHLIB_HOME)/cmod/include
INCDIRS += -I$(AC_TYPES)/include
INCDIRS += -I$(AC_SIMUTILS)/include
INCDIRS += -I$(SOURCE_DIR)../include

CPPFLAGS += $(INCDIRS)
CPPFLAGS_FAST = $(INCDIRS)
CPPFLAGS_FAST += -DCONNECTIONS_FAST_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES -O3
CPPFLAGS += -DCONNECTIONS_ACCURATE_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES

LIBS += -lsystemc -lpthread

.PHONY: all build run fast clean sim_clean

fast: run_fast

build: sim_sc

all: run

run: trace.vcd
run_fast: trace_fast.vcd

trace.vcd: sim_sc
	-@echo "Starting execution in directory `pwd`"
	@cp -f $(SOURCE_DIR)../file_io/bias.txt .
	@cp -f $(SOURCE_DIR)../file_io/data.txt .
	@cp -f $(SOURCE_DIR)../file_io/output.txt .
	@cp -f $(SOURCE_DIR)../file_io/kernel.txt.gz .
	@gzip -f -d kernel.txt.gz
	./$^ .

trace_fast.vcd: sim_sc_fast
	-@echo "Starting execution in directory `pwd`"
	@cp -f $(SOURCE_DIR)../file_io/bias.txt .
	@cp -f $(SOURCE_DIR)../file_io/data.txt .
	@cp -f $(SOURCE_DIR)../file_io/output.txt .
	@cp -f $(SOURCE_DIR)../file_io/kernel.txt.gz .
	@gzip -f -d kernel.txt.gz
	./$^ .

sim_sc: checkvars $(wildcard $(SOURCE_DIR)*.h) $(wildcard $(SOURCE_DIR)*.cpp)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIBDIRS) $(wildcard $(SOURCE_DIR)*.cpp) -o $@ $(LIBS)

sim_sc_fast: checkvars $(wildcard $(SOURCE_DIR)*.h) $(wildcard $(SOURCE_DIR)*.cpp)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS_FAST) $(LIBDIRS) $(wildcard $(SOURCE_DIR)*.cpp) -o $@ $(LIBS)
# These two targets assume that the QuestaSim utilities 'vcd2wlf' and 'vsim'
# are found in your PATH
%.wlf: %.vcd
	vcd2wlf $< $@

view_wave: trace.wlf
	vsim $< -nolog -do "add wave -r trace:/SystemC/*" -do "wave zoom full"

sim_clean:
	@rm -rf sim_*

help:
	-@echo "Makefile targets:"
	-@echo "  clean     - Clean up from previous make runs"
	-@echo "  fast      - Perform simulation using connections FAST mode"
	-@echo "  all       - Perform all of the targets below"
	-@echo "  sim_sc    - Compile SystemC design"
	-@echo "  run       - Execute SystemC design and generate trace.vcd"
	-@echo "  view_wave - Convert trace.vcd to QuestaSim wlf file and view in QuestaSim"
	-@echo ""
	-@echo "Environment/Makefile Variables:"
	-@echo "  CATAPULT_HOME      = $(CATAPULT_HOME)"
	-@echo "  SYSTEMC_HOME       = $(SYSTEMC_HOME)"
	-@echo "  CONNECTIONS_HOME   = $(CONNECTIONS_HOME)"
	-@echo "  MATCHLIB_HOME      = $(MATCHLIB_HOME)"
	-@echo "  AC_TYPES           = $(AC_TYPES)"
	-@echo "  AC_SIMUTILS        = $(AC_SIMUTILS)"
	-@echo "  CXX                = $(CXX)"
	-@echo "  LIBDIRS            = $(LIBDIRS)"
	-@echo "  LD_LIBRARY_PATH    = $(LD_LIBRARY_PATH)"
	-@echo "  SOURCE_DIR         = $(SOURCE_DIR)"
	-@echo "  INCDIRS            = $(INCDIRS)"
	-@echo ""

clean: sim_clean
	@rm -rf *.vcd *.wlf design_check* my_log* catapult.log .vscode bias.txt kernel.txt kernel.txt.gz output.txt data.txt


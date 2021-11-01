
# Determine the directory containing the source files from the path to this Makefile (var has trailing slash)
PWD := $(shell pwd)
SOURCE_DIR1 = $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
SOURCE_DIR = $(if $(subst ./,,$(SOURCE_DIR1)),$(SOURCE_DIR1),$(PWD)/)
export SW_MEM_FILES = $(SOURCE_DIR)

#====== Tool versions =====================================

QUESTA_VERSION          = 2020.3

#====== Tool directories, update for local installation ===

export QUESTA_HOME      = /u/release/$(QUESTA_VERSION)/modeltech

export FILE_IO_DIR      = .
export FILE_IO_SRC      = $(SOURCE_DIR)../../file_io

include $(SOURCE_DIR)env_vars.mk

#====== Tool definitions ===================================

CXX            = $(GNU_HOME)/bin/g++
LINK           = $(GNU_HOME)/bin/g++

VSIM           = $(QUESTA_HOME)/bin/vsim
VCD2WLF        = $(QUESTA_HOME)/bin/vcd2wlf

#====== gcc

CXX_INCLUDES += -I $(hw_sources)
CXX_INCLUDES += -I $(SYSTEMC_HOME)/include -I $(SYSTEMC_HOME)/src
CXX_INCLUDES += -I $(CONNECTIONS_HOME)/include
CXX_INCLUDES += -I $(MATCHLIB_HOME)/cmod/include
CXX_INCLUDES  += -I $(seminar_design)
CXX_INCLUDES  += -I $(seminar_includes)
CXX_INCLUDES += -I $(AC_TYPES)/include
CXX_INCLUDES += -I $(AC_SIMUTILS)/include

CXX_DEFINES    = -D SC_INCLUDE_DYNAMIC_PROCESSES
CXX_DEFINES   += -D CONNECTIONS_ACCURATE_SIM
CXX_DEFINES   += -D CLOCK_PERIOD=10
CXX_DEFINES   += -D INITIALIZE_RAM
CXX_DEFINES   += -D SYSTEMC
CXX_DEFINES   += -D MANUAL_RAND_STALL

CXX_FLAGS       = $(CXX_INCLUDES) $(CXX_DEFINES)
CXX_FLAGS      += -Wall 
CXX_FLAGS      += -Wno-unknown-pragmas 
CXX_FLAGS      += -Wno-unused-label 
CXX_FLAGS      += -Wno-maybe-uninitialized
CXX_FLAGS      += -c
CXX_FLAGS      += -m64
CXX_FLAGS      += -std=c++11

CXX_LIBDIRS   = -L $(SYSTEMC_HOME)/lib-linux64
CXX_LIBS      = -lsystemc
 
ifdef DEBUG
CXX_FLAGS      += -O0 -g
else
CXX_FLAGS      += -O3 
endif

CXXFLAGS_FAST += $(CXX_FLAGS) -D CONNECTIONS_FAST_SIM

CXX_LINK_FLAGS = $(CXX_LIBDIRS) $(CXX_LIBS)

#======Source/target defines ==================================

target        = ./sim_sc
fast_target   = ./sim_sc_fast
hw_sources    = $(SOURCE_DIR)../hardware_sources
object_dir    = ./object_dir
object_marker = $(object_dir)/object_marker.mark
seminar_design = $(SOURCE_DIR)../../step5
seminar_includes = $(SOURCE_DIR)../../include
datafiles     = kernel.txt bias.txt data.txt output.txt

#====== Miscellaneous defines =================================

CRUFT         = transcript 
CRUFT        += *.wlf 
CRUFT        += core* 
CRUFT        += chan_log_data.txt
CRUFT        += chan_log_names.txt
CRUFT        += trace.vcd

#====== Rules =================================================

.PHONY: all run build fast clean checkvars

all: run

run: trace.vcd

run_fast: fast

trace.vcd: $(target) $(datafiles) 
	$(target)

fast: $(fast_target) $(datafiles)
	$(fast_target)

$(target): checkvars $(object_dir)/testbench.o
	@echo "LINK    -o testbench"
	@echo $(LINK) $(CXX_LINK_FLAGS) -o $@ $(object_dir)/testbench.o >> make.out
	@$(LINK) $(CXX_LINK_FLAGS) -o $@ $(object_dir)/testbench.o >> make.out

$(fast_target): checkvars $(object_dir)/fast_testbench.o
	@echo "LINK    -o fast_testbench"
	@echo $(LINK) $(CXX_LINK_FLAGS) -o $@ $(object_dir)/fast_testbench.o >> make.out
	@$(LINK) $(CXX_LINK_FLAGS) -o $@ $(object_dir)/fast_testbench.o >> make.out

%.wlf: %.vcd
	@echo "VCD2WLF $< $@"
	@$(VCD2WLF) $< $@ >> make.out

view_wave: trace.wlf
	@echo "VSIM    $<"
	@$(VSIM) $< -nolog -do "add wave -r trace:/SystemC/*" -do "wave zoom full" >> make.out

TESTBENCH_DEPS  = $(hw_sources)/testbench.cpp
TESTBENCH_DEPS += $(hw_sources)/top.h
TESTBENCH_DEPS += $(hw_sources)/host_code_subsystem.h
TESTBENCH_DEPS += $(hw_sources)/host_code_fabric.h
TESTBENCH_DEPS += $(hw_sources)/sys_ram.h
TESTBENCH_DEPS += $(hw_sources)/accel_if.h
TESTBENCH_DEPS += $(hw_sources)/host_code_tb.h
TESTBENCH_DEPS += $(hw_sources)/sys_accelerator.h
TESTBENCH_DEPS += $(hw_sources)/bus_master_if.h
TESTBENCH_DEPS += $(hw_sources)/conv2d_if.h
TESTBENCH_DEPS += $(hw_sources)/sysbus_axi_struct.h

TESTBENCH_DEPS += $(seminar_design)/conv2d_combined.h
TESTBENCH_DEPS += $(seminar_design)/DPRAM.h
TESTBENCH_DEPS += $(seminar_design)/user_trans.h

TESTBENCH_DEPS += $(seminar_includes)/types.h

$(object_dir)/testbench.o: checkvars $(object_marker) $(TESTBENCH_DEPS)
	@echo "CXX    " $(hw_sources)/testbench.cpp
	@echo $(CXX) $(CXX_FLAGS) -o $@ $(hw_sources)/testbench.cpp >> make.out
	@$(CXX) $(CXX_FLAGS) -o $@ $(hw_sources)/testbench.cpp >> make.out

$(object_dir)/fast_testbench.o: checkvars $(object_marker) $(TESTBENCH_DEPS)
	@echo "CXX    " $(hw_sources)/fast_testbench.cpp
	@echo $(CXX) $(CXXFLAGS_FAST) -o $@ $(hw_sources)/fast_testbench.cpp >> make.out
	@$(CXX) $(CXXFLAGS_FAST) -o $@ $(hw_sources)/fast_testbench.cpp >> make.out

kernel.txt: 
	@echo "CP     $(FILE_IO_SRC)/$@.gz ."
	@cp $(FILE_IO_SRC)/$@.gz .
	@echo "UNZIP  $@.gz"
	@gunzip $@.gz
	
%.txt:
	@echo "CP     $(FILE_IO_SRC)/$@ ."
	@cp $(FILE_IO_SRC)/$@ .

#====== Generic compile rules ======
#
# note: create specific compile rules to
#       manage include dependencies
#

$(object_dir)/%.o: $(hw_sources)/%.cpp
	@echo "CXX     " $<
	@echo $(CXX) $(CXXFLAGS) -c -o $@ $< >> make.out
	@$(CXX) $(CXXFLAGS) -c -o $@ $< >> make.out

$(marker_dir)/%.sv.mark: $(hw_sources)/%.sv
	@echo "VLOG   " $<
	@$(VLOG) $(VLOG_FLAGS) $< >> make.out
	/@touch $@

$(marker_dir)/%.v.mark: $(hw_sources)/%.v
	@echo "VLOG   " $<
	@$(VLOG) $(VLOG_FLAGS) $< >> make.out
	@touch $@

$(marker_dir)/%.sc.mark: $(hw_sources)/%.cpp
	@echo "SCCOM  " $<
	@$(SCCOM) $(SCC_FLAGS) $< >> make.out
	@touch $@

#====== Working directories ========

$(object_marker):
	@echo " "
	@echo "  All tool output is redirected to \"make.out\" "
	@echo " "
	@echo "MKDIR  " $(object_dir)
	@mkdir $(object_dir) >> make.out
	@touch $@

#====== Clean up ===================

clean:
	@echo RM      targets and cruft
	@rm -rf $(target)        \
		$(object_dir)    \
	        $(CRUFT)         \
		$(datafiles)     \
		make.out

#====== Check environment ==========

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
	@echo "Environment variables are set correctly "

#====== Help ========================

help:
	-@echo "Please refer to README.txt for details on this example and instructions on how to run it"
	-@echo ""
	-@echo "Makefile targets for SystemC Model Simulation:"
	-@echo "  clean     - Clean up from previous make runs"
	-@echo "  checkvars - Check for proper environment variable settings"
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
	-@echo ""



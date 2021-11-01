
# Determine the directory containing the source files from the path to this Makefile (var has trailing slash)
PWD := $(shell pwd)
SOURCE_DIR1 = $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
SOURCE_DIR = $(if $(subst ./,,$(SOURCE_DIR1)),$(SOURCE_DIR1),$(PWD)/)
export SW_MEM_FILES = $(SOURCE_DIR)

#====== Tool versions =====================================

QUESTA_VERSION          = 2020.3

#====== Tool directories, update for local installation ===

export QUESTA_HOME      =  /wv/hlstools/questasim/2020.3_2/questasim

export FILE_IO_DIR      = .
export FILE_IO_SRC      = $(SOURCE_DIR)../../file_io

include $(SOURCE_DIR)env_vars.mk

#====== Design configuration variables =====================

CLOCK_PERIOD  = 10

#====== Tool definitions ===================================

CXX           = $(GNU_HOME)/bin/g++
LINK          = $(GNU_HOME)/bin/g++

GCC           = $(GNU_HOME)/bin/gcc

#====== Questa

VLIB          = $(QUESTA_HOME)/bin/vlib
VLOG          = $(QUESTA_HOME)/bin/vlog
VMAP          = $(QUESTA_HOME)/bin/vmap
VOPT          = $(QUESTA_HOME)/bin/vopt
VSIM          = $(QUESTA_HOME)/bin/vsim
SCCOM         = $(QUESTA_HOME)/bin/sccom

#====== Tool options 

DEFINES       = +define+CLOCK_PERIOD=$(CLOCK_PERIOD)
INCLUDES      = -I $(hw_sources)
INCLUDES     += -I $(CONNECTIONS_HOME)/include
INCLUDES     += -I $(MATCHLIB_HOME)/cmod/include
INCLUDES     += -I $(AC_TYPES)/include
INCLUDES     += -I $(AC_SIMUTILS)/include
INCLUDES     += -I $(seminar_design)

#====== vlog

VLOG_DEFINES  = +define+CLOCK_PERIOD=$(CLOCK_PERIOD)

VLOG_FLAGS    = -work $(work)
VLOG_FLAGS   += +acc
VLOG_FLAGS   += $(VLOG_DEFINES) $(VLOG_INCLUDES)

#====== vopt

VOPT_FLAGS    = -work $(work)
VOPT_FLAGS   += +acc
VOPT_FLAGS   += $(VLOG_DEFINES) $(VLOG_INCLUDES)
VOPT_FLAGS   += -g
VOPT_FLAGS   += -enablescstdout
VOPT_FLAGS   += -undefsyms=verbose
VOPT_FLAGS   += -noscelab
VOPT_FLAGS   += -L rocket_lib

#====== sccom

SCC_DEFINES   = -D CLOCK_PERIOD=$(CLOCK_PERIOD)
SCC_DEFINES  += -D SC_INCLUDE_DYNAMIC_PROCESSES
SCC_DEFINES  += -D SYSTEMC
SCC_DEFINES  += -D QUESTA
SCC_DEFINES  += -D SC_INCLUDE_MTI_AC
SCC_DEFINES  += -D INITIALIZE_RAM
SCC_DEFINES  += -D ROCKET
SCC_DEFINES  += -D CONNECTIONS_ACCURATE_SIM
# SCC_DEFINES  += -D CONNECTIONS_NAMING_ORIGINAL
SCC_DEFINES  += -D CONNECTIONS_ONLY_SIM
SCC_DEFINES  += -D INCLUDE_UART
ifdef GUI
SCC_DEFINES  += -D GUI
endif

SCC_INCLUDES  = -I $(hw_sources)
SCC_INCLUDES += -I $(CONNECTIONS_HOME)/include
SCC_INCLUDES += -I $(MATCHLIB_HOME)/cmod/include
SCC_INCLUDES += -I $(AC_TYPES)/include
SCC_INCLUDES += -I $(AC_SIMUTILS)/include
SCC_INCLUDES += -I $(seminar_design)
SCC_INCLUDES += -I $(seminar_includes)

SCC_FLAGS     = $(SCC_DEFINES) $(SCC_INCLUDES)
SCC_FLAGS    += -g
SCC_FLAGS    += -std=c++11

#====== vsim

VSIM_FLAGS    = -work $(work)
VSIM_FLAGS   += -voptargs="+acc"
VSIM_FLAGS   += -L rocket_lib
VSIM_FLAGS   += -do $(SOURCE_DIR)run.do
ifndef GUI
VSIM_FLAGS   += -c
endif

#====== gcc

CXX_INCLUDES += -I $(SOURCE_DIR)
CXX_INCLUDES += -I $(SYSTEMC_HOME)/include -I $(SYSTEMC_HOME)/src
CXX_INCLUDES += -I $(CONNECTIONS_HOME)/include
CXX_INCLUDES += -I $(MATCHLIB_HOME)/cmod/include
CXX_INCLUDES += -I $(AC_TYPES)/include
CXX_INCLUDES += -I $(AC_SIMUTILS)/include


CXX_DEFINES   = -D CONNECTIONS_ACCURATE_SIM 
CXX_DEFINES  += -D SC_INCLUDE_DYNAMIC_PROCESSES
CXX_DEFINES  += -D CLOCK_PERIOD=$(CLOCK_PERIOD)
CXX_DEFINES  += -D MANUAL_RAND_STALL

CXX_FLAGS     = $(CXX_INCLUDES) $(CXX_DEFINES)
CXX_FLAGS    += -c
CXX_FLAGS    += -m64
CXX_FLAGS    += -std=c++11

CXX_LIBDIRS   = -L $(SYSTEMC_HOME)/lib-linux64
CXX_LIBS      = -lsystemc

#====== Source/target defines =================================

marker_dir    = ./marker_dir
work          = ./work
opt_target    = testbench_opt
hw_sources    = $(SOURCE_DIR)../hardware_sources
sw_sources    = $(SOURCE_DIR)./source_sw_dir
seminar_design = $(SOURCE_DIR)../../step5
seminar_includes = $(SOURCE_DIR)../../include
datafiles     = kernel.txt bias.txt data.txt output.txt

sc_link       = $(marker_dir)/sc_link.mark
optimize      = $(marker_dir)/optimize.mark
compile       = $(marker_dir)/compile.mark

work_marker   = $(marker_dir)/work.mark
marker_marker = $(marker_dir)/marker.mark
var_check     = $(marker_dir)/var_check.mark
sc_markers    = $(marker_dir)/systemc_subsystem.sc.mark

hdl_markers   = $(marker_dir)/testbench.sv.mark
hdl_markers  += $(marker_dir)/top.sv.mark

#====== Miscellaneous defines =================================

CRUFT         = transcript 
CRUFT        += *.wlf 
CRUFT        += core* 
CRUFT        += modelsim.ini 
CRUFT        += v*_stacktrace.*

#====== Rules =================================================

.PHONY: run 

all: run

run: $(optimize) $(datafiles) $(SOURCE_DIR)sw/bootrom.mem $(SOURCE_DIR)sw/hw_inference.mem
	@echo "VSIM   " $(VSIM_FLAGS) $(opt_target)
	@$(VSIM) $(VSIM_FLAGS) $(opt_target)

$(optimize): $(marker_marker) $(compile)
	@echo "VOPT   " testbench -o $(opt_target)
	@echo $(VOPT) $(VOPT_FLAGS) testbench -o $(opt_target) >> make.out
	@$(VOPT) $(VOPT_FLAGS) testbench -o $(opt_target) >> make.out
	@touch $@
	@echo " "
	@echo ">>----> Design compiled "
	@echo " "

$(compile): $(var_check) $(marker_marker) $(work_marker) $(sc_link) $(hdl_markers)
	@echo "MAKE    rocket_design"
	@$(MAKE) -f $(SOURCE_DIR)Makefile.rocket_design all >> make.out
	@touch $@

$(sc_link): $(sc_markers)
	@echo "SCCOM  " -link
	@echo $(SCCOM) -link >> make.out
	@$(SCCOM) -link >> make.out
	@touch $@

SYSC_SYS_DEPS  = $(hw_sources)/systemc_subsystem_wrapper.cpp 
SYSC_SYS_DEPS += $(hw_sources)/systemc_subsystem.h
SYSC_SYS_DEPS += $(hw_sources)/proc_fabric.h
SYSC_SYS_DEPS += $(hw_sources)/sys_ram.h
SYSC_SYS_DEPS += $(hw_sources)/sys_accelerator.h
SYSC_SYS_DEPS += $(hw_sources)/accel_if.h
SYSC_SYS_DEPS += $(hw_sources)/bus_master_if.h
SYSC_SYS_DEPS += $(hw_sources)/conv2d_if.h
SYSC_SYS_DEPS += $(hw_sources)/uart_if.h
SYSC_SYS_DEPS += $(hw_sources)/terminal_class.h       
SYSC_SYS_DEPS += $(hw_sources)/sysbus_axi_struct.h

SYSC_SYS_DEPS += $(seminar_design)/conv2d_combined.h
SYSC_SYS_DEPS += $(seminar_design)/DPRAM.h
SYSC_SYS_DEPS += $(seminar_includes)/types.h
SYSC_SYS_DEPS += $(seminar_design)/user_trans.h

$(marker_dir)/systemc_subsystem.sc.mark: $(SYSC_SYS_DEPS) terminal_emulator 
	@echo "SCCOM  " $<
	@echo $(SCCOM) $(SCC_FLAGS) $< >> make.out
	@$(SCCOM) $(SCC_FLAGS) $< >> make.out
	@touch $@

HOST_CODE_TB_DEPS  = $(hw_sources)/host_code_tb.cpp
HOST_CODE_TB_DEPS += $(hw_sources)/host_code_tb.h
HOST_CODE_TB_DEPS += $(hw_sources)/conv2d_if.h
HOST_CODE_TB_DEPS += $(hw_sources)/defines.h

$(marker_dir)/host_code_tb.sc.mark: $(HOST_CODE_TB_DEPS)
	@echo "SCCOM  " $<
	@echo $(SCCOM) $(SCC_FLAGS) $< >> make.out
	@$(SCCOM) $(SCC_FLAGS) $< >> make.out
	@touch $@

#====== Stimulus and results data files

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

$(object_dir)/%.o: %.cpp
	@echo "CC     " $<
	@echo $(GCC) $(CFLAGS) $(INCLUDES) -c -o $@ $< >> make.out
	@$(GCC) $(CFLAGS) $(INCLUDES) -c -o $@ $< >> make.out

$(marker_dir)/%.sv.mark: $(hw_sources)/%.sv
	@echo "VLOG   " $<
	@echo $(VLOG) $(VLOG_FLAGS) $< >> make.out
	@$(VLOG) $(VLOG_FLAGS) $< >> make.out
	@touch $@

$(marker_dir)/%.v.mark: $(hw_sources)/%.v
	@echo "VLOG   " $<
	@echo $(VLOG) $(VLOG_FLAGS) $< >> make.out
	@$(VLOG) $(VLOG_FLAGS) $< >> make.out
	@touch $@

$(marker_dir)/%.sc.mark: $(hw_sources)/%.cpp
	@echo "SCCOM  " $<
	@echo $(SCCOM) $(SCC_FLAGS) $< >> make.out
	@$(SCCOM) $(SCC_FLAGS) $< >> make.out
	@touch $@

#====== Working directories ========

$(marker_marker): 
	@echo " "
	@echo "  All tool output is redirected to \"make.out\" "
	@echo " "
	@echo "MKDIR  " $@
	@mkdir $(marker_dir) >> make.out
	@touch $@

$(work_marker):
	@echo "VLIB   " $(work)
	@echo $(VLIB) $(work) >> make.out
	@$(VLIB) $(work) >> make.out
	@echo "VMAP   " -work $(work)
	@echo $(VMAP) work $(work) >> make.out
	@$(VMAP) work $(work) >> make.out
	@touch $@

#====== Support software============

terminal_emulator: $(SOURCE_DIR)terminal/terminal_emulator.c
	@echo "GCC     $^"
	@echo $(GCC) -o $@ $^ -lpthread >> make.out
	@$(GCC) -o $@ $^ -lpthread >> make.out

#====== Clean up ===================

.PHONY: clean clean_all

clean:
	@echo RM      targets and cruft
	@rm -rf $(target)        \
	        $(work)          \
	        $(marker_dir)    \
	        $(CRUFT)         \
		$(datafiles)     \
		terminal_emulator \
		bias.txt data.txt kernel.txt \
		make.out

clean_all: clean
	@rm -rf rocket_lib

#====== Check environment ==========

#.PHONY: checkvars

# Rule to check that environment variables are set correctly

checkvars: $(var_check)

#$(var_check): $(marker_marker) checkvar_SYSTEMC_HOME checkvar_CONNECTIONS_HOME checkvar_MATCHLIB_HOME checkvar_AC_TYPES checkvar_AC_SIMUTILS
$(var_check): $(marker_marker) 
	@echo "        checking SYSTEMC_HOME "
	@ls $(SYSTEMC_HOME)/include/systemc.h > /dev/null
	@echo "        checking CONNECTIONS_HOME "
	@ls $(CONNECTIONS_HOME)/include/connections/connections.h > /dev/null
	@echo "        checking MATCHLIB_HOME "
	@ls $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h > /dev/null
	@echo "        checking AC_TYPES "
	@ls $(AC_TYPES)/include/ac_int.h > /dev/null
	@echo "        checking AC_STIMUTILS "
	@ls $(AC_SIMUTILS)/include/mc_scverify.h > /dev/null
	@echo "        Environment variables are set correctly "
	@touch $@

#====== Help ========================

help:
	-@echo "Please refer to README.txt for details on this example and instructions on how to run it"
	-@echo ""
	-@echo "Makefile targets for SystemC Model Simulation:"
	-@echo "  clean     - Clean up from previous make runs"
	-@echo "  clean_all - Clena up from previous rns and delete rocket_lib"
	-@echo "  checkvars - Check for proper environment variable settings"
	-@echo "  fast      - Perform simulation using connections FAST mode"
	-@echo "  all       - Perform all of the targets below"
	-@echo "  run       - Execute Verilog/SystemC design and generate wave.wlf"
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


//  Catapult Ultra Synthesis 10.7/935013 (Nightly Build) Fri Apr 23 18:37:53 PDT 2021
//  
//  Copyright (c) Mentor Graphics Corporation, 1996-2021, All Rights Reserved.
//                        UNPUBLISHED, LICENSED SOFTWARE.
//             CONFIDENTIAL AND PROPRIETARY INFORMATION WHICH IS THE
//                 PROPERTY OF MENTOR GRAPHICS OR ITS LICENSORS
//  
//  Running on Linux michaelf@orw-pulpo-rh7 3.10.0-693.21.1.el7.x86_64 x86_64 aol
//  
//  Package information: SIFLIBS v23.7_0.0, HLS_PKGS v23.7_0.0, 
//                       SIF_TOOLKITS v23.7_0.0, SIF_XILINX v23.7_0.0, 
//                       SIF_ALTERA v23.7_0.0, CCS_LIBS v23.7_0.0, 
//                       CDS_PPRO v10.4_3, CDS_DesigChecker v10.6a_1, 
//                       CDS_OASYS v20.1_3.6, CDS_PSR v20.2_1.8, 
//                       DesignPad v2.78_1.0
//  
solution new -state initial
solution options defaults
solution options set /ComponentLibs/SearchPath . -append
solution options set /Interface/DefaultClockPeriod 2
solution options set /Input/CppStandard c++11
solution options set /Input/CompilerFlags {-DCONNECTIONS_NAMING_ORIGINAL -DCONNECTIONS_ACCURATE_SIM}
solution options set /Input/SearchPath {../include {$MGC_HOME/shared/examples/matchlib/toolkit/include} {$MGC_HOME/shared/pkgs/matchlib/cmod/include}}
solution options set /Flows/QuestaSIM/SCCOM_OPTS {-O3 -x c++ -Wall -Wno-unused-label -Wno-unknown-pragmas}
solution options set /Flows/QuestaSIM/MSIM_AC_TYPES false
solution options set /Flows/SCVerify/USE_NCSIM true
solution options set /Flows/SCVerify/USE_VCS true
flow package require /MemGen
flow package option set /SCVerify/INVOKE_ARGS ../file_io
solution file add ./testbench.cpp -type C++
directive set -DESIGN_GOAL area
directive set -SPECULATE true
directive set -MERGEABLE true
directive set -REGISTER_THRESHOLD 256
directive set -MEM_MAP_THRESHOLD 32
directive set -LOGIC_OPT false
directive set -FSM_ENCODING none
directive set -FSM_BINARY_ENCODING_THRESHOLD 64
directive set -REG_MAX_FANOUT 0
directive set -NO_X_ASSIGNMENTS true
directive set -SAFE_FSM false
directive set -REGISTER_SHARING_MAX_WIDTH_DIFFERENCE 8
directive set -REGISTER_SHARING_LIMIT 0
directive set -ASSIGN_OVERHEAD 0
directive set -TIMING_CHECKS true
directive set -MUXPATH true
directive set -REALLOC true
directive set -UNROLL no
directive set -IO_MODE super
directive set -CHAN_IO_PROTOCOL use_library
directive set -ARRAY_SIZE 1024
directive set -IDLE_SIGNAL {}
directive set -STALL_FLAG_SV off
directive set -STALL_FLAG false
directive set -TRANSACTION_DONE_SIGNAL true
directive set -DONE_FLAG {}
directive set -READY_FLAG {}
directive set -START_FLAG {}
directive set -TRANSACTION_SYNC ready
directive set -RESET_CLEARS_ALL_REGS use_library
directive set -CLOCK_OVERHEAD 20.000000
directive set -ON_THE_FLY_PROTOTYPING false
directive set -OPT_CONST_MULTS use_library
directive set -CHARACTERIZE_ROM false
directive set -PROTOTYPE_ROM true
directive set -ROM_THRESHOLD 64
directive set -CLUSTER_ADDTREE_IN_WIDTH_THRESHOLD 0
directive set -CLUSTER_ADDTREE_IN_COUNT_THRESHOLD 0
directive set -CLUSTER_OPT_CONSTANT_INPUTS true
directive set -CLUSTER_RTL_SYN false
directive set -CLUSTER_FAST_MODE false
directive set -CLUSTER_TYPE combinational
directive set -PROTOTYPING_ENGINE oasys
directive set -PIPELINE_RAMP_UP true
go new
directive set -ARRAY_INDEX_OPTIMIZATION true
directive set -ASSUME_ARRAY_INDEX_IN_RANGE true
solution design set conv2d -top
go analyze
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND sync -RESET_SYNC_ACTIVE high -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
go compile
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0 -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0}}
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
solution library add DPRAM
go libraries
directive set /accelerator/conv2d/mem_buffer/K_X0 -PIPELINE_INIT_INTERVAL 1
directive set /accelerator/conv2d/mem_buffer/K_X0 -PIPELINE_STALL_MODE flush
directive set /accelerator/conv2d/mem_buffer/ROW -PIPELINE_INIT_INTERVAL 1
directive set /accelerator/conv2d/mem_buffer/ROW -PIPELINE_STALL_MODE flush
directive set /accelerator/conv2d/conv/COL -PIPELINE_INIT_INTERVAL 1
directive set /accelerator/conv2d/conv/K_X -UNROLL yes
directive set /accelerator/conv2d/conv/K_Y -UNROLL yes
directive set /accelerator/conv2d/conv/ROW_CPY -PIPELINE_INIT_INTERVAL 1
directive set /accelerator/conv2d/conv/ROW_CPY -PIPELINE_STALL_MODE flush
directive set /accelerator/conv2d/conv/line_buffers:rsc -BLOCK_SIZE 416
go architect
ignore_memory_precedences -from *write_mem(line_buffers* -to *read_mem(line_buffers*
ignore_memory_precedences -from *write_mem(acc_buf* -to *read_mem(acc_buf*
ignore_memory_precedences -from *write_mem(max* -to *read_mem(max*
ignore_memory_precedences -from *write_mem(acc_buf* -to *read_mem(acc_buf*)

go allocate
go extract

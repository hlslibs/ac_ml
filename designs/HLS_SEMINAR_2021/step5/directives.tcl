#------------------------------------------------------------
# Machine Learning Example using MatchLib
#  combined_ML_cached 
#------------------------------------------------------------

# Establish the location of this script and use it to reference all
# other files in this example
set sfd [file dirname [info script]]

# Reset the options to the factory defaults
options defaults

options set Input/SearchPath "$sfd/../include \$MGC_HOME/shared/examples/matchlib/toolkit/include \$MGC_HOME/shared/pkgs/matchlib/cmod/include"
options set ComponentLibs/SearchPath $sfd -append
options set Interface/DefaultClockPeriod 2
options set Input/CppStandard c++11
options set Input/CompilerFlags {-DCONNECTIONS_NAMING_ORIGINAL -DCONNECTIONS_ACCURATE_SIM}
options set Output/GenerateCycleNetlist false

# Start a new project for option changes to take place
project new

flow package require /SCVerify
flow package option set /SCVerify/USE_MSIM true
flow package option set /SCVerify/USE_NCSIM true
flow package option set /SCVerify/USE_VCS true
flow package option set /SCVerify/INVOKE_ARGS "$sfd/../file_io"
flow package require /QuestaSIM
flow package option set /QuestaSIM/SCCOM_OPTS {-O3 -x c++ -Wall -Wno-unused-label -Wno-unknown-pragmas}
flow package option set /QuestaSIM/MSIM_AC_TYPES false
flow package require /MemGen

solution file add $sfd/testbench.cpp -type C++
directive set -ARRAY_INDEX_OPTIMIZATION true
directive set -ASSUME_ARRAY_INDEX_IN_RANGE true
solution design set conv2d -top
go analyze
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0 -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0}}
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
solution library add DPRAM
go libraries
directive set /conv2d/mem_buffer/K_X0 -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/mem_buffer/K_X0 -PIPELINE_STALL_MODE flush
directive set /conv2d/mem_buffer/ROW -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/mem_buffer/ROW -PIPELINE_STALL_MODE flush
directive set /conv2d/conv2d_core/run_conv_core/COL -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/conv2d_core/run_conv_core/K_X -UNROLL yes
directive set /conv2d/conv2d_core/run_conv_core/K_Y -UNROLL yes
directive set /conv2d/conv2d_core/run_conv_core/ROW_CPY -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/conv2d_core/run_conv_core/ROW_CPY -PIPELINE_STALL_MODE flush
directive set /conv2d/conv2d_core/run_conv_core/line_buffers:rsc -BLOCK_SIZE 416
go architect
ignore_memory_precedences -from *write_mem(line_buffers* -to *read_mem(line_buffers*
ignore_memory_precedences -from *write_mem(acc_buf* -to *read_mem(acc_buf*
ignore_memory_precedences -from *write_mem(max* -to *read_mem(max*
ignore_memory_precedences -from *write_mem(acc_buf* -to *read_mem(acc_buf*)

go allocate
go extract

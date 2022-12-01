#------------------------------------------------------------
# Machine Learning Example using MatchLib
#  combined_ML
#------------------------------------------------------------

# Establish the location of this script and use it to reference all
# other files in this example
set sfd [file dirname [info script]]

# Reset the options to the factory defaults
options defaults

options set Input/SearchPath $sfd/../include
options set ComponentLibs/SearchPath . -append
options set Interface/DefaultClockPeriod 2
options set Input/CppStandard c++11
options set Input/CompilerFlags -DCONNECTIONS_ACCURATE_SIM
options set Output/GenerateCycleNetlist false

# Start a new project for option changes to take place
project new

flow package require /SCVerify
flow package option set /SCVerify/USE_MSIM true
flow package option set /SCVerify/USE_NCSIM true
flow package option set /SCVerify/USE_VCS true
flow package option set /SCVerify/INVOKE_ARGS "$sfd/../file_io"
flow package require /QuestaSIM
flow package option set /QuestaSIM/SCCOM_OPTS {-g -O3 -x c++ -Wall -Wno-unused-label -Wno-unknown-pragmas}

solution file add $sfd/testbench.cpp  -type C++

directive set /ARRAY_INDEX_OPTIMIZATION true
directive set /ASSUME_ARRAY_INDEX_IN_RANGE true
directive set -CLOCKS {clk {-CLOCK_PERIOD 2.0 -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.0}}
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
go libraries
directive set /conv2d/run/K_X0 -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/run/COL -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/run/line_buffers:rsc -BLOCK_SIZE 416
directive set /conv2d/run/K_X -UNROLL yes
directive set /conv2d/run/K_Y -UNROLL yes
directive set /conv2d/run/COL_CPY -PIPELINE_INIT_INTERVAL 1
go architect
ignore_memory_precedences -from *write_mem(line* -to *read_mem(line*
go allocate
go extract

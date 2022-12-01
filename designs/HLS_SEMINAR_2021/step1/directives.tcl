#------------------------------------------------------------
# Machine Learning Example using MatchLib
#  conv2d_bit_accurate 
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
flow package option set /QuestaSIM/SCCOM_OPTS {-O3 -x c++ -Wall -Wno-unused-label -Wno-unknown-pragmas}
flow package option set /QuestaSIM/MSIM_AC_TYPES false

solution file add $sfd/testbench.cpp -type C++
directive set -PIPELINE_RAMP_UP true

directive set -CLOCKS {clk {-CLOCK_PERIOD 500.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 250.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND sync -RESET_SYNC_ACTIVE high -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
go analyze
directive set -CLOCKS {clk {-CLOCK_PERIOD 500.0 -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 250.0}}
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
go libraries
directive set /conv2d/run/COL -PIPELINE_INIT_INTERVAL 2
directive set /conv2d/run/COL -PIPELINE_STALL_MODE flush
directive set /conv2d/run/ROW_CPY -PIPELINE_INIT_INTERVAL 1
directive set /conv2d/run/ROW_CPY -PIPELINE_STALL_MODE flush
directive set /conv2d/run/ROW_CLR -PIPELINE_INIT_INTERVAL 1
go architect
go extract


/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright 2021, Mentor Graphics Corporation,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *  
 **************************************************************************
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      * 
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *      http://www.apache.org/licenses/LICENSE-2.0                        *
 *                                                                        *
 *  Unless required by applicable law or agreed to in writing, software   * 
 *  distributed under the License is distributed on an "AS IS" BASIS,     * 
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or       *
 *  implied.                                                              * 
 *  See the License for the specific language governing permissions and   * 
 *  limitations under the License.                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package is available at github.       *
 *                                                                        *
 *************************************************************************/
#include <systemc.h>
#include <sstream>

#include "ac_sysc_macros.h"
#include "ac_sysc_trace.h"
//#include "connections/connections_trace.h"

#include "top.h"

SC_MODULE(testbench) {

public:

  //== Instances

  top CCS_INIT_S1(design_top);

  //== Local signals

  sc_clock          clk;
  sc_signal<bool>   reset_bar;

  //== Local methods

  void reset_gen()
  {
    reset_bar.write(0);
    wait(10);
    reset_bar.write(1);
    wait();
  }

  //== Constructor

  SC_CTOR(testbench)
     :  clk("clk", CLOCK_PERIOD, SC_NS, 0.5, 0, SC_NS, true)
  {
      design_top.clk(clk);
      design_top.reset_bar(reset_bar);

      SC_CTHREAD(reset_gen, clk);
  }
};

int sc_main(int argc, char **argv) 
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  testbench testbench_inst("testbench");
  trace_hierarchy(&testbench_inst, trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(testbench_inst);

  sc_start();

  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}

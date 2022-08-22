/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright 2021 Siemens                                                *
 *                                                                        *
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
#include "types.h"

#include "sys_ram.h"
#include "accel_if.h"
#include "host_code_tb.h"



#include "host_code_fabric.h"

#include "axi/axi4.h"
#include "axi4_segment.h"
#include "sysbus_axi_struct.h"

class systemc_subsystem : public sc_module, public sysbus_axi {
public:

  //== Ports

  sc_in<bool>     clk;
  sc_in<bool>     reset_bar;

  r_slave<>       CCS_INIT_S1(r_cpu);
  w_slave<>       CCS_INIT_S1(w_cpu);


  //== Local signals

  r_chan<>        CCS_INIT_S1(r_acc_regs); 
  w_chan<>        CCS_INIT_S1(w_acc_regs);

  r_chan<>        CCS_INIT_S1(r_acc_master);
  w_chan<>        CCS_INIT_S1(w_acc_master);

  r_chan<>        CCS_INIT_S1(r_memory);
  w_chan<>        CCS_INIT_S1(w_memory);
  

  //== Instances

  fabric          CCS_INIT_S1(io_matrix);
  accel_if        CCS_INIT_S1(go_fast);
  sys_ram         CCS_INIT_S1(shared_memory);

  //== Constructor

  SC_CTOR(systemc_subsystem)
  {
    io_matrix.clk          (clk);
    io_matrix.rst_bar      (reset_bar);
    io_matrix.r_mem        (r_memory);
    io_matrix.w_mem        (w_memory);
    io_matrix.r_reg        (r_acc_regs);
    io_matrix.w_reg        (w_acc_regs);
    io_matrix.r_cpu        (r_cpu);
    io_matrix.w_cpu        (w_cpu);
    io_matrix.r_acc        (r_acc_master);
    io_matrix.w_acc        (w_acc_master);

    shared_memory.clk      (clk);
    shared_memory.rst_bar  (reset_bar);
    shared_memory.r_slave0 (r_memory);
    shared_memory.w_slave0 (w_memory);

    go_fast.clk            (clk);
    go_fast.rst_bar        (reset_bar);
    go_fast.r_reg_if       (r_acc_regs);
    go_fast.w_reg_if       (w_acc_regs);
    go_fast.r_mem_if       (r_acc_master);
    go_fast.w_mem_if       (w_acc_master);
  }
};


#ifdef QUESTA
SC_MODULE_EXPORT(systemc_subsystem);
#endif

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
#pragma once

#include "systemc_subsystem.h"
#include "host_code_tb.h"

class top : public sc_module, public sysbus_axi {
public:

  //== Ports

  sc_in<bool>        CCS_INIT_S1(clk);
  sc_in<bool>        CCS_INIT_S1(reset_bar);

  //== Instances

  systemc_subsystem  CCS_INIT_S1(sc_design);
  host_code_tb       CCS_INIT_S1(sw);

  //== Local signals

  r_chan<>           r_cpu;
  w_chan<>           w_cpu;

  //== Constructor

  SC_CTOR(top) :
    r_cpu("r_cpu"),
    w_cpu("w_cpu")
  {
     sc_design.clk(clk);
     sc_design.reset_bar(reset_bar);

     sc_design.r_cpu(r_cpu);
     sc_design.w_cpu(w_cpu);

     sw.clk(clk);
     sw.rst_bar(reset_bar);

     sw.r_master(r_cpu);
     sw.w_master(w_cpu);
  }
};


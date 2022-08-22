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

#include <sstream>

#pragma once
#include "axi4_segment.h"
#include "sysbus_axi_struct.h"

#include "conv2d_if.h"
#include "defines.h"

class host_code_tb : public sc_module, public sysbus_axi {
public:

  //== Ports 

  sc_in<bool>   CCS_INIT_S1(clk);
  sc_in<bool>   CCS_INIT_S1(rst_bar);

  r_master<>    CCS_INIT_S1(r_master);
  w_master<>    CCS_INIT_S1(w_master);

  //== Local methods

#include "common_stim_results.h"

  void sw_thread()
  {
    w_master.reset();
    r_master.reset();

    wait();

    // weights, biases, image, and expected results are preloaded
    // into shared memory, so no need to load them here

    inference();
    check_results();
    sc_stop();
  }

  //== Constructor

  SC_CTOR(host_code_tb)
  {
    SC_THREAD(sw_thread);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }
};

/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Mon Nov  1 05:56:21 PDT 2021                        *
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

#include "axi4_segment.h"
#include "my_axi_struct.h"
#include "types.h"

/**
 *  \brief A simple RAM module with 1 axi4 read slave and 1 axi4 write slave
*/

class ram : public sc_module, public local_axi
{
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rstn);
  r_slave<AUTO_PORT>     CCS_INIT_S1(r_slave0);
  w_slave<AUTO_PORT>     CCS_INIT_S1(w_slave0);

  static const int sz = MEM_SIZE; // size in axi_cfg::dataWidth words

  typedef NVUINTW(my_axi4_config::dataWidth) arr_t;
  arr_t *array {0};

  SC_CTOR(ram) {
    array = new arr_t[sz];

    SC_THREAD(slave_r_process);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    SC_THREAD(slave_w_process);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    for (int i=0; i < sz; i++)
    { array[i] = 0; }
  }

  NVUINTW(axi_cfg::dataWidth) debug_read_addr(uint32_t addr) {
    if (addr >= (sz * bytesPerBeat)) {
      SC_REPORT_ERROR("ram", "invalid addr");
      return 0;
    }

    return (array[addr / bytesPerBeat]);
  }

  void slave_r_process() {
    r_slave0.reset();

    wait();

    while (1) {
      ar_payload ar;
      r_slave0.start_multi_read(ar);

      //CCS_LOG("ram read  addr: " << std::hex << ar.addr << " len: " << ar.len);

      while (1) {
        r_payload r;

        if (ar.addr >= (sz * bytesPerBeat)) {
          SC_REPORT_ERROR("ram", "invalid addr");
          r.resp = Enc::XRESP::SLVERR;
        } else {
          r.data = array[ar.addr / bytesPerBeat];
        }

        if (!r_slave0.next_multi_read(ar, r)) { break; }
      }
    }
  }

  void slave_w_process() {
    w_slave0.reset();
    wait();

    while (1) {
      aw_payload aw;
      b_payload b;

      w_slave0.start_multi_write(aw, b);

      //CCS_LOG("ram write addr: " << std::hex << aw.addr << " len: " << aw.len);

      while (1) {
        w_payload w = w_slave0.w.Pop();

        if (aw.addr >= (sz * bytesPerBeat)) {
          SC_REPORT_ERROR("ram", "invalid addr");
          b.resp = Enc::XRESP::SLVERR;
        } else {
          decltype(w.wstrb) all_on{~0};

          if (w.wstrb == all_on) {
            array[aw.addr / bytesPerBeat] = w.data.to_uint64();
          } else {
            //CCS_LOG("write strobe enabled");
            arr_t orig  = array[aw.addr / bytesPerBeat];
            arr_t wdata = w.data.to_uint64();

#pragma unroll
            for (int i=0; i<WSTRB_WIDTH; i++)
              if (w.wstrb[i]) {
                orig = nvhls::set_slc(orig, nvhls::get_slc<8>(wdata, (i*8)), (i*8));
              }

            array[aw.addr / bytesPerBeat] = orig;
          }
        }

        if (!w_slave0.next_multi_write(aw)) { break; }
      }

      w_slave0.b.Push(b);
    }
  }
};


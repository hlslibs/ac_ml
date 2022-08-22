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
#include "types.h"
#include "axi4_segment.h"
#include "sysbus_axi_struct.h"

#define SHIFT 2
#define WEIGHT_MEMORY_BASE 0x70000000

class bus_master_if : public sc_module, public sysbus_axi {
 public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rstn);
//Bus I/Fs
  r_master<> CCS_INIT_S1(r_master0);
  w_master<> CCS_INIT_S1(w_master0);

//User side
//Read
  Connections::In<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_in_addr);
  Connections::In<BURST_TYPE>     CCS_INIT_S1(mem_in_burst);
  Connections::Out<DTYPE>         CCS_INIT_S1(mem_in_data);
//Write
  Connections::In<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_out_addr);
  Connections::In<BURST_TYPE>     CCS_INIT_S1(mem_out_burst);
  Connections::In<DTYPE>          CCS_INIT_S1(mem_out_data);

  SC_CTOR(bus_master_if) {

    SC_THREAD(write_master_process);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    SC_THREAD(read_master_process);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

    AXI4_W_SEGMENT_BIND(w_segment0, clk, rstn, w_master0);
    AXI4_R_SEGMENT_BIND(r_segment0, clk, rstn, r_master0);
  }

 private:

  // write and read segmenters segment long bursts to conform to AXI4 protocol (which allows 256 beats maximum).
  AXI4_W_SEGMENT(w_segment0)
  AXI4_R_SEGMENT(r_segment0)

  void read_master_process() {
    AXI4_R_SEGMENT_RESET(r_segment0, r_master0);
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();

    wait();

    while (1) {
      ex_ar_payload ar;

      ar.addr = uint32(mem_in_addr.Pop()) << SHIFT; //convert to byte address
      ar.addr += WEIGHT_MEMORY_BASE;
      ar.ex_len = mem_in_burst.Pop();
      r_segment0_ex_ar_chan.Push(ar);

      #pragma hls_pipeline_init_interval 1
      #pragma pipeline_stall_mode flush
      while (1) {
        r_payload r = r_master0.r.Pop();
        DTYPE data;
        data.set_slc(0,r.data.slc<DTYPE::width>(0));
        mem_in_data.Push(data);
        if (ar.ex_len-- == 0) { break; }
      }
    }
  }

  void write_master_process() {
    AXI4_W_SEGMENT_RESET(w_segment0, w_master0);
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    wait();

    while (1) {
      ex_aw_payload aw;

      aw.addr = uint32(mem_out_addr.Pop()) << SHIFT; // convert to byte address
      aw.ex_len = mem_out_burst.Pop();
      aw.addr += WEIGHT_MEMORY_BASE;
      w_segment0_ex_aw_chan.Push(aw);

      #pragma hls_pipeline_init_interval 1
      #pragma pipeline_stall_mode flush
      while (1) {
        w_payload w;
        w.data = mem_out_data.Pop().slc<DTYPE::width>(0);
        w_segment0_w_chan.Push(w);

        if (aw.ex_len-- == 0) { break; }
      }

      b_payload b = w_segment0_b_chan.Pop();
    }
  }
};

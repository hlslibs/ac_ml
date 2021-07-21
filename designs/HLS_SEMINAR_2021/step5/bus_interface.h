/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.4                                                 *
 *                                                                        *
 *  Release Date    : Wed Jul 21 10:23:21 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.4.0                                               *
 *                                                                        *
 *  Copyright , Mentor Graphics Corporation,                     *
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
#pragma once

#include "types.h"
#include "axi4_segment.h"
#include "my_axi_struct.h"

// CPU address map
struct address_map {
  uint64_t  num_in_fmaps;
  uint64_t  num_out_fmaps;
  uint64_t  height;
  uint64_t  width;
  uint64_t  read_offset;
  uint64_t  write_offset;
  uint64_t  weight_offset;
  uint64_t  bias_offset;
  uint64_t  relu;
  uint64_t  pointwise;
  uint64_t  pool;
};

// Bus I/F module with AXI4 master and slave
class bus_interface : public sc_module, public local_axi
{
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rstn);
  // Bus I/Fs
  r_master<> CCS_INIT_S1(r_master0);
  w_master<> CCS_INIT_S1(w_master0);
  typename local_axi4_lite::write::template slave<> CCS_INIT_S1(w_slave0);

  // User I/F to DUT
  // Read I/F
  Connections::In<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_in_addr);
  Connections::In<BURST_TYPE>     CCS_INIT_S1(mem_in_burst);
  Connections::Out<DTYPE>         CCS_INIT_S1(mem_in_data);
  // Write I/F
  Connections::In<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_out_addr);
  Connections::In<BURST_TYPE>     CCS_INIT_S1(mem_out_burst);
  Connections::In<DTYPE>          CCS_INIT_S1(mem_out_data);

  // DUT configuration status inputs
  sc_out<IN_FMAP_TYPE>              CCS_INIT_S1(num_in_fmaps);
  sc_out<OUT_FMAP_TYPE>             CCS_INIT_S1(num_out_fmaps);
  sc_out<HEIGHT_TYPE>               CCS_INIT_S1(height);
  sc_out<WIDTH_TYPE>                CCS_INIT_S1(width);
  sc_out<OFFSET_TYPE>               CCS_INIT_S1(read_offset);
  sc_out<OFFSET_TYPE>               CCS_INIT_S1(write_offset);
  sc_out<OFFSET_TYPE>               CCS_INIT_S1(weight_offset);
  sc_out<BIAS_OFFSET_TYPE>          CCS_INIT_S1(bias_offset);
  sc_out<bool>                      CCS_INIT_S1(pointwise); // true does 1x1 convolution
  sc_out<bool>                      CCS_INIT_S1(relu); // enable/disable RELU
  sc_out<uint2>                     CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2

  SC_CTOR(bus_interface) {
    SC_THREAD(slave_process);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);

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

  AXI4_W_SEGMENT(w_segment0)
  AXI4_R_SEGMENT(r_segment0)

  //Master recieves read requests from DUT and reads data from system memory
  void read_master_process() {
    AXI4_R_SEGMENT_RESET(r_segment0, r_master0);
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();
    //dma_cmd_chan.ResetRead();
    //dma_dbg.Reset();
    //dma_done.Reset();

    wait();

    while (1) {
      ex_ar_payload ar;
      ar.ex_len = mem_in_burst.Pop();
      ar.addr = uint32(mem_in_addr.Pop())<<1;//convert to byte address
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

  // Master recieves write requests from DUT and writes data to system memory
  void write_master_process() {
    AXI4_W_SEGMENT_RESET(w_segment0, w_master0);
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    wait();
    while (1) {
      ex_aw_payload aw;
      aw.ex_len = mem_out_burst.Pop();
      aw.addr = uint32(mem_out_addr.Pop())<<1;//hard coded for 2 bytes per beat
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
  // slave_process accepts incoming axi4 requests from CPU and programs DUT CSRs
  // when the start register is written to, a dma_cmd transaction is sent to the dma master_process
  void slave_process() {
    w_slave0.reset();
    num_in_fmaps.write(0);
    num_out_fmaps.write(0);
    height.write(0);
    width.write(0);
    read_offset.write(0);
    write_offset.write(0);
    weight_offset.write(0);
    bias_offset.write(0);
    relu.write(0);
    pointwise.write(0);
    pool.write(0);
    wait();

    while (1) {
      local_axi4_lite::AddrPayload aw;
      local_axi4_lite::WritePayload w;
      local_axi4_lite::WRespPayload b;
      w_slave0.wread(aw, w);
      b.resp = Enc::XRESP::SLVERR;
      switch (aw.addr) {
        case offsetof(address_map, num_in_fmaps):
          num_in_fmaps.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, num_out_fmaps):
          num_out_fmaps.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, height):
          height.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, width):
          width.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, read_offset):
          read_offset.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, write_offset):
          write_offset.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, weight_offset):
          weight_offset.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, bias_offset):
          bias_offset.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, relu):
          relu.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, pointwise):
          pointwise.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        case offsetof(address_map, pool):
          pool.write(w.data);
          b.resp = Enc::XRESP::OKAY;
          break;
        default:
          b.resp = Enc::XRESP::OKAY;
          break;
      }
      w_slave0.b.Push(b);
    }
  }
};


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
#include "axi4_segment.h"
#include "conv2d_combined.h"
#include "sysbus_axi_struct.h"

#include "sys_accelerator.h"

 // register map

#define GO_REG                     0
#define GO_READY_REG               1
#define DONE_REG                   2
#define DONE_VALID_REG             3
#define NUM_IN_FMAPS_REG           4
#define NUM_OUT_FMAPS_REG          5
#define HEIGHT_REG                 6
#define WIDTH_REG                  7
#define READ_OFFSET_REG            8
#define WRITE_OFFSET_REG           9
#define WEIGHT_OFFSET_REG         10
#define BIAS_OFFSET_REG           11
#define POINTWISE_REG             12
#define RELU_REG                  13
#define POOL_REG                  14
#define ADDR_OFFSET_LOW_REG       15
#define ADDR_OFFSET_HIGH_REG      16
#define BURST_SIZE_REG            17

class accel_if : public sc_module, public sysbus_axi {
public:

  //== Ports

  sc_in<bool>   CCS_INIT_S1(clk);
  sc_in<bool>   CCS_INIT_S1(rst_bar);

  r_slave<>     CCS_INIT_S1(r_reg_if);
  w_slave<>     CCS_INIT_S1(w_reg_if);

  r_master<>    CCS_INIT_S1(r_mem_if);
  w_master<>    CCS_INIT_S1(w_mem_if);

  //== Instances

  sys_accelerator   CCS_INIT_S1(accel_inst);

  //== Local signals

  bool chatty = false;

    // interface signals

  sc_signal<ac_int<10, false> >     num_in_fmaps;
  sc_signal<ac_int<10, false> >     num_out_fmaps;
  sc_signal<ac_int< 9, false> >     height;
  sc_signal<ac_int< 9, false> >     width;
  sc_signal<ac_int<25, false> >     read_offset;
  sc_signal<ac_int<25, false> >     write_offset;
  sc_signal<ac_int<25, false> >     weight_offset;
  sc_signal<ac_int<25, false> >     bias_offset;
  sc_signal<bool>                   pointwise;
  sc_signal<bool>                   relu;
  sc_signal<ac_int< 2, false> >     pool;

  Connections::SyncChannel          CCS_INIT_S1(start);
  Connections::SyncChannel          CCS_INIT_S1(done);

  sc_uint<64>                       register_bank[18];
  const int                         num_regs    = 18;

  sc_signal<bool>                   go;
  sc_signal<bool>                   idle;
  sc_signal<bool>                   busy;

  //== Local methods

  void reg_reader()
  {
    ar_payload ar;
    r_payload  r;
    int        reg_addr;

    done.reset_sync_in();
    r_reg_if.reset();
    idle.write(1);

    wait();
    
    while (1) {
      r_reg_if.start_multi_read(ar);
      reg_addr = ar.addr >> 2;  // registers are 32-bit wide word addressable
      if (chatty) printf("Read from register: %d \n", reg_addr);
      while (1) {
        if (reg_addr > num_regs) {
          SC_REPORT_ERROR("accel_if", "invalid addr");
          r.resp = Enc::XRESP::SLVERR;
        } else {
          r.data = register_bank[reg_addr].to_int64();
          r.resp = Enc::XRESP::OKAY;
	  r.id   = ar.id;
        }
        if (!r_reg_if.next_multi_read(ar, r)) break;
      } 
      if (reg_addr == DONE_REG) {
        done.sync_in();
        idle.write(1);
      } 

      if (busy) idle.write(0);
      wait();
    }
  }

  void reg_writer() 
  {
    int     reg_addr;

    start.reset_sync_out();
    busy.write(0);
    w_reg_if.reset();

    wait();

    while(1) {
      aw_payload  aw;
      w_payload   w;
      b_payload   b;

      if (w_reg_if.aw.PopNB(aw)) {

        w = w_reg_if.w.Pop();

        reg_addr = aw.addr >> 2;  // registers are 32-bit wide and word addressable

        if (chatty) printf("Writing to register: %d = 0x%08x \n", reg_addr, w.data.to_int());  

        if (reg_addr > num_regs) {
          SC_REPORT_ERROR("accel_if", "invalid addr");
          b.resp = Enc::XRESP::SLVERR;
        } else {
          register_bank[reg_addr] = w.data.to_uint64();
          b.resp = Enc::XRESP::OKAY;
          b.id   = aw.id;
	}

        w_reg_if.b.Push(b);
      
        if (reg_addr == GO_REG) {
          busy.write(1);
          start.sync_out();
        } 
      } 

      if (idle) busy.write(0);

      // drive outputs 

      register_bank[GO_READY_REG]    = start.rdy; // !busy.read();
      register_bank[DONE_VALID_REG]  = done.vld.read();  // idle.read();
      register_bank[DONE_REG]        = idle.read();

      wait();
    }
  }

  void set_inputs()
  {
    go =             register_bank[GO_REG];
    num_in_fmaps =   register_bank[NUM_IN_FMAPS_REG].to_int();
    num_out_fmaps =  register_bank[NUM_OUT_FMAPS_REG].to_int();
    height =         register_bank[HEIGHT_REG].to_int();
    width =          register_bank[WIDTH_REG].to_int();
    read_offset =    register_bank[READ_OFFSET_REG].to_int();
    write_offset =   register_bank[WRITE_OFFSET_REG].to_int();
    weight_offset =  register_bank[WEIGHT_OFFSET_REG].to_int();
    bias_offset =    register_bank[BIAS_OFFSET_REG].to_int();
    pointwise =      register_bank[POINTWISE_REG].to_int();
    relu =           register_bank[RELU_REG].to_int();
    pool =           register_bank[POOL_REG].to_int();
  }


  SC_CTOR(accel_if)
  {

    SC_THREAD(reg_reader);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(reg_writer);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_METHOD(set_inputs);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    accel_inst.clk             (clk);
    accel_inst.rstn            (rst_bar);
    
    accel_inst.r_master0       (r_mem_if);
    accel_inst.w_master0       (w_mem_if);

    accel_inst.num_in_fmaps    (num_in_fmaps);
    accel_inst.num_out_fmaps   (num_out_fmaps);
    accel_inst.height          (height);
    accel_inst.width           (width);
    accel_inst.read_offset     (read_offset);
    accel_inst.write_offset    (write_offset);
    accel_inst.weight_offset   (weight_offset);
    accel_inst.bias_offset     (bias_offset);
    accel_inst.pointwise       (pointwise);
    accel_inst.relu            (relu);
    accel_inst.pool            (pool);

    accel_inst.start           (start);
    accel_inst.done            (done);
  }
};

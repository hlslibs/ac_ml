/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.9                                                 *
 *                                                                        *
 *  Release Date    : Mon Oct 14 17:47:36 PDT 2024                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.9.0                                               *
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
#ifndef __CONV_2D__
#define __CONV_2D__

#include "types.h"
#include <ac_int.h>
#include <ac_sync.h>
#include "DPRAM.h"
#include "user_trans.h"

#pragma hls_design ccore
#pragma hls_ccore_type combinational
template<typename T>
T pass(T din)
{
  return din;
}

// convolution with stride 1 and odd size kernel

class mem_buffer: public sc_module
{
public:
  sc_in<bool> clk;
  sc_in<bool> rstn;

  Connections::Out<MEM_ADDR_TYPE>   CCS_INIT_S1(mem_in_addr);
  Connections::Out<BURST_TYPE>      CCS_INIT_S1(mem_in_burst);
  Connections::In<DTYPE>            CCS_INIT_S1(mem_in_data);

  #pragma hls_direct_input
  sc_in<IN_FMAP_TYPE>               CCS_INIT_S1(num_in_fmaps);
  #pragma hls_direct_input
  sc_in<OUT_FMAP_TYPE>              CCS_INIT_S1(num_out_fmaps);
  #pragma hls_direct_input
  sc_in<HEIGHT_TYPE>                CCS_INIT_S1(height);
  #pragma hls_direct_input
  sc_in<WIDTH_TYPE>                 CCS_INIT_S1(width);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                CCS_INIT_S1(read_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                CCS_INIT_S1(write_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                CCS_INIT_S1(weight_offset);
  #pragma hls_direct_input
  sc_in<BIAS_OFFSET_TYPE>           CCS_INIT_S1(bias_offset);
  #pragma hls_direct_input
  sc_in<bool>                       CCS_INIT_S1(pointwise); // true does 1x1 convolution
  #pragma hls_direct_input
  sc_in<bool>                       CCS_INIT_S1(relu); // enable/disable RELU
  #pragma hls_direct_input
  sc_in<uint2>                      CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2
  Connections::SyncIn               CCS_INIT_S1(start);

  DPRAM_model<>::wr_port<DTYPE,692224>    mem;
  Connections::Out <array_t<DTYPE,KSIZE>> CCS_INIT_S1(weight_chan);
  Connections::Out <DTYPE>                CCS_INIT_S1(bias_chan);
  Connections::Out <bool>                 CCS_INIT_S1(sync_chan);

  SC_CTOR(mem_buffer) {
    SC_THREAD(run_mem_buffer);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

  void run_mem_buffer() {
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();
    mem.reset();
    weight_chan.Reset();
    bias_chan.Reset();
    sync_chan.Reset();
    start.Reset();
    array_t<DTYPE,KSIZE> weights;
    for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
        weights.data[kr][kc] = 0;
      }
    }
    wait();
    while (1) {
      start.sync_in();
      OFM: for (OUT_FMAP_TYPE ofm=0; ; ofm++) { //output feature map
        mem_in_addr.Push(bias_offset.read() + ofm);
        mem_in_burst.Push(1-1);
        DTYPE bias = mem_in_data.Pop();
        bias_chan.Push(bias);
        IFM: for (IN_FMAP_TYPE ifm=0; ; ifm++) { //input feature map
          IN_FMAP_TYPE in_fmaps = num_in_fmaps.read();
          ac_int<IN_FMAP_TYPE::width+1,false> in_fmaps_p1 = in_fmaps+1;
          int ksqidx = pointwise.read() ? 1: KSIZESQ;
          unsigned int weight_idx = (weight_offset.read() + ofm*in_fmaps_p1*ksqidx + ifm*ksqidx).to_int();
          mem_in_addr.Push(weight_idx);
          mem_in_burst.Push(ksqidx-1);
          weights.Reset();
          if (pointwise.read()) {
            weights.data[0][0] = mem_in_data.Pop();
          } else {
            K_X0: for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
              K_Y0: for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
                weights.data[kr][kc] = mem_in_data.Pop();
              }
            }
          }
          weight_chan.Push(weights);
          //Burst a fmap of data
          HEIGHT_TYPE ht = height.read();
          HEIGHT_TYPE htp1 = ht+1;
          WIDTH_TYPE wd = width.read();
          HEIGHT_TYPE wdp1 = wd+1;
          unsigned int data_idx = read_offset.read() + ifm*htp1*wdp1;
          if (ofm == 0) {
            mem_in_addr.Push(data_idx);
            mem_in_burst.Push(htp1*wdp1-1);
          }
          ROW: for (HEIGHT_TYPE r=0; ; r++) { //process feature map
            COL: for (WIDTH_TYPE c=0; ; c++) {
              if (ofm == 0) { //Keep array reads in bounds
                int mem_idx = ifm*htp1*wdp1 + r*wdp1 + c;
                mem.write(mem_idx,mem_in_data.Pop());
              }
              if (c == wd) { break; }
            }
            if (r == ht) { break; }
          }
          if (ofm ==0) {
            sync_chan.Push(1);
          }
          if (ifm == in_fmaps) { break; }
        }
        if (ofm == num_out_fmaps.read()) { break; }
      }
    }
  }
};

class conv2d_core: public sc_module
{
public:
  sc_in<bool> clk;
  sc_in<bool> rstn;

  Connections::Out<MEM_ADDR_TYPE>    CCS_INIT_S1(mem_out_addr);
  Connections::Out<BURST_TYPE>       CCS_INIT_S1(mem_out_burst);
  Connections::Out<DTYPE>            CCS_INIT_S1(mem_out_data);
  #pragma hls_direct_input
  sc_in<IN_FMAP_TYPE>                CCS_INIT_S1(num_in_fmaps);
  #pragma hls_direct_input
  sc_in<OUT_FMAP_TYPE>               CCS_INIT_S1(num_out_fmaps);
  #pragma hls_direct_input
  sc_in<HEIGHT_TYPE>                 CCS_INIT_S1(height);
  #pragma hls_direct_input
  sc_in<WIDTH_TYPE>                  CCS_INIT_S1(width);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                 CCS_INIT_S1(read_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                 CCS_INIT_S1(write_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                 CCS_INIT_S1(weight_offset);
  #pragma hls_direct_input
  sc_in<BIAS_OFFSET_TYPE>            CCS_INIT_S1(bias_offset);
  #pragma hls_direct_input
  sc_in<bool>                        CCS_INIT_S1(pointwise); // true does 1x1 convolution
  #pragma hls_direct_input
  sc_in<bool>                        CCS_INIT_S1(relu); // enable/disable RELU
  #pragma hls_direct_input
  sc_in<uint2>                       CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2

  Connections::SyncOut               CCS_INIT_S1(done);
  DPRAM_model<>::rd_port<DTYPE,692224>   mem;
  Connections::In <array_t<DTYPE,KSIZE>> CCS_INIT_S1(weight_chan);
  Connections::In <DTYPE>                CCS_INIT_S1(bias_chan);
  Connections::In <bool>                 CCS_INIT_S1(sync_chan);

  ACC_TYPE acc_buf[MAX_HEIGHT][MAX_WIDTH];
  SC_CTOR(conv2d_core) {
    SC_THREAD(run_conv_core);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }


  void run_conv_core() {
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    mem.reset();
    weight_chan.Reset();
    bias_chan.Reset();
    sync_chan.Reset();
    bool s;
    done.Reset();
    ACC_TYPE acc = 0;
    array_t<DTYPE,KSIZE>  weights;
    for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
        weights.data[kr][kc] = 0;
      }
    }
    for (int i=0; i<3; i++) {
      data[i] = 0;
    }
    wait();
    while (1) {
      OFM: for (OUT_FMAP_TYPE ofm=0; ; ofm++) { //output feature map
        printf("Ofm %d\n",(int)ofm);
        DTYPE bias = bias_chan.Pop();
        IFM: for (IN_FMAP_TYPE ifm=0; ; ifm++) { //input feature map
          weights = weight_chan.Pop();
          if (ofm==0) {
            s = sync_chan.Pop(); (void)s; // return value unused
          }
          ROW: for (HEIGHT_TYPE r=0; ; r++) { //process feature map
            HEIGHT_TYPE ht = height.read();
            HEIGHT_TYPE htp1 = ht+1;
            COL: for (WIDTH_TYPE c=0; ; c++) {
              WIDTH_TYPE wd = width.read();
              HEIGHT_TYPE wdp1 = wd+1;
              acc = 0;
              int mem_idx = ifm*htp1*wdp1 + r*wdp1 + c;
              if (r != htp1 && c != wdp1) //Keep array reads in bounds
              { data[0] = mem.read(mem_idx); }
              #ifndef __SYNTHESIS__
              wait();
              #endif
              if (c != wdp1)
              { shift_line_buffers(c); }
              shift_registers();
              copy_window();
              zero_pad(r,c,htp1,wdp1);
              if (pointwise.read()) {
                acc =   window[1][1]*weights.data[0][0];
              } else {
                K_X: for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
                  K_Y: for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
                    DTYPE wdat = window[2-kr][2-kc];
                    acc +=  wdat*weights.data[kr][kc];//perform convolution against input fmap
                  }
                }
              }
              if (r !=0 && c != 0) {
                if (ifm==0) { // Clear the accum buffer
                  acc_buf[r-1][c-1] = acc;
                } else {
                  acc_buf[r-1][c-1] += acc; // sum current fmap activation across all input channels/feature maps
                }
              }
              if (c == wdp1) { break; }
            }
            if (r == htp1) { break; }
          }
          if (ifm == num_in_fmaps.read()) { break; }
        }

        // Write output feature map
        unsigned int out_idx = 0;
        int burst_size;
        uint2 pool_i = pass(pool.read()); // pass-through CCORE used to fix scheduling failure
        HEIGHT_TYPE ht = pass(height.read());
        HEIGHT_TYPE htp1 = ht+1;
        WIDTH_TYPE wd = pass(width.read());
        WIDTH_TYPE wdp1 = wd+1;
        bool relu_i = relu.read();
        wait();
        if (pool_i==0) { // no pooling
          out_idx = write_offset.read() + ofm*htp1*wdp1;
          burst_size = htp1*wdp1;
        } else if (pool_i==1) { // pool stride=1
          out_idx = write_offset.read() + ofm*htp1*wdp1;
          burst_size = htp1*wdp1;
        } else { // pool stride =2
          out_idx = write_offset.read()  + ofm*(htp1>>1)*(wdp1>>1);
          burst_size = (htp1>>1)*(wdp1>>1);
        }

        // Burst a row of data
        mem_out_addr.Push(out_idx);
        mem_out_burst.Push(burst_size-1);

        ROW_CPY: for (HEIGHT_TYPE r=0;; r++) { // feature map
          COL_CPY: for (WIDTH_TYPE c=0;; c++) {
            SAT_TYPE add_bias = 0;
            if (r < MAX_HEIGHT && c < MAX_WIDTH) {
              add_bias = acc_buf[r][c] + bias; // bias
            }
            if (relu_i) {
              if (add_bias < 0) {// RELU
                add_bias = add_bias * SAT_TYPE(0.1); 
              }
            }
            if (pool_i==0) {
              mem_out_data.Push(SAT_TYPE(add_bias));
            } else {
              DTYPE pdat =  maxpool_buffer[c];
              if ((r==htp1) | (c==wdp1)) { // if padding
                add_bias = 0;
                add_bias[DTYPE::width-1] = 1; // max neg val
              }
              pool_data[0] = add_bias;
              pool_data[1] = pdat;
              shift_pool_registers();
              get_max();
              if ((r!=0) & (c!=0)) {
                if (pool_i==1) {
                  mem_out_data.Push(max);
                } else if ((r&1) & (c&1)) { // stride 2  & (pool_i==2)
                  mem_out_data.Push(max);
                }
              }
              maxpool_buffer[c] = SAT_TYPE(add_bias);
            }
            #ifndef __SYNTHESIS__
            wait();//Model effect of rolled loop
            #endif
            if (c==wd + WIDTH_TYPE((pool_i==1)?1:0)) {
              break;
            }
          }
          if (r == HEIGHT_TYPE(ht + WIDTH_TYPE((pool_i==1)?1:0))) {
            break;
          }
        }
        if (ofm == num_out_fmaps.read()) { break; }
      }
      done.sync_out();//Send done to processor
    }
  }

  DTYPE data[3];
  DTYPE shift_regs[3][3];
  DTYPE window[3][3];
  DTYPE line_buffers[2][MAX_WIDTH];
  ACC_TYPE acc = 0;

  DTYPE pool_data[2];
  DTYPE pool_regs[2][2];
  DTYPE maxpool_buffer[MAX_WIDTH];
  DTYPE max;

  // copy the shift registers to the window array to apply zero padding
  void copy_window() {
#pragma hls_unroll yes
    for (int r=0; r<3; r++) {
#pragma hls_unroll yes
      for (int c=0; c<3; c++) {
        window[r][c] = shift_regs[r][c];
      }
    }
  }

  // shift the data vertically through the line buffer memories
  void shift_line_buffers(int x) {
#pragma hls_unroll yes
    for (int i = 0; i < 2; i++) {//read line buffers, slide the window down
      data[i + 1] = line_buffers[i][x];
    }
#pragma hls_unroll yes
    for (int i = 0; i < 2; i++) {//write the line buffers
      line_buffers[i][x] = data[i];
    }
  }

  // shift the data horizontally through the shift registers
  void shift_registers() {
#pragma hls_unroll yes
    for (int i = 0; i < 3; i++) {
      //shift the shift registers, slide the window to the right
#pragma hls_unroll yes
      for (int j = 2; j >= 0; j--) {
        if (j == 0)
        { shift_regs[i][0] = data[i]; }
        else
        { shift_regs[i][j] = shift_regs[i][j - 1]; }
      }
    }
  }

  // zero pad outside the image boundary
  void zero_pad(int y, int x, HEIGHT_TYPE height, WIDTH_TYPE width) {
    // Zero pad window when out of bounds
#pragma hls_unroll yes
    for (int i = 0; i < 3; i++) {
      if (y == 1) {
        window[2][i] = 0;
      } else if (y == height) {
        window[0][i] = 0;
      }

      if (x == 1) {
        window[i][2] = 0;
      } else if (x == width) {
        window[i][0] = 0;
      }
    }
  }

  // shift the data horizontally through the shift registers
  void shift_pool_registers() {
#pragma hls_unroll yes
    for (int i = 0; i < 2; i++) {
      //shift the shift registers, slide the window to the right
#pragma hls_unroll yes
      for (int j = 1; j >= 0; j--) {
        if (j == 0) {
          pool_regs[i][0] = pool_data[i];
        } else {
          pool_regs[i][j] = pool_regs[i][j - 1];
        }
      }
    }
  }

  // Find maximum of 2x2
  void get_max() {
    max = 0;
    max[DTYPE::width-1] = 1;//maximum negative value
    DTYPE max0,max1;
    if (pool_regs[0][0] > pool_regs[0][1]) {
      max0 = pool_regs[0][0];
    } else {
      max0 = pool_regs[0][1];
    }
    if (pool_regs[1][0] > pool_regs[1][1]) {
      max1 = pool_regs[1][0];
    } else {
      max1 = pool_regs[1][1];
    }
    if (max0 > max1) {
      max = max0;
    } else {
      max = max1;
    }
  }
};

class conv2d: public sc_module
{
public:
  sc_in<bool> clk;
  sc_in<bool> rstn;

  Connections::Out<MEM_ADDR_TYPE>            CCS_INIT_S1(mem_in_addr);
  Connections::Out<BURST_TYPE>               CCS_INIT_S1(mem_in_burst);
  Connections::In<DTYPE>                     CCS_INIT_S1(mem_in_data);
  Connections::Out<MEM_ADDR_TYPE>            CCS_INIT_S1(mem_out_addr);
  Connections::Out<BURST_TYPE>               CCS_INIT_S1(mem_out_burst);
  Connections::Out<DTYPE>                    CCS_INIT_S1(mem_out_data);
  #pragma hls_direct_input
  sc_in<IN_FMAP_TYPE>                        CCS_INIT_S1(num_in_fmaps);
  #pragma hls_direct_input
  sc_in<OUT_FMAP_TYPE>                       CCS_INIT_S1(num_out_fmaps);
  //#pragma hls_direct_input
  sc_in<HEIGHT_TYPE>                         CCS_INIT_S1(height);
  //#pragma hls_direct_input
  sc_in<WIDTH_TYPE>                          CCS_INIT_S1(width);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                         CCS_INIT_S1(read_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                         CCS_INIT_S1(write_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>                         CCS_INIT_S1(weight_offset);
  #pragma hls_direct_input
  sc_in<BIAS_OFFSET_TYPE>                    CCS_INIT_S1(bias_offset);
  #pragma hls_direct_input
  sc_in<bool>                                CCS_INIT_S1(pointwise); // true does 1x1 convolution
  #pragma hls_direct_input
  sc_in<bool>                                CCS_INIT_S1(relu); // enable/disable RELU
  #pragma hls_direct_input
  sc_in<uint2>                               CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2
  Connections::SyncIn                        CCS_INIT_S1(start);
  Connections::SyncOut                       CCS_INIT_S1(done);

  DPRAM_model<>::mem<DTYPE,692224>                   mem;
  Connections::Combinational <array_t<DTYPE,KSIZE>>  CCS_INIT_S1(weight_chan);
  Connections::Combinational <DTYPE>                 CCS_INIT_S1(bias_chan);
  Connections::Combinational <bool>                  CCS_INIT_S1(sync_chan);

  mem_buffer                                         CCS_INIT_S1(mem_buffer_inst);
  conv2d_core                                        CCS_INIT_S1(conv2d_core_inst);

  SC_CTOR(conv2d) {
    mem_buffer_inst.clk(clk);
    mem_buffer_inst.rstn(rstn);
    mem_buffer_inst.mem_in_addr(mem_in_addr);
    mem_buffer_inst.mem_in_burst(mem_in_burst);
    mem_buffer_inst.mem_in_data(mem_in_data);
    mem_buffer_inst.num_in_fmaps(num_in_fmaps);
    mem_buffer_inst.num_out_fmaps(num_out_fmaps);
    mem_buffer_inst.height(height);
    mem_buffer_inst.width(width);
    mem_buffer_inst.read_offset(read_offset);
    mem_buffer_inst.write_offset(write_offset);
    mem_buffer_inst.weight_offset(weight_offset);
    mem_buffer_inst.bias_offset(bias_offset);
    mem_buffer_inst.relu(relu);
    mem_buffer_inst.pointwise(pointwise);
    mem_buffer_inst.pool(pool);
    mem_buffer_inst.start(start);
    mem_buffer_inst.weight_chan(weight_chan);
    mem_buffer_inst.bias_chan(bias_chan);
    mem_buffer_inst.sync_chan(sync_chan);
    mem_buffer_inst.mem(mem);

    conv2d_core_inst.clk(clk);
    conv2d_core_inst.rstn(rstn);
    conv2d_core_inst.mem_out_addr(mem_out_addr);
    conv2d_core_inst.mem_out_burst(mem_out_burst);
    conv2d_core_inst.mem_out_data(mem_out_data);
    conv2d_core_inst.num_in_fmaps(num_in_fmaps);
    conv2d_core_inst.num_out_fmaps(num_out_fmaps);
    conv2d_core_inst.height(height);
    conv2d_core_inst.width(width);
    conv2d_core_inst.read_offset(read_offset);
    conv2d_core_inst.write_offset(write_offset);
    conv2d_core_inst.weight_offset(weight_offset);
    conv2d_core_inst.bias_offset(bias_offset);
    conv2d_core_inst.relu(relu);
    conv2d_core_inst.pointwise(pointwise);
    conv2d_core_inst.pool(pool);
    conv2d_core_inst.done(done);
    conv2d_core_inst.weight_chan(weight_chan);
    conv2d_core_inst.bias_chan(bias_chan);
    conv2d_core_inst.sync_chan(sync_chan);
    conv2d_core_inst.mem(mem);

    mem.WCLK(clk);
    mem.RCLK(clk);
  }
};
#endif


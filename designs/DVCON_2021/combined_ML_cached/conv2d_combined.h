/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.0                                                 *
 *                                                                        *
 *  Release Date    : Tue Feb 23 16:08:07 PST 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.0.0                                               *
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
#ifndef _INCLUDED_CONV2D_COMBINED_H_
#define _INCLUDED_CONV2D_COMBINED_H_

#include "types.h"
#include <ac_int.h>
#include <ac_sync.h>
#include "DPRAM.h"
#include "user_trans.h"

// Convolution with stride 1 and odd size kernel

#pragma hls_design top
class conv2d: public sc_module
{
public:
  sc_in<bool> clk;
  sc_in<bool> rstn;

  Connections::Out<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_in_addr);
  Connections::Out<BURST_TYPE>     CCS_INIT_S1(mem_in_burst);
  Connections::In<DTYPE>           CCS_INIT_S1(mem_in_data);
  Connections::Out<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_out_addr);
  Connections::Out<BURST_TYPE>     CCS_INIT_S1(mem_out_burst);
  Connections::Out<DTYPE>          CCS_INIT_S1(mem_out_data);
  #pragma hls_direct_input
  sc_in<IN_FMAP_TYPE>              CCS_INIT_S1(num_in_fmaps);
  #pragma hls_direct_input
  sc_in<OUT_FMAP_TYPE>             CCS_INIT_S1(num_out_fmaps);
  #pragma hls_direct_input
  sc_in<HEIGHT_TYPE>               CCS_INIT_S1(height);
  #pragma hls_direct_input
  sc_in<WIDTH_TYPE>                CCS_INIT_S1(width);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(read_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(write_offset);
  #pragma hls_direct_input
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(weight_offset);
  #pragma hls_direct_input
  sc_in<BIAS_OFFSET_TYPE>          CCS_INIT_S1(bias_offset);
  #pragma hls_direct_input
  sc_in<bool>                      CCS_INIT_S1(pointwise); // true does 1x1 convolution
  #pragma hls_direct_input
  sc_in<bool>                      CCS_INIT_S1(relu); // enable/disable RELU
  #pragma hls_direct_input
  sc_in<uint2>                     CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2
  Connections::SyncIn              CCS_INIT_S1(start);
  Connections::SyncOut             CCS_INIT_S1(done);

  ACC_TYPE acc_buf[MAX_HEIGHT][MAX_WIDTH];
  DPRAM_model<>::mem<DTYPE,692224> mem;

  Connections::Combinational <array_t<DTYPE,KSIZE>> CCS_INIT_S1(weight_chan);
  Connections::Combinational <DTYPE>                CCS_INIT_S1(bias_chan);
  Connections::SyncChannel                          CCS_INIT_S1(sync_chan);

  SC_CTOR(conv2d) {
    SC_THREAD(mem_buffer);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    SC_THREAD(conv);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
    mem.WCLK(clk);
    mem.RCLK(clk);
  }

  void mem_buffer() {
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();
    mem.reset_write();
    weight_chan.ResetWrite();
    bias_chan.ResetWrite();
    sync_chan.ResetWrite();
    start.Reset();
    DTYPE mem_data = 0;
    array_t<DTYPE,KSIZE> weights;
    ACC_TYPE acc = 0;
    for (int kr=0; kr<KSIZE; kr++) { // odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { // odd size kernel
        weights.data[kr][kc] = 0;
      }
    }
    wait();
    while (1) {
      start.sync_in();
      OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // output feature map
        mem_in_addr.Push(bias_offset.read() + ofm);
        mem_in_burst.Push(1);
        DTYPE bias = mem_in_data.Pop();
        bias_chan.Push(bias);
        IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // input feature map
          int ksqidx = pointwise.read() ? 1: KSIZESQ;
          unsigned int weight_idx = (weight_offset.read() + ofm*num_in_fmaps.read()*ksqidx + ifm*ksqidx).to_int();
          mem_in_addr.Push(weight_idx);
          mem_in_burst.Push(ksqidx);
          K_X0: for (int kr=0; kr<KSIZE; kr++) { // odd size kernel
            K_Y0: for (int kc=0; kc<KSIZE; kc++) { // odd size kernel
              weights.data[kr][kc] = mem_in_data.Pop();
              if (pointwise.read()) { break; }
            }
            if (pointwise.read()) { break; }
          }
          weight_chan.Push(weights);
          // Burst a fmap of data
          unsigned int data_idx = read_offset.read() + ifm*height.read()*width.read();
          if (ofm == 0) {
            mem_in_addr.Push(data_idx);
            mem_in_burst.Push(height.read()*width.read());
          }
          ROW: for (int r=0; r<MAX_HEIGHT; r++) { // process feature map
            COL: for (int c=0; c<MAX_WIDTH; c++) {
              if (ofm == 0) { // Keep array reads in bounds
                int mem_idx = ifm*height.read()*width.read() + r*width.read() + c;
                mem.write(mem_idx,mem_in_data.Pop());
              }
              if (c == width.read()-1) { break; }
            }
            if (r == height.read()-1) { break; }
          }
          if (ofm ==0) {
            sync_chan.SyncPush();
          }
          if (ifm == num_in_fmaps.read()-1) { break; }
        }
        if (ofm == num_out_fmaps.read()-1) { break; }
      }
    }
  }

  void conv() {
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    mem.reset_read();
    weight_chan.ResetRead();
    bias_chan.ResetRead();
    sync_chan.ResetRead();
    done.Reset();
    ACC_TYPE acc = 0;
    array_t<DTYPE,KSIZE>  weights;
    for (int kr=0; kr<KSIZE; kr++) { // odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { // odd size kernel
        weights.data[kr][kc] = 0;
      }
    }
    for (int i=0; i<3; i++) {
      data[i] = 0;
    }
    wait();
    while (1) {
      OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // output feature map
        printf("Ofm %d\n",ofm);
        DTYPE bias = bias_chan.Pop();
        IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // input feature map
          weights = weight_chan.Pop();
          if (ofm==0) {
            sync_chan.SyncPop();
          }
          ROW: for (int r=0; r<MAX_HEIGHT+1; r++) { // process feature map
            COL: for (int c=0; c<MAX_WIDTH+1; c++) {
              acc = 0;
              int mem_idx = ifm*height.read()*width.read() + r*width.read() + c;
              if (r != height.read() && c != width.read()) { // Keep array reads in bounds
                data[0] = mem.read(mem_idx);
              }
              #ifndef __SYNTHESIS__
              wait();
              #endif
              if (c != width.read()) { shift_line_buffers(c); }
              shift_registers();
              copy_window();
              zero_pad(r,c,height.read(),width.read());

              K_X: for (int kr=0; kr<KSIZE; kr++) { // odd size kernel
                K_Y: for (int kc=0; kc<KSIZE; kc++) { // odd size kernel
                  DTYPE wdat = pointwise.read() ? window[1][1]:window[2-kr][2-kc];
                  acc +=  wdat*weights.data[kr][kc]; // perform convolution against input fmap
                  if (pointwise.read()) { break; } // 1x1
                }
                if (pointwise.read()) { break; }
              }
              if ((r != 0) && (c != 0)) {
                if (ifm==0) { // Clear the accum buffer
                  acc_buf[r-1][c-1] = acc;
                } else {
                  acc_buf[r-1][c-1] += acc; // sum current fmap activation across all input channels/feature maps
                }
              }
              if (c == width.read()) { break; }
            }
            if (r == height.read()) { break; }
          }
          if (ifm == num_in_fmaps.read()-1) { break; }
        }
        // Write output feature map
        unsigned int out_idx = 0;
        int burst_size;
        DTYPE out_data = 0;
        bool send_data = false;
        uint2 pool_i = pool.read();
        WIDTH_TYPE width_i = width.read();
        HEIGHT_TYPE height_i = height.read();
        bool relu_i = relu.read();
        wait();
        if (pool_i==0) { // no pooling
          out_idx = write_offset.read() + ofm*height_i*width_i;
          burst_size = height_i*width_i;
          send_data = true;
        } else if (pool_i==1) { // pool stride=1
          out_idx = write_offset.read() + ofm*height_i*width_i;
          burst_size = height_i*width_i;
          send_data = true;
        } else if (pool_i==2) { // pool stride =2
          out_idx = write_offset.read()  + ofm*(height_i>>1)*(width_i>>1);
          burst_size = (height_i>>1)*(width_i>>1);
          send_data = true;
        }
        if (send_data) {
          // Burst a row of data
          mem_out_addr.Push(out_idx);
          mem_out_burst.Push(burst_size);
        }
        ROW_CPY: for (int r=0; r<MAX_HEIGHT+1; r++) { // feature map
          if (r == height_i + (pool_i==1)?1:0) { break; }
          COL_CPY: for (int c=0; c<MAX_WIDTH+1; c++) {
            if (c == width_i + (pool_i==1)?1:0) { break; }
            bool en = false;
            DTYPE mem_dat = 0;
            add_bias = acc_buf[r][c] + bias; // bias
            if (relu_i) {
              if (add_bias < 0) { add_bias = add_bias * SAT_TYPE(0.1); } // RELU
            }
            if (pool_i==0 && c < width_i) {
              en = true;
              mem_dat = SAT_TYPE(add_bias);
              // mem_out_data.Push(SAT_TYPE(add_bias));
            } else {
              DTYPE pdat =  maxpool_buffer[c];
              if ((r==height_i) | (c==width_i)) { // if padding
                add_bias = 0;
                add_bias[DTYPE::width-1] = 1; // max neg val
              }
              pool_data[0] = add_bias;
              pool_data[1] = pdat;
              shift_pool_registers();
              get_max();
              if ((r!=0) & (c!=0)) {
                if (pool_i==1) {
                  // mem_out_data.Push(max);
                  en = true;
                  mem_dat = max;
                } else if ((r&1) & (c&1) & (pool_i==2)) { // stride 2
                  // mem_out_data.Push(max);
                  en = true;
                  mem_dat = max;
                }
              }
              maxpool_buffer[c] = SAT_TYPE(add_bias);
            }
            if (en) { mem_out_data.Push(mem_dat); }
          }
        }
        if (ofm == num_out_fmaps.read()-1) { break; }
      }
      done.sync_out(); // Send done to processor
    }
  }

  SAT_TYPE add_bias;
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
#pragma unroll yes
    for (int r=0; r<3; r++) {
#pragma unroll yes
      for (int c=0; c<3; c++) {
        window[r][c] = shift_regs[r][c];
      }
    }
  }

  // shift the data vertically through the line buffer memories
  void shift_line_buffers(int x) {
#pragma unroll yes
    for (int i = 0; i < 2; i++) { // read line buffers, slide the window down
      data[i + 1] = line_buffers[i][x];
    }
#pragma unroll yes
    for (int i = 0; i < 2; i++) { // write the line buffers
      line_buffers[i][x] = data[i];
    }
  }

  // shift the data horizontally through the shift registers
  void shift_registers() {
#pragma unroll yes
    for (int i = 0; i < 3; i++) {
      // shift the shift registers, slide the window to the right
#pragma unroll yes
      for (int j = 2; j >= 0; j--) {
        if (j == 0) {
          shift_regs[i][0] = data[i];
        } else {
          shift_regs[i][j] = shift_regs[i][j - 1];
        }
      }
    }
  }

  // zero pad outside the image boundary
  void zero_pad(int y, int x, HEIGHT_TYPE height, WIDTH_TYPE width) {
    // Zero pad window when out of bounds
#pragma unroll yes
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
#pragma unroll yes
    for (int i = 0; i < 2; i++) {
      // shift the shift registers, slide the window to the right
#pragma unroll yes
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
    max[DTYPE::width-1] = 1; // maximum negative value
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
    /*
    #pragma unroll yes
    for (int i = 0; i < 2; i++) {
      // shift the shift registers, slide the window to the right
      #pragma unroll yes
      for (int j = 0; j <2; j++) {
        if (pool_regs[i][j]>max) { max = pool_regs[i][j]; }
      }
    }
    */
  }
};
#endif


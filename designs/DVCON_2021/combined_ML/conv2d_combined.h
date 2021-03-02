/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.0                                                 *
 *                                                                        *
 *  Release Date    : Tue Mar  2 08:54:51 PST 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.0.2                                               *
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
  sc_in<IN_FMAP_TYPE>              CCS_INIT_S1(num_in_fmaps);
  sc_in<OUT_FMAP_TYPE>             CCS_INIT_S1(num_out_fmaps);
  sc_in<HEIGHT_TYPE>               CCS_INIT_S1(height);
  sc_in<WIDTH_TYPE>                CCS_INIT_S1(width);
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(read_offset);
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(write_offset);
  sc_in<OFFSET_TYPE>               CCS_INIT_S1(weight_offset);
  sc_in<BIAS_OFFSET_TYPE>          CCS_INIT_S1(bias_offset);
  sc_in<bool>                      CCS_INIT_S1(pointwise); // True does 1x1 convolution
  sc_in<bool>                      CCS_INIT_S1(relu); // Enable/disable RELU
  sc_in<uint2>                     CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2
  Connections::SyncIn              CCS_INIT_S1(start);
  Connections::SyncOut             CCS_INIT_S1(done);

  ACC_TYPE acc_buf[MAX_HEIGHT][MAX_WIDTH];

  SC_CTOR(conv2d) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }

  void run() {
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    start.Reset();
    done.Reset();
    ACC_TYPE acc = 0;
    for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
        weights[kr][kc] = 0;
      }
    }
    for (int i=0; i<3; i++) {
      data[i] = 0;
    }
    wait();
    while (1) {
      start.sync_in();
      OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // Output feature map
        printf("Ofm %d\n",ofm);
        IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // Input feature map
          // Cache weights
          int ksqidx = pointwise.read() ? 1: KSIZESQ;
          unsigned int weight_idx = (weight_offset.read() + ofm*num_in_fmaps.read()*ksqidx + ifm*ksqidx).to_int();
          mem_in_addr.Push(weight_idx);
          mem_in_burst.Push(ksqidx);
          K_X0: for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
            K_Y0: for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
              weights[kr][kc] = mem_in_data.Pop();
              if (pointwise.read()) { break; }
            }
            if (pointwise.read()) { break; }
          }
          ROW: for (int r=0; r<MAX_HEIGHT+1; r++) { // Process feature map
            // Burst a row of data
            unsigned int data_idx = read_offset.read() + ifm*height.read()*width.read() + r*width.read();
            if (r != height.read()) {
              mem_in_addr.Push(data_idx);
              mem_in_burst.Push(width.read());
            }
            COL: for (int c=0; c<MAX_WIDTH+1; c++) {
              acc = 0;
              if (r != height.read() && c != width.read()) { // Keep array reads in bounds
                data[0] = mem_in_data.Pop();
              }
              if (c != width.read()) { shift_line_buffers(c); }
              shift_registers();
              copy_window();
              zero_pad(r,c,height.read(),width.read());

              K_X: for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
                K_Y: for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
                  DTYPE wdat = pointwise.read() ? window[1][1]:window[2-kr][2-kc];
                  acc +=  wdat*weights[kr][kc]; // Perform convolution against input fmap
                  if (pointwise.read()) { break; } // 1x1
                }
                if (pointwise.read()) { break; }
              }
              if ((r != 0) && (c != 0)) {
                if (ifm==0) { // Clear the accum buffer
                  acc_buf[r-1][c-1] = acc;
                } else {
                  acc_buf[r-1][c-1] += acc; // Sum current fmap activation across all input channels/feature maps
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
        mem_in_addr.Push(bias_offset.read() + ofm);
        mem_in_burst.Push(1);
        DTYPE bias = mem_in_data.Pop();
        ROW_CPY: for (int r=0; r<MAX_HEIGHT+1; r++) { // Feature map
          bool send_data = false;
          if (pool.read()==0) { // No pooling
            out_idx = write_offset.read() + ofm*height.read()*width.read() + r*width.read();
            burst_size = width.read();
            send_data = true;
          } else if (pool.read()==1 && r!=0) { // Pool stride=1
            out_idx = write_offset.read() + ofm*height.read()*width.read() + (r-1)*width.read();
            burst_size = width.read();
            send_data = true;
          } else if (r&1) { // Pool stride =2
            out_idx = write_offset.read()  + ofm*(height.read()>>1)*(width.read()>>1) + (r>>1)*(width.read()>>1);
            burst_size = (width.read()>>1);
            send_data = true;
          }
          if (send_data) {
            // Burst a row of data
            mem_out_addr.Push(out_idx);
            mem_out_burst.Push(burst_size);
          }
          COL_CPY: for (int c=0; c<MAX_WIDTH+1; c++) {
            if (c == width.read() + (pool.read()==1)?1:0) { break; }
            add_bias = acc_buf[r][c] + bias; // Bias
            if (relu) {
              if (add_bias < 0) { add_bias = add_bias * SAT_TYPE(0.1); } // ReLU
            }
            if (pool.read()==0 && c < width.read()) {
              mem_out_data.Push(SAT_TYPE(add_bias));
            } else {
              DTYPE pdat =  maxpool_buffer[c];
              if (r==height.read() || c==width.read()) { // If padding
                add_bias = 0;
                add_bias[DTYPE::width-1] = 1; // Max neg val
              }
              pool_data[0] = add_bias;
              pool_data[1] = pdat;
              shift_pool_registers();
              get_max();
              if (r!=0 && c!=0) {
                if (pool.read()==1) {
                  mem_out_data.Push(SAT_TYPE(max));
                } else if ((r&1) && (c&1) && pool.read()==2) { // Stride 2
                  mem_out_data.Push(SAT_TYPE(max));
                }
              }
              maxpool_buffer[c] = SAT_TYPE(add_bias);
            }

          }
          if (r == height.read()-1 + (pool.read()==1)?1:0) { break; }
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
  DTYPE weights[3][3];
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

  // Shift the data vertically through the line buffer memories
  void shift_line_buffers(int x) {
#pragma unroll yes
    for (int i = 0; i < 2; i++) { // Read line buffers, slide the window down
      data[i + 1] = line_buffers[i][x];
    }
#pragma unroll yes
    for (int i = 0; i < 2; i++) { // Write the line buffers
      line_buffers[i][x] = data[i];
    }
  }

  // Shift the data horizontally through the shift registers
  void shift_registers() {
#pragma unroll yes
    for (int i = 0; i < 3; i++) {
      // Shift the shift registers, slide the window to the right
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

  // Zero pad outside the image boundary
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

  // Shift the data horizontally through the shift registers
  void shift_pool_registers() {
#pragma unroll yes
    for (int i = 0; i < 2; i++) {
      // Shift the shift registers, slide the window to the right
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
    max[DTYPE::width-1] = 1; // Maximum negative value
#pragma unroll yes
    for (int i = 0; i < 2; i++) {
      // Shift the shift registers, slide the window to the right
#pragma unroll yes
      for (int j = 0; j <2; j++) {
        if (pool_regs[i][j]>max) { max = pool_regs[i][j]; }
      }
    }
  }
};
#endif


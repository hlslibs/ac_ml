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
#ifndef _INCLUDED_CONV2D_H_
#define _INCLUDED_CONV2D_H_

#include "types.h"
#include <ac_int.h>
#include <ac_sync.h>

// convolution with stride 1 and odd size kernel

#pragma hls_design top
class conv2d: public sc_module
{
public:
  sc_in<bool> clk;
  sc_in<bool> rstn;

  Connections::Out<MEM_ADDR_TYPE> CCS_INIT_S1(mem_in_addr);
  Connections::Out<BURST_TYPE>    CCS_INIT_S1(mem_in_burst);
  Connections::In<DTYPE>          CCS_INIT_S1(mem_in_data);
  Connections::Out<MEM_ADDR_TYPE> CCS_INIT_S1(mem_out_addr);
  Connections::Out<BURST_TYPE>    CCS_INIT_S1(mem_out_burst);
  Connections::Out<DTYPE>         CCS_INIT_S1(mem_out_data);
  sc_in<IN_FMAP_TYPE>             CCS_INIT_S1(num_in_fmaps);
  sc_in<OUT_FMAP_TYPE>            CCS_INIT_S1(num_out_fmaps);
  sc_in<HEIGHT_TYPE>              CCS_INIT_S1(height);
  sc_in<WIDTH_TYPE>               CCS_INIT_S1(width);
  sc_in<OFFSET_TYPE>              CCS_INIT_S1(read_offset);
  sc_in<OFFSET_TYPE>              CCS_INIT_S1(write_offset);
  sc_in<OFFSET_TYPE>              CCS_INIT_S1(weight_offset);
  Connections::SyncIn             CCS_INIT_S1(start);
  Connections::SyncOut            CCS_INIT_S1(done);

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
    for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
      for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
        weights[kr][kc] = 0;
      }
    }
    for (int i=0; i<3; i++) {
      data[i] = 0;
    }
    wait();
    while (1) {
      start.sync_in();
      OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { //output feature map
        IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { //input feature map
          //Cache 9 weights
          unsigned int weight_idx = (weight_offset.read() + ofm*num_in_fmaps.read()*KSIZESQ + ifm*KSIZESQ).to_int();
          mem_in_addr.Push(weight_idx);
          mem_in_burst.Push(9);
          K_X0: for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
            K_Y0: for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
              weights[kr][kc] = mem_in_data.Pop();
            }
          }
          ROW: for (int r=0; r<MAX_HEIGHT+1; r++) { //process feature map
            unsigned int data_idx = read_offset.read() + ifm*height.read()*width.read() + r*width.read();
            //Burst a row of data
            if (r != height.read()) {
              mem_in_addr.Push(data_idx);
              mem_in_burst.Push(width.read());
            }
            COL: for (int c=0; c<MAX_WIDTH+1; c++) {
              acc = 0;
              if (r != height.read() && c != width.read()) //Keep array reads in bounds
              { data[0] = mem_in_data.Pop(); }
              if (c != width.read())
              { shift_line_buffers(c); }
              shift_registers();
              copy_window();
              zero_pad(r,c,height.read(),width.read());

              K_X: for (int kr=0; kr<KSIZE; kr++) { //odd size kernel
                K_Y: for (int kc=0; kc<KSIZE; kc++) { //odd size kernel
                  acc +=  window[2-kr][2-kc]*weights[kr][kc]; // perform convolution against input fmap
                }
              }
              if ((r != 0) && (c != 0)) {
                if (ifm == 0) {
                  acc_buf[r-1][c-1] = acc; //clears acc buf on first ifmap
                } else {
                  acc_buf[r-1][c-1] += acc; //sum current fmap activation across all input channels/feature maps
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
        ROW_CPY: for (int r=0; r<MAX_HEIGHT; r++) { //feature map
          out_idx = write_offset.read() + ofm*height.read()*width.read() + r*width.read();
          //Burst a row of data
          mem_out_addr.Push(out_idx);
          mem_out_burst.Push(width.read());
          COL_CPY: for (int c=0; c<MAX_WIDTH; c++) {
            mem_out_data.Push(SAT_TYPE(acc_buf[r][c]));
            if (c == width.read()-1) { break; }
          }
          if (r == height.read()-1) { break; }
        }
        if (ofm == num_out_fmaps.read()-1) { break; }
      }

      done.sync_out(); // Send done to processor
    }
  }

  DTYPE data[3];
  DTYPE shift_regs[3][3];
  DTYPE window[3][3];
  DTYPE line_buffers[2][MAX_WIDTH];
  ACC_TYPE acc = 0;
  DTYPE weights[3][3];

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
    for (int i = 0; i < 2; i++) {//read line buffers, slide the window down
      data[i + 1] = line_buffers[i][x];
    }
#pragma unroll yes
    for (int i = 0; i < 2; i++) {//write the line buffers
      line_buffers[i][x] = data[i];
    }
  }

  // shift the data horizontally through the shift registers
  void shift_registers() {
#pragma unroll yes
    for (int i = 0; i < 3; i++) {
      //shift the shift registers, slide the window to the right
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
    //Zero pad window when out of bounds
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
};

#endif


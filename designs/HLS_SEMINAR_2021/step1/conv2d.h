/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.2                                                 *
 *                                                                        *
 *  Release Date    : Wed Jun 30 11:14:16 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.2.0                                               *
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
#ifndef _INCLUDED_CONV2D_H_
#define _INCLUDED_CONV2D_H_

#include "types.h"
#include <ac_int.h>
#include <ac_sync.h>

// Convolution with stride 1 and odd size kernel

#pragma hls_design top
class conv2d: public sc_module
{
public:
  // Design Interface
  sc_in<bool>                     clk;
  sc_in<bool>                     rstn;
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

protected:
  ACC_TYPE acc_buf[MAX_HEIGHT][MAX_WIDTH];

public:
  SC_CTOR(conv2d) {
    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
  }
  void run() {
    ACC_TYPE acc = 0;
    DTYPE data;
    mem_in_addr.Reset();
    mem_in_burst.Reset();
    mem_in_data.Reset();
    mem_out_addr.Reset();
    mem_out_burst.Reset();
    mem_out_data.Reset();
    start.Reset();
    done.Reset();
    wait();
    while (1) {
      start.sync_in();
      OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // Output feature map
        printf("OFM = %d\n",ofm);
        // Clear the accum buffer
        ROW_CLR: for (int r=0; r<MAX_HEIGHT; r++) { // Feature map
          COL_CLR: for (int c=0; c<MAX_WIDTH; c++) {
            acc_buf[r][c] = 0;
            if (c == width.read()-1) { break; }
          }
          if (r == height.read()-1) { break; }
        }
        IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // Input feature map
          ROW: for (int r=0; r<MAX_HEIGHT; r++) { // Process feature map
            COL: for (int c=0; c<MAX_WIDTH; c++) {
              acc = 0;
              K_X: for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
                K_Y: for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
                  int ridx = r + kr - KSIZE/2; // Compute indices based on filter kernel location
                  int cidx = c + kc - KSIZE/2;
                  if (ridx < 0 || ridx >= height.read() || cidx < 0 || cidx >= width.read()) { // Zero pad boundary when index out of bounds
                    data = 0;
                  } else {
                    unsigned int data_idx = read_offset.read() + ifm*height.read()*width.read() + ridx*width.read() + cidx;
                    mem_in_addr.Push(data_idx); // Send bus IF data read address
                    mem_in_burst.Push(1); // Can only burst 1, this is bad
                    data = mem_in_data.Pop(); // Read data from bus IF
                  }
                  unsigned int weight_idx = (weight_offset.read() + ofm*num_in_fmaps.read()*KSIZESQ + ifm*KSIZESQ + kr*KSIZE + kc).to_int();
                  mem_in_addr.Push(weight_idx); // Send bus IF weight read address
                  mem_in_burst.Push(1); // Can only burst 1, this is bad
                  acc += data*mem_in_data.Pop(); // Perform convolution against input fmap
                }
              }
              acc_buf[r][c] += acc; // Sum current fmap activation across all input channels/feature maps
              if (c == width.read()-1) { break; }
            }
            if (r == height.read()-1) { break; }
          }
          if (ifm == num_in_fmaps.read()-1) { break; }
        }

        // Write output feature map
        unsigned int out_idx = 0;
        ROW_CPY: for (int r=0; r<MAX_HEIGHT; r++) { // Feature map
          COL_CPY: for (int c=0; c<MAX_WIDTH; c++) {
            out_idx = write_offset.read() + ofm*height.read()*width.read() + r*width.read() + c;
            mem_out_addr.Push(out_idx);
            mem_out_burst.Push(1);
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
};
#endif


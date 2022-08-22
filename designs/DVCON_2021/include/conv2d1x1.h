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
#ifndef _INCLUDED_CONV2D1X1_H_
#define _INCLUDED_CONV2D1X1_H_

#include "types.h"
#include <ac_int.h>
#include <ac_sync.h>

template<typename DTYPE, int MAX_HEIGHT, int MAX_WIDTH, int IN_FMAP, int OUT_FMAP, int KSIZE>
class conv2d1x1;

#include <mc_scverify.h>

//convolution with stride 1 and odd size kernel
template<typename DTYPE, int MAX_HEIGHT, int MAX_WIDTH, int IN_FMAP, int OUT_FMAP, int KSIZE>
class conv2d1x1
{
public:
  // Compute internal data type widths
  enum {
    ACCU_MAX_WIDTH_TOT = DTYPE::width*2 + ac::nbits<KSIZE*KSIZE+IN_FMAP>::val,
    ACCU_MAX_WIDTH_INT = DTYPE::i_width*2 + ac::nbits<KSIZE*KSIZE+IN_FMAP>::val,
    HEIGHT_BITS = ac::nbits<MAX_HEIGHT>::val,
    WIDTH_BITS = ac::nbits<MAX_WIDTH>::val,
    IN_FMAP_BITS = ac::nbits<IN_FMAP>::val,
    OUT_FMAP_BITS = ac::nbits<OUT_FMAP>::val,
    IMG_SIZE = MAX_HEIGHT*MAX_WIDTH,          // Max feature map size
    KSIZESQ = KSIZE*KSIZE,                    // Number of kernel elements
    IF_KSIZE = KSIZESQ*IN_FMAP,               // Number of kernel elements per output channel
    MEM_SIZE = 19386656,                      // Storage size for input/output fmaps and kernels
    MEM_SIZE_BITS = ac::nbits<MEM_SIZE>::val,
    WEIGHT_OFFSET = IMG_SIZE*16 + IMG_SIZE*16/4
  };

  typedef ac_fixed<ACCU_MAX_WIDTH_TOT,ACCU_MAX_WIDTH_INT> ACC_TYPE;
  typedef ac_int<HEIGHT_BITS,false> HEIGHT_TYPE;
  typedef ac_int<WIDTH_BITS,false> WIDTH_TYPE;
  typedef ac_int<IN_FMAP_BITS,false> IN_FMAP_TYPE;
  typedef ac_int<OUT_FMAP_BITS,false> OUT_FMAP_TYPE;
  typedef ac_int<MEM_SIZE_BITS,false> OFFSET_TYPE;
  typedef ac_fixed<DTYPE::width,DTYPE::i_width,true,AC_RND,AC_SAT> SAT_TYPE;

  ACC_TYPE acc_buf[MAX_HEIGHT][MAX_WIDTH];

public:
  conv2d1x1() {}

  #pragma hls_design interface
  void CCS_BLOCK(run)(DTYPE mem_in[MEM_SIZE],
                      DTYPE mem_out[MEM_SIZE],
                      IN_FMAP_TYPE num_in_fmaps,
                      OUT_FMAP_TYPE num_out_fmaps,
                      HEIGHT_TYPE height,
                      WIDTH_TYPE width,
                      OFFSET_TYPE read_offset,
                      OFFSET_TYPE write_offset,
                      OFFSET_TYPE weight_offset) {
    ACC_TYPE acc = 0;

    DTYPE data;

    OFM: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // Output feature map
      // Clear the accum buffer
      ROW_CLR: for (int r=0; r<MAX_HEIGHT; r++) { // Feature map
        COL_CLR: for (int c=0; c<MAX_WIDTH; c++) {
          acc_buf[r][c] = 0;
          if (c == width-1) { break; }
        }
        if (r == height-1) { break; }
      }
      IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // Input feature map
        ROW: for (int r=0; r<MAX_HEIGHT; r++) { // Process feature map
          COL: for (int c=0; c<MAX_WIDTH; c++) {
            acc = 0;
            K_X: for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
              K_Y: for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
                int ridx = r + kr - KSIZE/2;   // Compute indices based on filter kernel location
                int cidx = c + kc - KSIZE/2;
                if (ridx < 0 || ridx >= height || cidx < 0 || cidx >= width) { // Zero pad boundary when index out of bounds
                  data = 0;
                } else {
                  unsigned int data_idx = read_offset + ifm*height*width + ridx*width + cidx;
                  data = mem_in[data_idx];
                }
                unsigned int weight_idx = (weight_offset + ofm*num_in_fmaps*KSIZESQ + ifm*KSIZESQ + kr*KSIZE + kc).to_int();
                acc += data*mem_in[weight_idx];  // Perform convolution against input fmap
              }
            }
            acc_buf[r][c] += acc; // Sum current fmap activation across all input channels/feature maps
            if (c == width-1) { break; }
          }
          if (r == height-1) { break; }
        }
        if (ifm == num_in_fmaps-1) { break; }
      }

      // Write output feature map
      unsigned int out_idx = 0;
      ROW_CPY: for (int r=0; r<MAX_HEIGHT; r++) { // Feature map
        COL_CPY: for (int c=0; c<MAX_WIDTH; c++) {
          out_idx = write_offset + ofm*height*width + r*width + c;
          mem_out[out_idx] = SAT_TYPE(acc_buf[r][c]);
          if (c == width-1) { break; }
        }
        if (r == height-1) { break; }
      }
      if (ofm == num_out_fmaps-1) { break; }
    }

  }
};
#endif


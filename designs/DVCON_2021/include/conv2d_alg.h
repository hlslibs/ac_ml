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
#ifndef _INCLUDED_CONV2D_ALG_H_
#define _INCLUDED_CONV2D_ALG_H_

#include "types.h"
#include <mc_scverify.h>

// Convolution with stride 1 and odd size kernel
template<int HEIGHT, int WIDTH, int IN_FMAP, int OUT_FMAP, int KSIZE>
class ac_conv2d_alg
{
public:
  ac_conv2d_alg() {}

  #pragma hls_design interface
  void CCS_BLOCK(run)(float in_fmaps[IN_FMAP][HEIGHT][WIDTH], float kernel[OUT_FMAP][IN_FMAP][KSIZE][KSIZE], float out_fmaps[OUT_FMAP][HEIGHT][WIDTH]) {
    float acc = 0;
    float acc_buf[HEIGHT][WIDTH];
    float data;

    OF: for (int ofm=0; ofm<OUT_FMAP; ofm++) { // Output feature map
      // Clear the accum buffer
      ROW_CLR: for (int r=0; r<HEIGHT; r++) { // Feature map
        COL_CLR: for (int c=0; c<WIDTH; c++) {
          acc_buf[r][c] = 0;
        }
      }
      IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // Input feature map
        ROW: for (int r=0; r<HEIGHT; r++) { // Process feature map
          COL: for (int c=0; c<WIDTH; c++) {
            acc = 0;
            K_X: for (int kr=0; kr<KSIZE; kr++) { // Odd size kernel
              K_Y: for (int kc=0; kc<KSIZE; kc++) { // Odd size kernel
                int ridx = r + kr - KSIZE/2; // Compute indices based on filter kernel location
                int cidx = c + kc - KSIZE/2;
                if (ridx < 0 || ridx >= HEIGHT || cidx < 0 || cidx >= WIDTH) { // Zero pad boundary when index out of bounds
                  data = 0;
                } else {
                  data = in_fmaps[ifm][ridx][cidx];
                }
                acc += data*kernel[ofm][ifm][kr][kc]; // Perform convolution against input fmap
              }
            }
            acc_buf[r][c] += acc; // Sum current fmap activation across all input channels/feature maps
          }
        }
      }
      // Write output feature map
      ROW_CPY: for (int r=0; r<HEIGHT; r++) { // Feature map
        COL_CPY: for (int c=0; c<WIDTH; c++) {
          out_fmaps[ofm][r][c] = acc_buf[r][c];
        }
      }
    }
  }
};
#endif


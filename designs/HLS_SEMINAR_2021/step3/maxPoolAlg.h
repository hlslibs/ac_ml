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
#ifndef _INCLUDED_MAXPOOLALG_H_
#define _INCLUDED_MAXPOOLALG_H_

#include "types.h"
#include <mc_scverify.h>

// convolution with stride 1 and odd size kernel
void maxPoolAlg(    IN_FMAP_TYPE num_in_fmaps,
                    HEIGHT_TYPE height,
                    WIDTH_TYPE width,
                    OFFSET_TYPE read_offset,
                    OFFSET_TYPE write_offset,
                    int stride)
{
  DTYPE data,max;
  int addr = 0;
  IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // input feature map
    ROW: for (int r=0; r<MAX_HEIGHT; r+=stride) { // process feature map
      COL: for (int c=0; c<MAX_WIDTH; c+=stride) {
        K_X: for (int kr=0; kr<2; kr++) { // odd size kernel
          K_Y: for (int kc=0; kc<2; kc++) { // odd size kernel
            if (r + kr >=height || c + kc >= width) {
              data = 0;
              data[DTYPE::width-1] = 1;
            } else {
              int idx = read_offset + ifm*height*width + (r+kr)*width + c+kc;
              wait(50);
              mem_in_addr_cpu.Push(idx);
              mem_in_burst_cpu.Push(1);
              data = mem_in_data_cpu.Pop();
            }
            if ((kr == 0 && kc == 0) || data > max) { max = data; }
          }
        }
        int idx = write_offset + addr;
        mem_out_addr_cpu.Push(idx);
        mem_out_burst_cpu.Push(1);
        mem_out_data_cpu.Push(max);
        addr++;
        if (c == width-stride) { break; }
      }
      if (r == height-stride) { break; }
    }
    if (ifm == num_in_fmaps-1) { break; }
  }
}
#endif


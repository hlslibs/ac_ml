/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.8                                                 *
 *                                                                        *
 *  Release Date    : Sun Jul 16 19:01:51 PDT 2023                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.8.0                                               *
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
#ifndef _INCLUDED_LEAKYRELUALG_H_
#define _INCLUDED_LEAKYRELUALG_H_

#include "types.h"
#include <mc_scverify.h>

void leakyReLuAlg(  IN_FMAP_TYPE num_in_fmaps,
                    HEIGHT_TYPE height,
                    WIDTH_TYPE width,
                    OFFSET_TYPE read_offset,
                    OFFSET_TYPE write_offset)
{
  SAT_TYPE data;

  IFM: for (int ifm=0; ifm<IN_FMAP; ifm++) { // input feature map
    ROW: for (int r=0; r<MAX_HEIGHT; r++) { // process feature map
      int idx = read_offset + ifm*height*width + r*width;
      mem_in_addr_cpu.Push(idx);
      mem_in_burst_cpu.Push(width);
      idx = write_offset + ifm*height*width + r*width;
      mem_out_addr_cpu.Push(idx);
      mem_out_burst_cpu.Push(width);
      COL: for (int c=0; c<MAX_WIDTH; c++) {
        data = mem_in_data_cpu.Pop();
        if (data < 0) { data = data * SAT_TYPE(0.1); }
        wait(50);
        mem_out_data_cpu.Push(data);
        if (c == width-1) { break; }
      }
      if (r == height-1) { break; }
    }
    if (ifm == num_in_fmaps-1) { break; }
  }
}
#endif


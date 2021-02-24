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
#ifndef _INCLUDED_BIASALG_H_
#define _INCLUDED_BIASALG_H_

#include "types.h"
#include <mc_scverify.h>

void biasAlg(
  DTYPE bias[IN_FMAP],
  IN_FMAP_TYPE num_in_fmaps,
  HEIGHT_TYPE height,
  WIDTH_TYPE width,
  OFFSET_TYPE read_offset,
  OFFSET_TYPE write_offset,
  int bias_offset
)
{
  DTYPE data,max;
  IFM:for (int ifm=0; ifm<IN_FMAP; ifm++) { // input feature map
    ROW:for (int r=0; r<MAX_HEIGHT; r++) { // process feature map
      int idx = read_offset + ifm*height*width + r*width;
      mem_in_addr_cpu.Push(idx);
      mem_in_burst_cpu.Push(width);
      idx = write_offset + ifm*height*width + r*width;
      mem_out_addr_cpu.Push(idx);
      mem_out_burst_cpu.Push(width);
      COL:for (int c=0; c<MAX_WIDTH; c++) {
        DTYPE dat = mem_in_data_cpu.Pop();
        SAT_TYPE d = dat + bias[bias_offset + ifm];
        wait(50);
        mem_out_data_cpu.Push(d);
        if (c == width-1) { break; }
      }
      if (r == height-1) { break; }
    }
    if (ifm == num_in_fmaps-1) { break; }
  }
}
#endif


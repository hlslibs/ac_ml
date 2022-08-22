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
#ifndef _INCLUDED_TYPES_H_
#define _INCLUDED_TYPES_H_

#include <ac_sysc_macros.h>
#include <ac_fixed.h>
#include <ac_sync.h>
#include <ac_channel.h>
#include <mc_connections.h>

typedef ac_fixed<16,7> WTYPE;
typedef ac_fixed<16,7> DTYPE;
typedef ac_fixed<16,7,true,AC_RND,AC_SAT> SAT_TYPE;

const int MAX_HEIGHT=416;
const int MAX_WIDTH=416;
const int IN_FMAP=1024;
const int OUT_FMAP=1024;
const int KSIZE=3;
const int KWIDTH=3;
const int NUM_PAR_OUTPUTS = 2;

enum {
  ACCU_MAX_WIDTH_TOT = DTYPE::width*2 + ac::nbits<KSIZE*KSIZE+IN_FMAP>::val,
  ACCU_MAX_WIDTH_INT = DTYPE::i_width*2 + ac::nbits<KSIZE*KSIZE+IN_FMAP>::val,
  HEIGHT_BITS = ac::nbits<MAX_HEIGHT>::val,
  WIDTH_BITS = ac::nbits<MAX_WIDTH>::val,
  IN_FMAP_BITS = ac::log2_ceil<IN_FMAP>::val,
  OUT_FMAP_BITS = ac::log2_ceil<OUT_FMAP>::val,
  IMG_SIZE = MAX_HEIGHT*MAX_WIDTH,//Max feature map size
  KSIZESQ = KSIZE*KSIZE,//number of kernel elements
  IF_KSIZE = KSIZESQ*IN_FMAP,//number of kernel elements per output channel
  MEM_SIZE = 20000000, //Storage size for input/output fmaps and kernels
  MEM_SIZE_BITS = ac::nbits<MEM_SIZE>::val,
  WEIGHT_OFFSET = IMG_SIZE*16 + IMG_SIZE*16/4,
  WEIGHT_SIZE = 3*16*9+16*32*9+32*64*9+64*128*9+128*256*9+256*512*9+512*1024*9+1024*1024*9+1024*125*1,
  MEM_OFFSET = IMG_SIZE*16/4,//TODO Recheck
  BIAS_SIZE = 16+32+64+128+256+512+1024+1024+125,
  BIAS_SIZE_BITS = ac::nbits<MEM_SIZE>::val,
  BIAS_OFFSET = WEIGHT_OFFSET+WEIGHT_SIZE,
  MEM_ADDR_BITS = ac::log2_ceil<MEM_SIZE>::val
};

typedef ac_fixed<ACCU_MAX_WIDTH_TOT,ACCU_MAX_WIDTH_INT>          ACC_TYPE;
typedef ac_int<HEIGHT_BITS,false>                                HEIGHT_TYPE;
typedef ac_int<WIDTH_BITS,false>                                 WIDTH_TYPE;
typedef ac_int<IN_FMAP_BITS,false>                               IN_FMAP_TYPE;
typedef ac_int<OUT_FMAP_BITS,false>                              OUT_FMAP_TYPE;
typedef ac_int<MEM_SIZE_BITS,false>                              OFFSET_TYPE;
typedef ac_fixed<DTYPE::width,DTYPE::i_width,true,AC_RND,AC_SAT> SAT_TYPE;
typedef ac_int<MEM_ADDR_BITS,false>                              MEM_ADDR_TYPE;
typedef ac_int<18,false>                                         BURST_TYPE;
typedef ac_int<BIAS_SIZE_BITS,false>                             BIAS_OFFSET_TYPE;

#endif


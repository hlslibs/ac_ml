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
/***************************************************************************  
 *  accelerator interface header                                              
 ***************************************************************************/ 
   
#define ACCEL_ADDR                         (0x60000000)

// register map 
   
#define WORD_SIZE (4)

#define GO_ADDR                            (ACCEL_ADDR + ( 0 * (WORD_SIZE))) 
#define GO_READY_ADDR                      (ACCEL_ADDR + ( 1 * (WORD_SIZE))) 
#define DONE_ADDR                          (ACCEL_ADDR + ( 2 * (WORD_SIZE))) 
#define DONE_VALID_ADDR                    (ACCEL_ADDR + ( 3 * (WORD_SIZE))) 
#define NUM_IN_FMAPS_ADDR                  (ACCEL_ADDR + ( 4 * (WORD_SIZE))) 
#define NUM_OUT_FMAPS_ADDR                 (ACCEL_ADDR + ( 5 * (WORD_SIZE))) 
#define HEIGHT_ADDR                        (ACCEL_ADDR + ( 6 * (WORD_SIZE))) 
#define WIDTH_ADDR                         (ACCEL_ADDR + ( 7 * (WORD_SIZE))) 
#define READ_OFFSET_ADDR                   (ACCEL_ADDR + ( 8 * (WORD_SIZE))) 
#define WRITE_OFFSET_ADDR                  (ACCEL_ADDR + ( 9 * (WORD_SIZE))) 
#define WEIGHT_OFFSET_ADDR                 (ACCEL_ADDR + (10 * (WORD_SIZE))) 
#define BIAS_OFFSET_ADDR                   (ACCEL_ADDR + (11 * (WORD_SIZE))) 
#define POINTWISE_ADDR                     (ACCEL_ADDR + (12 * (WORD_SIZE))) 
#define RELU_ADDR                          (ACCEL_ADDR + (13 * (WORD_SIZE))) 
#define POOL_ADDR                          (ACCEL_ADDR + (14 * (WORD_SIZE))) 
#define ADDR_OFFSET_LOW_ADDR               (ACCEL_ADDR + (15 * (WORD_SIZE))) 
#define ADDR_OFFSET_HIGH_ADDR              (ACCEL_ADDR + (16 * (WORD_SIZE))) 
#define BURST_SIZE_ADDR                    (ACCEL_ADDR + (17 * (WORD_SIZE))) 

#ifdef EMBEDDED

#define TB_READ(ADDR)                     (*((volatile unsigned int *) (ADDR))) 
#define TB_WRITE(ADDR, DATA)              *((volatile unsigned int *) (ADDR)) = (DATA)

#define REG_READ(ADDR)                    (*((volatile unsigned int *) (ADDR)))
#define REG_WRITE(ADDR, DATA)             *((volatile unsigned int *) (ADDR)) = (DATA)

#else // HOST SystemC/Matchlib

#define TB_READ(ADDR)                     (r_master.single_read(ADDR).data)
#define TB_WRITE(ADDR, DATA)              (w_master.single_write((ADDR), (DATA)))

#define REG_READ(ADDR)                    (r_master.single_read(ADDR).data)
#define REG_WRITE(ADDR, DATA)             (w_master.single_write((ADDR), (DATA)))

#endif

#define GO                                { while (0 == TB_READ(GO_READY_ADDR)); TB_WRITE(GO_ADDR, 1); } 
#define WAIT_FOR_DONE                     { while (0 == TB_READ(DONE_VALID_ADDR)); (volatile void) TB_READ(DONE_ADDR); } 
#define SET_NUM_IN_FMAPS(X)               { REG_WRITE(NUM_IN_FMAPS_ADDR, X); } 
#define SET_NUM_OUT_FMAPS(X)              { REG_WRITE(NUM_OUT_FMAPS_ADDR, X); } 
#define SET_HEIGHT(X)                     { REG_WRITE(HEIGHT_ADDR, X); } 
#define SET_WIDTH(X)                      { REG_WRITE(WIDTH_ADDR, X); } 
#define SET_READ_OFFSET(X)                { REG_WRITE(READ_OFFSET_ADDR, X); } 
#define SET_WRITE_OFFSET(X)               { REG_WRITE(WRITE_OFFSET_ADDR, X); } 
#define SET_WEIGHT_OFFSET(X)              { REG_WRITE(WEIGHT_OFFSET_ADDR, X); } 
#define SET_BIAS_OFFSET(X)                { REG_WRITE(BIAS_OFFSET_ADDR, X); } 
#define SET_POINTWISE(X)                  { REG_WRITE(POINTWISE_ADDR, X); } 
#define SET_RELU(X)                       { REG_WRITE(RELU_ADDR, X); } 
#define SET_POOL(X)                       { REG_WRITE(POOL_ADDR, X); } 


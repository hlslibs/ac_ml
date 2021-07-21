/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.4                                                 *
 *                                                                        *
 *  Release Date    : Wed Jul 21 10:23:21 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.4.0                                               *
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
#ifndef _INCLUDED_ACCELERATOR_H_
#define _INCLUDED_ACCELERATOR_H_

#include "bus_interface.h"
#include "conv2d_combined.h"

#include <mc_scverify.h>

#pragma hls_design top
class accelerator : public sc_module, public local_axi
{
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rstn);
  // Bus I/Fs
  r_master<> CCS_INIT_S1(r_master0);
  w_master<> CCS_INIT_S1(w_master0);
  typename local_axi4_lite::write::template slave<> CCS_INIT_S1(w_slave0);
  // User side
  // Read
  Connections::Combinational<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_in_addr);
  Connections::Combinational<BURST_TYPE>     CCS_INIT_S1(mem_in_burst);
  Connections::Combinational<DTYPE>         CCS_INIT_S1(mem_in_data);
  // Write
  Connections::Combinational<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_out_addr);
  Connections::Combinational<BURST_TYPE>     CCS_INIT_S1(mem_out_burst);
  Connections::Combinational<DTYPE>          CCS_INIT_S1(mem_out_data);

  Connections::SyncIn CCS_INIT_S1(start);
  Connections::SyncOut CCS_INIT_S1(done);

  // Accelerator config
  sc_signal<IN_FMAP_TYPE>              CCS_INIT_S1(num_in_fmaps);
  sc_signal<OUT_FMAP_TYPE>             CCS_INIT_S1(num_out_fmaps);
  sc_signal<HEIGHT_TYPE>               CCS_INIT_S1(height);
  sc_signal<WIDTH_TYPE>                CCS_INIT_S1(width);
  sc_signal<OFFSET_TYPE>               CCS_INIT_S1(read_offset);
  sc_signal<OFFSET_TYPE>               CCS_INIT_S1(write_offset);
  sc_signal<OFFSET_TYPE>               CCS_INIT_S1(weight_offset);
  sc_signal<BIAS_OFFSET_TYPE>          CCS_INIT_S1(bias_offset);
  sc_signal<bool>                      CCS_INIT_S1(pointwise); // true does 1x1 convolution
  sc_signal<bool>                      CCS_INIT_S1(relu); // enable/disable RELU
  sc_signal<uint2>                     CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2

  bus_interface CCS_INIT_S1(bus_if_inst);
  //CCS_DESIGN(conv2d) CCS_INIT_S1(conv2d_inst);
  conv2d CCS_INIT_S1(conv2d_inst);
  SC_CTOR(accelerator) {
    bus_if_inst.clk(clk);
    bus_if_inst.rstn(rstn);
    bus_if_inst.r_master0(r_master0);
    bus_if_inst.w_master0(w_master0);
    bus_if_inst.w_slave0(w_slave0);
    bus_if_inst.mem_in_addr(mem_in_addr);
    bus_if_inst.mem_in_burst(mem_in_burst);
    bus_if_inst.mem_in_data(mem_in_data);
    bus_if_inst.mem_out_addr(mem_out_addr);
    bus_if_inst.mem_out_burst(mem_out_burst);
    bus_if_inst.mem_out_data(mem_out_data);
    bus_if_inst.num_in_fmaps(num_in_fmaps);
    bus_if_inst.num_out_fmaps(num_out_fmaps);
    bus_if_inst.height(height);
    bus_if_inst.width(width);
    bus_if_inst.read_offset(read_offset);
    bus_if_inst.write_offset(write_offset);
    bus_if_inst.weight_offset(weight_offset);
    bus_if_inst.bias_offset(bias_offset);
    bus_if_inst.relu(relu);
    bus_if_inst.pointwise(pointwise);
    bus_if_inst.pool(pool);

    conv2d_inst.clk(clk);
    conv2d_inst.rstn(rstn);
    conv2d_inst.mem_in_addr(mem_in_addr);
    conv2d_inst.mem_in_burst(mem_in_burst);
    conv2d_inst.mem_in_data(mem_in_data);
    conv2d_inst.mem_out_addr(mem_out_addr);
    conv2d_inst.mem_out_burst(mem_out_burst);
    conv2d_inst.mem_out_data(mem_out_data);
    conv2d_inst.num_in_fmaps(num_in_fmaps);
    conv2d_inst.num_out_fmaps(num_out_fmaps);
    conv2d_inst.height(height);
    conv2d_inst.width(width);
    conv2d_inst.read_offset(read_offset);
    conv2d_inst.write_offset(write_offset);
    conv2d_inst.weight_offset(weight_offset);
    conv2d_inst.bias_offset(bias_offset);
    conv2d_inst.relu(relu);
    conv2d_inst.pointwise(pointwise);
    conv2d_inst.pool(pool);
    conv2d_inst.start(start);
    conv2d_inst.done(done);
  }
};
#endif


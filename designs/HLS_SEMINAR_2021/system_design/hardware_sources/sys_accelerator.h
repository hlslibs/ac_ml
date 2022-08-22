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
#ifndef __ACCEL__
#define __ACCEL__

#include "bus_master_if.h"
#include "conv2d_combined.h"
#include <mc_scverify.h>
// #pragma hls_design top

class sys_accelerator : public sc_module, public sysbus_axi {
 public:

    //== Ports 

    sc_in<bool>                      CCS_INIT_S1(clk);
    sc_in<bool>                      CCS_INIT_S1(rstn);

    r_master<>                       CCS_INIT_S1(r_master0);
    w_master<>                       CCS_INIT_S1(w_master0);

    Connections::SyncIn              CCS_INIT_S1(start);
    Connections::SyncOut             CCS_INIT_S1(done);

      //Accelerator configuration signals

    sc_in<IN_FMAP_TYPE>              CCS_INIT_S1(num_in_fmaps);
    sc_in<OUT_FMAP_TYPE>             CCS_INIT_S1(num_out_fmaps);
    sc_in<HEIGHT_TYPE>               CCS_INIT_S1(height);
    sc_in<WIDTH_TYPE>                CCS_INIT_S1(width);
    sc_in<OFFSET_TYPE>               CCS_INIT_S1(read_offset);
    sc_in<OFFSET_TYPE>               CCS_INIT_S1(write_offset);
    sc_in<OFFSET_TYPE>               CCS_INIT_S1(weight_offset);
    sc_in<BIAS_OFFSET_TYPE>          CCS_INIT_S1(bias_offset);
    sc_in<bool>                      CCS_INIT_S1(pointwise); // true does 1x1 convolution
    sc_in<bool>                      CCS_INIT_S1(relu); // enable/disable RELU
    sc_in<uint2>                     CCS_INIT_S1(pool); // Max pooling, 1=stride 1,2=stride 2
  
    //== Local signals

    Connections::Combinational<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_in_addr);
    Connections::Combinational<BURST_TYPE>     CCS_INIT_S1(mem_in_burst);
    Connections::Combinational<DTYPE>          CCS_INIT_S1(mem_in_data);

    Connections::Combinational<MEM_ADDR_TYPE>  CCS_INIT_S1(mem_out_addr);
    Connections::Combinational<BURST_TYPE>     CCS_INIT_S1(mem_out_burst);
    Connections::Combinational<DTYPE>          CCS_INIT_S1(mem_out_data);

    //== Instances

    bus_master_if                    CCS_INIT_S1(bus_if_inst);
    CCS_DESIGN(conv2d)               CCS_INIT_S1(conv2d_inst);

    //== Constructor

    SC_CTOR(sys_accelerator) {
        bus_if_inst.clk              (clk);
        bus_if_inst.rstn             (rstn);
        bus_if_inst.r_master0        (r_master0);
        bus_if_inst.w_master0        (w_master0);
        bus_if_inst.mem_in_addr      (mem_in_addr);
        bus_if_inst.mem_in_burst     (mem_in_burst);
        bus_if_inst.mem_in_data      (mem_in_data);
        bus_if_inst.mem_out_addr     (mem_out_addr);
        bus_if_inst.mem_out_burst    (mem_out_burst);
        bus_if_inst.mem_out_data     (mem_out_data);
    
        conv2d_inst.clk              (clk);
        conv2d_inst.rstn             (rstn);
        conv2d_inst.mem_in_addr      (mem_in_addr);
        conv2d_inst.mem_in_burst     (mem_in_burst);
        conv2d_inst.mem_in_data      (mem_in_data);
        conv2d_inst.mem_out_addr     (mem_out_addr);
        conv2d_inst.mem_out_burst    (mem_out_burst);
        conv2d_inst.mem_out_data     (mem_out_data);
        conv2d_inst.num_in_fmaps     (num_in_fmaps);
        conv2d_inst.num_out_fmaps    (num_out_fmaps);
        conv2d_inst.height           (height);
        conv2d_inst.width            (width);
        conv2d_inst.read_offset      (read_offset);
        conv2d_inst.write_offset     (write_offset);
        conv2d_inst.weight_offset    (weight_offset);
        conv2d_inst.bias_offset      (bias_offset);
        conv2d_inst.relu             (relu);
        conv2d_inst.pointwise        (pointwise);
        conv2d_inst.pool             (pool);
        conv2d_inst.start            (start);
        conv2d_inst.done             (done);
    }
};
#endif

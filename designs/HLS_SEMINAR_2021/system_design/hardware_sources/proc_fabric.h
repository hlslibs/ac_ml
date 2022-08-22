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
#pragma once
#include "axi/axi4.h"
#include "axi4_segment.h"
#include "sysbus_axi_struct.h"
#include "axi/AxiSplitter.h"
#include "axi/AxiArbiter.h"


typedef axi::axi4_segment<sysbus_axi4_config> sysbus_axi;

class fabric : public sc_module, public sysbus_axi {
public:
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);

  // ports to the slaves
  //  shared memory from               0x70000000 to 0x80000000
  //  register bank for accelerator at 0x60000000 to 0x60010000
  //  UART at                          0x60080000 to 0x60090000

  r_master<> CCS_INIT_S1(r_mem);
  w_master<> CCS_INIT_S1(w_mem);
  r_master<> CCS_INIT_S1(r_reg);
  w_master<> CCS_INIT_S1(w_reg);
  r_master<> CCS_INIT_S1(r_uart);
  w_master<> CCS_INIT_S1(w_uart);

  // ports to the masters
  //  master 0 = cpu
  //  master 1 = accelerator

  r_slave<>  CCS_INIT_S1(r_cpu);
  w_slave<>  CCS_INIT_S1(w_cpu);
  r_slave<>  CCS_INIT_S1(r_acc);
  w_slave<>  CCS_INIT_S1(w_acc);
  
  static const int numAddrBitsToInspect = 32;

  static const int numSlaves            =  3;
  static const int numMasters           =  2;

  sc_signal<NVUINTW(numAddrBitsToInspect)> addrBound[numSlaves][2];

  AxiSplitter<sysbus_axi4_config, numSlaves, numAddrBitsToInspect, false, true> CCS_INIT_S1(cpu_router);
  AxiSplitter<sysbus_axi4_config, numSlaves, numAddrBitsToInspect, false, true> CCS_INIT_S1(acc_router);

  AxiArbiter<sysbus_axi4_config, 2, 4> CCS_INIT_S1(mem_arbiter);
  AxiArbiter<sysbus_axi4_config, 2, 4> CCS_INIT_S1(reg_arbiter);
  AxiArbiter<sysbus_axi4_config, 2, 4> CCS_INIT_S1(uart_arbiter);
  
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_cpu2reg);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_cpu2reg);
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_cpu2mem);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_cpu2mem);
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_cpu2uart);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_cpu2uart);
  
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_acc2reg);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_acc2reg);
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_acc2mem);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_acc2mem);
  typename axi::axi4<sysbus_axi4_config>::read::template chan<>  CCS_INIT_S1(r_acc2uart);
  typename axi::axi4<sysbus_axi4_config>::write::template chan<> CCS_INIT_S1(w_acc2uart);

  SC_CTOR(fabric)
  {
    addrBound[0][0] = 0x70000000;
    addrBound[0][1] = 0x7fffffff;

    addrBound[1][0] = 0x60000000;
    addrBound[1][1] = 0x6000ffff;

    addrBound[2][0] = 0x60080000;
    addrBound[2][1] = 0x6008ffff;


    cpu_router.clk(clk);
    cpu_router.reset_bar(rst_bar);

    cpu_router.addrBound[0][0](addrBound[0][0]);
    cpu_router.addrBound[0][1](addrBound[0][1]);
    cpu_router.addrBound[1][0](addrBound[1][0]);
    cpu_router.addrBound[1][1](addrBound[1][1]);
    cpu_router.addrBound[2][0](addrBound[2][0]);
    cpu_router.addrBound[2][1](addrBound[2][1]);

    cpu_router.axi_rd_m(r_cpu);
    cpu_router.axi_wr_m(w_cpu);

    cpu_router.axi_rd_s_ar[0](r_cpu2mem.ar);
    cpu_router.axi_rd_s_r[0] (r_cpu2mem.r);
    cpu_router.axi_wr_s_aw[0](w_cpu2mem.aw);
    cpu_router.axi_wr_s_w[0] (w_cpu2mem.w);
    cpu_router.axi_wr_s_b[0] (w_cpu2mem.b);

    cpu_router.axi_rd_s_ar[1](r_cpu2reg.ar);
    cpu_router.axi_rd_s_r[1] (r_cpu2reg.r);
    cpu_router.axi_wr_s_aw[1](w_cpu2reg.aw);
    cpu_router.axi_wr_s_w[1] (w_cpu2reg.w);
    cpu_router.axi_wr_s_b[1] (w_cpu2reg.b);

    cpu_router.axi_rd_s_ar[2](r_cpu2uart.ar);
    cpu_router.axi_rd_s_r[2] (r_cpu2uart.r);
    cpu_router.axi_wr_s_aw[2](w_cpu2uart.aw);
    cpu_router.axi_wr_s_w[2] (w_cpu2uart.w);
    cpu_router.axi_wr_s_b[2] (w_cpu2uart.b);


    acc_router.clk(clk);
    acc_router.reset_bar(rst_bar);

    acc_router.addrBound[0][0](addrBound[0][0]);
    acc_router.addrBound[0][1](addrBound[0][1]);
    acc_router.addrBound[1][0](addrBound[1][0]);
    acc_router.addrBound[1][1](addrBound[1][1]);
    acc_router.addrBound[2][0](addrBound[1][0]);
    acc_router.addrBound[2][1](addrBound[1][1]);

    acc_router.axi_rd_m(r_acc);
    acc_router.axi_wr_m(w_acc);

    acc_router.axi_rd_s_ar[0](r_acc2mem.ar);
    acc_router.axi_rd_s_r[0] (r_acc2mem.r);
    acc_router.axi_wr_s_aw[0](w_acc2mem.aw);
    acc_router.axi_wr_s_w[0] (w_acc2mem.w);
    acc_router.axi_wr_s_b[0] (w_acc2mem.b);

    acc_router.axi_rd_s_ar[1](r_acc2reg.ar);
    acc_router.axi_rd_s_r[1] (r_acc2reg.r);
    acc_router.axi_wr_s_aw[1](w_acc2reg.aw);
    acc_router.axi_wr_s_w[1] (w_acc2reg.w);
    acc_router.axi_wr_s_b[1] (w_acc2reg.b);

    acc_router.axi_rd_s_ar[2](r_acc2uart.ar);
    acc_router.axi_rd_s_r[2] (r_acc2uart.r);
    acc_router.axi_wr_s_aw[2](w_acc2uart.aw);
    acc_router.axi_wr_s_w[2] (w_acc2uart.w);
    acc_router.axi_wr_s_b[2] (w_acc2uart.b);


    mem_arbiter.clk(clk);
    mem_arbiter.reset_bar(rst_bar);

    mem_arbiter.axi_rd_m_ar[0](r_cpu2mem.ar);
    mem_arbiter.axi_rd_m_r[0] (r_cpu2mem.r);
    mem_arbiter.axi_wr_m_aw[0](w_cpu2mem.aw);
    mem_arbiter.axi_wr_m_w[0] (w_cpu2mem.w);
    mem_arbiter.axi_wr_m_b[0] (w_cpu2mem.b);
    
    mem_arbiter.axi_rd_m_ar[1](r_acc2mem.ar);
    mem_arbiter.axi_rd_m_r[1] (r_acc2mem.r);
    mem_arbiter.axi_wr_m_aw[1](w_acc2mem.aw);
    mem_arbiter.axi_wr_m_w[1] (w_acc2mem.w);
    mem_arbiter.axi_wr_m_b[1] (w_acc2mem.b);
    
    mem_arbiter.axi_rd_s(r_mem);
    mem_arbiter.axi_wr_s(w_mem);
    
    
    reg_arbiter.clk(clk);
    reg_arbiter.reset_bar(rst_bar);

    reg_arbiter.axi_rd_m_ar[0](r_cpu2reg.ar);
    reg_arbiter.axi_rd_m_r[0] (r_cpu2reg.r);
    reg_arbiter.axi_wr_m_aw[0](w_cpu2reg.aw);
    reg_arbiter.axi_wr_m_w[0] (w_cpu2reg.w);
    reg_arbiter.axi_wr_m_b[0] (w_cpu2reg.b);
    
    reg_arbiter.axi_rd_m_ar[1](r_acc2reg.ar);
    reg_arbiter.axi_rd_m_r[1] (r_acc2reg.r);
    reg_arbiter.axi_wr_m_aw[1](w_acc2reg.aw);
    reg_arbiter.axi_wr_m_w[1] (w_acc2reg.w);
    reg_arbiter.axi_wr_m_b[1] (w_acc2reg.b);
    
    reg_arbiter.axi_rd_s(r_reg);
    reg_arbiter.axi_wr_s(w_reg);


    uart_arbiter.clk(clk);
    uart_arbiter.reset_bar(rst_bar);

    uart_arbiter.axi_rd_m_ar[0](r_cpu2uart.ar);
    uart_arbiter.axi_rd_m_r[0] (r_cpu2uart.r);
    uart_arbiter.axi_wr_m_aw[0](w_cpu2uart.aw);
    uart_arbiter.axi_wr_m_w[0] (w_cpu2uart.w);
    uart_arbiter.axi_wr_m_b[0] (w_cpu2uart.b);
    
    uart_arbiter.axi_rd_m_ar[1](r_acc2uart.ar);
    uart_arbiter.axi_rd_m_r[1] (r_acc2uart.r);
    uart_arbiter.axi_wr_m_aw[1](w_acc2uart.aw);
    uart_arbiter.axi_wr_m_w[1] (w_acc2uart.w);
    uart_arbiter.axi_wr_m_b[1] (w_acc2uart.b);
    
    uart_arbiter.axi_rd_s(r_uart);
    uart_arbiter.axi_wr_s(w_uart);
  }
};

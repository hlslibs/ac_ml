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
#include "axi4_segment.h"
#include "conv2d_combined.h"
#include "sysbus_axi_struct.h"

#include "terminal_class.h"

 // register map

#define CHAR_OUT                   0
#define CHAR_IN                    1
#define CHAR_READY                 2

#define SHIFT 2

class uart_if : public sc_module, public sysbus_axi {
public:

  //== Ports

  sc_in<bool>   CCS_INIT_S1(clk);
  sc_in<bool>   CCS_INIT_S1(rst_bar);

  r_slave<>     CCS_INIT_S1(r_uart_if);
  w_slave<>     CCS_INIT_S1(w_uart_if);

  //== Instances

  terminal      screen;

  //== Local signals

  bool chatty = false;

  // interface signals

  sc_signal<ac_int<8 , false> >     char_in;

  sc_uint<64>                       register_bank[256];
  const int                         num_regs    = 256;

  void reg_reader()
  {
    ar_payload ar;
    r_payload  r;
    int        reg_addr;

    r_uart_if.reset();

    wait();

    while (1) {
      r_uart_if.start_multi_read(ar);
      reg_addr = ar.addr >> SHIFT;
      if (chatty) printf("Read from register: %d \n", reg_addr);
      while (1) {
        if (reg_addr > num_regs) {
          SC_REPORT_ERROR("uart_if", "invalid addr");
          r.resp = Enc::XRESP::SLVERR;
        } else {
          r.data = register_bank[reg_addr].to_int64();
          r.resp = Enc::XRESP::OKAY;
        }
        if (!r_uart_if.next_multi_read(ar, r)) break;
      } 

      if (reg_addr == CHAR_IN) {
          unsigned char key_press = screen.get_key();
          register_bank[CHAR_IN] = key_press;
          r.data = key_press;
      }

      register_bank[CHAR_READY] = screen.key_ready(); 

      wait();
    }
  }

  void reg_writer() 
  {
    int     reg_addr;

    w_uart_if.reset();

    wait();

    while(1) {
      aw_payload  aw;
      w_payload   w;
      b_payload   b;

      if (w_uart_if.aw.PopNB(aw)) {

        w = w_uart_if.w.Pop();

        reg_addr = aw.addr >> SHIFT;

        if (chatty) printf("Writing to register: %d = 0x%08x \n", reg_addr, w.data.to_int64());  

        if (reg_addr > num_regs) {
          SC_REPORT_ERROR("uart_if", "invalid addr");
          b.resp = Enc::XRESP::SLVERR;
        } else {
          register_bank[reg_addr] = w.data.to_uint64();
	  b.resp = Enc::XRESP::OKAY;
          b.id   = aw.id;
	}

        w_uart_if.b.Push(b);
      
        if (reg_addr == CHAR_OUT) {
           screen.send_char(w.data.to_uint64());
        } 
      } 

      wait();
    }
  }

  SC_CTOR(uart_if)
  {
    SC_THREAD(reg_reader);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(reg_writer);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }
};

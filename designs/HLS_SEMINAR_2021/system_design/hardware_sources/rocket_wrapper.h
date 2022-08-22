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

#include <systemc.h>
#include "rocket_subsystem.h"

SC_MODULE(rocket_wrapper)
{
   //== Ports

   sc_in<bool>              clock;
   sc_in<bool>              reset;

   sc_out<sc_lv<44>>        periph_aw_msg;
   sc_out<bool>             periph_aw_valid;
   sc_in<bool>              periph_aw_ready;

   sc_out<sc_lv<37>>        periph_w_msg;
   sc_out<bool>             periph_w_valid;
   sc_in<bool>              periph_w_ready;

   sc_in<sc_lv<6>>          periph_b_msg;
   sc_in<bool>              periph_b_valid;
   sc_out<bool>             periph_b_ready;

   sc_out<sc_lv<44>>        periph_ar_msg;
   sc_out<bool>             periph_ar_valid;
   sc_in<bool>              periph_ar_ready;

   sc_in<sc_lv<39>>         periph_r_msg;
   sc_in<bool>              periph_r_valid;
   sc_out<bool>             periph_r_ready;

   //== Signals

   sc_signal<sc_logic>      my_sc_clock;
   sc_signal<sc_logic>      my_sc_reset;

   sc_signal<sc_lv<44>>     sc_periph_aw_msg;
   sc_signal<sc_logic>      sc_periph_aw_valid;
   sc_signal<sc_logic>      sc_periph_aw_ready;

   sc_signal<sc_lv<37>>     sc_periph_w_msg;
   sc_signal<sc_logic>      sc_periph_w_valid;
   sc_signal<sc_logic>      sc_periph_w_ready;

   sc_signal<sc_lv<6>>      sc_periph_b_msg;
   sc_signal<sc_logic>      sc_periph_b_valid;
   sc_signal<sc_logic>      sc_periph_b_ready;

   sc_signal<sc_lv<44>>     sc_periph_ar_msg;
   sc_signal<sc_logic>      sc_periph_ar_valid;
   sc_signal<sc_logic>      sc_periph_ar_ready;

   sc_signal<sc_lv<39>>     sc_periph_r_msg;
   sc_signal<sc_logic>      sc_periph_r_valid;
   sc_signal<sc_logic>      sc_periph_r_ready;
   //sc_logic                 sc_periph_r_ready;

   //== Instances

   sc_module_name rocket_sc_module_name{"rocket"};
   rocket_subsystem rocket{rocket_sc_module_name, "rocket_subsystem"};

   //== Assignment routines

   void set_clock()  { sc_logic  e = (sc_logic) clock.read();  my_sc_clock.write(e);  }
   void set_reset()  { sc_logic  e = (sc_logic) reset.read();  my_sc_reset.write(e);  }

   void set_aw_msg() { sc_lv<44> e =         sc_periph_aw_msg.read();      periph_aw_msg.write(e);               }
   void set_aw_vld() { sc_logic  e =         sc_periph_aw_valid.read();    periph_aw_valid.write(e.to_bool());   }
   void set_aw_rdy() { sc_logic  e = (sc_logic) periph_aw_ready.read(); sc_periph_aw_ready.write(e);             }

   void set_w_msg()  { sc_lv<37> e =         sc_periph_w_msg.read();       periph_w_msg.write(e);                }
   void set_w_vld()  { sc_logic  e =         sc_periph_w_valid.read();     periph_w_valid.write(e.to_bool());    }
   void set_w_rdy()  { sc_logic  e = (sc_logic) periph_w_ready.read();  sc_periph_w_ready.write(e);              }

   void set_b_msg()  { sc_lv<6>  e =            periph_b_msg.read();    sc_periph_b_msg.write(e);                }
   void set_b_vld()  { sc_logic  e = (sc_logic) periph_b_valid.read();  sc_periph_b_valid.write(e);              }
   void set_b_rdy()  { sc_logic  e =         sc_periph_b_ready.read();     periph_b_ready.write(e.to_bool());    }

   void set_ar_msg() { sc_lv<44> e =         sc_periph_ar_msg.read();      periph_ar_msg.write(e);               }
   void set_ar_vld() { sc_logic  e =         sc_periph_ar_valid.read();    periph_ar_valid.write(e.to_bool());   }
   void set_ar_rdy() { sc_logic  e = (sc_logic) periph_ar_ready.read(); sc_periph_ar_ready.write(e);             }

   void set_r_msg()  { sc_lv<39> e =            periph_r_msg.read();    sc_periph_r_msg.write(e);                }
   void set_r_vld()  { sc_logic  e = (sc_logic) periph_r_valid.read();  sc_periph_r_valid.write(e);              }
   void set_r_rdy()  { sc_logic  e =         sc_periph_r_ready.read();     periph_r_ready.write(e.to_bool());    }

   //== Constructor

   SC_CTOR(rocket_wrapper) 
   {
       rocket.clock(my_sc_clock);
       rocket.reset(my_sc_reset);

       rocket.periph_aw_msg    (sc_periph_aw_msg);
       rocket.periph_aw_valid  (sc_periph_aw_valid);
       rocket.periph_aw_ready  (sc_periph_aw_ready);

       rocket.periph_w_msg     (sc_periph_w_msg);
       rocket.periph_w_valid   (sc_periph_w_valid);
       rocket.periph_w_ready   (sc_periph_w_ready);

       rocket.periph_b_msg     (sc_periph_b_msg);
       rocket.periph_b_valid   (sc_periph_b_valid);
       rocket.periph_b_ready   (sc_periph_b_ready);

       rocket.periph_ar_msg    (sc_periph_ar_msg);
       rocket.periph_ar_valid  (sc_periph_ar_valid);
       rocket.periph_ar_ready  (sc_periph_ar_ready);

       rocket.periph_r_msg     (sc_periph_r_msg);
       rocket.periph_r_valid   (sc_periph_r_valid);
       rocket.periph_r_ready   (sc_periph_r_ready);

       SC_THREAD(set_clock);   sensitive << clock;
       SC_THREAD(set_reset);   sensitive << reset;

       SC_THREAD(set_aw_msg);  sensitive <<    periph_aw_msg;
       SC_THREAD(set_aw_vld);  sensitive <<    periph_aw_valid;
       SC_THREAD(set_aw_rdy);  sensitive << sc_periph_aw_ready;

       SC_THREAD(set_w_msg);   sensitive <<    periph_w_msg;
       SC_THREAD(set_w_vld);   sensitive <<    periph_w_valid;
       SC_THREAD(set_w_rdy);   sensitive << sc_periph_w_ready;

       SC_THREAD(set_b_msg);   sensitive << sc_periph_b_msg;
       SC_THREAD(set_b_vld);   sensitive << sc_periph_b_valid;
       SC_THREAD(set_b_rdy);   sensitive <<    periph_b_ready;

       SC_THREAD(set_ar_msg);  sensitive <<    periph_ar_msg;
       SC_THREAD(set_ar_vld);  sensitive <<    periph_ar_valid;
       SC_THREAD(set_ar_rdy);  sensitive << sc_periph_ar_ready;

       SC_THREAD(set_r_msg);   sensitive << sc_periph_r_msg;
       SC_THREAD(set_r_vld);   sensitive << sc_periph_r_valid;
       SC_THREAD(set_r_rdy);   sensitive <<    periph_r_ready;

   }
};

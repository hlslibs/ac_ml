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
#ifndef _SCGENMOD_rocket_subsystem_
#define _SCGENMOD_rocket_subsystem_

#include "systemc.h"

class rocket_subsystem : public sc_foreign_module
{
public:
    sc_in<sc_logic> clock;
    sc_in<sc_logic> reset;
    sc_out<sc_lv<44> > periph_aw_msg;
    sc_out<sc_logic> periph_aw_valid;
    sc_in<sc_logic> periph_aw_ready;
    sc_out<sc_lv<37> > periph_w_msg;
    sc_out<sc_logic> periph_w_valid;
    sc_in<sc_logic> periph_w_ready;
    sc_in<sc_lv<6> > periph_b_msg;
    sc_in<sc_logic> periph_b_valid;
    sc_out<sc_logic> periph_b_ready;
    sc_out<sc_lv<44> > periph_ar_msg;
    sc_out<sc_logic> periph_ar_valid;
    sc_in<sc_logic> periph_ar_ready;
    sc_in<sc_lv<39> > periph_r_msg;
    sc_in<sc_logic> periph_r_valid;
    sc_out<sc_logic> periph_r_ready;


    rocket_subsystem(sc_module_name nm, const char* hdl_name)
     : sc_foreign_module(nm),
       clock("clock"),
       reset("reset"),
       periph_aw_msg("periph_aw_msg"),
       periph_aw_valid("periph_aw_valid"),
       periph_aw_ready("periph_aw_ready"),
       periph_w_msg("periph_w_msg"),
       periph_w_valid("periph_w_valid"),
       periph_w_ready("periph_w_ready"),
       periph_b_msg("periph_b_msg"),
       periph_b_valid("periph_b_valid"),
       periph_b_ready("periph_b_ready"),
       periph_ar_msg("periph_ar_msg"),
       periph_ar_valid("periph_ar_valid"),
       periph_ar_ready("periph_ar_ready"),
       periph_r_msg("periph_r_msg"),
       periph_r_valid("periph_r_valid"),
       periph_r_ready("periph_r_ready")
    {
        elaborate_foreign_module(hdl_name);
    }
    ~rocket_subsystem()
    {}

};

#endif


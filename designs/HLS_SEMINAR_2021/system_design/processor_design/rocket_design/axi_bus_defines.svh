/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright 2021, Mentor Graphics Corporation,                     *
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


// defines needed for the rocket core

`define m_bits         4   
`define id_bits        4
`define wid_bits       4
`define rid_bits       4
`define addr_bits     32
`define len_bits       8
`define data_bits     64
`define strb_bits      (`data_bits/8)
`define size_bits      3
`define burst_bits     2
`define lock_bits      1
`define cache_bits     4
`define prot_bits      3
`define resp_bits      2
`define last_bits      1
`define ruser_bits     7
`define wuser_bits     9
`define qos_bits       4

// defines needed by matchlib axi

`define d_m_bits       0   
`define d_id_bits      4
`define d_wid_bits     4
`define d_rid_bits     4
`define d_reg_bits     0
`define d_addr_bits   32
`define d_len_bits     8
`define d_data_bits   32
`define d_strb_bits    (`d_data_bits/8)
`define d_size_bits    3
`define d_burst_bits   2
`define d_lock_bits    0
`define d_cache_bits   0
`define d_prot_bits    0
`define d_resp_bits    2
`define d_last_bits    1
`define d_ruser_bits   0
`define d_buser_bits   0
`define d_wuser_bits   0
`define d_aruser_bits  0
`define d_auser_bits   0
`define d_awuser_bits  0
`define d_qos_bits     0


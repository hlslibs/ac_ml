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



module segment_slave_io(
            BUS_GRANTS,
            ADDR,
            CHIP_SELECTS,
            SELECT_ERROR
    );

    parameter masters = 2;
    parameter slaves  = 2;

    input  [masters-1:0] BUS_GRANTS;
    input  [31:0]        ADDR[masters-1:0];
    output [slaves-1:0]  CHIP_SELECTS;
    output               SELECT_ERROR;

    wire   [31:0]        address;
    wire   [31:0]        local_selects; 
    wire                 active;

    bmux #(masters, 32) mux00 (address, BUS_GRANTS, ADDR);

    assign local_selects[00] = (address[31:16] == 16'h6000)   ? 1 : 0;   // UART at 0x60000000
    assign local_selects[01] = (address[31:16] == 16'h6060)   ? 1 : 0;   // CAT at  0x60600000
    assign local_selects[02] = 0;
    assign local_selects[03] = 0;
    assign local_selects[04] = 0;
    assign local_selects[05] = 0;
    assign local_selects[06] = 0;
    assign local_selects[07] = 0;
    assign local_selects[08] = 0;
    assign local_selects[09] = 0;
    assign local_selects[10] = 0;
    assign local_selects[11] = 0;
    assign local_selects[12] = 0;
    assign local_selects[13] = 0;
    assign local_selects[14] = 0;
    assign local_selects[15] = 0;
    assign local_selects[16] = 0;
    assign local_selects[17] = 0;
    assign local_selects[18] = 0;
    assign local_selects[19] = 0;
    assign local_selects[20] = 0;
    assign local_selects[21] = 0;
    assign local_selects[22] = 0;
    assign local_selects[23] = 0;
    assign local_selects[24] = 0;
    assign local_selects[25] = 0;
    assign local_selects[26] = 0;
    assign local_selects[27] = 0;
    assign local_selects[28] = 0;
    assign local_selects[29] = 0;
    assign local_selects[30] = 0;
    assign local_selects[31] = 0;
    
    assign CHIP_SELECTS = (active) ? local_selects[slaves-1:0] : 32'h00000000;
    assign SELECT_ERROR = ((!local_selects) && (active)) ? 1 : 0;
    assign active = (BUS_GRANTS) ? 1 : 0;
 
endmodule

module segment_slave_mem(
            BUS_GRANTS,
            ADDR,
            CHIP_SELECTS,
            SELECT_ERROR
    );

    parameter masters = 2;
    parameter slaves  = 2;

    input  [masters-1:0] BUS_GRANTS;
    input  [31:0]        ADDR[masters-1:0];
    output [slaves-1:0]  CHIP_SELECTS;
    output               SELECT_ERROR;

    wire   [31:0]        address;
    wire   [31:0]        local_selects; 
    wire                 active;

    bmux #(masters, 32) mux00 (address, BUS_GRANTS, ADDR);

    assign local_selects[00] = (address[31:24] == 8'h80)   ? 1 : 0;   // SRAM at 0x80000000
    assign local_selects[01] = 0;
    assign local_selects[02] = 0;
    assign local_selects[03] = 0;
    assign local_selects[04] = 0;
    assign local_selects[05] = 0;
    assign local_selects[06] = 0;
    assign local_selects[07] = 0;
    assign local_selects[08] = 0;
    assign local_selects[09] = 0;
    assign local_selects[10] = 0;
    assign local_selects[11] = 0;
    assign local_selects[12] = 0;
    assign local_selects[13] = 0;
    assign local_selects[14] = 0;
    assign local_selects[15] = 0;
    assign local_selects[16] = 0;
    assign local_selects[17] = 0;
    assign local_selects[18] = 0;
    assign local_selects[19] = 0;
    assign local_selects[20] = 0;
    assign local_selects[21] = 0;
    assign local_selects[22] = 0;
    assign local_selects[23] = 0;
    assign local_selects[24] = 0;
    assign local_selects[25] = 0;
    assign local_selects[26] = 0;
    assign local_selects[27] = 0;
    assign local_selects[28] = 0;
    assign local_selects[29] = 0;
    assign local_selects[30] = 0;
    assign local_selects[31] = 0;
    
    assign CHIP_SELECTS = (active) ? local_selects[slaves-1:0] : 32'h00000000;
    assign SELECT_ERROR = ((!local_selects) && (active)) ? 1 : 0;
    assign active = (BUS_GRANTS) ? 1 : 0;
 
endmodule

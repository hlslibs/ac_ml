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


module axi_data_latch
    ( 
            CLK,
            RESETN,

            MASTER,
            ID,
            DATA,
            LAST,

            VALID,
            READY,

            O_MASTER,
            O_ID,
            O_DATA,
            O_RESP,
            O_LAST,

            O_VALID,
            O_READY
    );

    parameter masters = 4;
    parameter id_bits = 2;
    parameter data_width = 64;

    input                   CLK;
    input                   RESETN;

    input  [masters-1:0]    MASTER;
    input  [id_bits-1:0]    ID;
    input  [data_width-1:0] DATA;
    input                   LAST;

    input                   VALID;
    output                  READY;

    output [masters-1:0]    O_MASTER;
    output [id_bits-1:0]    O_ID;
    output [data_width-1:0] O_DATA;
    output [1:0]            O_RESP;
    output                  O_LAST;

    output                  O_VALID;
    input                   O_READY;

    wire   [masters+id_bits+data_width:0] fifo_in;
    wire   [masters+id_bits+data_width:0] fifo_out;
    wire                    full;

    reg                     local_ready;
    reg                     send_req;

    reg    [31:0]           one  = 32'h00000001;
    reg    [31:0]           zero = 32'h00000000;

    reg                     fifo_ack;
    reg                     next;

    assign fifo_in = { MASTER, ID, DATA, LAST};
    assign READY = !full;
    assign { O_MASTER, O_ID, O_DATA, O_LAST } = fifo_out;

    bus_fifo #(masters+id_bits+data_width+1) fifo0 (
            .CLK            (CLK), 
            .RESET_N        (RESETN), 
            .DATA_STROBE    (VALID), 
            .DATA_IN        (fifo_in), 

            .DATA_READY     (O_VALID), 
            .DATA_OUT       (fifo_out), 
            .DATA_ACK       (O_READY), 
            .FULL           (full)
    );
 
endmodule
 

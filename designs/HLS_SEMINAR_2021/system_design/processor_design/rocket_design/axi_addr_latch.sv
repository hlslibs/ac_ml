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

`include "axi_bus_defines.svh"

module axi_addr_latch
    ( 
            CLK,
            RESETN,

            MASTER,
            ID,
            ADDR,
            LEN,
            SIZE,
            BURST,
            LOCK,
            CACHE,
            PROT,

            VALID,
            READY,

            O_MASTER,
            O_ID,
            O_ADDR,
            O_LAST,

            O_VALID,
            O_READY
    );

    parameter masters = 4;   // number of masters on the AXI fabric, needed to determine width of "return address"
    parameter width = 22;    // number of address bits in peripheral
    parameter p_size = 4;    // width of peripheral interface in 2^p_size bytes (defaults to 16)

 `define fifo_width ($bits(MASTER)+$bits(ID)+$bits(ADDR)+$bits(LEN)+$bits(SIZE)+$bits(BURST)+$bits(LOCK)+$bits(CACHE)+$bits(PROT))

    input                           CLK;
    input                           RESETN;

    input  [masters-1:0]            MASTER;
    input  [`id_bits-1:0]           ID;
    input  [width-1:0]              ADDR;
    input  [`len_bits-1:0]          LEN;
    input  [`size_bits-1:0]         SIZE;
    input  [`burst_bits-1:0]        BURST;
    input  [`lock_bits-1:0]         LOCK;
    input  [`cache_bits-1:0]        CACHE;
    input  [`prot_bits-1:0]         PROT;

    input                           VALID;
    output                          READY;

    output [masters-1:0]            O_MASTER;
    output [`id_bits-1:0]           O_ID;
    output [width-1:0]              O_ADDR;
    output                          O_LAST;

    output                          O_VALID;
    input                           O_READY;

    wire   [`fifo_width-1:0]        fifo_in;
    wire   [`fifo_width-1:0]        fifo_out;
    wire                            full;
    wire                            cycle_ready;

    reg    [masters-1:0]            L_MASTER;
    reg    [`id_bits-1:0]           L_ID;
    reg    [width-1:0]              L_ADDR;
    reg    [`len_bits-1:0]          L_LEN;
    reg    [`size_bits-1:0]         L_SIZE;
    reg    [`burst_bits-1:0]        L_BURST;
    reg    [`lock_bits-1:0]         L_LOCK;
    reg    [`cache_bits-1:0]        L_CACHE;
    reg    [`prot_bits-1:0]         L_PROT;

    reg                             send_req;
    reg    [width-1:12]             addr_high_bits;
    reg    [11:0]                   addr_low_bits;
    reg    [`strb_bits-1:0]         byte_enables;
    reg                             handling_cycle;
    reg    [4:0]                    number_of_beats;
    reg    [9:0]                    count;
    reg    [9:0]                    bytes_per_beat;
    reg                             start_of_cycle;

    reg    [31:0]                   one  = 32'h00000001;
    reg    [31:0]                   zero = 32'h00000000;

    reg                             fifo_ack;
    reg                             next;
    reg                             last;
    reg                             o_valid;

    assign fifo_in = { MASTER, ID, ADDR, LEN, SIZE, BURST, LOCK, CACHE, PROT };
    assign READY = !full;
    assign { L_MASTER, L_ID, L_ADDR, L_LEN, L_SIZE, L_BURST, L_LOCK, L_CACHE, L_PROT } = fifo_out;

    assign O_MASTER           = L_MASTER;
    assign O_ID               = L_ID;
    assign O_ADDR[width-1:12] = addr_high_bits;
    assign O_ADDR[11:0]       = addr_low_bits;
    assign O_VALID            = o_valid;

    bus_fifo #(`fifo_width) fifo0 (
            .CLK         (CLK), 
            .RESET_N     (RESETN), 
            .DATA_STROBE (VALID), 
            .DATA_IN     (fifo_in), 
            .DATA_READY  (cycle_ready), 
            .DATA_OUT    (fifo_out), 
            .DATA_ACK    (next & last), 
            .FULL        (full)
    );

/*  generate the "ready" at the next level up
    ready_gen #(10) ready_gen_sram (
            .CLK         (CLK), 
            .RESETn      (RESETN),
            .STROBE      (send_req), 
            .READY       (next)
    );
*/
    axi_byte_enables be0 (
            .ADDR        (addr_low_bits),
            .SIZE        (L_SIZE),
            .BE          (byte_enables)
    );

    addr_gen #(p_size) addr_gen_sram (
            .CLK         (CLK),
            .RESETn      (RESETN),
            .ADDR        (L_ADDR[11:0]), 
            .LEN         (L_LEN), 
            .SIZE        (L_SIZE), 
            .BURST       (L_BURST), 
            .START       (start_of_cycle), 
            .NEXT        (next), 
            .ADDR_OUT    (addr_low_bits)
    );

    assign last = ((count + 1 == number_of_beats) && handling_cycle) ? 1 : 0;
    assign O_LAST = last;

    always @(posedge CLK) begin
        if (!RESETN) begin
            o_valid <= 0;
            next <= 0;
        end else begin
            if (send_req) o_valid <= 1;
            if (o_valid && O_READY) begin
                o_valid <= 0;
                next <= 1;
            end
            if (next) next <= 0;
        end
    end

    always @(posedge CLK) begin
        if (!RESETN) begin
            handling_cycle <= 0;
            fifo_ack <= 0;
            send_req <= 0;
        end else begin
            if (!handling_cycle && cycle_ready) begin
                handling_cycle <= 1;
                start_of_cycle <= 1;
                addr_high_bits <= L_ADDR[width-1:12];
                // bytes_per_beat <= (1 << L_SIZE);
                // number_of_beats <= (L_LEN + one);
                bytes_per_beat <= (1 << ((L_SIZE > p_size) ? p_size : L_SIZE));
                number_of_beats <= (L_LEN + one) << ((L_SIZE > p_size) ? L_SIZE - p_size : 0);
                count <= zero;
                fifo_ack <= 1;
                send_req <= 1;
            end else begin
                start_of_cycle <= 0;
                fifo_ack <= 0;
                send_req <= 0;
            end
            if (next) begin  
               if ((count + 1) < number_of_beats) begin
                   count <= count + 1;
                   send_req <= 1;
               end else begin 
                   handling_cycle <= 0;
               end
            end
        end 
    end 

endmodule
 

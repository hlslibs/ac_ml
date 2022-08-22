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

module sram
    (
        CLK,
        RSTn,
        READ_ADDR,
        DATA_OUT,
        OE,
        WRITE_ADDR,
        DATA_IN,
        BE,
        WE
    );

    parameter address_width     = 22;
    parameter data_width        = 2; //   in 2^data_width bytes
                                     //   0 = 8 bits  1 = 16 bits  2 = 32 bits  3 = 64 bits
                                     //   memory is always byte addressible

    input                                CLK;
    input                                RSTn;
    input [address_width-1:0]            READ_ADDR;
    output [((1<<data_width)*8)-1:0]     DATA_OUT;
    input                                OE;
    input [address_width-1:0]            WRITE_ADDR;
    input [((1<<data_width)*8)-1:0]      DATA_IN;
    input [(1<<data_width)-1:0]          BE;
    input                                WE;
   
    
    reg [((1<<data_width)*8)-1:0] mem [(1<<address_width-data_width)-1:0];
    reg [((1<<data_width)*8)-1:0] read_data;

    genvar w;

    assign DATA_OUT = read_data;

    always @(posedge CLK) begin
        if (RSTn == 1'b0) begin
            read_data <= { ((1<<data_width) * 8) {1'b0}};
        end else begin
            if (OE) begin
                read_data <= mem[READ_ADDR];
            end
        end
    end

    generate 
        for (w=0; w<(1<<data_width); w = w + 1) 
            always @(posedge CLK) begin
                if (WE && BE[w]) mem[WRITE_ADDR][((w+1)*8)-1:(w*8)] <= DATA_IN[((w+1)*8)-1:(w*8)];
            end
    endgenerate

endmodule

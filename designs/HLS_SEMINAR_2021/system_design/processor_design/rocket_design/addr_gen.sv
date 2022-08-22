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


module addr_gen
    ( 
        CLK,
        RESETn,

        ADDR,
        LEN,
        SIZE,
        BURST,

        START,
        NEXT,

        ADDR_OUT
    );

    parameter p_size = 4;   // width of peripheral in 2^p_size bytes

    input           CLK;
    input           RESETn;

    input  [11:0]   ADDR;
    input  [7:0]    LEN;
    input  [2:0]    SIZE;
    input  [1:0]    BURST;

    input           START;
    input           NEXT;

    output [11:0]   ADDR_OUT;

    reg    [11:0]   start_address;
    reg    [11:0]   current_address;
    reg    [11:0]   bytes_per_beat;
    reg    [11:0]   number_of_beats;
    reg    [11:0]   orig_bytes_per_beat;
    reg    [11:0]   orig_number_of_beats;
    reg             fixed;
    reg             incr;
    reg             wrap;
    reg    [11:0]   total_bytes;
    reg    [11:0]   fixed_mask;
    reg    [11:0]   var_mask;
    reg    [11:0]   count;

    reg    [31:0]   one = 32'h00000001;

    assign ADDR_OUT = ((start_address & fixed_mask) | (current_address & var_mask)); 

    always @(posedge CLK) begin

        if (RESETn == 0) begin

            start_address      <= 12'h000;
            current_address    <= 12'h000;
            bytes_per_beat     <= 12'h000;
            number_of_beats    <= 12'h000;
            fixed              <= 0;
            incr               <= 0;
            wrap               <= 0;
            total_bytes        <= 12'h000;
            fixed_mask         <= 12'h000;
            var_mask           <= 12'hFFF;
            count              <= 12'h000;

        end else begin

            if (START) begin

                start_address     <= ADDR;
                current_address   <= ADDR;
                orig_bytes_per_beat    <= (1 << SIZE);
                orig_number_of_beats   <= LEN + 1;
                bytes_per_beat    <= (1 << ((SIZE > p_size) ? p_size : SIZE));
                number_of_beats   <= (LEN + one) << ((SIZE > p_size) ? SIZE - p_size : 0);
                count             <= 0;

                fixed <= (BURST == 2'b00) ? 1 : 0;
                incr  <= (BURST == 2'b01) ? 1 : 0;
                wrap  <= (BURST == 2'b10) ? 1 : 0;

                if (BURST == 2'b10) begin 
                    total_bytes <=   (LEN + 1) << SIZE;
                    var_mask   <=   ((LEN + 1) << SIZE) - 1;
                    fixed_mask <= ~(((LEN + 1) << SIZE) - 1);
                end else begin
                    total_bytes <=   (LEN + 1) << SIZE;
                    var_mask   <=    12'hFFF;
                    fixed_mask <=    12'h000;
                end

            end else begin
                if (NEXT) begin
                    count <= count + 1;
                    if ((count + 1) < number_of_beats) begin
                        if (incr || wrap) current_address <= current_address + bytes_per_beat;
                    end 
                end
            end
        end
    end   
endmodule

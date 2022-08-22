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

module to_index(idx, select);
    parameter index_bits  = 2;
    parameter select_bits = 4;

    output [(index_bits-1):0] idx;
    input [(select_bits-1):0] select;

    reg [3:0]   local_idx;
    wire [15:0] local_select;

//  assuming that synthesis tools will strip out the unused logic

    assign local_select[(select_bits-1):0] = select;
    assign local_select[15:select_bits] = 0;
    assign idx = local_idx[(index_bits-1):0];

//  there *should* be a way to "generate" this

    always @(local_select) begin
       if (!local_select) local_idx <= 4'b0000;  // no selection
       if (local_select[ 0]) local_idx <= 4'b0000;
       if (local_select[ 1]) local_idx <= 4'b0001;
       if (local_select[ 2]) local_idx <= 4'b0010;
       if (local_select[ 3]) local_idx <= 4'b0011;
       if (local_select[ 4]) local_idx <= 4'b0100;
       if (local_select[ 5]) local_idx <= 4'b0101;
       if (local_select[ 6]) local_idx <= 4'b0110;
       if (local_select[ 7]) local_idx <= 4'b0111;
       if (local_select[ 8]) local_idx <= 4'b1000;
       if (local_select[ 9]) local_idx <= 4'b1001;
       if (local_select[10]) local_idx <= 4'b1010;
       if (local_select[11]) local_idx <= 4'b1011;
       if (local_select[12]) local_idx <= 4'b1100;
       if (local_select[13]) local_idx <= 4'b1101;
       if (local_select[14]) local_idx <= 4'b1110;
       if (local_select[15]) local_idx <= 4'b1111;
    end

endmodule


module bmux(sig_out, select, sig_in);

    parameter count = 2;    // number of elements in the sig_in array
    parameter width = 32;   // size of the elements in the sig_in array

    output [width-1:0] sig_out;
    input [count-1:0]  select;
    input [width-1:0]  sig_in [count-1:0];

    reg [width-1:0] no_drive = 32'h00000000;  // 32'hZZZZZZZZ;
    
    reg [3:0] idx;

    to_index #(4, count) ti0 (idx, select);

    assign sig_out = (select == 4'b0000) ? no_drive : sig_in[idx];

endmodule

module mux(sig_out, select, sig_in1, sig_in0);

    parameter width = 32;

    output [width-1:0] sig_out;
    input select;
    input [width-1:0]  sig_in0;
    input [width-1:0]  sig_in1;
    
    assign sig_out = select ? sig_in1 : sig_in0;

endmodule


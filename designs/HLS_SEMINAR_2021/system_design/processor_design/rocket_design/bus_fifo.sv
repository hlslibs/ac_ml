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


module bus_fifo
    (
        CLK,
        RESET_N,
        DATA_STROBE,
        DATA_IN,
        DATA_READY,
        DATA_OUT,
        DATA_ACK,
        FULL
    );

    parameter width = 8;
    parameter depth = 8;

    input               CLK;
    input               RESET_N;
    input               DATA_STROBE;
    input  [width-1:0]  DATA_IN;
    output              DATA_READY;
    output [width-1:0]  DATA_OUT;
    input               DATA_ACK;
    output              FULL;

    reg    [width-1:0]  fifo_mem[(1<<depth)-1:0];
    reg    [depth-1:0]  input_pointer;
    reg    [depth-1:0]  output_pointer;

    wire                empty;
    wire   [depth:0]    input_pointer_plus_one;

    assign empty = (input_pointer == output_pointer) ? 1'b1 : 1'b0;
    assign FULL  = (input_pointer_plus_one == output_pointer) ? 1'b1 : 1'b0;
    assign DATA_READY = !empty;

    assign input_pointer_plus_one = input_pointer + {depth {1'b1}};
    assign DATA_OUT = (empty) ? {width {1'bz}} : fifo_mem[output_pointer];

    always @(posedge CLK) begin
        
        if (!RESET_N) begin 
            input_pointer <= {depth {1'b0}};
            output_pointer <= {depth {1'b0}};
        end else begin
            if (DATA_STROBE & !FULL) begin
                fifo_mem[input_pointer] <= DATA_IN;
                input_pointer <= input_pointer + {depth {1'b1}};
            end
            if (DATA_ACK & !empty) begin
                output_pointer <= output_pointer + {depth {1'b1}};
            end
        end
    end

endmodule

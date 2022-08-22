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
 

module ready_gen 
    ( 
        CLK,
        RESETn,
        STROBE,
        READY
    );

    parameter waits = 5;

    input CLK;
    input RESETn;
    input STROBE;
    output READY;

    reg   [11:0]    counter;
    reg   [31:0]    zero = 32'h00000000;
    reg   [31:0]    one = 32'h00000001;
    reg             local_ready;

    assign READY = (waits == 0) ? STROBE: local_ready;

    always @(posedge CLK) begin
        if (RESETn == 0) begin
            counter <= zero;
            local_ready <= 0;
        end else begin
            if (!STROBE) begin
                counter <= zero;
                local_ready <= 0;
            end else if (counter > 0) begin
                counter <= counter - one;
                if ((counter - one) == 0) local_ready <= 1;
            end else begin 
                local_ready <= 0;
                if (STROBE) begin
                    counter <= waits;
                end
            end
        end
    end
endmodule

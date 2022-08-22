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

module segment_arbiter(
     bus_grants,
     bus_requests,
     hclock,
     resetn
   );
    parameter masters = 2;

    output [masters-1:0] bus_grants;
    input                bus_requests [masters-1:0];
    input                hclock;
    input                resetn;

    wire [15:0]          bus_reqs;
    reg  [15:0]          raw_grants;
    reg  [15:0]          grants;
    reg  [15:0]          grts;
    reg  [15:0]          gnr;
    reg  [15:0]          zeros = 16'b0000;
    reg                  new_grant;

    genvar m;

    always @(posedge hclock) begin
        gnr <= bus_reqs & grants;
    end

    generate
        for (m=0; m<masters; m=m+1) begin
            assign bus_grants[m] = grants[m];
            assign bus_reqs[m] = bus_requests[m];
        end
    endgenerate

    assign new_grant = (gnr == zeros) ? 1'b1 : 1'b0;

    assign grts[0]  = (bus_reqs[0]    ===  1'b1)                ? 1'b1 : 1'b0;
    assign grts[1]  = (bus_reqs[1:0]  ===  2'b10)               ? 1'b1 : 1'b0;
    assign grts[2]  = (bus_reqs[2:0]  ===  3'b100)              ? 1'b1 : 1'b0;
    assign grts[3]  = (bus_reqs[3:0]  ===  4'b1000)             ? 1'b1 : 1'b0;
    assign grts[4]  = (bus_reqs[4:0]  ===  5'b10000)            ? 1'b1 : 1'b0;
    assign grts[5]  = (bus_reqs[5:0]  ===  6'b100000)           ? 1'b1 : 1'b0;
    assign grts[6]  = (bus_reqs[6:0]  ===  7'b1000000)          ? 1'b1 : 1'b0;
    assign grts[7]  = (bus_reqs[7:0]  ===  8'b10000000)         ? 1'b1 : 1'b0;
    assign grts[8]  = (bus_reqs[8:0]  ===  9'b100000000)        ? 1'b1 : 1'b0;
    assign grts[9]  = (bus_reqs[9:0]  === 10'b1000000000)       ? 1'b1 : 1'b0;
    assign grts[10] = (bus_reqs[10:0] === 11'b10000000000)      ? 1'b1 : 1'b0;
    assign grts[11] = (bus_reqs[11:0] === 12'b100000000000)     ? 1'b1 : 1'b0;
    assign grts[12] = (bus_reqs[12:0] === 13'b1000000000000)    ? 1'b1 : 1'b0;
    assign grts[13] = (bus_reqs[13:0] === 14'b10000000000000)   ? 1'b1 : 1'b0;
    assign grts[14] = (bus_reqs[14:0] === 15'b100000000000000)  ? 1'b1 : 1'b0;
    assign grts[15] = (bus_reqs[15:0] === 16'b1000000000000000) ? 1'b1 : 1'b0;

//     and so on as more masters are added

    assign raw_grants = (resetn) ? ((new_grant) ? grts : grants) : zeros;
    assign grants = bus_reqs & raw_grants;
 
endmodule

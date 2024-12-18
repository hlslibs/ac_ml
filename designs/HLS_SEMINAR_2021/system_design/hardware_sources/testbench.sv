/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.9                                                 *
 *                                                                        *
 *  Release Date    : Mon Oct 14 17:47:36 PDT 2024                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.9.0                                               *
 *                                                                        *
 *  Copyright 2021 Siemens                                                *
 *                                                                        *
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
`define reset_count  (10)
`define clock_delay  (`CLOCK_PERIOD / 2)

module testbench;
   
   logic clock      = 1'b1;
   logic reset_bar  = 1'b1;
   int i;

   initial begin
      reset_bar = 1'b0;
      for (i=0; i<`reset_count; i++) @(posedge clock);
      reset_bar = 1'b1;
   end

   always #`clock_delay clock = ~clock;

   top top(
     .clk       (clock),
     .reset_bar (reset_bar)
   );

endmodule

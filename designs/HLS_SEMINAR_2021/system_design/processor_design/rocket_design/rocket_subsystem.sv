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

// AXI field widths
// Must match matchlib implementation

`define aw_msg_size    (`d_wid_bits   +   \
                        `d_addr_bits  +   \
                        `d_burst_bits +   \
                        `d_len_bits   +   \
                        `d_size_bits  +   \
                        `d_cache_bits +   \
                        `d_qos_bits   +   \
                        `d_lock_bits  +   \
                        `d_prot_bits  +   \
                        `d_reg_bits   +   \
                        `d_awuser_bits)

`define w_msg_size     (`d_data_bits  +   \
                        `d_last_bits  +   \
                        `d_strb_bits  +   \
                        `d_wuser_bits)

`define b_msg_size     (`d_wid_bits   +   \
                        `d_resp_bits  +   \
                        `d_buser_bits)

`define ar_msg_size    (`d_rid_bits   +   \
                        `d_addr_bits  +   \
                        `d_burst_bits +   \
                        `d_len_bits   +   \
                        `d_size_bits  +   \
                        `d_cache_bits +   \
                        `d_qos_bits   +   \
                        `d_lock_bits  +   \
                        `d_prot_bits  +   \
                        `d_reg_bits   +   \
                        `d_auser_bits)

`define r_msg_size     (`d_rid_bits   +   \
                        `d_data_bits  +   \
                        `d_resp_bits  +   \
                        `d_last_bits  +   \
                        `d_ruser_bits)

module rocket_subsystem( 
  input                         clock, 
  input                         reset,

  // peripheral bus

  output [`aw_msg_size-6:0]     periph_aw_msg,
  output                        periph_aw_valid,
  input                         periph_aw_ready,

  output [`w_msg_size-1:0]      periph_w_msg,
  output                        periph_w_valid,
  input                         periph_w_ready,

  input  [`b_msg_size-1:0]      periph_b_msg,
  input                         periph_b_valid,
  output                        periph_b_ready,

  output [`ar_msg_size-6:0]     periph_ar_msg,
  output                        periph_ar_valid,
  input                         periph_ar_ready,

  input  [`r_msg_size-1:0]      periph_r_msg,
  input                         periph_r_valid,
  output                        periph_r_ready

);

`define mmasters 1
`define mslaves 1

  wire   [`id_bits-1:0]         M_AW_ID      [`mmasters-1:0];
  wire   [`addr_bits-1:0]       M_AW_ADDR    [`mmasters-1:0];
  wire   [`len_bits-1:0]        M_AW_LEN     [`mmasters-1:0];
  wire   [`size_bits-1:0]       M_AW_SIZE    [`mmasters-1:0];
  wire   [`burst_bits-1:0]      M_AW_BURST   [`mmasters-1:0];
  wire   [`lock_bits-1:0]       M_AW_LOCK    [`mmasters-1:0];
  wire   [`cache_bits-1:0]      M_AW_CACHE   [`mmasters-1:0];
  wire   [`prot_bits-1:0]       M_AW_PROT    [`mmasters-1:0];
  wire   [`qos_bits-1:0]        M_AW_QOS     [`mmasters-1:0];
  wire                          M_AW_VALID   [`mmasters-1:0];
  wire                          M_AW_READY   [`mmasters-1:0];

  wire   [`id_bits-1:0]         M_W_ID       [`mmasters-1:0];
  wire   [`data_bits-1:0]       M_W_DATA     [`mmasters-1:0];
  wire   [`strb_bits-1:0]       M_W_STRB     [`mmasters-1:0];
  wire                          M_W_LAST     [`mmasters-1:0];
  wire                          M_W_VALID    [`mmasters-1:0];
  wire                          M_W_READY    [`mmasters-1:0];

  wire   [`id_bits-1:0]         M_B_ID       [`mmasters-1:0];
  wire   [`resp_bits-1:0]       M_B_RESP     [`mmasters-1:0];
  wire                          M_B_VALID    [`mmasters-1:0];
  wire                          M_B_READY    [`mmasters-1:0];

  wire   [`id_bits-1:0]         M_AR_ID      [`mmasters-1:0];
  wire   [`addr_bits-1:0]       M_AR_ADDR    [`mmasters-1:0];
  wire   [`len_bits-1:0]        M_AR_LEN     [`mmasters-1:0];
  wire   [`size_bits-1:0]       M_AR_SIZE    [`mmasters-1:0];
  wire   [`burst_bits-1:0]      M_AR_BURST   [`mmasters-1:0];
  wire   [`lock_bits-1:0]       M_AR_LOCK    [`mmasters-1:0];
  wire   [`cache_bits-1:0]      M_AR_CACHE   [`mmasters-1:0];
  wire   [`prot_bits-1:0]       M_AR_PROT    [`mmasters-1:0];
  wire   [`qos_bits-1:0]        M_AR_QOS     [`mmasters-1:0];
  wire                          M_AR_VALID   [`mmasters-1:0];
  wire                          M_AR_READY   [`mmasters-1:0];

  wire   [`id_bits-1:0]         M_R_ID       [`mmasters-1:0];
  wire   [`data_bits-1:0]       M_R_DATA     [`mmasters-1:0];
  wire   [`resp_bits-1:0]       M_R_RESP     [`mmasters-1:0];
  wire                          M_R_LAST     [`mmasters-1:0];
  wire                          M_R_VALID    [`mmasters-1:0];
  wire                          M_R_READY    [`mmasters-1:0];

  wire   [`mmasters-1:0]        MS_AW_MASTER [`mslaves-1:0];
  wire   [`id_bits-1:0]         MS_AW_ID     [`mslaves-1:0];
  wire   [`addr_bits-1:0]       MS_AW_ADDR   [`mslaves-1:0];
  wire   [`len_bits-1:0]        MS_AW_LEN    [`mslaves-1:0];
  wire   [`size_bits-1:0]       MS_AW_SIZE   [`mslaves-1:0];
  wire   [`burst_bits-1:0]      MS_AW_BURST  [`mslaves-1:0];
  wire   [`lock_bits-1:0]       MS_AW_LOCK   [`mslaves-1:0];
  wire   [`cache_bits-1:0]      MS_AW_CACHE  [`mslaves-1:0];
  wire   [`prot_bits-1:0]       MS_AW_PROT   [`mslaves-1:0];
  wire   [`qos_bits-1:0]        MS_AW_QOS    [`mslaves-1:0];
  wire                          MS_AW_VALID  [`mslaves-1:0];
  wire                          MS_AW_READY  [`mslaves-1:0];

  wire   [`mmasters-1:0]        MS_W_MASTER  [`mslaves-1:0];
  wire   [`id_bits-1:0]         MS_W_ID      [`mslaves-1:0];
  wire   [`data_bits-1:0]       MS_W_DATA    [`mslaves-1:0];
  wire   [`strb_bits-1:0]       MS_W_STRB    [`mslaves-1:0];
  wire                          MS_W_LAST    [`mslaves-1:0];
  wire                          MS_W_VALID   [`mslaves-1:0];
  wire                          MS_W_READY   [`mslaves-1:0];

  wire   [`mmasters-1:0]        MS_B_MASTER  [`mslaves-1:0];
  wire   [`id_bits-1:0]         MS_B_ID      [`mslaves-1:0];
  wire   [`resp_bits-1:0]       MS_B_RESP    [`mslaves-1:0];
  wire                          MS_B_VALID   [`mslaves-1:0];
  wire                          MS_B_READY   [`mslaves-1:0];

  wire   [`mmasters-1:0]        MS_AR_MASTER [`mslaves-1:0];
  wire   [`id_bits-1:0]         MS_AR_ID     [`mslaves-1:0];
  wire   [`addr_bits-1:0]       MS_AR_ADDR   [`mslaves-1:0];
  wire   [`len_bits-1:0]        MS_AR_LEN    [`mslaves-1:0];
  wire   [`size_bits-1:0]       MS_AR_SIZE   [`mslaves-1:0];
  wire   [`burst_bits-1:0]      MS_AR_BURST  [`mslaves-1:0];
  wire   [`lock_bits-1:0]       MS_AR_LOCK   [`mslaves-1:0];
  wire   [`cache_bits-1:0]      MS_AR_CACHE  [`mslaves-1:0];
  wire   [`prot_bits-1:0]       MS_AR_PROT   [`mslaves-1:0];
  wire   [`qos_bits-1:0]        MS_AR_QOS    [`mslaves-1:0];
  wire                          MS_AR_VALID  [`mslaves-1:0];
  wire                          MS_AR_READY  [`mslaves-1:0];

  wire   [`mmasters-1:0]        MS_R_MASTER  [`mslaves-1:0];
  wire   [`id_bits-1:0]         MS_R_ID      [`mslaves-1:0];
  wire   [`data_bits-1:0]       MS_R_DATA    [`mslaves-1:0];
  wire   [`resp_bits-1:0]       MS_R_RESP    [`mslaves-1:0];
  wire                          MS_R_LAST    [`mslaves-1:0];
  wire                          MS_R_VALID   [`mslaves-1:0];
  wire                          MS_R_READY   [`mslaves-1:0];

  wire   [`id_bits-1:0]         P_AW_ID;
  wire   [`addr_bits-1:0]       P_AW_ADDR;
  wire   [`len_bits-1:0]        P_AW_LEN;
  wire   [`size_bits-1:0]       P_AW_SIZE;
  wire   [`burst_bits-1:0]      P_AW_BURST;
  wire   [`lock_bits-1:0]       P_AW_LOCK;
  wire   [`cache_bits-1:0]      P_AW_CACHE;
  wire   [`prot_bits-1:0]       P_AW_PROT;
  wire   [`qos_bits-1:0]        P_AW_QOS;
  wire                          P_AW_VALID;
  wire                          P_AW_READY;

  wire   [`id_bits-1:0]         P_W_ID;
  wire   [`data_bits-1:0]       P_W_DATA;
  wire   [`strb_bits-1:0]       P_W_STRB;
  wire                          P_W_LAST;
  wire                          P_W_VALID;
  wire                          P_W_READY;

  wire   [`id_bits-1:0]         P_B_ID;
  wire   [`resp_bits-1:0]       P_B_RESP;
  wire                          P_B_VALID;
  wire                          P_B_READY;

  wire   [`id_bits-1:0]         P_AR_ID;
  wire   [`addr_bits-1:0]       P_AR_ADDR;
  wire   [`len_bits-1:0]        P_AR_LEN;
  wire   [`size_bits-1:0]       P_AR_SIZE;
  wire   [`burst_bits-1:0]      P_AR_BURST;
  wire   [`lock_bits-1:0]       P_AR_LOCK;
  wire   [`cache_bits-1:0]      P_AR_CACHE;
  wire   [`prot_bits-1:0]       P_AR_PROT;
  wire   [`qos_bits-1:0]        P_AR_QOS;
  wire                          P_AR_VALID;
  wire                          P_AR_READY;

  wire   [`id_bits-1:0]         P_R_ID;
  wire   [`data_bits-1:0]       P_R_DATA;
  wire   [`resp_bits-1:0]       P_R_RESP;
  wire                          P_R_LAST;
  wire                          P_R_VALID;
  wire                          P_R_READY;


  wire   [31:0]                 SRAM_READ_ADDR;
  wire   [63:0]                 SRAM_READ_DATA;
  wire                          SRAM_OE;
  wire   [31:0]                 SRAM_WRITE_ADDR;
  wire   [63:0]                 SRAM_WRITE_DATA;
  wire   [7:0]                  SRAM_WRITE_BE;
  wire                          SRAM_WRITE_STROBE;

  wire                          debug_clockeddmi_dmi_req_ready; 
  wire                          debug_clockeddmi_dmi_req_valid = 1'b0; 
  wire   [6:0]                  debug_clockeddmi_dmi_req_bits_addr = 7'b0000000; 
  wire   [31:0]                 debug_clockeddmi_dmi_req_bits_data = 32'h00000000; 
  wire   [1:0]                  debug_clockeddmi_dmi_req_bits_op = 2'b00;  
  wire                          debug_clockeddmi_dmi_resp_ready = 1'b0; 
  wire                          debug_clockeddmi_dmi_resp_valid; 
  wire   [31:0]                 debug_clockeddmi_dmi_resp_bits_data; 
  wire   [1:0]                  debug_clockeddmi_dmi_resp_bits_resp; 
  wire                          debug_clockeddmi_dmiClock = clock; 
  wire                          debug_clockeddmi_dmiReset = reset; 
  wire                          debug_ndreset; 
  wire                          debug_dmactive; 

  wire   [1:0]                  interrupts = 2'b00; 

  // l2_frontend_bus is unused, tie control signals to deasserted

  wire                          l2_frontend_bus_axi4_0_aw_ready; 
  wire                          l2_frontend_bus_axi4_0_aw_valid = 1'b0; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_aw_bits_id = 8'h00; 
  wire   [31:0]                 l2_frontend_bus_axi4_0_aw_bits_addr = 32'h00000000; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_aw_bits_len = 8'h00;  
  wire   [2:0]                  l2_frontend_bus_axi4_0_aw_bits_size = 3'b000; 
  wire   [1:0]                  l2_frontend_bus_axi4_0_aw_bits_burst = 2'b00; 
  wire                          l2_frontend_bus_axi4_0_aw_bits_lock = 1'b0; 
  wire   [3:0]                  l2_frontend_bus_axi4_0_aw_bits_cache = 4'h0; 
  wire   [2:0]                  l2_frontend_bus_axi4_0_aw_bits_prot = 3'b000; 
  wire   [3:0]                  l2_frontend_bus_axi4_0_aw_bits_qos = 4'h0; 
  wire                          l2_frontend_bus_axi4_0_w_ready; 
  wire                          l2_frontend_bus_axi4_0_w_valid = 1'b0; 
  wire   [63:0]                 l2_frontend_bus_axi4_0_w_bits_data = 64'h0000000000000000; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_w_bits_strb = 8'h00; 
  wire                          l2_frontend_bus_axi4_0_w_bits_last = 1'b0; 

  wire                          l2_frontend_bus_axi4_0_b_ready = 1'b0; 
  wire                          l2_frontend_bus_axi4_0_b_valid; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_b_bits_id; 
  wire   [1:0]                  l2_frontend_bus_axi4_0_b_bits_resp; 

  wire                          l2_frontend_bus_axi4_0_ar_ready; 
  wire                          l2_frontend_bus_axi4_0_ar_valid = 1'b0; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_ar_bits_id = 8'h00; 
  wire   [31:0]                 l2_frontend_bus_axi4_0_ar_bits_addr = 32'h00000000; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_ar_bits_len = 8'h00; 
  wire   [2:0]                  l2_frontend_bus_axi4_0_ar_bits_size = 3'b00; 
  wire   [1:0]                  l2_frontend_bus_axi4_0_ar_bits_burst = 2'b00; 
  wire                          l2_frontend_bus_axi4_0_ar_bits_lock = 1'b0; 
  wire   [3:0]                  l2_frontend_bus_axi4_0_ar_bits_cache = 4'h0; 
  wire   [2:0]                  l2_frontend_bus_axi4_0_ar_bits_prot = 3'b000; 
  wire   [3:0]                  l2_frontend_bus_axi4_0_ar_bits_qos = 4'h0; 

  wire                          l2_frontend_bus_axi4_0_r_ready = 1'b0; 
  wire                          l2_frontend_bus_axi4_0_r_valid; 
  wire   [7:0]                  l2_frontend_bus_axi4_0_r_bits_id; 
  wire   [63:0]                 l2_frontend_bus_axi4_0_r_bits_data; 
  wire   [1:0]                  l2_frontend_bus_axi4_0_r_bits_resp; 
  wire                          l2_frontend_bus_axi4_0_r_bits_last;
  
  wire   [`d_id_bits-1:0]       periph_aw_id;
  wire   [`d_addr_bits-1:0]     periph_aw_addr;
  wire   [`d_len_bits-1:0]      periph_aw_len;
  wire   [`d_size_bits-1:0]     periph_aw_size;
  wire   [`d_burst_bits-1:0]    periph_aw_burst;

  wire   [`d_id_bits-1:0]       periph_w_id;
  wire   [`d_data_bits-1:0]     periph_w_data;
  wire   [`d_strb_bits-1:0]     periph_w_strb;
  wire                          periph_w_last;

  wire   [`d_id_bits-1:0]       periph_b_id;
  wire   [`d_resp_bits-1:0]     periph_b_resp;

  wire   [`d_id_bits-1:0]       periph_ar_id;
  wire   [`d_addr_bits-1:0]     periph_ar_addr;
  wire   [`d_len_bits-1:0]      periph_ar_len;
  wire   [`d_size_bits-1:0]     periph_ar_size;
  wire   [`d_burst_bits-1:0]    periph_ar_burst;

  wire   [`d_id_bits-1:0]       periph_r_id;
  wire   [`d_data_bits-1:0]     periph_r_data;
  wire   [`d_resp_bits-1:0]     periph_r_resp;
  wire                          periph_r_last;

  reg                           w_bit;
  reg                           r_bit;

  always @(posedge clock) begin
    if (reset) w_bit = 0;
    else if (P_AW_VALID & periph_aw_ready) w_bit = P_AW_ADDR[2];
  end

  always @(posedge clock) begin
    if (reset) r_bit = 0;
    else if (P_AR_VALID & periph_ar_ready) r_bit = P_AR_ADDR[2];
  end

  assign periph_aw_id         = P_AW_ID;
  assign periph_aw_addr       = P_AW_ADDR;
  assign periph_aw_len        = P_AW_LEN;
  assign periph_aw_size       = P_AW_SIZE;
  assign periph_aw_burst      = P_AW_BURST;
  assign periph_aw_valid      = P_AW_VALID;
  assign P_AW_READY           = periph_aw_ready;

  assign periph_w_id          = P_W_ID;
  assign periph_w_data        = (w_bit) ? P_W_DATA[63:32] : P_W_DATA[31:0];
  assign periph_w_strb        = (w_bit) ? P_W_STRB[7:4]   : P_W_STRB[3:0];
  assign periph_w_last        = P_W_LAST;
  assign periph_w_valid       = P_W_VALID;
  assign P_W_READY            = periph_w_ready;

  assign P_B_ID               = periph_b_id;
  assign P_B_RESP             = periph_b_resp;
  assign P_B_VALID            = periph_b_valid;
  assign periph_b_ready       = P_B_READY;

  assign periph_ar_id         = P_AR_ID;
  assign periph_ar_addr       = P_AR_ADDR;
  assign periph_ar_len        = P_AR_LEN;
  assign periph_ar_size       = P_AR_SIZE;
  assign periph_ar_burst      = P_AR_BURST;
  assign periph_ar_valid      = P_AR_VALID;
  assign P_AR_READY           = periph_ar_ready;

  assign P_R_ID               = periph_r_id;
  assign P_R_DATA             = (r_bit) ? { periph_r_data, {32{1'b0}}} : { {32{1'b0}}, periph_r_data};
  assign P_R_RESP             = periph_r_resp;
  assign P_R_LAST             = periph_r_last;
  assign P_R_VALID            = periph_r_valid;
  assign periph_r_ready       = P_R_READY;

  assign periph_aw_msg        = { periph_aw_len, periph_aw_addr, periph_aw_id };
  assign periph_w_msg         = { periph_w_strb, periph_w_last, periph_w_data };
  assign { periph_b_resp, periph_b_id } 
                              = periph_b_msg;
  assign periph_ar_msg        = { periph_ar_len, periph_ar_addr, periph_ar_id };
  assign { periph_r_last, periph_r_resp, periph_r_data, periph_r_id } 
                              = periph_r_msg;

  axi_matrix #(`mmasters, `mslaves, 1, 1) mem_complex (

        .ACLK        (clock),
        .ARESETn     (!reset),

        .AWID        (M_AW_ID),
        .AWADDR      (M_AW_ADDR),
        .AWLEN       (M_AW_LEN),
        .AWSIZE      (M_AW_SIZE),
        .AWBURST     (M_AW_BURST),
        .AWLOCK      (M_AW_LOCK),
        .AWCACHE     (M_AW_CACHE),
        .AWPROT      (M_AW_PROT),
        .AWQOS       (M_AW_QOS),
        .AWVALID     (M_AW_VALID),
        .AWREADY     (M_AW_READY),

        .WDATA       (M_W_DATA),
        .WSTRB       (M_W_STRB),
        .WLAST       (M_W_LAST),
        .WVALID      (M_W_VALID),
        .WREADY      (M_W_READY),

        .BID         (M_B_ID),
        .BRESP       (M_B_RESP),
        .BVALID      (M_B_VALID),
        .BREADY      (M_B_READY),

        .ARID        (M_AR_ID),
        .ARADDR      (M_AR_ADDR),
        .ARLEN       (M_AR_LEN),
        .ARSIZE      (M_AR_SIZE),
        .ARBURST     (M_AR_BURST),
        .ARLOCK      (M_AR_LOCK),
        .ARCACHE     (M_AR_CACHE),
        .ARPROT      (M_AR_PROT),
        .ARQOS       (M_AR_QOS),
        .ARVALID     (M_AR_VALID),
        .ARREADY     (M_AR_READY),

        .RID         (M_R_ID),
        .RDATA       (M_R_DATA),
        .RRESP       (M_R_RESP),
        .RLAST       (M_R_LAST),
        .RVALID      (M_R_VALID),
        .RREADY      (M_R_READY),

        .S_AWMASTER  (MS_AW_MASTER),
        .S_AWID      (MS_AW_ID),
        .S_AWADDR    (MS_AW_ADDR),
        .S_AWLEN     (MS_AW_LEN),
        .S_AWSIZE    (MS_AW_SIZE),
        .S_AWBURST   (MS_AW_BURST),
        .S_AWLOCK    (MS_AW_LOCK),
        .S_AWCACHE   (MS_AW_CACHE),
        .S_AWPROT    (MS_AW_PROT),
        .S_AWQOS     (MS_AW_QOS),
        .S_AWVALID   (MS_AW_VALID),
        .S_AWREADY   (MS_AW_READY),

        .S_WMASTER   (MS_W_MASTER),
        .S_WID       (MS_W_ID),
        .S_WDATA     (MS_W_DATA),
        .S_WSTRB     (MS_W_STRB),
        .S_WLAST     (MS_W_LAST),
        .S_WVALID    (MS_W_VALID),
        .S_WREADY    (MS_W_READY),

        .S_BMASTER   (MS_B_MASTER),
        .S_BID       (MS_B_ID),
        .S_BRESP     (MS_B_RESP),
        .S_BVALID    (MS_B_VALID),
        .S_BREADY    (MS_B_READY),

        .S_ARMASTER  (MS_AR_MASTER),
        .S_ARID      (MS_AR_ID),
        .S_ARADDR    (MS_AR_ADDR),
        .S_ARLEN     (MS_AR_LEN),
        .S_ARSIZE    (MS_AR_SIZE),
        .S_ARBURST   (MS_AR_BURST),
        .S_ARLOCK    (MS_AR_LOCK),
        .S_ARCACHE   (MS_AR_CACHE),
        .S_ARPROT    (MS_AR_PROT),
        .S_ARQOS     (MS_AR_QOS),
        .S_ARVALID   (MS_AR_VALID),
        .S_ARREADY   (MS_AR_READY),

        .S_RMASTER   (MS_R_MASTER),
        .S_RID       (MS_R_ID),
        .S_RDATA     (MS_R_DATA),
        .S_RRESP     (MS_R_RESP),
        .S_RLAST     (MS_R_LAST),
        .S_RVALID    (MS_R_VALID),
        .S_RREADY    (MS_R_READY)

  );

  axi_slave_interface
        #(
        .masters   (`mmasters),
        .width     (24),
        .id_bits   (`id_bits),
        .p_size    (3),
        .b_size    (3))

    mem_if (

        .ACLK      (clock),
        .ARESETn   (!reset),

        .AWMASTER  (MS_AW_MASTER[0]),
        .AWID      (MS_AW_ID[0]),
        .AWADDR    (MS_AW_ADDR[0][23:0]),
        .AWLEN     (MS_AW_LEN[0]),
        .AWSIZE    (MS_AW_SIZE[0]),
        .AWBURST   (MS_AW_BURST[0]),
        .AWLOCK    (MS_AW_LOCK[0]),
        .AWCACHE   (MS_AW_CACHE[0]),
        .AWPROT    (MS_AW_PROT[0]),
        .AWVALID   (MS_AW_VALID[0]),
        .AWREADY   (MS_AW_READY[0]),

        .WMASTER   (MS_W_MASTER[0]),
        .WID       (MS_W_ID[0]),
        .WDATA     (MS_W_DATA[0]),
        .WSTRB     (MS_W_STRB[0]),
        .WLAST     (MS_W_LAST[0]),
        .WVALID    (MS_W_VALID[0]),
        .WREADY    (MS_W_READY[0]),

        .BMASTER   (MS_B_MASTER[0]),
        .BID       (MS_B_ID[0]),
        .BRESP     (MS_B_RESP[0]),
        .BVALID    (MS_B_VALID[0]),
        .BREADY    (MS_B_READY[0]),

        .ARMASTER  (MS_AR_MASTER[0]),
        .ARID      (MS_AR_ID[0]),
        .ARADDR    (MS_AR_ADDR[0][23:0]),
        .ARLEN     (MS_AR_LEN[0]),
        .ARSIZE    (MS_AR_SIZE[0]),
        .ARBURST   (MS_AR_BURST[0]),
        .ARLOCK    (MS_AR_LOCK[0]),
        .ARCACHE   (MS_AR_CACHE[0]),
        .ARPROT    (MS_AR_PROT[0]),
        .ARVALID   (MS_AR_VALID[0]),
        .ARREADY   (MS_AR_READY[0]),

        .RMASTER   (MS_R_MASTER[0]),
        .RID       (MS_R_ID[0]),
        .RDATA     (MS_R_DATA[0]),
        .RRESP     (MS_R_RESP[0]),
        .RLAST     (MS_R_LAST[0]),
        .RVALID    (MS_R_VALID[0]),
        .RREADY    (MS_R_READY[0]),

        .SRAM_READ_ADDRESS       (SRAM_READ_ADDR[23:0]),
        .SRAM_READ_DATA          (SRAM_READ_DATA[63:0]),
        .SRAM_OUTPUT_ENABLE      (SRAM_OE),

        .SRAM_WRITE_ADDRESS      (SRAM_WRITE_ADDR[23:0]),
        .SRAM_WRITE_DATA         (SRAM_WRITE_DATA[63:0]),
        .SRAM_WRITE_BYTE_ENABLE  (SRAM_WRITE_BE[7:0]),
        .SRAM_WRITE_STROBE       (SRAM_WRITE_STROBE)
  );

  sram #(.address_width (24), .data_width (3)) code_mem (
        .CLK                     (clock),
        .RSTn                    (!reset),

        .READ_ADDR               (SRAM_READ_ADDR[23:0]),
        .DATA_OUT                (SRAM_READ_DATA),
        .OE                      (SRAM_OE),

        .WRITE_ADDR              (SRAM_WRITE_ADDR[23:0]),
        .DATA_IN                 (SRAM_WRITE_DATA),
        .WE                      (SRAM_WRITE_STROBE),
        .BE                      (SRAM_WRITE_BE)
  );

  assign P_AW_ADDR[31]      = 1'b0;
  assign P_AR_ADDR[31]      = 1'b0;

  ExampleRocketSystem riscv_complex(
       .clock                                   (clock),
       .reset                                   (reset),
       .debug_clockeddmi_dmi_req_ready          (debug_clockeddmi_dmi_req_ready),
       .debug_clockeddmi_dmi_req_valid          (debug_clockeddmi_dmi_req_valid),
       .debug_clockeddmi_dmi_req_bits_addr      (debug_clockeddmi_dmi_req_bits_addr),
       .debug_clockeddmi_dmi_req_bits_data      (debug_clockeddmi_dmi_req_bits_data),
       .debug_clockeddmi_dmi_req_bits_op        (debug_clockeddmi_dmi_req_bits_op),
       .debug_clockeddmi_dmi_resp_ready         (debug_clockeddmi_dmi_resp_ready),
       .debug_clockeddmi_dmi_resp_valid         (debug_clockeddmi_dmi_resp_valid),
       .debug_clockeddmi_dmi_resp_bits_data     (debug_clockeddmi_dmi_resp_bits_data),
       .debug_clockeddmi_dmi_resp_bits_resp     (debug_clockeddmi_dmi_resp_bits_resp),
       .debug_clockeddmi_dmiClock               (debug_clockeddmi_dmiClock),
       .debug_clockeddmi_dmiReset               (debug_clockeddmi_dmiReset),
       .debug_ndreset                           (debug_ndreset),
       .debug_dmactive                          (debug_dmactive),
       .interrupts                              (interrupts),
       .mem_axi4_0_aw_ready                     (M_AW_READY[0]),
       .mem_axi4_0_aw_valid                     (M_AW_VALID[0]),
       .mem_axi4_0_aw_bits_id                   (M_AW_ID[0]),
       .mem_axi4_0_aw_bits_addr                 (M_AW_ADDR[0]),
       .mem_axi4_0_aw_bits_len                  (M_AW_LEN[0]),
       .mem_axi4_0_aw_bits_size                 (M_AW_SIZE[0]),
       .mem_axi4_0_aw_bits_burst                (M_AW_BURST[0]),
       .mem_axi4_0_aw_bits_lock                 (M_AW_LOCK[0]),
       .mem_axi4_0_aw_bits_cache                (M_AW_CACHE[0]),
       .mem_axi4_0_aw_bits_prot                 (M_AW_PROT[0]),
       .mem_axi4_0_aw_bits_qos                  (M_AW_QOS[0]),
       .mem_axi4_0_w_ready                      (M_W_READY[0]),
       .mem_axi4_0_w_valid                      (M_W_VALID[0]),
       .mem_axi4_0_w_bits_data                  (M_W_DATA[0]),
       .mem_axi4_0_w_bits_strb                  (M_W_STRB[0]),
       .mem_axi4_0_w_bits_last                  (M_W_LAST[0]),
       .mem_axi4_0_b_ready                      (M_B_READY[0]),
       .mem_axi4_0_b_valid                      (M_B_VALID[0]),
       .mem_axi4_0_b_bits_id                    (M_B_ID[0]),
       .mem_axi4_0_b_bits_resp                  (M_B_RESP[0]),
       .mem_axi4_0_ar_ready                     (M_AR_READY[0]),
       .mem_axi4_0_ar_valid                     (M_AR_VALID[0]),
       .mem_axi4_0_ar_bits_id                   (M_AR_ID[0]),
       .mem_axi4_0_ar_bits_addr                 (M_AR_ADDR[0]),
       .mem_axi4_0_ar_bits_len                  (M_AR_LEN[0]),
       .mem_axi4_0_ar_bits_size                 (M_AR_SIZE[0]),
       .mem_axi4_0_ar_bits_burst                (M_AR_BURST[0]),
       .mem_axi4_0_ar_bits_lock                 (M_AR_LOCK[0]),
       .mem_axi4_0_ar_bits_cache                (M_AR_CACHE[0]),
       .mem_axi4_0_ar_bits_prot                 (M_AR_PROT[0]),
       .mem_axi4_0_ar_bits_qos                  (M_AR_QOS[0]),
       .mem_axi4_0_r_ready                      (M_R_READY[0]),
       .mem_axi4_0_r_valid                      (M_R_VALID[0]),
       .mem_axi4_0_r_bits_id                    (M_R_ID[0]),
       .mem_axi4_0_r_bits_data                  (M_R_DATA[0]),
       .mem_axi4_0_r_bits_resp                  (M_R_RESP[0]),
       .mem_axi4_0_r_bits_last                  (M_R_LAST[0]),
       .mmio_axi4_0_aw_ready                    (P_AW_READY),
       .mmio_axi4_0_aw_valid                    (P_AW_VALID),
       .mmio_axi4_0_aw_bits_id                  (P_AW_ID),
       .mmio_axi4_0_aw_bits_addr                (P_AW_ADDR[30:0]),
       .mmio_axi4_0_aw_bits_len                 (P_AW_LEN),
       .mmio_axi4_0_aw_bits_size                (P_AW_SIZE),
       .mmio_axi4_0_aw_bits_burst               (P_AW_BURST),
       .mmio_axi4_0_aw_bits_lock                (P_AW_LOCK),
       .mmio_axi4_0_aw_bits_cache               (P_AW_CACHE),
       .mmio_axi4_0_aw_bits_prot                (P_AW_PROT),
       .mmio_axi4_0_aw_bits_qos                 (P_AW_QOS),
       .mmio_axi4_0_w_ready                     (P_W_READY),
       .mmio_axi4_0_w_valid                     (P_W_VALID),
       .mmio_axi4_0_w_bits_data                 (P_W_DATA),
       .mmio_axi4_0_w_bits_strb                 (P_W_STRB),
       .mmio_axi4_0_w_bits_last                 (P_W_LAST),
       .mmio_axi4_0_b_ready                     (P_B_READY),
       .mmio_axi4_0_b_valid                     (P_B_VALID),
       .mmio_axi4_0_b_bits_id                   (P_B_ID),
       .mmio_axi4_0_b_bits_resp                 (P_B_RESP),
       .mmio_axi4_0_ar_ready                    (P_AR_READY),
       .mmio_axi4_0_ar_valid                    (P_AR_VALID),
       .mmio_axi4_0_ar_bits_id                  (P_AR_ID),
       .mmio_axi4_0_ar_bits_addr                (P_AR_ADDR[30:0]),
       .mmio_axi4_0_ar_bits_len                 (P_AR_LEN),
       .mmio_axi4_0_ar_bits_size                (P_AR_SIZE),
       .mmio_axi4_0_ar_bits_burst               (P_AR_BURST),
       .mmio_axi4_0_ar_bits_lock                (P_AR_LOCK),
       .mmio_axi4_0_ar_bits_cache               (P_AR_CACHE),
       .mmio_axi4_0_ar_bits_prot                (P_AR_PROT),
       .mmio_axi4_0_ar_bits_qos                 (P_AR_QOS),
       .mmio_axi4_0_r_ready                     (P_R_READY),
       .mmio_axi4_0_r_valid                     (P_R_VALID),
       .mmio_axi4_0_r_bits_id                   (P_R_ID),
       .mmio_axi4_0_r_bits_data                 (P_R_DATA),
       .mmio_axi4_0_r_bits_resp                 (P_R_RESP),
       .mmio_axi4_0_r_bits_last                 (P_R_LAST),
       .l2_frontend_bus_axi4_0_aw_ready         (l2_frontend_bus_axi4_0_aw_ready),
       .l2_frontend_bus_axi4_0_aw_valid         (l2_frontend_bus_axi4_0_aw_valid),
       .l2_frontend_bus_axi4_0_aw_bits_id       (l2_frontend_bus_axi4_0_aw_bits_id),
       .l2_frontend_bus_axi4_0_aw_bits_addr     (l2_frontend_bus_axi4_0_aw_bits_addr),
       .l2_frontend_bus_axi4_0_aw_bits_len      (l2_frontend_bus_axi4_0_aw_bits_len),
       .l2_frontend_bus_axi4_0_aw_bits_size     (l2_frontend_bus_axi4_0_aw_bits_size),
       .l2_frontend_bus_axi4_0_aw_bits_burst    (l2_frontend_bus_axi4_0_aw_bits_burst),
       .l2_frontend_bus_axi4_0_aw_bits_lock     (l2_frontend_bus_axi4_0_aw_bits_lock),
       .l2_frontend_bus_axi4_0_aw_bits_cache    (l2_frontend_bus_axi4_0_aw_bits_cache),
       .l2_frontend_bus_axi4_0_aw_bits_prot     (l2_frontend_bus_axi4_0_aw_bits_prot),
       .l2_frontend_bus_axi4_0_aw_bits_qos      (l2_frontend_bus_axi4_0_aw_bits_qos),
       .l2_frontend_bus_axi4_0_w_ready          (l2_frontend_bus_axi4_0_w_ready),
       .l2_frontend_bus_axi4_0_w_valid          (l2_frontend_bus_axi4_0_w_valid),
       .l2_frontend_bus_axi4_0_w_bits_data      (l2_frontend_bus_axi4_0_w_bits_data),
       .l2_frontend_bus_axi4_0_w_bits_strb      (l2_frontend_bus_axi4_0_w_bits_strb),
       .l2_frontend_bus_axi4_0_w_bits_last      (l2_frontend_bus_axi4_0_w_bits_last),
       .l2_frontend_bus_axi4_0_b_ready          (l2_frontend_bus_axi4_0_b_ready),
       .l2_frontend_bus_axi4_0_b_valid          (l2_frontend_bus_axi4_0_b_valid),
       .l2_frontend_bus_axi4_0_b_bits_id        (l2_frontend_bus_axi4_0_b_bits_id),
       .l2_frontend_bus_axi4_0_b_bits_resp      (l2_frontend_bus_axi4_0_b_bits_resp),
       .l2_frontend_bus_axi4_0_ar_ready         (l2_frontend_bus_axi4_0_ar_ready),
       .l2_frontend_bus_axi4_0_ar_valid         (l2_frontend_bus_axi4_0_ar_valid),
       .l2_frontend_bus_axi4_0_ar_bits_id       (l2_frontend_bus_axi4_0_ar_bits_id),
       .l2_frontend_bus_axi4_0_ar_bits_addr     (l2_frontend_bus_axi4_0_ar_bits_addr),
       .l2_frontend_bus_axi4_0_ar_bits_len      (l2_frontend_bus_axi4_0_ar_bits_len),
       .l2_frontend_bus_axi4_0_ar_bits_size     (l2_frontend_bus_axi4_0_ar_bits_size),
       .l2_frontend_bus_axi4_0_ar_bits_burst    (l2_frontend_bus_axi4_0_ar_bits_burst),
       .l2_frontend_bus_axi4_0_ar_bits_lock     (l2_frontend_bus_axi4_0_ar_bits_lock),
       .l2_frontend_bus_axi4_0_ar_bits_cache    (l2_frontend_bus_axi4_0_ar_bits_cache),
       .l2_frontend_bus_axi4_0_ar_bits_prot     (l2_frontend_bus_axi4_0_ar_bits_prot),
       .l2_frontend_bus_axi4_0_ar_bits_qos      (l2_frontend_bus_axi4_0_ar_bits_qos),
       .l2_frontend_bus_axi4_0_r_ready          (l2_frontend_bus_axi4_0_r_ready),
       .l2_frontend_bus_axi4_0_r_valid          (l2_frontend_bus_axi4_0_r_valid),
       .l2_frontend_bus_axi4_0_r_bits_id        (l2_frontend_bus_axi4_0_r_bits_id),
       .l2_frontend_bus_axi4_0_r_bits_data      (l2_frontend_bus_axi4_0_r_bits_data),
       .l2_frontend_bus_axi4_0_r_bits_resp      (l2_frontend_bus_axi4_0_r_bits_resp),
       .l2_frontend_bus_axi4_0_r_bits_last      (l2_frontend_bus_axi4_0_r_bits_last)
   );

endmodule

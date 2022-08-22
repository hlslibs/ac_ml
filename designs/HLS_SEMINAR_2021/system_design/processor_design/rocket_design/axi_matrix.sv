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

module axi_matrix
   (
            ACLK,
            ARESETn,

            AWID,
            AWADDR,
            AWLEN,
            AWSIZE,
            AWBURST,
            AWLOCK,
            AWCACHE,
            AWPROT,
            AWQOS,
            AWVALID,
            AWREADY,

            //WID,
            WDATA,
            WSTRB,
            WLAST,
            WVALID,
            WREADY,

            BID,
            BRESP,
            BVALID,
            BREADY,

            ARID,
            ARADDR,
            ARLEN,
            ARSIZE,
            ARBURST,
            ARLOCK,
            ARCACHE,
            ARPROT,
            ARQOS,
            ARVALID,
            ARREADY,

            RID,
            RDATA,
            RRESP,
            RLAST,
            RVALID,
            RREADY,

            S_AWMASTER,
            S_AWID,
            S_AWADDR,
            S_AWLEN,
            S_AWSIZE,
            S_AWBURST,
            S_AWLOCK,
            S_AWCACHE,
            S_AWPROT,
            S_AWQOS,
            S_AWVALID,
            S_AWREADY,

            S_WMASTER,
            S_WID,
            S_WDATA,
            S_WSTRB,
            S_WLAST,
            S_WVALID,
            S_WREADY,

            S_BMASTER,
            S_BID,
            S_BRESP,
            S_BVALID,
            S_BREADY,

            S_ARMASTER,
            S_ARID,
            S_ARADDR,
            S_ARLEN,
            S_ARSIZE,
            S_ARBURST,
            S_ARLOCK,
            S_ARCACHE,
            S_ARPROT,
            S_ARQOS,
            S_ARVALID,
            S_ARREADY,

            S_RMASTER,
            S_RID,
            S_RDATA,
            S_RRESP,
            S_RLAST,
            S_RVALID,
            S_RREADY

    );

    parameter masters  = 3;
    parameter slaves   = 2;
    parameter channels = 1;
    parameter mem_type = 1;

    input                            ACLK;
    input                            ARESETn;

    input  [`id_bits-1:0]            AWID[masters-1:0];
    input  [`addr_bits-1:0]          AWADDR[masters-1:0];
    input  [`len_bits-1:0]           AWLEN[masters-1:0];
    input  [`size_bits-1:0]          AWSIZE[masters-1:0];
    input  [`burst_bits-1:0]         AWBURST[masters-1:0];
    input  [`lock_bits-1:0]          AWLOCK[masters-1:0];
    input  [`cache_bits-1:0]         AWCACHE[masters-1:0];
    input  [`prot_bits-1:0]          AWPROT[masters-1:0];
    input  [`qos_bits-1:0]           AWQOS[masters-1:0];
    input                            AWVALID[masters-1:0];
    output                           AWREADY[masters-1:0];

    //input  [`id_bits-1:0]            WID[masters-1:0];
    //reg    [`id_bits-1:0]            WID[masters-1:0] = { masters * `id_bits {1'b0}};
    input  [`data_bits-1:0]          WDATA[masters-1:0];
    input  [`strb_bits-1:0]          WSTRB[masters-1:0];
    input                            WLAST[masters-1:0];
    input                            WVALID[masters-1:0];
    output                           WREADY[masters-1:0];

    output [`id_bits-1:0]            BID[masters-1:0];
    output [`resp_bits-1:0]          BRESP[masters-1:0];
    output                           BVALID[masters-1:0];
    input                            BREADY[masters-1:0];

    input  [`id_bits-1:0]            ARID[masters-1:0];
    input  [`addr_bits-1:0]          ARADDR[masters-1:0];
    input  [`len_bits-1:0]           ARLEN[masters-1:0];
    input  [`size_bits-1:0]          ARSIZE[masters-1:0];
    input  [`burst_bits-1:0]         ARBURST[masters-1:0];
    input  [`lock_bits-1:0]          ARLOCK[masters-1:0];
    input  [`cache_bits-1:0]         ARCACHE[masters-1:0];
    input  [`prot_bits-1:0]          ARPROT[masters-1:0];
    input  [`qos_bits-1:0]           ARQOS[masters-1:0];
    input                            ARVALID[masters-1:0];
    output                           ARREADY[masters-1:0];

    output [`id_bits-1:0]            RID[masters-1:0];
    output [`data_bits-1:0]          RDATA[masters-1:0];
    output [`resp_bits-1:0]          RRESP[masters-1:0];
    output                           RLAST[masters-1:0];
    output                           RVALID[masters-1:0];
    input                            RREADY[masters-1:0];

    output [masters-1:0]             S_AWMASTER[slaves-1:0];
    output [`id_bits-1:0]            S_AWID[slaves-1:0];
    output [`addr_bits-1:0]          S_AWADDR[slaves-1:0];
    output [`len_bits-1:0]           S_AWLEN[slaves-1:0];
    output [`size_bits-1:0]          S_AWSIZE[slaves-1:0];
    output [`burst_bits-1:0]         S_AWBURST[slaves-1:0];
    output [`lock_bits-1:0]          S_AWLOCK[slaves-1:0];
    output [`cache_bits-1:0]         S_AWCACHE[slaves-1:0];
    output [`prot_bits-1:0]          S_AWPROT[slaves-1:0];
    output [`qos_bits-1:0]           S_AWQOS[slaves-1:0];
    output                           S_AWVALID[slaves-1:0];
    input                            S_AWREADY[slaves-1:0];

    output [masters-1:0]             S_WMASTER[slaves-1:0];
    output [`id_bits-1:0]            S_WID[slaves-1:0];
    output [`data_bits-1:0]          S_WDATA[slaves-1:0];
    output [`strb_bits-1:0]          S_WSTRB[slaves-1:0];
    output                           S_WLAST[slaves-1:0];
    output                           S_WVALID[slaves-1:0];
    input                            S_WREADY[slaves-1:0];

    input  [masters-1:0]             S_BMASTER[slaves-1:0];
    input  [`id_bits-1:0]            S_BID[slaves-1:0];
    input  [`resp_bits-1:0]          S_BRESP[slaves-1:0];
    input                            S_BVALID[slaves-1:0];
    output                           S_BREADY[slaves-1:0];

    output [masters-1:0]             S_ARMASTER[slaves-1:0];
    output [`id_bits-1:0]            S_ARID[slaves-1:0];
    output [`addr_bits-1:0]          S_ARADDR[slaves-1:0];
    output [`len_bits-1:0]           S_ARLEN[slaves-1:0];
    output [`size_bits-1:0]          S_ARSIZE[slaves-1:0];
    output [`burst_bits-1:0]         S_ARBURST[slaves-1:0];
    output [`lock_bits-1:0]          S_ARLOCK[slaves-1:0];
    output [`cache_bits-1:0]         S_ARCACHE[slaves-1:0];
    output [`prot_bits-1:0]          S_ARPROT[slaves-1:0];
    output [`qos_bits-1:0]           S_ARQOS[slaves-1:0];
    output                           S_ARVALID[slaves-1:0];
    input                            S_ARREADY[slaves-1:0];

    input  [masters-1:0]             S_RMASTER[slaves-1:0];
    input  [`id_bits-1:0]            S_RID[slaves-1:0];
    input  [`data_bits-1:0]          S_RDATA[slaves-1:0];
    input  [`resp_bits-1:0]          S_RRESP[slaves-1:0];
    input                            S_RLAST[slaves-1:0];
    input                            S_RVALID[slaves-1:0];
    output                           S_RREADY[slaves-1:0];


    reg    [masters-1:0]             C_AWMASTER[channels-1:0];
    reg    [`id_bits-1:0]            C_AWID[channels-1:0];
    reg    [`addr_bits-1:0]          C_AWADDR[channels-1:0];
    reg    [`len_bits-1:0]           C_AWLEN[channels-1:0];
    reg    [`size_bits-1:0]          C_AWSIZE[channels-1:0];
    reg    [`burst_bits-1:0]         C_AWBURST[channels-1:0];
    reg    [`lock_bits-1:0]          C_AWLOCK[channels-1:0];
    reg    [`cache_bits-1:0]         C_AWCACHE[channels-1:0];
    reg    [`prot_bits-1:0]          C_AWPROT[channels-1:0];
    reg    [`qos_bits-1:0]           C_AWQOS[channels-1:0];
    reg                              C_AWVALID[channels-1:0];
    reg                              C_AWREADY[channels-1:0];

    reg    [masters-1:0]             C_WMASTER[channels-1:0];
    reg    [`id_bits-1:0]            C_WID[channels-1:0];
    reg    [`data_bits-1:0]          C_WDATA[channels-1:0];
    reg    [`strb_bits-1:0]          C_WSTRB[channels-1:0];
    reg                              C_WLAST[channels-1:0];
    reg                              C_WVALID[channels-1:0];
    reg                              C_WREADY[channels-1:0];

    reg    [masters-1:0]             C_BMASTER[channels-1:0];
    reg    [`id_bits-1:0]            C_BID[channels-1:0];
    reg    [`resp_bits-1:0]          C_BRESP[channels-1:0];
    reg                              C_BVALID[channels-1:0];
    reg                              C_BREADY[channels-1:0];

    reg    [masters-1:0]             C_ARMASTER[channels-1:0];
    reg    [`id_bits-1:0]            C_ARID[channels-1:0];
    reg    [`addr_bits-1:0]          C_ARADDR[channels-1:0];
    reg    [`len_bits-1:0]           C_ARLEN[channels-1:0];
    reg    [`size_bits-1:0]          C_ARSIZE[channels-1:0];
    reg    [`burst_bits-1:0]         C_ARBURST[channels-1:0];
    reg    [`lock_bits-1:0]          C_ARLOCK[channels-1:0];
    reg    [`cache_bits-1:0]         C_ARCACHE[channels-1:0];
    reg    [`prot_bits-1:0]          C_ARPROT[channels-1:0];
    reg    [`qos_bits-1:0]           C_ARQOS[channels-1:0];
    reg                              C_ARVALID[channels-1:0];
    reg                              C_ARREADY[channels-1:0];

    reg    [masters-1:0]             C_RMASTER[channels-1:0];
    reg    [`id_bits-1:0]            C_RID[channels-1:0];
    reg    [`data_bits-1:0]          C_RDATA[channels-1:0];
    reg    [`resp_bits-1:0]          C_RRESP[channels-1:0];
    reg                              C_RLAST[channels-1:0];
    reg                              C_RVALID[channels-1:0];
    reg                              C_RREADY[channels-1:0];

    reg    [slaves-1:0]              slave_table[(1<<(masters+`id_bits))-1:0];

    reg    [slaves-1:0]              AWSLAVE;
    reg    [slaves-1:0]              AWSLAVE_delayed;
    reg    [slaves-1:0]              WSLAVE;
    reg    [masters-1:0]             BSLAVE;   // peripheral is the master
    reg    [slaves-1:0]              ARSLAVE;
    reg    [masters-1:0]             RSLAVE;   // peripheral is the master

    reg    [masters-1:0]             AWMASTER;
    reg    [masters-1:0]             WMASTER;
    reg    [slaves-1:0]              BMASTER;  // peripheral is the master
    reg    [masters-1:0]             ARMASTER;
    reg    [slaves-1:0]              RMASTER;  // peripheral is the master

    reg                              AR_BUSERR;
    reg                              AW_BUSERR;

    reg  [31:0]                      ONE = 32'hFFFFFFFF;
    reg  [31:0]                      ZERO = 32'h00000000;
    reg  [31:0]                      FLOAT = 32'h00000000;

    wire [(1<<(masters+1))-1:0]      slave_table_read_index;
    wire [(1<<(masters+1))-1:0]      resp_table_read_index;
    genvar m;
    genvar s;

    wire                             strobe_aw = C_AWVALID[0] && C_AWREADY[0];
    wire                             strobe_w  = C_WVALID[0]  && C_WREADY[0];
    wire                             strobe_b  = C_BVALID[0]  && C_BREADY[0];
    wire                             strobe_ar = C_ARVALID[0] && C_ARREADY[0];
    wire                             strobe_r  = C_RVALID[0]  && C_RREADY[0];

    wire [31:0]                      payload_aw = (strobe_aw) ? C_AWADDR[0] : 32'hzzzzzzzz;
    wire [63:0]                      payload_w  = (strobe_w)  ? C_WDATA[0]  : 64'hzzzzzzzzzzzzzzzz;
    wire [31:0]                      payload_ar = (strobe_ar) ? C_ARADDR[0] : 32'hzzzzzzzz;
    wire [63:0]                      payload_r  = (strobe_r)  ? C_RDATA[0]  : 64'hzzzzzzzzzzzzzzzz;

    wire [slaves-1:0]                wslave_array[masters-1:0];
    wire [masters-1:0]               write_slave_available;
    wire [masters-1:0]               wslave_fifo_full;
    
    reg  [masters-1:0]               save_wslave;
    reg  [masters-1:0]               wslave_done;
    reg  [masters-1:0]               last_awmaster;
    reg  [masters-1:0]               last_last;

    always @(posedge ACLK) begin
       AWSLAVE_delayed <= AWSLAVE;
    end

    generate 
       for (m=0; m<masters; m++) begin
          bus_fifo #(slaves, 4) slave_fifo (
             .CLK            (ACLK),
             .RESET_N        (ARESETn),
             .DATA_STROBE    (save_wslave[m]),
             .DATA_IN        (AWSLAVE_delayed),
             .DATA_READY     (write_slave_available[m]), 
             .DATA_OUT       (wslave_array[m]),
             .DATA_ACK       (wslave_done[m]),
             .FULL           (wslave_fifo_full[m])
          );
          always @(posedge ACLK) begin
   
             // load data into fifo on rising edge of AWMASTER[m]
             last_awmaster[m] <= AWMASTER[m];
             save_wslave[m] <= (last_awmaster[m] == 1'b0) && (AWMASTER[m] == 1'b1) && (wslave_fifo_full[m] == 1'b0);
             
             // unload data from fifo on falling edge of valid WLAST[m]
             last_last[m] <= WREADY[m] & WVALID[m] &  WLAST[m];
             wslave_done[m] <= (last_last[m] == 1'b1) && ((WREADY[m] & WVALID[m] & WLAST[m]) == 1'b0);
          end
       end
    endgenerate


    generate
       for (m=0; m<(1<<(masters+`id_bits)); m++) begin
           initial slave_table[m] = 32'b00000000;
       end
    endgenerate

    //  Write address segment logic

    segment_arbiter #(masters) address_write_channel_arbiter(AWMASTER, AWVALID, ACLK, ARESETn);
    generate
       if (mem_type) segment_slave_mem   #(masters, slaves) address_write_channel_slave(AWMASTER, AWADDR, AWSLAVE, AW_BUSERR);
       else          segment_slave_io    #(masters, slaves) address_write_channel_slave(AWMASTER, AWADDR, AWSLAVE, AW_BUSERR);
    endgenerate

    assign C_AWMASTER[0] = AWMASTER;

    bmux #(masters, `id_bits)      mux00 (C_AWID[0],    AWMASTER, AWID);
    bmux #(masters, `addr_bits)    mux01 (C_AWADDR[0],  AWMASTER, AWADDR);
    bmux #(masters, `len_bits)     mux02 (C_AWLEN[0],   AWMASTER, AWLEN);
    bmux #(masters, `size_bits)    mux03 (C_AWSIZE[0],  AWMASTER, AWSIZE);
    bmux #(masters, `burst_bits)   mux04 (C_AWBURST[0], AWMASTER, AWBURST);
    bmux #(masters, `lock_bits)    mux05 (C_AWLOCK[0],  AWMASTER, AWLOCK);
    bmux #(masters, `cache_bits)   mux06 (C_AWCACHE[0], AWMASTER, AWCACHE);
    bmux #(masters, `prot_bits)    mux07 (C_AWPROT[0],  AWMASTER, AWPROT);
    bmux #(masters, `qos_bits)     mux08 (C_AWQOS[0],   AWMASTER, AWQOS);
    bmux #(masters, 1)             mux09 (C_AWVALID[0], AWMASTER, AWVALID);

    bmux #(slaves,  1)             mux10 (C_AWREADY[0], AWSLAVE,  S_AWREADY);

    generate 
       for (m=0; m<masters; m++) begin
          assign AWREADY[m] = AWMASTER[m] ? C_AWREADY[0] : ZERO;
       end
    endgenerate

    // output enables for slaves

    generate 
       for (s=0; s<slaves; s++) begin
          assign S_AWMASTER[s] = AWSLAVE[s] ? C_AWMASTER[0] : FLOAT;
          assign S_AWID[s] =     AWSLAVE[s] ? C_AWID[0]     : FLOAT;
          assign S_AWADDR[s] =   AWSLAVE[s] ? C_AWADDR[0]   : FLOAT;
          assign S_AWLEN[s] =    AWSLAVE[s] ? C_AWLEN[0]    : FLOAT;
          assign S_AWSIZE[s] =   AWSLAVE[s] ? C_AWSIZE[0]   : FLOAT;
          assign S_AWBURST[s] =  AWSLAVE[s] ? C_AWBURST[0]  : FLOAT;
          assign S_AWLOCK[s] =   AWSLAVE[s] ? C_AWLOCK[0]   : FLOAT;
          assign S_AWCACHE[s] =  AWSLAVE[s] ? C_AWCACHE[0]  : FLOAT;
          assign S_AWPROT[s] =   AWSLAVE[s] ? C_AWPROT[0]   : FLOAT;
          assign S_AWQOS[s] =    AWSLAVE[s] ? C_AWQOS[0]    : FLOAT;
          assign S_AWVALID[s] =  AWSLAVE[s] ? C_AWVALID[0]  : FLOAT;
       end
    endgenerate

    // save master-id mapping in slave table
    generate
       for (m=0; m<masters; m++) begin
          always @(posedge ACLK) begin
             if (AWMASTER[m]) begin
                  // $display("Setting slave to ", AWSLAVE, " at ",{AWMASTER, AWID[m]});
                  // $display("master = ", AWMASTER, " transcation_id = ", AWID[m], " Slave = ", AWSLAVE);
                  slave_table[{AWMASTER, AWID[m]}] = AWSLAVE;  
             end
          end
       end
    endgenerate


    // Write data segment logic

    segment_arbiter #(masters) data_write_channel_arbiter(WMASTER, WVALID, ACLK, ARESETn);
    
    assign slave_table_read_index = {WMASTER, C_WID[0]};
    //assign WSLAVE = slave_table[slave_table_read_index];
    bmux #(masters, slaves) wslave_mux (WSLAVE, WMASTER & write_slave_available, wslave_array); 

        

    assign C_WMASTER[0] = WMASTER;

    // bmux #(masters, `id_bits)      mux11 (C_WID[0],    WMASTER, WID);
    assign C_WID[0] = {`id_bits {1'b0}};
    bmux #(masters, `data_bits)    mux12 (C_WDATA[0],  WMASTER, WDATA);
    bmux #(masters, `strb_bits)    mux13 (C_WSTRB[0],  WMASTER, WSTRB);
    bmux #(masters, 1)             mux14 (C_WLAST[0],  WMASTER, WLAST);
    bmux #(masters, 1)             mux15 (C_WVALID[0], WMASTER, WVALID);

    bmux #(slaves, 1)              mux16 (C_WREADY[0], WSLAVE,  S_WREADY);

    generate 
       for (m=0; m<masters; m++) begin
          assign WREADY[m] = WMASTER[m] ? C_WREADY[0] : ZERO;
       end
    endgenerate

    generate 
       for (s=0; s<slaves; s++) begin
          assign S_WMASTER[s] = WSLAVE[s] ? C_WMASTER[0] : FLOAT;
          assign S_WID[s] =     WSLAVE[s] ? C_WID[0]     : FLOAT;
          assign S_WDATA[s] =   WSLAVE[s] ? C_WDATA[0]   : FLOAT;
          assign S_WSTRB[s] =   WSLAVE[s] ? C_WSTRB[0]   : FLOAT;
          assign S_WLAST[s] =   WSLAVE[s] ? C_WLAST[0]   : FLOAT;
          assign S_WVALID[s] =  WSLAVE[s] ? C_WVALID[0]  : FLOAT;
       end
    endgenerate


    // Write response segment logic

    segment_arbiter #(slaves) write_response_channel_arbiter(BMASTER, S_BVALID, ACLK, ARESETn);

    bmux #(slaves, masters)        mux20 (C_BMASTER[0], BMASTER, S_BMASTER);
    bmux #(slaves, `id_bits)       mux21 (C_BID[0],     BMASTER, S_BID);
    bmux #(slaves, `resp_bits)     mux22 (C_BRESP[0],   BMASTER, S_BRESP);
    bmux #(slaves, 1)              mux23 (C_BVALID[0],  BMASTER, S_BVALID);

    bmux #(masters, 1)             mux24 (C_BREADY[0],  C_BMASTER[0],  BREADY);

    generate 
       for (s=0; s<slaves; s++) begin
          assign S_BREADY[s] = BMASTER[s] ? C_BREADY[0] : ZERO;
       end
    endgenerate

    assign BSLAVE = C_BMASTER[0];

    generate 
       for (m=0; m<masters; m++) begin
          assign BID[m] =     BSLAVE[m] ? C_BID[0]     : FLOAT;
          assign BRESP[m] =   BSLAVE[m] ? C_BRESP[0]   : FLOAT;
          assign BVALID[m] =  BSLAVE[m] ? C_BVALID[0]  : FLOAT;
       end
    endgenerate

    //  Read address segment logic

    segment_arbiter #(masters) address_read_channel_arbiter(ARMASTER, ARVALID, ACLK, ARESETn);
    generate
       if (mem_type) segment_slave_mem   #(masters, slaves) address_read_channel_slave(ARMASTER, ARADDR, ARSLAVE, AR_BUSERR);
       else          segment_slave_io    #(masters, slaves) address_read_channel_slave(ARMASTER, ARADDR, ARSLAVE, AR_BUSERR);
    endgenerate

    assign C_ARMASTER[0] = ARMASTER;

    bmux #(masters, `id_bits)      mux30 (C_ARID[0],    ARMASTER, ARID);
    bmux #(masters, `addr_bits)    mux31 (C_ARADDR[0],  ARMASTER, ARADDR);
    bmux #(masters, `len_bits)     mux32 (C_ARLEN[0],   ARMASTER, ARLEN);
    bmux #(masters, `size_bits)    mux33 (C_ARSIZE[0],  ARMASTER, ARSIZE);
    bmux #(masters, `burst_bits)   mux34 (C_ARBURST[0], ARMASTER, ARBURST);
    bmux #(masters, `lock_bits)    mux35 (C_ARLOCK[0],  ARMASTER, ARLOCK);
    bmux #(masters, `cache_bits)   mux36 (C_ARCACHE[0], ARMASTER, ARCACHE);
    bmux #(masters, `prot_bits)    mux37 (C_ARPROT[0],  ARMASTER, ARPROT);
    bmux #(masters, `qos_bits)     mux38 (C_ARQOS[0],   ARMASTER, ARQOS);
    bmux #(masters, 1)             mux39 (C_ARVALID[0], ARMASTER, ARVALID);

    bmux #(slaves, 1)              mux40 (C_ARREADY[0], ARSLAVE,  S_ARREADY);

    generate 
       for (m=0; m<masters; m++) begin
          assign ARREADY[m] = ARMASTER[m] ? C_ARREADY[0] : ZERO;
       end
    endgenerate

    // output enables for slaves

    generate 
       for (s=0; s<slaves; s++) begin
          assign S_ARMASTER[s] = ARSLAVE[s] ? C_ARMASTER[0] : FLOAT;
          assign S_ARID[s] =     ARSLAVE[s] ? C_ARID[0]     : FLOAT;
          assign S_ARADDR[s] =   ARSLAVE[s] ? C_ARADDR[0]   : FLOAT;
          assign S_ARLEN[s] =    ARSLAVE[s] ? C_ARLEN[0]    : FLOAT;
          assign S_ARSIZE[s] =   ARSLAVE[s] ? C_ARSIZE[0]   : FLOAT;
          assign S_ARBURST[s] =  ARSLAVE[s] ? C_ARBURST[0]  : FLOAT;
          assign S_ARLOCK[s] =   ARSLAVE[s] ? C_ARLOCK[0]   : FLOAT;
          assign S_ARCACHE[s] =  ARSLAVE[s] ? C_ARCACHE[0]  : FLOAT;
          assign S_ARPROT[s] =   ARSLAVE[s] ? C_ARPROT[0]   : FLOAT;
          assign S_ARQOS[s] =    ARSLAVE[s] ? C_ARQOS[0]    : FLOAT;
          assign S_ARVALID[s] =  ARSLAVE[s] ? C_ARVALID[0]  : FLOAT;
       end
    endgenerate

    // Read data/response segment logic

    segment_arbiter #(slaves) read_response_channel_arbiter(RMASTER, S_RVALID, ACLK, ARESETn);

    // assign RMASTER = S_RMASTER[BSLAVE];

    bmux #(slaves, masters)      mux41 (C_RMASTER[0], RMASTER, S_RMASTER);
    bmux #(slaves, `id_bits)     mux42 (C_RID[0],     RMASTER, S_RID);
    bmux #(slaves, `data_bits)   mux43 (C_RDATA[0],   RMASTER, S_RDATA);
    bmux #(slaves, `resp_bits)   mux44 (C_RRESP[0],   RMASTER, S_RRESP);
    bmux #(slaves, 1)            mux45 (C_RLAST[0],   RMASTER, S_RLAST);
    bmux #(slaves, 1)            mux46 (C_RVALID[0],  RMASTER, S_RVALID);

    bmux #(masters, 1)           mux47 (C_RREADY[0], C_RMASTER[0],  RREADY);

    generate 
       for (s=0; s<slaves; s++) begin
          assign S_RREADY[s] = RMASTER[s] ? C_RREADY[0] : ZERO;
       end
    endgenerate

    assign RSLAVE = C_RMASTER[0];

    generate 
       for (m=0; m<masters; m++) begin
          assign RID[m] =     RSLAVE[m] ? C_RID[0]     : FLOAT;
          assign RDATA[m] =   RSLAVE[m] ? C_RDATA[0]   : FLOAT;
          assign RRESP[m] =   RSLAVE[m] ? C_RRESP[0]   : FLOAT;
          assign RLAST[m] =   RSLAVE[m] ? C_RLAST[0]   : FLOAT;
          assign RVALID[m] =  RSLAVE[m] ? C_RVALID[0]  : FLOAT;
       end
    endgenerate

endmodule

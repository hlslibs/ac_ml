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

module axi_slave_interface
    (

// AXI slave side interface 

            ACLK,
            ARESETn,

            AWMASTER,
            AWID,
            AWADDR,
            AWLEN,
            AWSIZE,
            AWBURST,
            AWLOCK,
            AWCACHE,
            AWPROT,
            AWVALID,
            AWREADY,

            WMASTER,
            WID,
            WDATA,
            WSTRB,
            WLAST,
            WVALID,
            WREADY,

            BMASTER,
            BID,
            BRESP,
            BVALID,
            BREADY,
        
            ARMASTER,
            ARID,
            ARADDR,
            ARLEN,
            ARSIZE,
            ARBURST,
            ARLOCK,
            ARCACHE,
            ARPROT,
            ARVALID,
            ARREADY,

            RMASTER,
            RID,
            RDATA,
            RRESP,
            RLAST,
            RVALID,
            RREADY,

// SRAM interface

            SRAM_READ_ADDRESS,
            SRAM_READ_DATA,
            SRAM_OUTPUT_ENABLE,

            SRAM_WRITE_ADDRESS,
            SRAM_WRITE_DATA,
            SRAM_WRITE_BYTE_ENABLE,
            SRAM_WRITE_STROBE
    );

    parameter masters  = 4;
    parameter width    = 22;
    parameter id_bits  = 2;
    parameter p_size   = 3;   // peripheral data width in 2^p_size bytes
    parameter b_size   = 3;   // bus width in 2^b_size bytes

`define p_bytes (1<<p_size)
`define b_bytes (1<<b_size)
`define lanes   (1<<(b_size-p_size))
`define b_bits  (8 * (1<<b_size))
`define p_bits  (8 * (1<<p_size))

    input                             ACLK;
    input                             ARESETn;

    input  [masters-1:0]              AWMASTER;
    input  [id_bits-1:0]              AWID;
    input  [width-1:0]                AWADDR;
    input  [`len_bits-1:0]            AWLEN;
    input  [`size_bits-1:0]           AWSIZE;
    input  [`burst_bits-1:0]          AWBURST;
    input  [`lock_bits-1:0]           AWLOCK;
    input  [`cache_bits-1:0]          AWCACHE;
    input  [`prot_bits-1:0]           AWPROT;
    input                             AWVALID;
    output                            AWREADY;

    input  [masters-1:0]              WMASTER;
    input  [id_bits-1:0]              WID;
    input  [(`b_bits)-1:0]            WDATA;
    input  [(`b_bytes)-1:0]           WSTRB;
    input                             WLAST;
    input                             WVALID;
    output                            WREADY;

    output [masters-1:0]              BMASTER;
    output [id_bits-1:0]              BID;
    output [`resp_bits-1:0]           BRESP;
    output                            BVALID;
    input                             BREADY;

    input  [masters-1:0]              ARMASTER;
    input  [id_bits-1:0]              ARID;
    input  [width-1:0]                ARADDR;
    input  [`len_bits-1:0]            ARLEN;
    input  [`size_bits-1:0]           ARSIZE;
    input  [`burst_bits-1:0]          ARBURST;
    input  [`lock_bits-1:0]           ARLOCK;
    input  [`cache_bits-1:0]          ARCACHE;
    input  [`prot_bits-1:0]           ARPROT;
    input                             ARVALID;
    output                            ARREADY;

    output [masters-1:0]              RMASTER;
    output [id_bits-1:0]              RID;
    output [(`b_bits)-1:0]            RDATA;
    output [`resp_bits-1:0]           RRESP;
    output                            RLAST;
    output                            RVALID;
    input                             RREADY;
    
    output [width-1:0]                SRAM_READ_ADDRESS;
    input  [(`p_bits)-1:0]            SRAM_READ_DATA;
    output                            SRAM_OUTPUT_ENABLE;

    output [width-1:0]                SRAM_WRITE_ADDRESS;
    output [(`p_bits)-1:0]            SRAM_WRITE_DATA;
    output [(`p_bytes)-1:0]           SRAM_WRITE_BYTE_ENABLE;
    output                            SRAM_WRITE_STROBE;


    wire   [masters-1:0]              O_RMASTER;
    wire   [width-1:0]                O_RADDR;
    wire   [id_bits-1:0]              O_RID;
    wire                              O_RLAST;

    wire   [masters-1:0]              O_WMASTER;
    wire   [width-1:0]                O_WADDR;
    wire   [id_bits-1:0]              O_WID;
    wire                              O_WLAST;

    reg                               O_BVALID;
    reg    [masters-1:0]              O_BMASTER;
    reg    [id_bits-1:0]              O_BID;

    reg    [masters-1:0]              D_MASTER;
    reg    [id_bits-1:0]              D_ID;
    wire                              WRITE_ADDR_VALID;
    wire                              WRITE_ADDR_READY;
    wire                              D_RADDR_VALID;
    wire                              D_RADDR_READY;
    reg    [(`b_bytes)-1:0]           D_BE;
    reg                               D_LAST;
    reg    [(`b_bits)-1:0]            D_RDATA;
    reg    [(`b_bits)-1:0]            D_WDATA;
    reg                               read_partial_data_valid;
    wire                              READ_DATA_VALID;
    wire                              READ_DATA_READY;

    wire [1:0]                        rresp_tmp;
    reg                               write_cycle_active;

    reg  [masters:0]                  write_master;   // holds master for current write cycle
    reg  [id_bits-1:0]                write_id;       // holds transaction id for current write cycle
    reg  [7:0]                        sent_byte_count;
    reg  [7:0]                        bus_width = `b_bytes;

    wire                              strobe_aw           = AWVALID && AWREADY;
    wire                              strobe_w            = WVALID  && WREADY;
    wire                              strobe_wd           = WRITE_ADDR_VALID && WRITE_ADDR_READY;
    wire                              strobe_b            = BVALID  && BREADY;
    wire                              strobe_ar           = ARVALID && ARREADY;
    wire                              strobe_r            = RVALID  && RREADY;

    wire [(`p_bits)-1:0]              p_wdata; 
    wire [(`p_bits)-1:0]              p_rdata; 
    wire [(`p_bytes)-1:0]             p_be;

    wire [31:0]                       payload_aw     = strobe_aw ? AWADDR : 32'hzzzzzzzz;
    wire [(`b_bits)-1:0]              payload_w      = strobe_w  ? WDATA  : 64'hzzzzzzzzzzzzzzzz;
    wire [(`b_bits)-1:0]              payload_wd     = strobe_wd ? p_wdata: 64'hzzzzzzzzzzzzzzzz;
    wire [31:0]                       payload_ra     = strobe_ar ? ARADDR : 32'hzzzzzzzz;
    wire [(`b_bits)-1:0]              payload_r      = strobe_r  ? RDATA  : 64'hzzzzzzzzzzzzzzzz;

    wire                              write_done          = strobe_w;
    wire                              generate_response   = strobe_w && WLAST;
    wire                              send_response;
    reg                               b_wait;
    reg                               address_error;
   
    reg                               more_data;
    wire                              get_more_data;

    genvar n;

    assign RRESP = 2'b00;
    assign BRESP = 2'b00;

    axi_addr_latch #(masters, width, p_size) write_address_request_bus (
        .CLK           (ACLK),
        .RESETN        (ARESETn),

        .MASTER        (AWMASTER),
        .ID            (AWID),
        .ADDR          (AWADDR),
        .LEN           (AWLEN),
        .SIZE          (AWSIZE),
        .BURST         (AWBURST),
        .LOCK          (AWLOCK),
        .CACHE         (AWCACHE),
        .PROT          (AWPROT),

        .VALID         (AWVALID),
        .READY         (AWREADY),

        .O_MASTER      (O_WMASTER),
        .O_ID          (O_WID),
        .O_ADDR        (O_WADDR),
        .O_LAST        (O_WLAST),

        .O_VALID       (WRITE_ADDR_VALID),
        .O_READY       (WRITE_ADDR_READY)
    );

    always @(posedge ACLK) begin
        if (ARESETn == 0) begin
            write_master <= 32'h00000000;
            write_cycle_active <= 0;
            more_data <= 0;
        end else begin
            if (strobe_wd) begin
                write_master <= O_WMASTER;
                write_id <= O_WID;
                more_data <= !O_WLAST;
                write_cycle_active <= 1;
            end
            if (strobe_b) begin
                write_cycle_active <= 0;
            end
            if (send_response) begin
                write_master <= 32'h00000000;
                more_data <= 1'b0;
            end
        end
    end

    assign get_more_data = more_data || !write_cycle_active;

    ready_gen #(0) ready_gen_write_data (  // wait states on the address write data bus
            .CLK         (ACLK),
            .RESETn      (ARESETn),
            .STROBE      (WVALID && WRITE_ADDR_VALID & get_more_data),  
            .READY       (WREADY)
    );
  
    assign WRITE_ADDR_READY = WREADY;

    always @(posedge ACLK) begin
        if (ARESETn) begin
             address_error <= 0;
        end else begin
            if (strobe_w) begin
                 if ((write_master != WMASTER) || (write_id != WID)) address_error <= 1;
            end
            if (strobe_wd) begin
                 if ((write_master != O_WMASTER) || (write_id != O_WID)) address_error <= 1;
            end
            if (address_error) begin
                $display("Address error");
                $finish();
            end
        end
    end

    always @(posedge ACLK) begin
        if (ARESETn == 0) begin
            b_wait <= 1'b0;
        end else begin
            if ((generate_response == 1) && (b_wait == 0)) b_wait <= 1;
            if ((send_response == 1) && (b_wait == 1)) b_wait <= 0;
        end
    end

    ready_gen #(0)   ready_gen_b_resp (  // period to wait before sending the b_valid response to master
            .CLK         (ACLK),
            .RESETn      (ARESETn),
            .STROBE      (b_wait),
            .READY       (send_response)
    );
             
    assign BVALID = O_BVALID;
    assign BMASTER = O_BMASTER;
    assign BID = O_BID;

    always @(posedge ACLK) begin
        if (ARESETn == 0) begin
            O_BVALID <= 0;
            O_BMASTER <= 32'hzzzzzzzz;
            O_BID <= 32'hzzzzzzzz;
        end else begin
            if (send_response) begin
                O_BVALID <= 1;
                O_BMASTER <= write_master;
                O_BID <= write_id;
            end
            if (O_BVALID && BREADY) begin
                O_BVALID <= 0;
                O_BMASTER <= 32'hzzzzzzzz;
                O_BID <= 32'hzzzzzzzz;
            end
        end
    end

    axi_addr_latch #(masters, width, p_size) read_address_request_bus (
        .CLK           (ACLK),
        .RESETN        (ARESETn),

        .MASTER        (ARMASTER),
        .ID            (ARID),
        .ADDR          (ARADDR),
        .LEN           (ARLEN),
        .SIZE          (ARSIZE),
        .BURST         (ARBURST),
        .LOCK          (ARLOCK),
        .CACHE         (ARCACHE),
        .PROT          (ARPROT),

        .VALID         (ARVALID),
        .READY         (ARREADY),

        .O_MASTER      (O_RMASTER),
        .O_ID          (O_RID),
        .O_ADDR        (O_RADDR),
        .O_LAST        (O_RLAST),

        .O_VALID       (D_RADDR_VALID),
        .O_READY       (D_RADDR_READY)
    );

    ready_gen #(0) data_ready (ACLK, ARESETn, D_RADDR_VALID, D_RADDR_READY);
         
    always @(posedge ACLK) begin
         D_MASTER <= O_RMASTER;
         D_ID     <= O_RID;
         D_LAST   <= O_RLAST;
    end 

    // connections out to the SRAM device

    assign SRAM_READ_ADDRESS        = O_RADDR[width-1:p_size];
    assign p_rdata                  = SRAM_READ_DATA;
    assign SRAM_OUTPUT_ENABLE       = D_RADDR_VALID;
 
    assign SRAM_WRITE_ADDRESS       = O_WADDR[width-1:p_size];
    assign SRAM_WRITE_DATA          = p_wdata;
    assign SRAM_WRITE_BYTE_ENABLE   = p_be;
    assign SRAM_WRITE_STROBE        = strobe_w;   

    assign D_WDATA = (strobe_w) ? WDATA : D_WDATA;   // latch and hold the WDATA from the master
    assign D_BE    = (strobe_w) ? WSTRB : D_BE;      // latch and hold the WSTRB (byte enables) from the master


    generate 

        if (b_size == p_size) begin

            assign D_RDATA = p_rdata;
            assign p_wdata = D_WDATA;
            assign p_be    = D_BE;

        end 
        if (b_size > p_size) begin

            assign p_wdata = D_WDATA >> 8 * { (O_WADDR[(b_size-1):p_size]), {(p_size) {1'b0}}};
            assign p_pe    = D_BE    >>     { (O_WADDR[(b_size-1):p_size]), {(p_size) {1'b0}}};

            for (n=0; n<`lanes; n=n+1) begin
                assign D_RDATA[`p_bits*(n+1)-1:`p_bits*n] = (read_partial_data_valid && 
                    (O_RADDR[b_size-1:p_size] == n)) ?  p_rdata : D_RDATA[`p_bits*(n+1)-1:`p_bits*n];
            end

        end
        if (b_size < p_size) begin
           always @(posedge ACLK) $display("this combination yet not implemented");
        end

    endgenerate

    // assign correct lane of WDATA to p_wdata

    axi_data_latch #(masters, id_bits) read_data_response (
         .CLK          (ACLK),
         .RESETN       (ARESETn),
         .MASTER       (D_MASTER),
         .ID           (D_ID),
         .DATA         (D_RDATA),
         .LAST         (D_LAST),

         .VALID        (READ_DATA_VALID),
         .READY        (READ_DATA_READY),

         .O_MASTER     (RMASTER),
         .O_ID         (RID),
         .O_DATA       (RDATA),
         .O_RESP       (rresp_tmp),
         .O_LAST       (RLAST),

         .O_VALID      (RVALID),
         .O_READY      (RREADY)
    );

    generate 
        if (b_size == p_size) begin
            assign READ_DATA_VALID = read_partial_data_valid;
        end else begin  
            assign READ_DATA_VALID = read_partial_data_valid && (D_LAST || (sent_byte_count == bus_width));
        end
    endgenerate

    always @(posedge ACLK) begin
        if (!ARESETn) begin
            read_partial_data_valid <= 0;
            sent_byte_count <= 0;
        end else begin
            if (D_RADDR_VALID && D_RADDR_READY) begin
                read_partial_data_valid <= 1;
            end 
            if (read_partial_data_valid && READ_DATA_READY) begin
                read_partial_data_valid <= 0;
                sent_byte_count <= (D_LAST) ? 8'h00 : sent_byte_count + 1;
            end
        end
    end

endmodule

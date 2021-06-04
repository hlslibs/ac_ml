
//dual port RAM with active low read and write chip selects, active low write enable and separate read/write clocks
module DPRAM (Q, RCLK, WCLK, RCSN, WCSN, WEN, D, RA, WA);
   parameter words = 'd16;
   parameter width = 'd16;
   parameter addr_width = 4;
        
   input [width-1:0] D;//Input write data
   input [addr_width-1:0] RA;//Read address
   input [addr_width-1:0] WA;//Write address

   output                 reg [width-1:0] Q;//Read output data
   input                  RCLK;
   input                  WCLK;
   input                  RCSN;//Active low read chip select
   input                  WCSN;//Active low write chip select
   input                  WEN;//Active low write enable
   
   // synopsys translate_off
   
   reg [width-1:0]        mem [words-1:0];
   

      always @(posedge RCLK)
        begin
           if ( RCSN == 0 & (WEN == 0 & WCSN == 0 & WA != RA | WEN == 1 | WCSN == 1 ))
             Q <= mem[RA];
           else if ( RCSN == 0 & WEN == 0 & WCSN == 0 & WA == RA )
             Q <= {(width){1'bX}};             
        end
 
      always @(posedge WCLK)
        begin
           if (WEN == 0 & WCSN == 0)
             mem[WA] <= D;            
        end
 
   // synopsys translate_on
   
endmodule
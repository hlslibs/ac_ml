This example adds AXI bus read/write masters to allow the ML accelerator to talk to an AXI4 based platform.
It has an AXI4 slave interface for programming the various configuration parameters

Files:
  conv2d_combined.h - implements convolution, relu, bias, and max pooling
  bus_interface.h - AXI4 read/write masterss and slave interface
  accerator.h - connects the conv2d accerator with the AXI4 bus interfaces
  testbench.cpp - runs the ML convultion and compares against a reference
  axi4_segment.h - AXI4 segmenter to handle 4k boundarys and bursts > 256
  DPRAM.h - SystemC memory model
  my_axi_struct - AXI4 config struct
  types.h - type defs
  user_trans.h - user defined data types
  ram.h systemC memory simulation model
  Makefile - to compile and execute the design

To compile and execute the design do: "make all"

Use: "make help" for a list of options

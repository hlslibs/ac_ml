This example implements an ML convolution in SystemC using MathcLib.  It is written algorithmically so the performce is poor due to redundant memory reads for the weights and feature map data

Files:
  conv2d.h - implements just convolution
  testbench.cpp - runs the conv2d alogrithm along with calls to relu, bias, and max pooling and compares against a reference
  Makefile - to compile and execute the design

To compile and execute the design do: "make all"

Use: "make help" for a list of options


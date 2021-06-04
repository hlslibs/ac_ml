This example combines the ReLU, bias, and max pooling with conv2d to eliminate the competition between the accelrator and CPU for system memory

Files:
  conv2d_combined.h - implements combined convolution, relu, bias, and max pooling
  testbench.cpp - runs the conv2d and compares against a reference
  Makefile - to compile and execute the design

To compile and execute the design do: "make all"

Use: "make help" for a list of options

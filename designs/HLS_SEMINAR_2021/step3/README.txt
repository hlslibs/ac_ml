This example implements an ML convolution in SystemC using MathcLib.
It is written to add a sliding-window memory architecture to reduce redundant memory reads and to achieve higher performance.  The testbench has been modified to show how the accelerator interacts with the CPU when the system meory is shared.  It demonstrates how the accelerator must wait while the CPU is performing ReLU, bias, and max pooling.

Files:
  conv2d.h - implements just convolution
  testbench.cpp - runs the conv2d sliding window architecture along with calls to relu, bias, and max pooling and compares againsta reference
  Makefile - to compile and execute the design

To compile and execute the design do: "make all"

Use: "make help" for a list of options

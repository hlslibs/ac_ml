echo $env(SW_MEM_FILES)
mem load -infile $env(SW_MEM_FILES)/sw/bootrom.mem -format hex /testbench/top/riscv/riscv_complex/bootrom/bootrom_russk
mem load -filldata 0 /testbench/top/riscv/code_mem/mem
mem load -infile $env(SW_MEM_FILES)/sw/hw_inference.mem -format hex /testbench/top/riscv/code_mem/mem
run -all

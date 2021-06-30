/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 1.2                                                 *
 *                                                                        *
 *  Release Date    : Wed Jun 30 11:14:16 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.2.0                                               *
 *                                                                        *
 *  Copyright , Mentor Graphics Corporation,                     *
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
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
using namespace std;

#include "maxPoolAlg.h"
#include "leakyReLuAlg.h"
#include "biasAlg.h"
#include "conv2d.h"
#include "conv2d1x1.h"
#include "types.h"
#include <mc_scverify.h>

class testbench: public sc_module
{
public:
  sc_clock clk;
  SC_SIG(bool, rstn);

  // Connections and sync channels
  Connections::Combinational<ac_int<ac::log2_ceil<MEM_SIZE>::val,false>> CCS_INIT_S1(mem_in_addr);
  Connections::Combinational<BURST_TYPE>                                 CCS_INIT_S1(mem_in_burst);
  Connections::Combinational<DTYPE>                                      CCS_INIT_S1(mem_in_data);
  Connections::Combinational<ac_int<ac::log2_ceil<MEM_SIZE>::val,false>> CCS_INIT_S1(mem_out_addr);
  Connections::Combinational<BURST_TYPE>                                 CCS_INIT_S1(mem_out_burst);
  Connections::Combinational<DTYPE>                                      CCS_INIT_S1(mem_out_data);
  Connections::SyncChannel                                               CCS_INIT_S1(start);
  Connections::SyncChannel                                               CCS_INIT_S1(done);

  // CSR inputs to the DUT
  sc_signal<IN_FMAP_TYPE>          CCS_INIT_S1(num_in_fmaps);
  sc_signal<OUT_FMAP_TYPE>         CCS_INIT_S1(num_out_fmaps);
  sc_signal<HEIGHT_TYPE>           CCS_INIT_S1(height);
  sc_signal<WIDTH_TYPE>            CCS_INIT_S1(width);
  sc_signal<OFFSET_TYPE>           CCS_INIT_S1(read_offset);
  sc_signal<OFFSET_TYPE>           CCS_INIT_S1(write_offset);
  sc_signal<OFFSET_TYPE>           CCS_INIT_S1(weight_offset);

  // System memory
  DTYPE *mem ;

  // DUT instance
  CCS_DESIGN(conv2d) CCS_INIT_S1(conv2d3x3);

  SC_HAS_PROCESS(testbench);
  testbench(const sc_module_name &name, const std::string &fileio_dir) :
    clk("clk", 2, SC_NS, 1,0,SC_NS,true),
    d_fileio_dir(fileio_dir) {
    sc_object_tracer<sc_clock> trace_clk(clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);

    SC_THREAD(reset);
    sensitive << clk.posedge_event();

    SC_THREAD(memory);
    sensitive << clk.posedge_event();

    conv2d3x3.clk(clk);
    conv2d3x3.rstn(rstn);
    conv2d3x3.mem_in_addr(mem_in_addr);
    conv2d3x3.mem_in_burst(mem_in_burst);
    conv2d3x3.mem_in_data(mem_in_data);
    conv2d3x3.mem_out_addr(mem_out_addr);
    conv2d3x3.mem_out_burst(mem_out_burst);
    conv2d3x3.mem_out_data(mem_out_data);
    conv2d3x3.start(start);
    conv2d3x3.done(done);
    conv2d3x3.num_in_fmaps(num_in_fmaps);
    conv2d3x3.num_out_fmaps(num_out_fmaps);
    conv2d3x3.height(height);
    conv2d3x3.width(width);
    conv2d3x3.read_offset(read_offset);
    conv2d3x3.write_offset(write_offset);
    conv2d3x3.weight_offset(weight_offset);

    mem = new DTYPE[MEM_SIZE]; // Storage for activations and weights
  }

  // Memory thread to talk to DUT
  void memory() {
    mem_in_addr.ResetRead();
    mem_in_burst.ResetRead();
    mem_in_data.ResetWrite();
    mem_out_addr.ResetRead();
    mem_out_burst.ResetRead();
    mem_out_data.ResetRead();
    wait();
    while (1) {
      MEM_ADDR_TYPE rd_addr,wr_addr;
      BURST_TYPE rd_burst, wr_burst;
      bool rd = mem_in_addr.PopNB(rd_addr);
      bool rb = mem_in_burst.PopNB(rd_burst); (void)rb; // return value unused
      bool wr = mem_out_addr.PopNB(wr_addr);
      bool wb = mem_out_burst.PopNB(wr_burst); (void)wb; // return value unused
      int rd_offset = 0;
      int wr_offset = 0;
      if (rd) {
        while (rd_burst != 0) {
          mem_in_data.Push(mem[rd_addr+rd_offset]);
          rd_burst--;
          rd_offset++;
        }
      }
      if (wr) {
        while (wr_burst != 0) {
          mem[wr_addr+wr_offset] = mem_out_data.Pop();
          wr_burst--;
          wr_offset++;
        }
      }
      wait();
    }
  }

  // Stimulus and response thread
  void stim() {
    start.reset_sync_out();
    done.reset_sync_in();
    wait();
    conv2d1x1<DTYPE,13,13,OUT_FMAP,125,1> conv2d1x1_inst;
    unsigned long int sec = time(NULL);
    maxPoolAlg<DTYPE,MAX_HEIGHT,MAX_WIDTH,IN_FMAP,MEM_SIZE> max_pool;
    leakyReLuAlg<DTYPE,MAX_HEIGHT,MAX_WIDTH,OUT_FMAP,MEM_SIZE> leaky_relu;
    biasAlg<DTYPE,MAX_HEIGHT,MAX_WIDTH,OUT_FMAP,MEM_SIZE> bias;
    DTYPE bias_mem[BIAS_SIZE];

    // File IO
    string kernel_filename(d_fileio_dir);
    kernel_filename.append("/kernel.txt");
    cout << "Opening kernel file '" << kernel_filename << "'" << endl;
    ifstream kernel_file;
    kernel_file.open(kernel_filename);
    if (!kernel_file.is_open()) {
      SC_REPORT_ERROR(this->name(),"Error opening kernel file");
      return;
    }

    string data_filename(d_fileio_dir);
    data_filename.append("/data.txt");
    cout << "Opening data file '" << data_filename << "'" << endl;
    ifstream data_file;
    data_file.open(data_filename);
    if (!data_file.is_open()) {
      SC_REPORT_ERROR(this->name(),"Error opening data file");
      return;
    }

    string output_filename(d_fileio_dir);
    output_filename.append("/output.txt");
    cout << "Opening data file '" << output_filename << "'" << endl;
    ifstream output_file;
    output_file.open(output_filename);
    if (!output_file.is_open()) {
      SC_REPORT_ERROR(this->name(),"Error opening output file");
      return;
    }

    string bias_filename(d_fileio_dir);
    bias_filename.append("/bias.txt");
    cout << "Opening bias file '" << bias_filename << "'" << endl;
    ifstream bias_file;
    bias_file.open(bias_filename);
    if (!bias_file.is_open()) {
      SC_REPORT_ERROR(this->name(),"Error opening bias file");
      return;
    }

    // Load the biases
    float bias_val;
    for (int i=0; i<BIAS_SIZE; i++) {
      bias_file >> bias_val;
      bias_mem[i] = DTYPE(bias_val); // Biases are zero for this C++ example
    }

    float tmp;
    int offset = WEIGHT_OFFSET;
    int addr = 0;

    // Load the weights
    printf("mem size = %d \n",MEM_SIZE);
    for (int i=0; i<MEM_SIZE; i++) { mem[i] = 0; }

    cout << "Reading kernel..." << endl;
    while (!kernel_file.eof()) {
      kernel_file >> tmp;
      mem[offset + addr] = SAT_TYPE(tmp);
      addr++;
    }
    printf("addr = %d \n",offset + addr);
    addr = 0;
    // Load the activations
    cout << "Reading data..." << endl;
    while (!data_file.eof()) {
      data_file >> tmp;
      mem[addr] = SAT_TYPE(tmp);
      addr++;
    }

    int in_fmaps[9] =  {3,16,32,64,128,256,512,1024,1024};
    int out_fmaps[9] = {16,32,64,128,256,512,1024,1024,125};
    int height_width[9] = {416, 208, 104, 52, 26, 13, 13, 13, 13};

    OUT_FMAP_TYPE output_feature_maps;
    IN_FMAP_TYPE input_feature_maps;
    OFFSET_TYPE roffset,woffset,wt_offset;
    int bias_offset=0;
    HEIGHT_TYPE fmap_height;
    WIDTH_TYPE fmap_width;
    int stride = 2;

    wt_offset = WEIGHT_OFFSET;
    weight_offset.write(wt_offset);
    roffset = 0;
    read_offset.write(roffset);
    woffset = MEM_OFFSET;
    write_offset.write(woffset);

    wait();
    cout << "Starting testbench..." << endl;
    #ifdef ONLY_LAYER_8
    for (int layer=7 ; layer<8; layer++)
    #else
    for (int layer=0 ; layer<9; layer++)
    #endif
    {
      printf("Layer = %d \n",layer+1);
      fmap_height = fmap_width = height_width[layer]; // current fmap input size

      // Get CSR layer data
      input_feature_maps = in_fmaps[layer];
      output_feature_maps = out_fmaps[layer];
      num_in_fmaps.write(input_feature_maps);
      num_out_fmaps.write(output_feature_maps);
      height.write(fmap_height);
      width.write(fmap_width);

      cout << "fmap shape = " << fmap_height.to_int() << " x " << fmap_width.to_int() << endl;
      cout << "infmaps = " << (int)input_feature_maps << endl;
      cout << "outfmaps = " << (int)output_feature_maps << endl;
      cout << "read offset = " << (int)read_offset.read() << endl;
      cout << "write offset = " << (int)write_offset.read() << endl;
      cout << "weight offset = " << (int)weight_offset.read() << endl;
      cout << "bias offset = " << (int)bias_offset << endl;

      if (layer < 8) {
        start.sync_out();
        done.sync_in();
        bias.run(mem,bias_mem,mem,output_feature_maps,fmap_height,fmap_width,woffset,woffset,bias_offset); // Add bias in place
        bias_offset += out_fmaps[layer];
        wt_offset += input_feature_maps*output_feature_maps*KSIZESQ; // move to next set of weights
        weight_offset.write(wt_offset);
        leaky_relu.run(mem,mem,output_feature_maps,fmap_height,fmap_width,woffset,woffset); // WReLu in place
        if (layer < 6) { // Max pool on first 6 layers
          if (layer == 5) { stride = 1; }
          max_pool.run(mem,mem,output_feature_maps,fmap_height,fmap_width,woffset,roffset,stride); // Write back to orignal fmap location
        } else { // Otherwise swap buffers
          roffset = (layer&1) ? 0 : MEM_OFFSET;
          woffset = (layer&1) ? MEM_OFFSET: 0;
          read_offset.write(roffset);
          write_offset.write(woffset);
        }
      } else { // Just 1x1 on final layer
        printf("HWeights = %f\n", (mem[woffset]).to_double());
        conv2d1x1_inst.run(mem,mem,input_feature_maps,output_feature_maps,fmap_height,fmap_width,roffset,woffset,wt_offset);
        bias.run(mem,bias_mem,mem,output_feature_maps,fmap_height,fmap_width,woffset,woffset,bias_offset); // Add bias in place
      }

    }
    cout << "Finished" << endl;
    int mem_offset = MEM_OFFSET;
    int err = 0;

    // Check against reference
    for (int i=0; i<13*13*125; i++) {
      output_file >> tmp;
      if (SAT_TYPE(tmp) != mem[mem_offset+i]) {
        printf("%f  %f \n", SAT_TYPE(tmp).to_double(), mem[mem_offset+i].to_double());
        err++;
      }
    }
    if (err !=0) {
      cout << "Errors = " << err << endl;
    } else {
      cout << "Passed " << endl;
    }
    sec = time(NULL) - sec;
    cout << "Finished in "<< sec << "seconds" << endl;
    wait();
    sc_stop();
  }

  void reset() {
    rstn.write(0);
    wait(5);
    std::ostringstream msg;
    msg << "De-asserting reset signal '" << rstn.name() << "' @ " << sc_time_stamp();
    SC_REPORT_INFO(this->name(),msg.str().c_str());
    rstn.write(1);
    wait();
  }

private: // data
  std::string d_fileio_dir;
};

int sc_main(int argc, char *argv[])
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file_io_dir>" << std::endl;
    std::cerr << "where:  <file_io_dir> - path to directory containing weights/biases files" << std::endl;
    return -1;
  }
  std::string fileio_dir = argv[1];

  testbench top("top", fileio_dir);
  #ifndef CONNECTIONS_FAST_SIM
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");
  trace_hierarchy(&top, trace_file_ptr);
  channel_logs logs;
  logs.enable("my_log");
  logs.log_hierarchy(top);
  #endif
  sc_start();
  int errcnt = sc_report_handler::get_count(SC_ERROR);
  if (errcnt > 0) {
    std::cout << "Simulation FAILED";
  } else {
    std::cout << "Simulation PASSED" <<endl << endl;
  }
  return errcnt;
}


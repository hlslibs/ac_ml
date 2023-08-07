/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.8                                                 *
 *                                                                        *
 *  Release Date    : Sun Jul 16 19:01:51 PDT 2023                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.8.0                                               *
 *                                                                        *
 *  Copyright 2021 Siemens                                                *
 *                                                                        *
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
#include <ac_sysc_macros.h>
#include <ac_sysc_trace.h>
#include "accelerator.h"
#include "ram.h"
#include "my_axi_struct.h"
#include <mc_scverify.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <sys/time.h>
using namespace std;

class Top : public sc_module, public local_axi
{
public:
  ram             CCS_INIT_S1(ram1);//System memory model

  CCS_DESIGN(accelerator) CCS_INIT_S1(accelerator_inst);

  sc_clock clk;
  SC_SIG(bool, rstn);

  local_axi4_lite::write::template chan<> CCS_INIT_S1(dma_w_slave);   // AXI4 slave channel
  w_chan<>                                CCS_INIT_S1(dma_w_master);  // AXI4 write master channel
  r_chan<>                                CCS_INIT_S1(dma_r_master);  // AXI4 read master channel

  local_axi4_lite_segment::template w_master<>    CCS_INIT_S1(tb_w_master);

  Connections::SyncChannel         CCS_INIT_S1(start);
  Connections::SyncChannel         CCS_INIT_S1(done);

  std::string d_fileio_dir;
  SC_HAS_PROCESS(Top);
  Top(const sc_module_name &name, const std::string &fileio_dir)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),d_fileio_dir(fileio_dir) {
    Connections::set_sim_clk(&clk);

    tb_w_master(dma_w_slave);

    ram1.clk(clk);
    ram1.rstn(rstn);
    ram1.r_slave0(dma_r_master);
    ram1.w_slave0(dma_w_master);

    accelerator_inst.clk(clk);
    accelerator_inst.rstn(rstn);
    accelerator_inst.r_master0(dma_r_master);
    accelerator_inst.w_master0(dma_w_master);
    accelerator_inst.w_slave0(dma_w_slave);
    accelerator_inst.start(start);
    accelerator_inst.done(done);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rstn, false);

    /*
        SC_THREAD(done_mon);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rstn, false);
    */
    sc_object_tracer<sc_clock> trace_clk(clk);
  }

  sc_time start_time, end_time;
  int beats = 0x40;
  int source_addr = 0x1000;
  int target_addr = 0x4000;

  void stim() {
    CCS_LOG("Stimulus started");
    tb_w_master.reset();
    start.reset_sync_out();
    done.reset_sync_in();

    wait();

// File IO
    string kernel_filename(d_fileio_dir);
    kernel_filename.append("/kernel.txt");
    cout << "Opening kernel file '" << kernel_filename << "'" << endl;
    ifstream kernel_file;
    kernel_file.open(kernel_filename);
    if (!kernel_file.is_open()) {
      SC_REPORT_ERROR(this->name(), "Error opening kernel file\n");
      return;
    }

    string data_filename(d_fileio_dir);
    data_filename.append("/data.txt");
    cout << "Opening data file '" << data_filename << "'" << endl;
    ifstream data_file;
    data_file.open(data_filename);
    if (!data_file.is_open()) {
      SC_REPORT_ERROR(this->name(), "Error opening data file\n");
      return;
    }

    string output_filename(d_fileio_dir);
    output_filename.append("/output.txt");
    cout << "Opening data file '" << output_filename << "'" << endl;
    ifstream output_file;
    output_file.open(output_filename);
    if (!output_file.is_open()) {
      SC_REPORT_ERROR(this->name(), "Error opening output file\n");
      return;
    }

    string bias_filename(d_fileio_dir);
    bias_filename.append("/bias.txt");
    cout << "Opening bias file '" << bias_filename << "'" << endl;
    ifstream bias_file;
    bias_file.open(bias_filename);
    if (!bias_file.is_open()) {
      SC_REPORT_ERROR(this->name(), "Error opening bias file\n");
      return;
    }

    unsigned long int sec = time(NULL);
    float tmp;
    int offset = WEIGHT_OFFSET;
    int addr = 0;

    // Load the weights
    cout << "mem size = " << MEM_SIZE << endl;
    //for (int i=0; i<MEM_SIZE; i++) { mem[i] = 0; }

    cout << "Reading kernel..." << endl;
    while (!kernel_file.eof()) {
      kernel_file >> tmp;

      ram1.array[offset + addr] = SAT_TYPE(tmp).slc<16>(0).to_int();
      addr++;
    }
    cout << "addr = " << offset+addr << endl;
    addr = 0;

    // Load the activations
    cout << "Reading data..." << endl;
    while (!data_file.eof()) {
      data_file >> tmp;
      ram1.array[addr] = SAT_TYPE(tmp).slc<16>(0).to_int();
      addr++;
    }
    addr = BIAS_OFFSET;
    float bias_val;
    cout << "Reading bias..." << endl;
    for (int i=0; i<BIAS_SIZE; i++) {
      bias_file >> bias_val;
      ram1.array[addr+i] = SAT_TYPE(bias_val).slc<16>(0).to_int(); // Biases are zero for this C++ example
    }

    //Accelerator config parameters
    int in_fmaps[9] =  {3-1,16-1,32-1,64-1,128-1,256-1,512-1,1024-1,1024-1};
    int out_fmaps[9] = {16-1,32-1,64-1,128-1,256-1,512-1,1024-1,1024-1,125-1};
    int height_width[9] = {416-1, 208-1, 104-1, 52-1, 26-1, 13-1, 13-1, 13-1, 13-1};

    OUT_FMAP_TYPE output_feature_maps;
    IN_FMAP_TYPE input_feature_maps;
    OFFSET_TYPE roffset,woffset,wt_offset;
    BIAS_OFFSET_TYPE boffset;
    HEIGHT_TYPE fmap_height;
    WIDTH_TYPE fmap_width;

    wt_offset = WEIGHT_OFFSET;
    tb_w_master.single_write(offsetof(address_map, weight_offset), wt_offset);
    roffset = 0;
    tb_w_master.single_write(offsetof(address_map, read_offset), roffset);
    woffset = MEM_OFFSET;
    tb_w_master.single_write(offsetof(address_map, write_offset), woffset);

    boffset = BIAS_OFFSET;
    tb_w_master.single_write(offsetof(address_map, bias_offset), boffset);
    wait();
    cout << "Starting testbench..." << endl;
    #ifdef ONLY_LAYER_8
    for (int layer=2 ; layer<3; layer++)
    #else
    for (int layer=0 ; layer<9; layer++)
    #endif
    {
      cout << "Layer = " << layer+1 << endl;
      fmap_height = fmap_width = height_width[layer]; // current fmap input size

      input_feature_maps = in_fmaps[layer];
      output_feature_maps = out_fmaps[layer];

      tb_w_master.single_write(offsetof(address_map, num_in_fmaps), input_feature_maps);
      tb_w_master.single_write(offsetof(address_map, num_out_fmaps), output_feature_maps);
      tb_w_master.single_write(offsetof(address_map, height), fmap_height);
      tb_w_master.single_write(offsetof(address_map, width), fmap_width);


      cout << "fmap shape = " << fmap_height.to_int() << " x " << fmap_width.to_int() << endl;
      cout << "infmaps = " << (int)input_feature_maps << endl;
      cout << "outfmaps = " << (int)output_feature_maps << endl;
      cout << "read offset = " << (int)roffset << endl;
      cout << "write offset = " << (int)woffset << endl;
      cout << "weight offset = " << (int)wt_offset << endl;
      cout << "bias offset = " << (int)boffset << endl;

      tb_w_master.single_write(offsetof(address_map, pointwise), (layer==8) ? true:false);
      tb_w_master.single_write(offsetof(address_map, relu), (layer==8) ? false:true);
      if (layer < 6) {
        if (layer == 5) {
          tb_w_master.single_write(offsetof(address_map, pool), 1);
        } else {
          tb_w_master.single_write(offsetof(address_map, pool), 2);
        }
      } else {
        tb_w_master.single_write(offsetof(address_map, pool), 0);
      }
      INSERT_RAND_STALL;
      start.sync_out();

      //Wait for done to force stall
      while (!done.vld) {
        wait();
      }
      INSERT_RAND_STALL;
      done.sync_in();
      wait(100);

      boffset += out_fmaps[layer]+1;
      tb_w_master.single_write(offsetof(address_map, bias_offset), boffset);
      wt_offset += (input_feature_maps+1)*(output_feature_maps+1)*KSIZESQ; // move to next set of weights
      tb_w_master.single_write(offsetof(address_map, weight_offset), wt_offset);
      roffset = (layer&1) ? 0 : MEM_OFFSET;
      woffset = (layer&1) ? MEM_OFFSET: 0;
      tb_w_master.single_write(offsetof(address_map, read_offset), roffset);
      tb_w_master.single_write(offsetof(address_map, write_offset), woffset);
      wait(100);
    }
    sec = time(NULL) - sec;

    int mem_offset = MEM_OFFSET;
    int err = 0;
    float perr=0;
    for (int i=0; i<13*13*125; i++) {
      output_file >> tmp;
      SAT_TYPE from_mem;
      from_mem.set_slc(0,ram1.array[mem_offset+i].slc<DTYPE::width>(0));
      if (SAT_TYPE(tmp) != from_mem) {
        //printf("%f  %f \n", SAT_TYPE(tmp).to_double(), mem[mem_offset+i].to_double());
        perr += abs((SAT_TYPE(tmp) - from_mem).to_double());
        err++;
      }
    }
    if (err != 0) {
      std::ostringstream msg;
      msg << "% Error = " << perr/err;
      //SC_REPORT_ERROR(this->name(), msg.str().c_str());
      CCS_LOG("% Error = " << perr/err << endl);
    } else {
      CCS_LOG("Passed " << endl);
    }

    cout << "Finished in "<< sec << " seconds" <<  endl;

    start_time = sc_time_stamp();

    wait(2000, SC_NS);
    CCS_LOG("stopping sim due to testbench timeout");
    sc_stop();
    wait();
  }

  void reset() {
    rstn.write(0);
    wait(5);
    rstn.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file_io_dir>" << std::endl;
    std::cerr << "where:  <file_io_dir> - path to directory containing weights/biases files" << std::endl;
    return -1;
  }
  std::string fileio_dir = argv[1];
  Top top("top",fileio_dir);
  trace_hierarchy(&top, trace_file_ptr);

  channel_logs logs;
  logs.enable("chan_log");
  logs.log_hierarchy(top);

  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}


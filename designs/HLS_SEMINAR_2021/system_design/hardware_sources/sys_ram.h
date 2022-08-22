/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
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
#pragma once
#include "types.h"
#include "defines.h"
#include "axi4_segment.h"
#include "sysbus_axi_struct.h"

#ifdef INITIALIZE_RAM

#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
using namespace std;

#endif

class sys_ram : public sc_module, public sysbus_axi {
public:
  sc_in<bool>            CCS_INIT_S1(clk);
  sc_in<bool>            CCS_INIT_S1(rst_bar);
  r_slave<AUTO_PORT>     CCS_INIT_S1(r_slave0);
  w_slave<AUTO_PORT>     CCS_INIT_S1(w_slave0);

  static const int sz = MEM_SIZE; // size in axi_cfg::dataWidth words

  typedef NVUINTW(axi_cfg::dataWidth) arr_t;
  arr_t* array {0};

  bool chatty = false;

  SC_CTOR(sys_ram)
  {
    array = new arr_t[sz];

    SC_THREAD(slave_r_process);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(slave_w_process);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    for (int i=0; i < sz; i++)
      array[i] = i * bytesPerBeat;

#ifdef INITIALIZE_RAM
    load_weights_and_biases();
#endif
  }

  NVUINTW(axi_cfg::dataWidth) debug_read_addr(uint32_t addr)
  {
    std::string address_error = "invalid addr, attempted access at " + std::to_string(addr);

    if (addr >= (sz * bytesPerBeat)) {
      SC_REPORT_ERROR("ram", address_error.c_str());
      return 0;
    }

    return(array[addr / bytesPerBeat]);
  }

  void slave_r_process() {

    r_slave0.reset();

    wait();

    while(1) {
      ar_payload ar;
      r_slave0.start_multi_read(ar);

      if (chatty) CCS_LOG("ram read  addr: " << std::hex << ar.addr.to_int() << " len: " << ar.len.to_int());
      if (chatty) printf("RAM read: Address: %08x \n", ar.addr.to_int());

      while (1) {
        r_payload r;

        if (ar.addr >= (sz * bytesPerBeat)) {
          std::string address_error = "invalid addr, attempted access at " + std::to_string(ar.addr);

          SC_REPORT_ERROR("ram", address_error.c_str());
          r.resp = Enc::XRESP::SLVERR;

        } else {
          r.data = array[ar.addr / bytesPerBeat];
          r.id = ar.id;
          r.resp = Enc::XRESP::OKAY;

          if (chatty) printf("Addr: %08x  Data: %08x \n", ar.addr.to_int(), r.data.to_int());
	}

        if (!r_slave0.next_multi_read(ar, r)) break;
      } 
    }
  }

  void slave_w_process() {

    w_slave0.reset();

    wait();

    while(1) {
      aw_payload aw;
      b_payload b;

      w_slave0.start_multi_write(aw, b);

      if (chatty) CCS_LOG("ram write addr: " << std::hex << aw.addr << " len: " << aw.len);

      while (1) {
        w_payload w = w_slave0.w.Pop();

        if (aw.addr >= (sz * bytesPerBeat)) {
          std::string address_error = "invalid addr, attempted access at " + std::to_string(aw.addr);

          SC_REPORT_ERROR("ram", address_error.c_str());
          b.resp = Enc::XRESP::SLVERR;
          b.id = aw.id;

        } else {
          decltype(w.wstrb) all_on{~0};

          if (w.wstrb == all_on) {
            array[aw.addr / bytesPerBeat] = w.data.to_uint64();
          } else {
	    if (chatty) CCS_LOG("write strobe enabled");
            arr_t orig  = array[aw.addr / bytesPerBeat];
	    arr_t wdata = w.data.to_uint64();

	    #pragma unroll
            for (int i=0; i<WSTRB_WIDTH; i++) {
              if (w.wstrb[i]) {
                orig = nvhls::set_slc(orig, nvhls::get_slc<8>(wdata, (i*8)), (i*8));
              }
            }

            array[aw.addr / bytesPerBeat] = orig;
          }
	}

        if (!w_slave0.next_multi_write(aw)) break;
      } 

      w_slave0.b.Push(b);
    }
  }

#ifdef INITIALIZE_RAM

  void load(char* dir, const char* base_filename, int offset)
  {
    ifstream input_file;
    std::string filename(dir);
    int addr;
    bool chatty = false;
    FILE *f;
   
    if (chatty) f = fopen(base_filename, "w");

    filename.append("/");
    filename.append(base_filename);
    filename.append(".txt");

    cout << "Opening file \"" << filename << "\"" << endl;
    input_file.open(filename);

    if (!input_file.is_open()) {
      std::string err_str("Unable to open \"");
      err_str.append(base_filename);
      err_str.append(".txt\" for reading."); 
      SC_REPORT_ERROR(this->name(), "Error opening file for RAM initialization");
      cout << err_str << endl;
      return;
    }

    addr = 0;
    cout << "Reading " << base_filename << "... " << endl;
    while (!input_file.eof()) {
      float datum;

      input_file >> datum;
      array[offset + addr] = SAT_TYPE(datum).slc<16>(0).to_int();
      if (chatty) fprintf(f, "index(d) = %12d index(x) = %08x addr(d) = %12d addr(x) = %08x     value(d): %12d value(x): %08x \n",  
                              addr, addr, offset+addr*4, offset+addr*4, array[offset+addr].to_int(), 0xFFFF & array[offset+addr].to_int());

      addr++;
    }
    cout << addr << " values loaded: " << offset << " to " << offset+addr-1 << endl;

    if (chatty) fclose(f);
    return;
  }

  void load_weights_and_biases()
  {
    char *dir_ptr = getenv("FILE_IO_DIR");

    if (dir_ptr == NULL) {
       cout << "Please set environment variable \"FILE_IO_DIR\" to where the weight and bias files are located" << endl;
       SC_REPORT_ERROR(this->name(), "Unable to load weight and bias data. Set environment variable \"FILE_IO_DIR\".");
       return;
    }

    load(dir_ptr, "kernel", WEIGHT_OFFSET);
    load(dir_ptr, "bias", BIAS_OFFSET);
    load(dir_ptr, "data", DATA_OFFSET); 
    load(dir_ptr, "output", EXPECTED_RESULTS_OFFSET);

    cout << "Memory initialization complete. " << endl;
  }
#endif
};

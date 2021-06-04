/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.1                                                 *
 *                                                                        *
 *  Release Date    : Fri Jun  4 11:46:59 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.1.0                                               *
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
#pragma once

#include <ac_sysc_macros.h>
#include <ac_sysc_trace.h>
#include <mc_connections.h>

// Prevent redefine warnings from NVHLS
#undef CONNECTIONS_ASSERT_MSG
#undef CONNECTIONS_SIM_ONLY_ASSERT_MSG

#include "axi/axi4.h"
#include "stall.h"

namespace axi
{
  /**
   *  * \brief axi4_segment layers on the NVidia Matchlib axi4 class and adds automatic burst segmentation funtionality and other convenience functions.
  */

  template <typename Cfg>
  class axi4_segment : public axi::axi4<Cfg>
  {
  public:

    typedef Cfg axi_cfg;
    typedef AXI4_Encoding Enc;

    typedef typename axi::axi4<Cfg>::AddrPayload ar_payload;
    typedef typename axi::axi4<Cfg>::AddrPayload aw_payload;
    typedef typename axi::axi4<Cfg>::ReadPayload r_payload;
    typedef typename axi::axi4<Cfg>::WritePayload w_payload;
    typedef typename axi::axi4<Cfg>::WRespPayload b_payload;

    template <Connections::connections_port_t PortType = AUTO_PORT>
    using r_chan = typename axi::axi4<Cfg>::read::template chan<PortType>;
    template <Connections::connections_port_t PortType = AUTO_PORT>
    using w_chan = typename axi::axi4<Cfg>::write::template chan<PortType>;

// override for segmentBurstSize makes it easier to see segment functionality in waveforms for testing
    #ifdef SEGMENT_BURST_SIZE
    static const int segmentBurstSize = SEGMENT_BURST_SIZE;
    #else
    static const int segmentBurstSize = Cfg::maxBurstSize;
    #endif

    static const int page_size = 4096;  // axi4 requires segmentation at 4k boundaries
    static const int page_adr_bits = 12; // 2^12 = 4096 ; number of bits in a page address
    static const int bytesPerBeat = axi::axi4<Cfg>::DATA_WIDTH >> 3;

    struct ex_ar_payload : public ar_payload {
      // this payload enables burst reads of unlimited length - it extends from ar_payload
      NVUINTW(32) ex_len{0};

      ex_ar_payload() {
        ar_payload::burst = Enc::AXBURST::INCR;
      }

      static const unsigned int width = 32 + ar_payload::width;
      template <unsigned int Size> void Marshall(Marshaller<Size> &m) {
        m & ((ar_payload &)*this);
        m &ex_len;
      }

      inline friend void sc_trace(sc_trace_file *tf, const ex_ar_payload &v, const std::string &NAME ) {
        sc_trace(tf,(const ar_payload &)v,    NAME + ".ar_payload");
        sc_trace(tf,v.ex_len,   NAME + ".ex_len");
      }
    };

    struct ex_aw_payload : public aw_payload {
      // this payload enables burst writes of unlimited length - it extends from aw_payload
      NVUINTW(32) ex_len{0};

      ex_aw_payload() {
        aw_payload::burst = Enc::AXBURST::INCR;
      }

      static const unsigned int width = 32 + aw_payload::width;
      template <unsigned int Size> void Marshall(Marshaller<Size> &m) {
        m & ((aw_payload &)*this);
        m &ex_len;
      }

      inline friend void sc_trace(sc_trace_file *tf, const ex_aw_payload &v, const std::string &NAME ) {
        sc_trace(tf,(const aw_payload &)v,    NAME + ".aw_payload");
        sc_trace(tf,v.ex_len,   NAME + ".ex_len");
      }
    };

    template <Connections::connections_port_t PortType = AUTO_PORT>
    struct w_master: public axi::axi4<Cfg>::write::template master<PortType> {
      typedef typename axi::axi4<Cfg>::write::template master<PortType> base;
      w_master(sc_module_name nm) : base(nm) {}

      b_payload single_write(uint32 addr, uint32 data) {
        aw_payload aw_item;
        aw_item.addr = addr;
        aw_item.len = 0;
        base::aw.Push(aw_item);
        rand_stall(7,7);

        w_payload w_item;
        w_item.data = data;
        w_item.last = true;
        base::w.Push(w_item);
        rand_stall(7,7);

        return base::b.Pop();
      }

      b_payload small_write(uint32 addr, uint64 data, uint32 bytes) {
        uint32 byte_field = bytesPerBeat - 1;
        aw_payload aw_item;
        aw_item.addr = addr & (~byte_field);
        aw_item.len = 0;
        base::aw.Push(aw_item);
        rand_stall(7,7);

        uint32 mask = (1 << bytes) - 1;
        w_payload w_item;
        w_item.data = data << (8 * (addr & byte_field));
        w_item.last = true;
        w_item.wstrb = mask << (addr & byte_field);
        base::w.Push(w_item);
        rand_stall(7,7);

        return base::b.Pop();
      }

      b_payload write_32(uint32 addr, uint64 data) {
        return this->small_write(addr, data, 4);
      }

      b_payload write_16(uint32 addr, uint64 data) {
        return this->small_write(addr, data, 2);
      }

      b_payload write_8(uint32 addr, uint64 data) {
        return this->small_write(addr, data, 1);
      }
    };

    template <Connections::connections_port_t PortType = AUTO_PORT>
    struct r_master: public axi::axi4<Cfg>::read::template master<PortType> {
      typedef typename axi::axi4<Cfg>::read::template master<PortType> base;
      r_master(sc_module_name nm) : base(nm) {}

      r_payload single_read(uint32 addr) {
        ar_payload ar_item;
        ar_item.addr = addr;
        ar_item.len = 0;
        base::ar.Push(ar_item);
        rand_stall(7,7);

        return base::r.Pop();
      }

      r_payload small_read(uint32 addr, uint32 size) {
        uint32 byte_field = bytesPerBeat -1;
        ar_payload ar_item;
        r_payload r;
        ar_item.addr = addr & (~byte_field);
        ar_item.len = 0;
        base::ar.Push(ar_item);
        rand_stall(7,7);
        r = base::r.Pop();
        rand_stall(7,7);

        uint64 data_mask = (1 << size) -1;
        r.data = (r.data >> (8 * (addr & byte_field))) & data_mask;
        return r;
      }

      r_payload read_32(uint32 addr) {
        return this->small_read(addr, 32);
      }

      r_payload read_16(uint32 addr) {
        return this->small_read(addr, 16);
      }

      r_payload read_8(uint32 addr) {
        return this->small_read(addr, 8);
      }
    };

    template <Connections::connections_port_t PortType = AUTO_PORT>
    struct r_slave : public axi::axi4<Cfg>::read::template slave<PortType> {
      typedef typename axi::axi4<Cfg>::read::template slave<PortType> base;

      r_slave(sc_module_name nm) : base(nm) {}

      bool single_read(ar_payload &ret_ar, r_payload &ret_r) {
        ret_ar = base::ar.Pop();
        rand_stall(7,7);

        ret_r.id = ret_ar.id;
        ret_r.last = true;

        if (ret_ar.len == 0) { return true; }

        ret_r.resp = Enc::XRESP::SLVERR;
        ret_r.last = false;
        base::r.Push(ret_r);
        rand_stall(7,7);

        while (ret_ar.len-- > 0) {
          ret_r.last = (ret_ar.len == 1);
          base::r.Push(ret_r);
          rand_stall(7,7);
        }

        return false;
      }

      bool start_multi_read(ar_payload &ret_ar) {
        ret_ar = base::ar.Pop();
        rand_stall(7,7);
        return true;
      }

      bool next_multi_read(ar_payload &ret_ar, r_payload &ret_r) {
        ret_r.id = ret_ar.id;

        if (ret_ar.len == 0) { ret_r.last = true; }

        base::r.Push(ret_r);
        rand_stall(7,7);

        if (ret_r.last == true) { return false; }

        --ret_ar.len;

        if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ret_ar.burst.to_uint64() == Enc::AXBURST::INCR)) {
          ret_ar.addr += bytesPerBeat;
        }
        return true;
      }
    };

    template <Connections::connections_port_t PortType = AUTO_PORT>
    struct w_slave : public axi::axi4<Cfg>::write::template slave<PortType> {
      typedef typename axi::axi4<Cfg>::write::template slave<PortType> base;
      w_slave(sc_module_name nm) : base(nm) {}

      void reset() {
        base::aw.Reset();
        base::w.Reset();
        base::b.Reset();
      }

      bool get_single_write(aw_payload &ret_aw, w_payload &ret_w, b_payload &ret_b) {
        ret_aw = base::aw.Pop();
        rand_stall(7,7);
        ret_w = base::w.Pop();
        rand_stall(7,7);
        ret_b.id = ret_aw.id;

        if (ret_aw.len == 0) { return true; }

        while (ret_aw.len-- > 0) {
          ret_w = base::w.Pop();
          rand_stall(7,7);
        }

        ret_b.resp = Enc::XRESP::SLVERR;
        base::b.Push(ret_b);
        rand_stall(7,7);
        return false;
      }

      bool start_multi_write(aw_payload &ret_aw, b_payload &ret_b) {
        ret_aw = base::aw.Pop();
        rand_stall(7,7);
        ret_b.id = ret_aw.id;

        return true;
      }

      bool next_multi_write(aw_payload &ret_aw) {
        if (ret_aw.len == 0) { return false; }

        --ret_aw.len;

        if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ret_aw.burst.to_uint64() == Enc::AXBURST::INCR)) {
          ret_aw.addr += bytesPerBeat;
        }
        return true;
      }
    };

// These macros are needed until Catapult supports sc_export,
// which will allow pushing the fifos into the segment module
//
#define AXI4_W_SEGMENT(n) \
  w_segment CCS_INIT_S1(n); \
  Connections::Combinational<ex_aw_payload> CCS_INIT_S1(n ## _ex_aw_chan); \
  Connections::Combinational<w_payload>     CCS_INIT_S1(n ## _w_chan); \
  Connections::Combinational<b_payload> CCS_INIT_S1(n ## _b_chan);

#define AXI4_W_SEGMENT_BIND(n, _clk, _rst_bar, _w_master) \
    n .clk(_clk); \
    n .rst_bar(_rst_bar); \
    n .aw_out(_w_master.aw); \
    n .w_out(_w_master.w); \
    n .b_in(_w_master.b); \
    n .ex_aw_chan(n ## _ex_aw_chan ); \
    n .w_chan(n ## _w_chan ); \
    n .b_chan(n ## _b_chan );

#define AXI4_W_SEGMENT_RESET(n, _w_master) \
    n ## _ex_aw_chan.ResetWrite(); \
    n ## _w_chan.ResetWrite(); \
    n ## _b_chan.ResetRead();


    SC_MODULE(w_segment) {
      sc_in<bool> CCS_INIT_S1(clk);
      sc_in<bool> CCS_INIT_S1(rst_bar);
      Connections::Out<aw_payload>   CCS_INIT_S1(aw_out);
      Connections::Out<w_payload>    CCS_INIT_S1(w_out);
      Connections::In<b_payload> CCS_INIT_S1(b_in);

      // queue incoming ex_aw_payload items
      Connections::In<ex_aw_payload> CCS_INIT_S1(ex_aw_chan);

      // queue incoming w_payload items
      Connections::In<w_payload>     CCS_INIT_S1(w_chan);

      // queue outgoing (combined) b items
      Connections::Out<b_payload> CCS_INIT_S1(b_chan);

      // sets last bit for w_payload item
      Connections::Combinational<bool>          CCS_INIT_S1(last_bit_chan);

      // one bit per each aw_payload item, true iff it is last burst in overall segmented burst
      Connections::Combinational<bool>          CCS_INIT_S1(last_burst_chan);

      SC_CTOR(w_segment) {
        SC_THREAD(ex_aw_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(w_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);

        SC_THREAD(b_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
      }

      void ex_aw_process() {
        aw_out.Reset();
        ex_aw_chan.Reset();
        last_burst_chan.ResetWrite();
        last_bit_chan.ResetWrite();
        wait();

        #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
        while (1) {
          ex_aw_payload ex = ex_aw_chan.Pop();
          rand_stall(7,7);
          #ifndef __SYNTHESIS__
          if (ex.addr &  (bytesPerBeat-1)) {
            SC_REPORT_ERROR("ex_aw_process", "unaligned address");
          }
          #endif
          while (1) {
            bool last_burst = false;
            aw_payload aw = ex;
            // segment bursts to max of 256 beats as per axi4 protocol
            if (ex.ex_len > (segmentBurstSize-1)) {
              aw.len = (segmentBurstSize-1);
              ex.ex_len -= segmentBurstSize;
              if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
                ex.addr += bytesPerBeat * segmentBurstSize;
              }
            } else {
              aw.len = ex.ex_len;
              last_burst = true;
              // these 3 lines needed in case we need to segment at 4k boundary
              ex.ex_len = ~0;
              if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
                ex.addr += bytesPerBeat * (aw.len+1);
              }
            }

            // Here we segment at 4k address boundaries as per axi4 protocol
            if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
              NVUINTW(page_adr_bits) page_adr = aw.addr;
              NVUINTW(page_adr_bits + 1) endbits = page_adr + ((aw.len + 1) * bytesPerBeat);
              if (endbits & page_size) {
                NVUINTW(axi::axi4<Cfg>::ALEN_WIDTH) adjust = (endbits & (page_size-1)) / bytesPerBeat;

                if (adjust) {
                  // LOG("endbits: " << std::hex << endbits);
                  // LOG("aw 4k segment. addr: " << std::hex << aw.addr << " len: " << aw.len << " adjust: " << adjust);
                  aw.len -= adjust;
                  ex.ex_len += adjust;
                  ex.addr -= adjust * bytesPerBeat;
                  // LOG("aw 4k segment. new ex.addr: " << std::hex << ex.addr);
                  last_burst = false;
                }
              }
            }

            // LOG("aw_out Push: " << aw.len);
            aw_out.Push(aw);
            rand_stall(7,7);

            #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
            while (1) {
              bool last = (aw.len == 0);
              last_bit_chan.Push(last);
              rand_stall(7,7);
              if (last) { break; }
              --aw.len;
            }

            last_burst_chan.Push(last_burst);
            rand_stall(7,7);

            if (last_burst) { break; }
          }
        }
      }

      void w_process() {
        w_out.Reset();
        w_chan.Reset();
        last_bit_chan.ResetRead();
        wait();

        #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
        while (1) {
          w_payload w = w_chan.Pop();
          rand_stall(7,7);
          w.last = last_bit_chan.Pop();
          rand_stall(7,7);
          // LOG("last bit fifo read: " << w.last);
          w_out.Push(w);
        }
      }

      void b_process() {
        b_in.Reset();
        last_burst_chan.ResetRead();
        b_chan.Reset();
        bool id_valid = false;
        wait();

        // user model using w_segment is not allowed to issue a new aw with a different ID from previous aw, until
        // it has received the combined b for the previous aw. (If user model does, write responses might come
        // back not in order, in which case combined b will indicate SLVERR. But, other than that, everything
        // will still work properly.)

        b_payload comb;

        #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
        while (1) {
          #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
          while (1) {
            b_payload r = b_in.Pop();
            rand_stall(7,7);
            // LOG("b read: ");
            if (id_valid) {
              if (comb.id != r.id) {
                #ifndef __SYNTHESIS__
                SC_REPORT_ERROR("combine_b", " ");
                #endif
                comb.resp = Enc::XRESP::SLVERR;
              }
            } else {
              id_valid = true;
              comb.id = r.id;
            }
            comb.resp = comb.resp | r.resp;

            bool last_burst = last_burst_chan.Pop();
            rand_stall(7,7);

            if (last_burst) {
              // LOG("b_chan_Push: ");
              b_chan.Push(comb);
              rand_stall(7,7);
              id_valid = false;
              comb.resp = Enc::XRESP::OKAY;
              break;
            }
          }
        }
      }
    };

// These macros are needed until Catapult supports sc_export,
// which will allow pushing the fifo into the segment module
//
#define AXI4_R_SEGMENT(n) \
  r_segment CCS_INIT_S1(n); \
  Connections::Combinational<ex_ar_payload> CCS_INIT_S1(n ## _ex_ar_chan);

#define AXI4_R_SEGMENT_BIND(n, _clk, _rst_bar, _r_master) \
    n .clk(_clk); \
    n .rst_bar(_rst_bar); \
    n .ar_out(_r_master .ar); \
    n .ex_ar_chan(n ## _ex_ar_chan);

#define AXI4_R_SEGMENT_RESET(n, _r_master) \
    n ## _ex_ar_chan.ResetWrite(); \
    _r_master . r.Reset();


    SC_MODULE(r_segment) {
      sc_in<bool> CCS_INIT_S1(clk);
      sc_in<bool> CCS_INIT_S1(rst_bar);
      Connections::Out<ar_payload>   CCS_INIT_S1(ar_out);
      Connections::In<ex_ar_payload> CCS_INIT_S1(ex_ar_chan);

      SC_CTOR(r_segment) {
        SC_THREAD(ex_ar_process);
        sensitive << clk.pos();
        async_reset_signal_is(rst_bar, false);
      }

      void ex_ar_process() {
        ar_out.Reset();
        ex_ar_chan.Reset();
        wait();

        #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
        while (1) {
          ex_ar_payload ex = ex_ar_chan.Pop();
          rand_stall(7,7);
          #ifndef __SYNTHESIS__
          if (ex.addr &  (bytesPerBeat-1)) {
            SC_REPORT_ERROR("ex_ar_process", "unaligned address");
          }
          #endif
          #pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
          while (1) {
            bool last_burst = false;
            ar_payload ar = ex;
            // segment bursts to max of 256 beats as per axi4 protocol
            if (ex.ex_len > (segmentBurstSize-1)) {
              ar.len = (segmentBurstSize-1);
              ex.ex_len -= segmentBurstSize;
              if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
                ex.addr += bytesPerBeat * segmentBurstSize;
              }
            } else {
              ar.len = ex.ex_len;
              last_burst = true;
              // these 3 lines needed in case we need to segment at 4k boundary
              ex.ex_len = ~0;
              if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
                ex.addr += bytesPerBeat * (ar.len+1);
              }

            }

            // Here we segment at 4k address boundaries as per axi4 protocol
            if ((axi::axi4<Cfg>::BURST_WIDTH == 0 ) || (ex.burst.to_uint64() == Enc::AXBURST::INCR)) {
              NVUINTW(page_adr_bits) page_adr = ar.addr;
              NVUINTW(page_adr_bits + 1) endbits = page_adr + ((ar.len + 1) * bytesPerBeat);
              if (endbits & page_size) {
                NVUINTW(axi::axi4<Cfg>::ALEN_WIDTH) adjust = (endbits & (page_size-1)) / bytesPerBeat;

                if (adjust) {
                  // LOG("endbits: " << std::hex << endbits);
                  // LOG("ar 4k segment. addr: " << std::hex << ar.addr << " len: " << ar.len << " adjust: " << adjust);
                  ar.len -= adjust;
                  ex.ex_len += adjust;
                  ex.addr -= adjust * bytesPerBeat;
                  // LOG("ar 4k segment. new ex.addr: " << std::hex << ex.addr);
                  last_burst = false;
                }
              }
            }

            ar_out.Push(ar);
            rand_stall(7,7);

            if (last_burst) { break; }
          }
        }
      }
    };

  }; // axi4_segment
}; // axi


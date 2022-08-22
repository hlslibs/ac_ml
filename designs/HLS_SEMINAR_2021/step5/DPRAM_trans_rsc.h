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
#ifndef __INCLUDED_DPRAM_trans_rsc_H__
#define __INCLUDED_DPRAM_trans_rsc_H__
#include <mc_transactors.h>

template <
  int words
  ,int width
  ,int addr_width
  >
class DPRAM_trans_rsc : public mc_wire_trans_rsc_base<width,words>
{
public:
  sc_out< sc_lv<width> >   Q;
  sc_in< bool >   RCLK;
  sc_in< bool >   WCLK;
  sc_in< sc_logic >   RCSN;
  sc_in< sc_logic >   WCSN;
  sc_in< sc_logic >   WEN;
  sc_in< sc_lv<width> >   D;
  sc_in< sc_lv<addr_width> >   RA;
  sc_in< sc_lv<addr_width> >   WA;

  typedef mc_wire_trans_rsc_base<width,words> base;
  MC_EXPOSE_NAMES_OF_BASE(base);

  SC_HAS_PROCESS( DPRAM_trans_rsc );
  DPRAM_trans_rsc(const sc_module_name &name, bool phase, double clk_skew_delay=0.0)
    : base(name, phase, clk_skew_delay)
    ,Q("Q")
    ,RCLK("RCLK")
    ,WCLK("WCLK")
    ,RCSN("RCSN")
    ,WCSN("WCSN")
    ,WEN("WEN")
    ,D("D")
    ,RA("RA")
    ,WA("WA")
    ,_is_connected_wr(true)
    ,_is_connected_wr_messaged(false) {
    SC_METHOD(at_active_clock_edge);
    sensitive << (phase ? RCLK.pos() : RCLK.neg());
    sensitive << (phase ? WCLK.pos() : WCLK.neg());
    this->dont_initialize();

    MC_METHOD(clk_skew_delay);
    this->sensitive << this->_clk_skew_event;
    this->dont_initialize();
  }

  virtual void start_of_simulation() {
    if ((base::_holdtime == 0.0) && this->get_attribute("CLK_SKEW_DELAY")) {
      base::_holdtime = ((sc_attribute<double> *)(this->get_attribute("CLK_SKEW_DELAY")))->value;
    }
    if (base::_holdtime > 0) {
      std::ostringstream msg;
      msg << "DPRAM_trans_rsc CLASS_STARTUP - CLK_SKEW_DELAY = "
          << base::_holdtime << " ps @ " << sc_time_stamp();
      SC_REPORT_INFO(this->name(), msg.str().c_str());
    }
    reset_memory();
  }

  virtual void inject_value(int addr, int idx_lhs, int mywidth, sc_lv_base &rhs, int idx_rhs) {
    this->set_value(addr, idx_lhs, mywidth, rhs, idx_rhs);
  }

  virtual void extract_value(int addr, int idx_rhs, int mywidth, sc_lv_base &lhs, int idx_lhs) {
    this->get_value(addr, idx_rhs, mywidth, lhs, idx_lhs);
  }

private:
  void at_active_clock_edge() {
    base::at_active_clk();
  }

  void clk_skew_delay() {
    this->exchange_value(0);
    if (RCSN.get_interface())
    { _RCSN = RCSN.read(); }
    if (WCSN.get_interface())
    { _WCSN = WCSN.read(); }
    if (WEN.get_interface())
    { _WEN = WEN.read(); }
    if (D.get_interface())
    { _D = D.read(); }
    else {
      _is_connected_wr = false;
    }
    if (RA.get_interface())
    { _RA = RA.read(); }
    if (WA.get_interface())
    { _WA = WA.read(); }
    else {
      _is_connected_wr = false;
    }

    //  Write
    int _w_addr_wr = -1;
    if ( _is_connected_wr && (_WCSN==0) && (_WEN==0)) {
      _w_addr_wr = get_addr(_WA, "WA");
      if (_w_addr_wr >= 0)
      { inject_value(_w_addr_wr, 0, width, _D, 0); }
    }
    if ( !_is_connected_wr && !_is_connected_wr_messaged) {
      std::ostringstream msg;
      msg << "wr is not fully connected and writes on it will be ignored";
      SC_REPORT_WARNING(this->name(), msg.str().c_str());
      _is_connected_wr_messaged = true;
    }

    //  Sync Read
    if ((_RCSN==0)) {
      const int addr = get_addr(_RA, "RA");
      if (addr >= 0) {
        if (addr==_w_addr_wr) {
          sc_lv<width> dc; // X
          _Q = dc;
        } else
        { extract_value(addr, 0, width, _Q, 0); }
      } else {
        sc_lv<width> dc; // X
        _Q = dc;
      }
    }
    if (Q.get_interface())
    { Q = _Q; }
    this->_value_changed.notify(SC_ZERO_TIME);
  }

  int get_addr(const sc_lv<addr_width> &addr, const char *pin_name) {
    if (addr.is_01()) {
      const int cur_addr = addr.to_uint();
      if (cur_addr < 0 || cur_addr >= words) {
        #ifdef CCS_SYSC_DEBUG
        std::ostringstream msg;
        msg << "Invalid address '" << cur_addr << "' out of range [0:" << words-1 << "]";
        SC_REPORT_WARNING(pin_name, msg.str().c_str());
        #endif
        return -1;
      } else {
        return cur_addr;
      }
    } else {
      #ifdef CCS_SYSC_DEBUG
      std::ostringstream msg;
      msg << "Invalid Address '" << addr << "' contains 'X' or 'Z'";
      SC_REPORT_WARNING(pin_name, msg.str().c_str());
      #endif
      return -1;
    }
  }

  void reset_memory() {
    this->zero_data();
    _RCSN = SC_LOGIC_X;
    _WCSN = SC_LOGIC_X;
    _WEN = SC_LOGIC_X;
    _D = sc_lv<width>();
    _RA = sc_lv<addr_width>();
    _WA = sc_lv<addr_width>();
    _is_connected_wr = true;
    _is_connected_wr_messaged = false;
  }

  sc_lv<width>  _Q;
  sc_logic _RCSN;
  sc_logic _WCSN;
  sc_logic _WEN;
  sc_lv<width>  _D;
  sc_lv<addr_width>  _RA;
  sc_lv<addr_width>  _WA;
  bool _is_connected_wr;
  bool _is_connected_wr_messaged;
};
#endif // ifndef __INCLUDED_DPRAM_trans_rsc_H__



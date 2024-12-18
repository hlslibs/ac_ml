/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.9                                                 *
 *                                                                        *
 *  Release Date    : Mon Oct 14 17:47:36 PDT 2024                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.9.0                                               *
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
#ifndef _INCLUDED_USER_TRANS_H_
#define _INCLUDED_USER_TRANS_H_

template<typename T,int SIZE>
struct array_t {

  T data[SIZE][SIZE];

  void Reset() {
#pragma hls_unroll yes
    for (int i=0; i<SIZE; i++) {
#pragma hls_unroll yes
      for (int j=0; j<SIZE; j++) {
        data[i][j] = 0;
      }
    }
  }

  static const unsigned int width = T::width * SIZE*SIZE;

  template <unsigned int Size> void Marshall(Marshaller<Size> &m) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) { m &data[i][j]; }
    }
  }
  inline friend void sc_trace(sc_trace_file *tf, const array_t &v, const std::string &NAME ) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) {
        sc_trace(tf,v.data[i][j],  NAME + ".data" + std::to_string(i));
      }
    }
  }
  inline friend std::ostream &operator<<(ostream &os, const array_t &rhs) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) {
        os << rhs.data[i][j] << " ";
      }
    }
    return os;
  }
  bool operator==(const array_t &rhs) const {
    for (int i=0; i<SIZE; i++)
      for (int j=0; j<SIZE; j++)
        if ( data[i][j] != rhs.data[i][j] ) return false;
    return true;
  }
};

#endif


/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.0                                                 *
 *                                                                        *
 *  Release Date    : Mon Mar  1 17:10:46 PST 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.0.1                                               *
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
#ifndef _INCLUDED_USER_TRANS_H_
#define _INCLUDED_USER_TRANS_H_

template<typename T,int SIZE>
struct array_t {

  T data[SIZE][SIZE];

  static const unsigned int width = T::width * SIZE*SIZE;

  template <unsigned int Size> void Marshall(Marshaller<Size> &m) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) { m &data[i][j]; }
    }
  }
  inline friend void sc_trace(sc_trace_file *tf, const array_t &v, const std::string &NAME ) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) { sc_trace(tf,v.data[i][j],  NAME + ".data" + std::to_string(i)); }
    }
  }
  inline friend std::ostream &operator<<(ostream &os, const array_t &rhs) {
    for (int i=0; i<SIZE; i++) {
      for (int j=0; j<SIZE; j++) { os << rhs.data[i][j] << " "; }
    }
    return os;
  }
};

#endif


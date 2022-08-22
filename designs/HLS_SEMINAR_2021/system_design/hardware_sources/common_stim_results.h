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

#define MEM_BASE 0x70000000

  void inference()
  {
    // note : must be C compatible, as the same code will be run
    //        as a bare metal app on an embedded processor
 
    int in_fmaps[9]     = {  3-1,  16-1,  32-1,  64-1, 128-1, 256-1,  512-1, 1024-1, 1024-1};
    int out_fmaps[9]    = { 16-1,  32-1,  64-1, 128-1, 256-1, 512-1, 1024-1, 1024-1,  125-1};
    int height_width[9] = {416-1, 208-1, 104-1,  52-1,  26-1,  13-1,   13-1,   13-1,   13-1};
    int pool[9]         = {    2,     2,     2,     2,     2,     1,      0,      0,      0};

    int output_feature_maps;
    int input_feature_maps;
    int fmap_height;
    int fmap_width;
    int roffset     = 0;
    int woffset     = MEM_OFFSET;
    int wt_offset   = WEIGHT_OFFSET;
    int boffset     = BIAS_OFFSET;

    printf("Starting testbench... \n");

    for (int layer=0 ; layer<9; layer++) {

      fmap_height = fmap_width = height_width[layer]; // current fmap input size
      input_feature_maps = in_fmaps[layer];
      output_feature_maps = out_fmaps[layer];

      printf("Layer = %d \n", layer+1);
      printf(" fmap shape = %d x %d \n", fmap_height+1, fmap_width+1);
      printf(" infmaps = %d \n", input_feature_maps+1);
      printf(" outfmaps = %d \n", output_feature_maps+1);
      printf(" read offset = %d \n", roffset);
      printf(" write offset = %d \n", woffset);
      printf(" weight offset = %d \n", wt_offset);
      printf(" bias offset = %d \n", boffset);

      SET_WEIGHT_OFFSET (wt_offset);
      SET_READ_OFFSET   (roffset);
      SET_WRITE_OFFSET  (woffset);
      SET_BIAS_OFFSET   (boffset);
      SET_NUM_IN_FMAPS  (input_feature_maps);
      SET_NUM_OUT_FMAPS (output_feature_maps);
      SET_HEIGHT        (fmap_height);
      SET_WIDTH         (fmap_width);
      SET_POINTWISE     ((layer==8) ? 1 : 0);
      SET_RELU          ((layer==8) ? 0 : 1);
      SET_POOL          (pool[layer]);

      GO;               // starts accelerator computation
      WAIT_FOR_DONE;    // waits for completion, you could do something useful here, you know

      boffset   += out_fmaps[layer]+1;
      wt_offset += (input_feature_maps+1)*(output_feature_maps+1)*KSIZESQ; // move to next set of weights
      roffset    = (layer&1) ? 0 : MEM_OFFSET;
      woffset    = (layer&1) ? MEM_OFFSET: 0;
    }

    printf("Inference complete. \n");
  }


  void check_results()
  {
    // results loaded into shared memory at

    int err = 0;
    int expected;
    int actual;
    int difference;

    for (int i=0; i<13*13*125; i++) {

      int actual_addr   = (MEM_BASE + (MEM_OFFSET + i)              * WORD_SIZE);
      int expected_addr = (MEM_BASE + (EXPECTED_RESULTS_OFFSET + i) * WORD_SIZE);

      expected = TB_READ(expected_addr);
      actual   = TB_READ(actual_addr);

      difference = (expected > actual) ? expected - actual : actual - expected;

      if (difference > 10) {  // expected rounding error corresponds to roughly +/- 0.012
        printf("%5d: expected: %04x actual: %04x \n", i, expected & 0xFFFF, actual & 0xFFFF);
        err++;
      }
    }

    if (err != 0) {
      printf("Errors = %d \n", err);

#ifdef SYSTEMC
      SC_REPORT_ERROR("testbench checker", "results did not match expected output");
#endif

    } else {
      printf("Passed \n");
    }
  }

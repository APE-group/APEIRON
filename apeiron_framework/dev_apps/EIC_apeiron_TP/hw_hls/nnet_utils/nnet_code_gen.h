#ifndef NNET_INSTR_GEN_H_
#define NNET_INSTR_GEN_H_

#include "nnet_conv1d_latency.h"
#include "nnet_helpers.h"

#include "hls_stream.h"
#include "nnet_common.h"
#include "nnet_function_stubs.h"
#include "nnet_mult.h"

namespace nnet {

template <class data_T, class res_T, typename CONFIG_T> class PointwiseConv1D {
  public:
    static void pointwise_conv(data_T data[CONFIG_T::in_width * CONFIG_T::n_chan],
                               res_T res[CONFIG_T::out_width * CONFIG_T::n_filt],
                               typename CONFIG_T::weight_t weights[CONFIG_T::n_chan * CONFIG_T::n_filt],
                               typename CONFIG_T::bias_t biases[CONFIG_T::n_filt]) {
        // To be implemented in subclasses
    }
};

// hls4ml insert code
template <typename inp_t, typename out_t>
void dense_da_3(inp_t model_inp[24], out_t model_out[1]) { // ap_fixed<14,12> -> ap_fixed<19,15>
    #pragma HLS INLINE
  ap_fixed<8, 6, AP_RND, AP_WRAP> model_inp_q_0 = model_inp[0];
    ap_fixed<7, 5, AP_RND, AP_WRAP> model_inp_q_1 = model_inp[1];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_2 = model_inp[2];
    ap_fixed<13, 11, AP_RND, AP_WRAP> model_inp_q_3 = model_inp[3];
    ap_fixed<12, 10, AP_RND, AP_WRAP> model_inp_q_4 = model_inp[4];
    ap_fixed<12, 10, AP_RND, AP_WRAP> model_inp_q_5 = model_inp[5];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_6 = model_inp[6];
    ap_fixed<8, 6, AP_RND, AP_WRAP> model_inp_q_7 = model_inp[7];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_8 = model_inp[8];
    ap_fixed<12, 10, AP_RND, AP_WRAP> model_inp_q_9 = model_inp[9];
    ap_fixed<14, 12, AP_RND, AP_WRAP> model_inp_q_10 = model_inp[10];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_11 = model_inp[11];
    ap_fixed<13, 11, AP_RND, AP_WRAP> model_inp_q_12 = model_inp[12];
    ap_fixed<13, 11, AP_RND, AP_WRAP> model_inp_q_13 = model_inp[13];
    ap_fixed<10, 8, AP_RND, AP_WRAP> model_inp_q_14 = model_inp[14];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_15 = model_inp[15];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_16 = model_inp[16];
    ap_fixed<8, 6, AP_RND, AP_WRAP> model_inp_q_17 = model_inp[17];
    ap_fixed<13, 11, AP_RND, AP_WRAP> model_inp_q_18 = model_inp[18];
    ap_fixed<9, 7, AP_RND, AP_WRAP> model_inp_q_19 = model_inp[19];
    ap_fixed<12, 10, AP_RND, AP_WRAP> model_inp_q_20 = model_inp[20];
    ap_fixed<8, 6, AP_RND, AP_WRAP> model_inp_q_21 = model_inp[21];
    ap_fixed<13, 11, AP_RND, AP_WRAP> model_inp_q_22 = model_inp[22];
    ap_fixed<7, 5, AP_RND, AP_WRAP> model_inp_q_23 = model_inp[23];

    ap_fixed<9,7> v0 = model_inp_q_2; // 0.0
    ap_fixed<13,11> v1 = model_inp_q_3; // 0.0
    ap_fixed<12,10> v2 = model_inp_q_4; // 0.0
    ap_fixed<12,10> v3 = model_inp_q_5; // 0.0
    ap_fixed<9,7> v4 = model_inp_q_6; // 0.0
    ap_fixed<8,6> v5 = model_inp_q_7; // 0.0
    ap_fixed<9,7> v6 = model_inp_q_8; // 0.0
    ap_fixed<12,10> v7 = model_inp_q_9; // 0.0
    ap_fixed<14,12> v8 = model_inp_q_10; // 0.0
    ap_fixed<13,11> v9 = model_inp_q_12; // 0.0
    ap_fixed<13,11> v10 = model_inp_q_13; // 0.0
    ap_fixed<10,8> v11 = model_inp_q_14; // 0.0
    ap_fixed<9,7> v12 = model_inp_q_16; // 0.0
    ap_fixed<8,6> v13 = model_inp_q_17; // 0.0
    ap_fixed<13,11> v14 = model_inp_q_18; // 0.0
    ap_fixed<12,10> v15 = model_inp_q_20; // 0.0
    ap_fixed<8,6> v16 = model_inp_q_21; // 0.0
    ap_fixed<13,11> v17 = model_inp_q_22; // 0.0
    ap_fixed<7,5> v18 = model_inp_q_23; // 0.0
    ap_fixed<14,12> v19 = v9 + v17; // 1.0
    ap_fixed<13,11> v20 = v2 + v15; // 1.0
    ap_fixed<9,7> v21 = v13 + v16; // 1.0
    ap_fixed<11,9> v22 = v12 + v11; // 1.0
    ap_fixed<11,8> v23 = v4 + bit_shift<-1>(v6); // 1.0
    ap_fixed<10,7> v24 = v5 + bit_shift<-1>(v0); // 1.0
    ap_fixed<14,12> v25 = v1 + v14; // 1.0
    ap_fixed<15,12> v26 = v10 + bit_shift<-1>(v7); // 1.0
    ap_fixed<14,11> v27 = v3 + bit_shift<-1>(v18); // 1.0
    ap_fixed<15,13> v28 = v19 + v20; // 2.0
    ap_fixed<13,9> v29 = v22 + bit_shift<-1>(v23); // 2.0
    ap_fixed<16,13> v30 = v24 - v8; // 2.0
    ap_fixed<17,13> v31 = v25 + bit_shift<-2>(v25); // 2.0
    ap_fixed<15,12> v32 = v26 + v27; // 2.0
    ap_fixed<15,13> v33 = v28 - v21; // 3.0
    ap_fixed<17,13> v34 = v29 + v30; // 3.0
    ap_fixed<18,14> v35 = v31 + v32; // 3.0
    ap_fixed<17,15> v36 = v33 + bit_shift<2>(v34); // 4.0
    ap_fixed<18,14> v37 = v35 + v33; // 4.0
    ap_fixed<19,17> v38 = v36 - bit_shift<2>(v37); // 5.0
    ap_fixed<19,17> v39 = v38 + ap_ufixed<3,3>(7.0); // 5.0

    model_out[0] = ap_fixed<19,15>(bit_shift<-2>(v39));
}

} // namespace nnet

#endif

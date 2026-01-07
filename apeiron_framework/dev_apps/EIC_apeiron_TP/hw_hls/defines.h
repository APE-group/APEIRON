#ifndef DEFINES_H_
#define DEFINES_H_

#include "ap_fixed.h"
#include "ap_int.h"
#include "nnet_utils/nnet_types.h"
#include <cstddef>
#include <cstdio>

// hls-fpga-machine-learning insert numbers
#define N_INPUT_1_1 40
#define N_LAYER_2 20
#define N_LAYER_2 20
#define N_LAYER_5 16
#define N_LAYER_5 16
#define N_LAYER_8 4
#define N_LAYER_8 4


// hls-fpga-machine-learning insert layer-precision
typedef nnet::array<ap_fixed<16,8>, 40*1> input_t;
typedef ap_fixed<16,8> model_default_t;
typedef nnet::array<ap_fixed<31,16>, 20*1> sector_fc4_5_result_t;
typedef ap_fixed<8,1> weight2_t;
typedef ap_fixed<8,1> bias2_t;
typedef ap_uint<1> layer2_index;
typedef nnet::array<ap_ufixed<8,0,AP_RND_CONV,AP_SAT,0>, 20*1> layer4_t;
typedef ap_fixed<18,8> sector_act4_5_table_t;
typedef nnet::array<ap_fixed<22,7>, 16*1> sector_fc5_5_result_t;
typedef ap_fixed<8,1> weight5_t;
typedef ap_fixed<8,1> bias5_t;
typedef ap_uint<1> layer5_index;
typedef nnet::array<ap_ufixed<8,0,AP_RND_CONV,AP_SAT,0>, 16*1> layer7_t;
typedef ap_fixed<18,8> sector_act5_5_table_t;
typedef nnet::array<ap_fixed<21,6>, 4*1> sector_fc6_5_result_t;
typedef ap_fixed<8,1> weight8_t;
typedef ap_fixed<8,1> bias8_t;
typedef ap_uint<1> layer8_index;
typedef nnet::array<ap_ufixed<8,0,AP_RND_CONV,AP_SAT,0>, 4*1> result_t;
typedef ap_fixed<18,8> sector_act6_5_table_t;


#endif

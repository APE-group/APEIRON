#ifndef DEFINES_H_
#define DEFINES_H_

#include "ap_fixed.h"
#include "ap_int.h"
#include "nnet_utils/nnet_types.h"
#include <cstddef>
#include <cstdio>

// hls-fpga-machine-learning insert numbers
#define N_INPUT_1_1 24
#define N_LAYER_2 1
#define N_LAYER_2 1


// hls-fpga-machine-learning insert layer-precision
typedef nnet::array<ap_fixed<16,8>, 24*1> input_t;
typedef ap_fixed<16,8> model_default_t;
//typedef nnet::array<ap_fixed<16,8>, 1*1> layer2_t;
typedef ap_fixed<8,1> weight2_t;
typedef ap_fixed<8,1> bias2_t;
typedef ap_uint<1> layer2_index;
typedef nnet::array<ap_fixed<16,8>, 1*1> result_t;
typedef ap_fixed<18,8> act8_final_table_t;


#endif

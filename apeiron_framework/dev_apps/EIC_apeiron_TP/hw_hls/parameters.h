#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "ap_fixed.h"
#include "ap_int.h"

#include "nnet_utils/nnet_code_gen.h"
#include "nnet_utils/nnet_helpers.h"
// hls-fpga-machine-learning insert includes
#include "nnet_utils/nnet_activation.h"
#include "nnet_utils/nnet_activation_stream.h"
#include "nnet_utils/nnet_dense.h"
#include "nnet_utils/nnet_dense_compressed.h"
#include "nnet_utils/nnet_dense_stream.h"

/*// hls-fpga-machine-learning insert weights
#include "weights/100kHz_10ns_weights_TP_5/w2.h"
#include "weights/100kHz_10ns_weights_TP_5/b2.h"
#include "weights/100kHz_10ns_weights_TP_5/w5.h"
#include "weights/100kHz_10ns_weights_TP_5/b5.h"
#include "weights/100kHz_10ns_weights_TP_5/w8.h"
#include "weights/100kHz_10ns_weights_TP_5/b8.h"
*/

// hls-fpga-machine-learning insert layer-config
// sector_fc4_5
struct config2 : nnet::dense_config {
    static const unsigned n_in = 40;
    static const unsigned n_out = 20;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned strategy = nnet::latency;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 8;
    static const unsigned n_nonzeros = 792;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    typedef model_default_t accum_t;
    typedef bias2_t bias_t;
    typedef weight2_t weight_t;
    typedef layer2_index index_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseLatency<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

// sector_act4_5
struct relu_config4 : nnet::activ_config {
    static const unsigned n_in = 20;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 1;
    typedef sector_act4_5_table_t table_t;
};

// sector_fc5_5
struct config5 : nnet::dense_config {
    static const unsigned n_in = 20;
    static const unsigned n_out = 16;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned strategy = nnet::latency;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 4;
    static const unsigned n_nonzeros = 316;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    typedef model_default_t accum_t;
    typedef bias5_t bias_t;
    typedef weight5_t weight_t;
    typedef layer5_index index_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseLatency<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

// sector_act5_5
struct relu_config7 : nnet::activ_config {
    static const unsigned n_in = 16;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 1;
    typedef sector_act5_5_table_t table_t;
};

// sector_fc6_5
struct config8 : nnet::dense_config {
    static const unsigned n_in = 16;
    static const unsigned n_out = 4;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned strategy = nnet::latency;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 1;
    static const unsigned n_nonzeros = 63;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    typedef model_default_t accum_t;
    typedef bias8_t bias_t;
    typedef weight8_t weight_t;
    typedef layer8_index index_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseLatency<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

// sector_act6_5
struct relu_config10 : nnet::activ_config {
    static const unsigned n_in = 4;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 1;
    typedef sector_act6_5_table_t table_t;
};



#endif

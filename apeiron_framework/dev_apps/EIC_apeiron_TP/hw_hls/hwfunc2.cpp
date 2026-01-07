
//    rfnoc-hls-neuralnet: Vivado HLS code for neural-net building blocks
////    Copyright (C) 2017 EJ Kreinar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <iostream>
#include <ap_axi_sdata.h>


#include "ap_fixed.h"
#include "ap_int.h"
#include "hls_stream.h"

#include "defines.h"
#include "parameters.h"

#define N_INPUT_CHANNELS 1
#define N_OUTPUT_CHANNELS 1
#include "methods.hpp"

// hls-fpga-machine-learning insert weights
#include "weights/100kHz_10ns_weights_TP_1/w2.h"
#include "weights/100kHz_10ns_weights_TP_1/b2.h"
#include "weights/100kHz_10ns_weights_TP_1/w5.h"
#include "weights/100kHz_10ns_weights_TP_1/b5.h"
#include "weights/100kHz_10ns_weights_TP_1/w8.h"
#include "weights/100kHz_10ns_weights_TP_1/b8.h"

//#include "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework_256/include/ape_hls/hapecom.hpp"
//size_t tot_words = 0;


extern "C" void hwfunc2(
    hls::stream<input_t> &input_310,
    hls::stream<result_t> &layer10_out
) {

    // hls-fpga-machine-learning insert IO
    //#pragma HLS INTERFACE axis port=input_310,layer10_out 
    //#pragma HLS dataflow
    #pragma HLS interface ap_ctrl_none port=return
    #pragma HLS dataflow

    // hls-fpga-machine-learning insert load weights
#ifndef __SYNTHESIS__
    static bool loaded_weights = false;
    if (!loaded_weights) {
        nnet::load_weights_from_txt<weight2_t, 800>(w2, "100kHz_10ns_weights_TP_1/w2.txt");
        nnet::load_weights_from_txt<bias2_t, 20>(b2, "100kHz_10ns_weights_TP_1/b2.txt");
        nnet::load_weights_from_txt<weight5_t, 320>(w5, "100kHz_10ns_weights_TP_1/w5.txt");
        nnet::load_weights_from_txt<bias5_t, 16>(b5, "100kHz_10ns_weights_TP_1/b5.txt");
        nnet::load_weights_from_txt<weight8_t, 64>(w8, "100kHz_10ns_weights_TP_1/w8.txt");
        nnet::load_weights_from_txt<bias8_t, 4>(b8, "100kHz_10ns_weights_TP_1/b8.txt");
        loaded_weights = true;    }
#endif
    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    // hls-fpga-machine-learning insert layers

    hls::stream<sector_fc4_5_result_t> layer2_out("layer2_out");
    #pragma HLS STREAM variable=layer2_out depth=100
    nnet::dense<input_t, sector_fc4_5_result_t, config2>(input_310, layer2_out, w2, b2); // sector_fc4_5

    hls::stream<layer4_t> layer4_out("layer4_out");
    #pragma HLS STREAM variable=layer4_out depth=100
    nnet::relu<sector_fc4_5_result_t, layer4_t, relu_config4>(layer2_out, layer4_out); // sector_act4_5

    hls::stream<sector_fc5_5_result_t> layer5_out("layer5_out");
    #pragma HLS STREAM variable=layer5_out depth=100
    nnet::dense<layer4_t, sector_fc5_5_result_t, config5>(layer4_out, layer5_out, w5, b5); // sector_fc5_5

    hls::stream<layer7_t> layer7_out("layer7_out");
    #pragma HLS STREAM variable=layer7_out depth=100
    nnet::relu<sector_fc5_5_result_t, layer7_t, relu_config7>(layer5_out, layer7_out); // sector_act5_5

    hls::stream<sector_fc6_5_result_t> layer8_out("layer8_out");
    #pragma HLS STREAM variable=layer8_out depth=100
    nnet::dense<layer7_t, sector_fc6_5_result_t, config8>(layer7_out, layer8_out, w8, b8); // sector_fc6_5

    nnet::relu<sector_fc6_5_result_t, result_t, relu_config10>(layer8_out, layer10_out); // sector_act6_5

}



/*extern "C" void feature_sender(int coord, unsigned ch_id, unsigned task_id, hls::stream<input_t> &feature_out, hls::stream<ap_axis<128,0,0,0>> &message_data_out)
{

#pragma HLS pipeline enable_flush
	auto result = feature_out.read();
	word_t output_word_feature[N_OUTPUT_FEATUREs];
	word_t tmp[N_OUTPUT_FEATUREs];
    	ap_axis<128,0,0,0> data_out;
	data_out.data=0;
#pragma HLS array partition variable=tmp
	
read_output_features_loop:
	for(unsigned i=0; i<N_OUTPUT_FEATUREs; ++i){
#pragma HLS pipeline
		tmp[i].range() = result[i].range();
		data_out.data += tmp[i] << 16*i;
	}
    size_t size = sizeof(word_t);
    send(&data_out, size, coord, ch_id, task_id, message_data_out);
}

*/

#include "hwfunc_blocks.hpp"

extern "C" void top_TP_block_2( int coord, unsigned ch_id, unsigned task_id, bool start,
		hls::stream<ap_fixed<16,8>> sector_channels_PDUs[40],
		message_stream_t message_data_out[1]

){
//while(true){
#pragma HLS interface axis port=sector_channels_PDUs
#pragma HLS interface axis port=message_data_out
	coord=0;
	ch_id=0;
	task_id=0;

#pragma HLS dataflow
#pragma HLS interface ap_ctrl_none port=return
    	hls::stream<input_t> mlp_dam_input0;
	hls::stream<result_t> mlp_dam_output0;
   #pragma HLS stream variable=mlp_dam_input0 depth=100
   #pragma HLS stream variable=mlp_dam_output0 depth=100
    	preprocessing_block(sector_channels_PDUs,mlp_dam_input0);
    	hwfunc2(mlp_dam_input0, mlp_dam_output0);
	feature_sender(coord, ch_id, task_id, mlp_dam_output0, message_data_out[0]);
//}
}

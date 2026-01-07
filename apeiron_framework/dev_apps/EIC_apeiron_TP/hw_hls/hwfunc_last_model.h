
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

#include "defines_mlp.h"
#include "parameters_mlp.h"

#define N_INPUT_CHANNELS 1
#define N_OUTPUT_CHANNELS 1
#include "methods.hpp"

//#include "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework_256/include/ape_hls/hapecom.hpp"
//size_t tot_words = 0;


extern "C" void hwfunc(
    hls::stream<input_t> &input_7,
    hls::stream<result_t> &layer2_out
    //weight2_t w2[24],
   // bias2_t b2[1]) 
    )
{

    // hls-fpga-machine-learning insert IO
    #pragma HLS interface ap_ctrl_none port=return

    #pragma HLS INTERFACE axis port=input_7,layer2_out 
    #pragma HLS pipeline

	
    // hls-fpga-machine-learning insert load weights
#ifndef __SYNTHESIS__
    static bool loaded_weights = false;
    if (!loaded_weights) {
        nnet::load_weights_from_txt<weight2_t, 24>(w2, "100kHz_10ns_weights_LAST_MLP_5/w2.txt");
        nnet::load_weights_from_txt<bias2_t, 1>(b2, "100kHz_10ns_weights_LAST_MLP_5/b2.txt");
        loaded_weights = true;    }
#endif


    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    // hls-fpga-machine-learning insert layers

    nnet::dense<input_t, result_t, config2>(input_7, layer2_out, w2, b2); // sector_fc4_5
}

#define N_INPUT_SECTORs 6
#define N_INPUT_MLP 4

const int parts = N_INPUT_SECTORs*N_INPUT_MLP;

extern "C" void merge_block( 
		message_stream_t message_data_in[N_INPUT_SECTORs], 
		hls::stream<input_t> &grid_out ) 
{

	#pragma HLS pipeline
	input_t input_grid_array;//[N_INPUT_PDUs];
	//#pragma HLS ARRAY_PARTITION variable=input_grid_array factor=parts
        #pragma HLS array partition variable=input_grid_array complete 

read_input_channels_loop:
	for(int ch_i=0; ch_i<N_INPUT_SECTORs; ++ch_i){
	#pragma HLS unroll
		auto hd = message_data_in[ch_i].read();
		word_t word = message_data_in[ch_i].read();//.data;	
		auto ftr = message_data_in[ch_i].read();
		for(int i=0; i<N_INPUT_MLP; i++){
			#pragma HLS pipeline
			unsigned index=ch_i*N_INPUT_MLP+i;
			input_grid_array[index].range() = word.range(16*(i+1),16*i);//.data;
		}
	}
	grid_out.write(input_grid_array);
}

#define N_OUTPUT_FEATUREs

extern "C" void feature_extraction(unsigned stride, word_t* mem_out, hls::stream<result_t> &feature_out, bool ddr)
{

#pragma HLS pipeline
	auto result = feature_out.read();
	
	if(ddr){
		if(result[0]>=0.5) mem_out[stride]=1;
		else mem_out[stride] = 0;
	}

}




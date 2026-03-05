#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_INPUT_CHANNELS 1

extern "C" {
//typedef ap_uint<256> word_t;

void krnl_dummy(message_stream_t message_data_in[N_INPUT_CHANNELS])//, message_stream_t output_channels_PDUs[N_OUTPUT_CHANNELS])
 {
#pragma HLS INTERFACE axis port=message_data_in
	for(int i =0; i<N_INPUT_CHANNELS; ++i){
	#pragma HLS pipeline
		auto flush = message_data_in[i].read();	
	}
}
}

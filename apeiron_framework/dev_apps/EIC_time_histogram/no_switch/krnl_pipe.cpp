
#include <stdint.h>
#include <hls_stream.h>
#include <hls_print.h>
#include "ap_axi_sdata.h"
#include <iostream>
#define N_INPUT_CHANNELS 4
#define N_OUTPUT_CHANNELS 1

#define MAX_WORDS 5120

#define N_BINS 16
#define BINWIDTH 2.54*4/N_BINS

#define BITWIDTH_BIN 128/N_BINS

//typedef ap_uint<32> RDO_word_t;
typedef ap_axiu<32,0,0,0> RDO_word_t;
constexpr int partitions = 6;

constexpr int NWORDS=32<<20;

extern "C" {
typedef ap_uint<128> word_t;

void krnl_pipe(int nevents_recv, 
//#if N_OUTPUT_CHANNELS>0
	//	message_stream_t message_data_out[N_OUTPUT_CHANNELS], 
//#endif
		hls::stream<RDO_word_t> message_data_in[N_INPUT_CHANNELS]) {
#pragma HLS INTERFACE axis port=message_data_in
	
	int events = 0;
	flush_loop_pipe: while( events<nevents_recv){
			#pragma HLS pipeline
flush_loop: for(unsigned ch_id=0; ch_id<N_INPUT_CHANNELS; ch_id++){
			#pragma HLS unroll
					int flush = (int) message_data_in[ch_id].read().data;
					hls::print("channel loop index: %u", ch_id);
					hls::print("flush = 0x%.8lx\n", flush);
					if(ch_id==0 && flush == 0xffffffff){
						events++;
						std::printf("event counter = %d \n", events);

					}
			}
		}
}
}



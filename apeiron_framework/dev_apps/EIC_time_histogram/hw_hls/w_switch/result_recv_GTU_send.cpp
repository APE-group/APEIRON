#include <stdint.h>
#include <hls_stream.h>
#include <hls_print.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"



#define N_SECTORS 3
#define N_INPUT_CHANNELS 1
#define N_OUTPUT_CHANNELS 5
#define N_PORTS 3
#define MAX_WORDS 5120

#define N_BINS 4

#define BITWIDTH_BIN 32


void reading4result_ddr(word_t *mem_out_0, float threshold, message_stream_t &fifo_result, message_stream_t message_data_in[N_INPUT_CHANNELS]){
#pragma HLS inline off
		ap_uint<BITWIDTH_BIN> time_histo[N_BINS];
		#pragma HLS array_partition variable=time_histo type=complete dim=0	
		word_t histo_sum_arr[N_SECTORS];
		#pragma HLS array_partition variable=histo_sum_arr type=complete dim=0
	
		word_t result = 0;	
		for(unsigned sec=0; sec<N_SECTORS; sec++){
		#pragma HLS pipeline
			word_t word[1];
		       	auto size = receive(0, word, message_data_in);
			histo_sum_arr[sec] = word[0];
		}
		word_t histo_sum = histo_sum_arr[0] + histo_sum_arr[1] + histo_sum_arr[2];// + histo_sum_arr[3] + histo_sum_arr[4] + histo_sum_arr[5];
	
	
		for(unsigned bin=0; bin<N_BINS; bin++){
		#pragma HLS unroll
			//time_histo[bin].range() = histo_sum.range(BITWIDTH_BIN*(bin+1)-1,BITWIDTH_BIN*bin);
			time_histo[bin] = (histo_sum >> BITWIDTH_BIN*bin) & 0xFFFFFFFF;
			if(time_histo[bin]>threshold) result++;
		}
	
		mem_out_0[0] = result;
			
		fifo_result.write(result);					
	
}

void reading4result_bram(float threshold, message_stream_t &fifo_result, message_stream_t message_data_in[N_INPUT_CHANNELS]){
#pragma HLS inline off
		ap_uint<BITWIDTH_BIN> time_histo[N_BINS];
		#pragma HLS array_partition variable=time_histo type=complete dim=0	
		word_t histo_sum_arr[N_SECTORS];
		#pragma HLS array_partition variable=histo_sum_arr type=complete dim=0
	
		word_t result = 0;	
		for(unsigned sec=0; sec<N_SECTORS; sec++){
		#pragma HLS pipeline
			word_t word[1];
		       	auto size = receive(0, word, message_data_in);
			histo_sum_arr[sec] = word[0];
		}
		word_t histo_sum = histo_sum_arr[0] + histo_sum_arr[1] + histo_sum_arr[2];// + histo_sum_arr[3] + histo_sum_arr[4] + histo_sum_arr[5];
	
	
		for(unsigned bin=0; bin<N_BINS; bin++){
		#pragma HLS unroll
			//time_histo[bin].range() = histo_sum.range(BITWIDTH_BIN*(bin+1)-1,BITWIDTH_BIN*bin);
			time_histo[bin] = (histo_sum >> BITWIDTH_BIN*bin) & 0xFFFFFFFF;
			if(time_histo[bin]>threshold) result++;
		}
			
		fifo_result.write(result);					

}

void GTU_sending(message_stream_t &fifo_result, message_stream_t message_data_out[N_OUTPUT_CHANNELS]){
#pragma HLS inline off
	
	word_t result = fifo_result.read();
	if(result==0){
		for(unsigned port=0; port<N_PORTS; port++){
		#pragma HLS pipeline
			for(unsigned ch_out=0; ch_out<N_OUTPUT_CHANNELS; ch_out++){
			#pragma HLS unroll
				word_t signal_back=1;
				send(&signal_back, sizeof(word_t), 0, port, ch_out, message_data_out, false);
			}
		}
	}
	
}



constexpr int partitions = 6;

constexpr int NWORDS=32<<20;

extern "C" {
typedef ap_uint<128> word_t;

void result_recv_GTU_send(int npackets_recv, int packet_size, 
		word_t *mem_out_0,
	       	int mem_usage_test, int setup, bool eth, float threshold, int n_sectors,
		message_stream_t message_data_out[N_OUTPUT_CHANNELS], 
		message_stream_t message_data_in[N_INPUT_CHANNELS]) {
#pragma HLS INTERFACE m_axi port=mem_out_0  depth=NWORDS bundle=gmem0

#pragma HLS INTERFACE axis port=message_data_out
#pragma HLS INTERFACE axis port=message_data_in

	int size = 0;
	int stride[N_INPUT_CHANNELS];
	#pragma HLS ARRAY_PARTITION block variable=stride factor=partitions	
	
	if(setup==69){
		if(mem_usage_test==0){ //DDR
			for(int i=0; i<npackets_recv; ++i){
				#pragma HLS dataflow
				message_stream_t fifo_result;
				#pragma HLS STREAM variable=fifo_result depth=1 type=fifo
				reading4result_ddr(mem_out_0+i, threshold, fifo_result,message_data_in);
				GTU_sending(fifo_result,message_data_out);
			}
		}
		else{ //BRAM
			for(int i=0; i<npackets_recv; ++i){
				#pragma HLS dataflow
				message_stream_t fifo_result;
				#pragma HLS STREAM variable=fifo_result depth=1 type=fifo
				reading4result_bram(threshold, fifo_result,message_data_in);
				GTU_sending(fifo_result,message_data_out);
			}
		}
	}

	if(setup==96){
		for(int i=0; i<npackets_recv; ++i){
			#pragma HLS pipeline
				for(unsigned sec=0; sec<N_SECTORS; sec++){
				#pragma HLS pipeline
					word_t word[1];
				       	auto size = receive(0, word, message_data_in);
					mem_out_0[i*N_SECTORS+sec] = word[0];
				}
		}
	}

	if(setup==99){
flush_loop: for(unsigned ch_id=0; ch_id<N_INPUT_CHANNELS; ch_id++){
			#pragma HLS unroll
	flush_loop_pipe: for(unsigned j=0; j<npackets_recv;j++){
			#pragma HLS pipeline
			word_t flush[N_INPUT_CHANNELS];
			//#pragma HLS array_reshape variable=flush complete dim=0
			#pragma HLS ARRAY_PARTITION block variable=flush factor=partitions
					flush[ch_id] = message_data_in[ch_id].read();
			}
		}
	
	}

}
}



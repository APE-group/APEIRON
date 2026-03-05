#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_OUTPUT_CHANNELS 1
#define N_INPUT_CHANNELS 1
#define MAX_WORDS 5120


extern "C" {
typedef ap_uint<128> word_t;

void net_krnl(int num_of_hdrs, int npackets, int packet_size, word_t *mem_in, word_t *mem_out, int mem_usage_test, 
message_stream_t message_data_out[N_INPUT_CHANNELS], message_stream_t message_data_in[N_OUTPUT_CHANNELS]) {
#pragma HLS INTERFACE m_axi port = mem_in bundle = gmem0
#pragma HLS INTERFACE m_axi port = mem_out bundle = gmem0
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE axis port=message_data_out

	int size = 0;

	int coord = 0;
	int task_id = 0;
	unsigned ch_id = 0;

	int stride = 0;
	int nword = packet_size / sizeof(word_t);
	//unsigned task_id = 0;
	
	//1by1 LATENCY MODE
	if(mem_usage_test==0){ //DRAM
		for (int i=0; i<npackets; ++i){
		//Communication library APIs
			word_t local_hdrs[MAX_WORDS];
			#pragma HLS ARRAY_PARTITION variable=local_hdrs type=complete
			local_hdrs[0] = num_of_hdrs;
			for(int j=1; j<=num_of_hdrs; j++){
				task_id = j;
				local_hdrs[j] = forge_hdr(packet_size, coord, task_id, ch_id); 
			}
			send_gu(mem_in+stride, packet_size, local_hdrs,message_data_out); 
			size = receive_gu(ch_id, mem_out+stride, local_hdrs, message_data_in);
			stride += size/sizeof(word_t);
			}
		}
	else{ //BRAM
		for (int i=0; i<npackets; ++i){
		//Communication library APIs
			//Local buffers for BRAM test
			word_t local_send[MAX_WORDS]; 
			#pragma HLS ARRAY_PARTITION variable=local_send type=complete
			word_t local_hdrs[MAX_WORDS];
			#pragma HLS ARRAY_PARTITION variable=local_hdrs type=complete
			word_t local_receive[MAX_WORDS];
			#pragma HLS ARRAY_PARTITION variable=local_receive type=complete
			
			//Send buffer initialization
			for(int j=0; j<nword; j++) {
				local_send[j]=j;
			}

			local_hdrs[0] = num_of_hdrs;
			for(int j=1; j<=num_of_hdrs; j++){
				task_id = j;
				local_hdrs[j] = forge_hdr(packet_size, coord, task_id, ch_id); 
			}

			send_gu(local_in, packet_size, local_hdrs,message_data_out); 
			size = receive_gu(ch_id, local_receive+stride, local_hdrs, message_data_in);
			stride += size/sizeof(word_t);
		}
	}
}
}

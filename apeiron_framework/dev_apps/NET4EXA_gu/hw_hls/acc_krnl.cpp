#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_OUTPUT_CHANNELS 1
#define N_INPUT_CHANNELS 1
#define MAX_WORDS 5120

extern "C" {
typedef ap_uint<128> word_t;

void acc_krnl(message_stream_t message_data_out[N_INPUT_CHANNELS], message_stream_t message_data_in[N_OUTPUT_CHANNELS]) {
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE axis port=message_data_out

	unsigned ch_id = 0;
	
	//1by1 LATENCY MODE
        word_t hdrs_local_buff[MAX_WORDS];
	#pragma HLS ARRAY_PARTITION variable=hdrs_local_buff type=complete
        word_t dt_recv_buff[MAX_WORDS];
	#pragma HLS ARRAY_PARTITION variable=dt_recv_buff type=complete
        word_t dt_send_buff[MAX_WORDS];
	#pragma HLS ARRAY_PARTITION variable=dt_send_buff type=complete
	
	word_t accum = 0;

        int size = receive_gu(ch_id, dt_recv_buff, hdrs_local_buff, message_data_in);
	for(int i=0; i<size/sizeof(word_t); i++){
	#pragma HLS pipeline
		accum += dt_recv_buff[i];
	}
	for(int i=0; i<size/sizeof(word_t); i++){
	#pragma HLS unroll
		dt_send_buff[i] = accum;
	}
        
	send_gu(dt_send_buff, size, hdrs_local_buff,message_data_out);
	
}
}

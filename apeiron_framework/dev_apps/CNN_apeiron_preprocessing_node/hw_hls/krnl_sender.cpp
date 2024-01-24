// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "apenet_packet.h"

#define N_OUTPUT_CHANNELS 1
#define N_PORTS 1

#include "ape_hls/hapecom.hpp"

#ifndef NKERNEL
#define NKERNEL 1
#endif


void RoundRobin(unsigned pck_counter, unsigned& inter_dest, int nboards, unsigned& task_id, unsigned& cu){
	unsigned ch_counter = pck_counter % N_OUTPUT_CHANNELS;
	if(ch_counter == N_OUTPUT_CHANNELS - 1){
		task_id++;
		if(task_id > N_PORTS) task_id =  1;
		if(task_id == N_PORTS){
			inter_dest++;
			inter_dest = (inter_dest % nboards);
		}
	}
	cu=ch_counter;
	
}

extern "C" {

typedef ap_uint<128> word_t;
void krnl_sender(int nboards, int npackets,  word_t *data, hls::stream<word_t> message_data_out[N_OUTPUT_CHANNELS]) {
#pragma HLS INTERFACE axis port=message_data_out

	unsigned start = 0;
	
	unsigned pck_counter = 0;
	unsigned cu = 0;
	unsigned reminder = 0;
	unsigned inter_dest = 0;
	unsigned task_id = 1;

main_loop:
	for (int i=0; i<npackets; ++i){
	#pragma HLS pipeline
		//Bit-management of the M2EGP header to obtain event's total_words
		//word_t word_buffer[10];
		//word_buffer[0]=data[0];
		word_t word = data[start];
		unsigned total_hits = word.range(47, 40);
		unsigned total_words = total_hits >> 3; // /8
		reminder = total_hits & 0x7;
		if (reminder) total_words++;
	
		//RoundRobin(pck_counter, inter_dest, nboards, task_id, cu);	
		//send() api
		
	//	unsigned ch_counter = i % NKERNEL;
		//for(int i=0; i<npackets; ++i){
//#pragma HLS pipeline	
		unsigned ch_id = i % N_OUTPUT_CHANNELS;
		send(data+start, (total_words+1)*sizeof(word_t), 0, 1, ch_id, message_data_out);
	//	}
		//RoundRobin(i , inter_dest, nboards, task_id, cu);

		start+=total_words+1;
		
	/*	if(ch_counter == NKERNEL - 1){
			task_id++;
			if(task_id > N_PORTS) task_id = 1;
			if(task_id == N_PORTS){
				inter_dest++;
				inter_dest = (inter_dest % nboards);
			}
		}
	*/	
	}
}

}





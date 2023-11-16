// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "apenet_packet.h"
#include "ape_hls/hapecom.hpp"

#define N_INPUT_CHANNELS 1

extern "C" {


typedef ap_uint<128> word_t;


void krnl_receiver(word_t* out, int npackets, hls::stream<word_t> message_data_in[N_INPUT_CHANNELS]) {
#pragma HLS INTERFACE m_axi port = out bundle = gmem0
#pragma HLS INTERFACE axis port=message_data_in

	int pck_counter=0;
	for (int i=0; i<npackets; ++i){
	#pragma HLS loop_tripcount max=64
	#pragma HLS pipeline	
		//receive api
		unsigned ch_id = i % N_INPUT_CHANNELS;
		int size=receive(ch_id,out+pck_counter,message_data_in);
		pck_counter+=size/sizeof(word_t);
	}

}
}



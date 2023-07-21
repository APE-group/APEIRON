// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#ifndef AGGREGATOR_HPP
#define AGGREGATOR_HPP

#include <stdint.h>
#include <cstdlib>
#include <ap_int.h>
#include <hls_stream.h>

#include "apenet_packet.h"
#include "methods.hpp"
#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

typedef ap_uint<128> uint128_t;
typedef uint128_t word_t;
typedef hls::stream<uint128_t> message_stream_t;
typedef hls::stream<ap_uint<2>> kernel_stream_t;
typedef hls::stream<apenet_header_t> header_stream_t;
typedef short channel_id_t;
typedef short task_id_t;


namespace ape {

template <unsigned NCHAN>
void aggregator_template(
		unsigned nevents,
		message_stream_t fifo_data_in[NCHAN],
		header_stream_t &fifo_hdr_out,
		message_stream_t &fifo_data_out)
{
#pragma HLS INLINE
	
	for(unsigned ch=0; ch<NCHAN; ch++){
		#pragma HLS LOOP_TRIPCOUNT min=1 max=NCHAN
		#pragma HLS unroll 
		
		if(!fifo_data_in[ch].empty()){ //continue;
		
		//Send header
		apenet_header_t hdr = {0};
		auto tmp = fifo_data_in[ch].read();
		
		hdr = word_2_apenet(tmp);
		/*hdr.s.dest_x = tmp.range(106,101);
		hdr.s.intra_dest = tmp.range(90,87);
		hdr.s.packet_size = tmp.range(79,66);
		hdr.s.proc_id = tmp.range(122,107);
		hdr.s.dest_addr = 0xfafbfcfd;*/
		fifo_hdr_out.write(hdr);

		for(int i=0; i<hdr.s.packet_size/sizeof(word_t); i++){
			#pragma HLS LOOP_TRIPCOUNT min=1 max=256
			#pragma HLS pipeline
			auto pippo = fifo_data_in[ch].read();
			fifo_data_out.write(pippo);
		} 

		apenet_header_t ftr = {0};
		auto tmp2 = fifo_data_in[ch].read();
		
		ftr = word_2_apenet(tmp2);
		
		/*ftr.s.dest_x = tmp.range(106,101);
		ftr.s.intra_dest = tmp.range(90,87);
		ftr.s.packet_size = tmp.range(79,66);
		ftr.s.proc_id = tmp.range(122,107);
		ftr.s.dest_addr = 0xaaaeabac;
		ftr.s.edac = 0x99;*/
		fifo_hdr_out.write(ftr);
		}
		//if(fifo_data_in[ch].empty()) continue;
	}
	}
}



#endif

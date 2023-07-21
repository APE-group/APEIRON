// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP


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
typedef hls::stream<apenet_header_t> header_stream_t;
typedef short channel_id_t;
typedef short task_id_t;


namespace ape{

template <unsigned NCHAN>
void reader(
		header_stream_t &fifo_hdr_in,
		message_stream_t &fifo_data_in,
		//header_stream_t fifo_hdr_out[NCHAN],
		message_stream_t fifo_data_out[NCHAN])
{
	//while (true) {
		apenet_header_t hdr = fifo_hdr_in.read();

		unsigned input_channel = hdr.s.proc_id;
		unsigned size = hdr.s.packet_size;
		
		word_t tmp_header = apenet_2_word(hdr);
		/*tmp_header(127,123)=hdr.s.virt_chan;
		tmp_header(122,107)=hdr.s.proc_id;
		tmp_header(106,101)=hdr.s.dest_x;
		tmp_header(100,96)=hdr.s.dest_y;
		tmp_header(95,91)=hdr.s.dest_z;
		tmp_header(90,87)=hdr.s.intra_dest;
		tmp_header(86,86)=hdr.s.reserved;
		tmp_header(85,85)=hdr.s.out_of_lattice;
		tmp_header(84,80)=hdr.s.packet_type;
		tmp_header(79,66)=hdr.s.packet_size;
		tmp_header(65,18)=hdr.s.dest_addr;
		tmp_header(17,8)=hdr.s.num_of_hops;
		tmp_header(7,0)=hdr.s.edac;
*/
		fifo_data_out[input_channel].write(tmp_header);

		for (unsigned i = 0; i < size/sizeof(uint128_t); ++i) {
			#pragma HLS LOOP_TRIPCOUNT min=1 max=256
			//#pragma HLS UNROLL
			auto tmp = fifo_data_in.read();
			fifo_data_out[input_channel].write(tmp);
		}

		apenet_header_t ftr = fifo_hdr_in.read();
		word_t tmp_footer = apenet_2_word(ftr);
		/*tmp_footer(127,123)=ftr.s.virt_chan;
		tmp_footer(122,107)=ftr.s.proc_id;
		tmp_footer(106,101)=ftr.s.dest_x;
		tmp_footer(100,96)=ftr.s.dest_y;
		tmp_footer(95,91)=ftr.s.dest_z;
		tmp_footer(90,87)=ftr.s.intra_dest;
		tmp_footer(86,86)=ftr.s.reserved;
		tmp_footer(85,85)=ftr.s.out_of_lattice;
		tmp_footer(84,80)=ftr.s.packet_type;
		tmp_footer(79,66)=ftr.s.packet_size;
		tmp_footer(65,18)=ftr.s.dest_addr;
		tmp_footer(17,8)=ftr.s.num_of_hops;
		tmp_footer(7,0)=ftr.s.edac;*/
		
		fifo_data_out[input_channel].write(tmp_footer); //footer
	//}
}

template <typename T>
void writer(T &fifo_in, T &fifo_out) {
	//while (true) {
		auto tmp = fifo_in.read();
		fifo_out.write(tmp);
	//}
}

template <unsigned NCHAN, unsigned HD_DEPTH, unsigned DT_DEPTH>
void dispatcher_template(
		header_stream_t &fifo_hdr_in,
		message_stream_t &fifo_data_in,
		message_stream_t fifo_data_out[NCHAN])
{
#pragma HLS INLINE

	//message_stream_t dt_stream[NCHAN];
	//#pragma HLS STREAM variable=dt_stream depth=DT_DEPTH

	reader<NCHAN>(fifo_hdr_in, fifo_data_in, fifo_data_out);
	/*for (unsigned i=0; i<NCHAN; i++) {
	#pragma HLS PIPELINE style = flp	
		//if(dt_stream[i].empty()) continue;
		writer<message_stream_t>(dt_stream[i], fifo_data_out[i]);
	}*/
}

}

#endif

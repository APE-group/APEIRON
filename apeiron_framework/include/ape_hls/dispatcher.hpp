// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP


#include <stdint.h>
#include <cstdlib>
#include <ap_int.h>
#include <hls_stream.h>

#include "apenet_packet.h"
#include "hapecom.hpp"
#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

/*typedef ap_uint<256> word_t;
typedef uint128_t word_t;
typedef hls::stream<word_t> message_stream_t;
typedef hls::stream<apenet_header_t> header_stream_t;
typedef short channel_id_t;
typedef short task_id_t;
*/

namespace ape{

template <unsigned NCHAN>
void reader(
		header_stream_t &fifo_hdr_in,
		message_stream_t &fifo_data_in,
		//header_stream_t fifo_hdr_out[NCHAN],
		message_stream_t fifo_data_out[NCHAN])
{
	//while (true) {
#pragma HLS PIPELINE II=1
		apenet_header_t hdr = fifo_hdr_in.read();

		unsigned input_channel = hdr.s.proc_id;
		unsigned size = hdr.s.packet_size;
	
		word_t tmp_header = apenet_2_word(hdr);
		fifo_data_out[input_channel].write(tmp_header);

		unsigned nwords = (size & (sizeof(word_t)-1)) ? (size/sizeof(word_t)+1) : size/sizeof(word_t);

		for (unsigned i = 0; i < nwords; ++i) {
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_TRIPCOUNT min=1 max=256
			fifo_data_out[input_channel].write(fifo_data_in.read());
		}

		apenet_header_t ftr = fifo_hdr_in.read();
		word_t tmp_footer = apenet_2_word(ftr);
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
#pragma HLS DATAFLOW

	//message_stream_t dt_stream[NCHAN];
	//#pragma HLS STREAM variable=dt_stream depth=DT_DEPTH

	reader<NCHAN>(fifo_hdr_in, fifo_data_in, fifo_data_out);
}

}

#endif

#ifndef METHODS_HPP
#define METHODS_HPP

#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <ap_int.h>
#include <hls_stream.h>

#include "apenet_packet.h"
#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

#ifndef N_INPUT_CHANNELS
#define N_INPUT_CHANNELS  4
#endif

#ifndef N_OUTPUT_CHANNELS
#define N_OUTPUT_CHANNELS  4
#endif


typedef ap_uint<128> uint128_t;
typedef uint128_t word_t;
typedef hls::stream<uint128_t> message_stream_t;
typedef hls::stream<apenet_header_t> header_stream_t;
typedef short channel_id_t;
typedef short task_id_t;

apenet_header_t word_2_apenet(word_t wrd){
		apenet_header_t header;
		header.s.dest_x = wrd.range(106,101);
		header.s.intra_dest = wrd.range(90,87);
		header.s.packet_size = wrd.range(79,66);
		header.s.proc_id = wrd.range(122,107);
		header.s.dest_addr = 0xaaaeabac;
		header.s.edac = 0x99;
		
		return header;
}

word_t apenet_2_word(apenet_header_t hd){
		word_t header;
		header(127,123) = hd.s.virt_chan;
		header(122,107) = hd.s.proc_id;
		header(106,101) = hd.s.dest_x;
		header(100,96) = hd.s.dest_y;
		header(95,91) = hd.s.dest_z;
		header(90,87) = hd.s.intra_dest;
		header(86,86) = hd.s.reserved;
		header(85,85) = hd.s.out_of_lattice;
		header(84,80) = hd.s.packet_type;
		header(79,66) = hd.s.packet_size;
		header(65,18) = hd.s.dest_addr;
		header(17,8) = hd.s.num_of_hops;
		header(7,0) = hd.s.edac;
		
		return header;
}

// (Blocking) receive api ==> allows the user to read data from the network without knowing the protocol
int receive(channel_id_t ch_id, uint128_t *buff, 
		message_stream_t message_data_in[N_INPUT_CHANNELS]){

	word_t hdr = message_data_in[ch_id].read(); 

	int size = hdr.range(79,66);

	for (short i = 0; i < size/sizeof(word_t); ++i){ 
	#pragma HLS pipeline
		buff[i] = message_data_in[ch_id].read(); 
	}

	word_t ftr = message_data_in[ch_id].read();  
	
	return size;

}

//(Streaming) receive api ==> allows the user to read data from the network declaring a local stream
int receive_streaming(channel_id_t ch_id, message_stream_t &kernel_stream, message_stream_t message_data_in[N_INPUT_CHANNELS]){

	static int size[N_INPUT_CHANNELS];
	static bool init = false;
	static int v_size = 0;
	if(!init){
		for(unsigned i=0; i < N_INPUT_CHANNELS; i++) size[i]=0;
		init = true;
	}
	
	if(size[ch_id]==0) {
		word_t hdr = message_data_in[ch_id].read(); 
		size[ch_id] = hdr.range(79,66);
		v_size = size[ch_id];
	}
	
	if(size[ch_id]>0){
		kernel_stream.write(message_data_in[ch_id].read());
		size[ch_id]--;
	}
	else word_t ftr = message_data_in[ch_id].read();

	return v_size;	
	
}

// (Non-Blocking) send api ==> allows the user to write data on the network without knowing the protocol
size_t send(uint128_t *buff, size_t size, int coord,
			task_id_t task_id, channel_id_t ch_id, 
			message_stream_t message_data_out[N_OUTPUT_CHANNELS]
			){
	  
	//create and write hdr + data + footer 
		apenet_header_t tmp_hd = {0};
		tmp_hd.s.dest_x = coord & 0b111111;
		//tmp_header.s.dest_y = (coord>>6) & 0b11111;
		//tmp_header.s.dest_z = (coord>>11) & 0b11111;
		//tmp_hd.s.intra_dest = (coord>>16) & 0b1111;
		tmp_hd.s.intra_dest = task_id;
		tmp_hd.s.packet_size = size;//packet_size;
		tmp_hd.s.dest_addr = 0xfafbfcfd;
		tmp_hd.s.proc_id = ch_id;
		
		word_t tmp_header = apenet_2_word(tmp_hd);
		
		message_data_out[ch_id].write(tmp_header); 

		for (short i = 0; i < size/sizeof(word_t); ++i){
		//#pragma HLS pipeline 
//#pragma HLS LOOP_TRIPCOUNT min=1 max=256
			message_data_out[ch_id].write(buff[i]);
			} 
		
		apenet_header_t tmp_ftr = {0};
		tmp_ftr.s.dest_addr = 0xaaaeabac;
		tmp_ftr.s.edac = 0x99;
		
		word_t tmp_footer = apenet_2_word(tmp_ftr);
		message_data_out[ch_id].write(tmp_footer); //footer 

	return size;
}

//(Streaming) send api ==> allows the user to write data on the network declaring a local stream
size_t send_streaming(message_stream_t &kernel_stream, size_t size, int coord, 
			task_id_t task_id, channel_id_t ch_id, 
			message_stream_t message_data_out[N_OUTPUT_CHANNELS]){
	
	static size_t send_size[N_OUTPUT_CHANNELS];
	static bool send_init = false;
	if(!send_init){
		for(unsigned i=0; i < N_OUTPUT_CHANNELS; i++) send_size[i]=0;
		send_init = true;
	}
	
	if(send_size[ch_id]==0){
		apenet_header_t tmp_hd = {0};
		tmp_hd.s.dest_x = coord & 0b111111;
		//tmp_header.s.dest_y = (coord>>6) & 0b11111;
		//tmp_header.s.dest_z = (coord>>11) & 0b11111;
		//tmp_hd.s.intra_dest = (coord>>16) & 0b1111;
		tmp_hd.s.intra_dest = task_id;
		tmp_hd.s.packet_size = size;//packet_size;
		tmp_hd.s.dest_addr = 0xfafbfcfd;
		tmp_hd.s.proc_id = ch_id;
		
		word_t tmp_header = apenet_2_word(tmp_hd);
		
		
		message_data_out[ch_id].write(tmp_header);
		send_size[ch_id]=size; 
	}
	
	if(send_size[ch_id]>0){
		message_data_out[ch_id].write(kernel_stream.read());
		send_size[ch_id]--;
	}
	else{
		word_t ftr = 0x99;
		message_data_out[ch_id].write(ftr);
	}
			
	return send_size[ch_id];
}

#endif


#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"
#include <hls_print.h>

#define N_OUTPUT_CHANNELS 42
#define N_SUBSECTORS 1
#define N_WORDS 2

//typedef ap_uint<32> RDO_word_t;
typedef ap_axiu<32,0,0,0> RDO_word_t;
typedef ap_uint<128> APE_word_t;

//***** Loop for DDR reading *****//
void ddr_loop( unsigned (&last_words_counted)[N_OUTPUT_CHANNELS], unsigned channel_offset[N_OUTPUT_CHANNELS], const unsigned int *mem_in, 
				hls::stream<RDO_word_t> output_channels_PDUs[N_OUTPUT_CHANNELS]) {
#pragma HLS inline off
//#pragma HLS pipeline
    
	bool channel_done[N_OUTPUT_CHANNELS];
#pragma HLS array_partition variable=channel_done complete
    
    	init_loop: for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
		#pragma HLS unroll
        		channel_done[ch] = false;
    	}

    	bool all_done = false;
    	// loop principale: gira finché tutti i canali hanno finito
    	main_loop: while (!all_done) {
		#pragma HLS pipeline II=1
        		all_done = true;
        		channel_scan: for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
				#pragma HLS unroll
            				if (!channel_done[ch]) {
                				unsigned addr = channel_offset[ch] + last_words_counted[ch];
                				RDO_word_t w;
                				w.data = mem_in[addr];
                				output_channels_PDUs[ch].write(w);
                				last_words_counted[ch]++;
                				if (w.data == 0xffffffff) channel_done[ch] = true;
                				else all_done = false;
              	
            				}
        			}
		   }
}


//***** Loop for BRAM reading *****//
void bram_loop(hls::stream<RDO_word_t> output_channels_PDUs[N_OUTPUT_CHANNELS]){ 
#pragma HLS inline off
//#pragma HLS pipeline
		for(int ch=0; ch<N_OUTPUT_CHANNELS; ch++){
		#pragma HLS UNROLL
    			for (int w = 0; w < N_WORDS; w++) {
			#pragma HLS PIPELINE II=1
        		RDO_word_t word;
        		word.data = (w < N_WORDS-1) ? 0xd0044009 : 0xffffffff;
        		output_channels_PDUs[ch].write(word);
    			}
		}
}

//***** GTU message management *****//
void GTU_message_send(hls::stream<bool> GTU_link_out[N_SUBSECTORS], hls::stream<APE_word_t> message_data_in[1]){
#pragma HLS inline off
//#pragma HLS pipeline
	word_t local_receive;
	if(!message_data_in[0].empty()){
		if(receive(0, &local_receive, message_data_in) > 0){	
			for(int subsec = 0; subsec<N_SUBSECTORS; subsec++){
			#pragma HLS unroll
				GTU_link_out[subsec].write(1);
			}
		}
	}
}

extern "C" {

void krnl_load(unsigned nevents, 
	       const unsigned int *mem_in_0, //Host memory buffer per subsector
	       const unsigned int *channel_offset_0, //Number of words per channel per subsectors
	       bool ddr, //Memory usage flag
	       hls::stream<RDO_word_t> output_channels_PDUs_0[N_OUTPUT_CHANNELS], //Arrays of output streams

	       hls::stream<bool> start_signal_in[1],
	       hls::stream<bool> GTU_link_out[N_SUBSECTORS],

	       hls::stream<APE_word_t> message_data_in[1]
		)
 {
#pragma HLS INTERFACE axis port=start_signal_in
#pragma HLS INTERFACE axis port=GTU_link_out
#pragma HLS INTERFACE axis port=output_channels_PDUs_0
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE m_axi port = mem_in_0 bundle = gmem0
#pragma HLS INTERFACE m_axi port = channel_offset_0 bundle=gmem5

//#pragma HLS DATAFLOW
//***** Word Counting array for each subsector *****//

	unsigned words_counted_0[N_OUTPUT_CHANNELS];
#pragma HLS ARRAY_PARTITION variable=words_counted_0 type=complete	
	for(int i=0; i<N_OUTPUT_CHANNELS; ++i){
	#pragma HLS PIPELINE II=1
		words_counted_0[i] = 0;
	}
	
//*****  Offset to read each channel from a flat mem_in_* array*****//
	unsigned offset_l_0[N_OUTPUT_CHANNELS];
#pragma HLS ARRAY_PARTITION variable=offset_l_0 type=complete	
	for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
	#pragma HLS PIPELINE II=1
	    offset_l_0[ch] = channel_offset_0[ch];
	}

	bool start = start_signal_in[0].read();
	if(start){
//*****  Main Core  *****//
		if(ddr){
	    		for(int ev=0; ev<nevents; ev++){
			#pragma HLS dataflow
				ddr_loop(words_counted_0, offset_l_0, mem_in_0, output_channels_PDUs_0);
				GTU_message_send(GTU_link_out, message_data_in);
			}		
	 	}
	
		else{
	    		for(int ev=0; ev<nevents; ev++){
			#pragma HLS dataflow
				/*for(int ch=0; ch<N_OUTPUT_CHANNELS; ch++){
				#pragma HLS UNROLL
	    				bram_loop(output_channels_PDUs_0[ch]);
				}*/
	    			bram_loop(output_channels_PDUs_0);
				GTU_message_send(GTU_link_out, message_data_in);
			}
	
	 	}
	}
	
}
}

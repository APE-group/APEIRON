#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"
#include <hls_print.h>

#define N_OUTPUT_CHANNELS 42
#define N_SUBSECTORS 5

//typedef ap_uint<32> RDO_word_t;
typedef ap_axiu<32,0,0,0> RDO_word_t;
typedef ap_uint<128> APE_word_t;

//***** Loop for DDR reading *****//
void ddr_loop( unsigned (&last_words_counted)[N_OUTPUT_CHANNELS], unsigned channel_offset[N_OUTPUT_CHANNELS], const unsigned int *mem_in, 
				hls::stream<RDO_word_t> output_channels_PDUs[N_OUTPUT_CHANNELS]) {
#pragma HLS inline off

    bool channel_done[N_OUTPUT_CHANNELS];
//#pragma HLS array_partition variable=channel_done complete
    
    	init_loop: for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
//		#pragma HLS unroll
        		channel_done[ch] = false;
    	}

    	bool all_done = false;
    	// loop principale: gira finché tutti i canali hanno finito
    	main_loop: while (!all_done) {
//		#pragma HLS pipeline II=1
        		all_done = true;
        		channel_scan: for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
//				#pragma HLS unroll
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
void bram_loop(hls::stream<RDO_word_t> &output_channels_PDUs){ 
#pragma HLS inline off
    		for (int w = 0; w < 10; w++) {
		#pragma HLS PIPELINE II=1
        	RDO_word_t word;
        	word.data = (w < 9) ? 0xd0044009 : 0xffffffff;
        	output_channels_PDUs.write(word);
    		}
}

//***** GTU message management *****//
void GTU_message_send(hls::stream<bool> GTU_link_out[N_SUBSECTORS], hls::stream<APE_word_t> message_data_in[1]){
#pragma HLS inline off
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
	       const unsigned int *mem_in_1,
	       const unsigned int *mem_in_2,
	       const unsigned int *mem_in_3,
	       const unsigned int *mem_in_4,
	       const unsigned int *channel_offset_0, //Number of words per channel per subsectors
	       const unsigned int *channel_offset_1, 
	       const unsigned int *channel_offset_2, 
	       const unsigned int *channel_offset_3, 
	       const unsigned int *channel_offset_4,
	       bool ddr, //Memory usage flag
	       hls::stream<RDO_word_t> output_channels_PDUs_0[N_OUTPUT_CHANNELS], //Arrays of output streams
	       hls::stream<RDO_word_t> output_channels_PDUs_1[N_OUTPUT_CHANNELS],
	       hls::stream<RDO_word_t> output_channels_PDUs_2[N_OUTPUT_CHANNELS],
	       hls::stream<RDO_word_t> output_channels_PDUs_3[N_OUTPUT_CHANNELS],
	       hls::stream<RDO_word_t> output_channels_PDUs_4[N_OUTPUT_CHANNELS],

	       hls::stream<bool> GTU_link_out[N_SUBSECTORS],

	       hls::stream<APE_word_t> message_data_in[1]
		)
 {
#pragma HLS INTERFACE axis port=output_channels_PDUs_0
#pragma HLS INTERFACE axis port=output_channels_PDUs_1
#pragma HLS INTERFACE axis port=output_channels_PDUs_2
#pragma HLS INTERFACE axis port=output_channels_PDUs_3
#pragma HLS INTERFACE axis port=output_channels_PDUs_4

#pragma HLS INTERFACE axis port=message_data_in

#pragma HLS INTERFACE m_axi port = mem_in_0 bundle = gmem0
#pragma HLS INTERFACE m_axi port = mem_in_1 bundle = gmem1
#pragma HLS INTERFACE m_axi port = mem_in_2 bundle = gmem2
#pragma HLS INTERFACE m_axi port = mem_in_3 bundle = gmem3
#pragma HLS INTERFACE m_axi port = mem_in_4 bundle = gmem4

#pragma HLS INTERFACE m_axi port = channel_offset_0 bundle=gmem5
#pragma HLS INTERFACE m_axi port = channel_offset_1 bundle=gmem6
#pragma HLS INTERFACE m_axi port = channel_offset_2 bundle=gmem7
#pragma HLS INTERFACE m_axi port = channel_offset_3 bundle=gmem8
#pragma HLS INTERFACE m_axi port = channel_offset_4 bundle=gmem9


//#pragma HLS DATAFLOW
//***** Word Counting array for each subsector *****//

//	unsigned words_counted_0[N_OUTPUT_CHANNELS];
//	unsigned words_counted_1[N_OUTPUT_CHANNELS];
//	unsigned words_counted_2[N_OUTPUT_CHANNELS];
//	unsigned words_counted_3[N_OUTPUT_CHANNELS];
//	unsigned words_counted_4[N_OUTPUT_CHANNELS];
	
//	for(int i=0; i<N_OUTPUT_CHANNELS; ++i){
//	#pragma HLS PIPELINE II=1
//		words_counted_0[i] = 0;
//		words_counted_1[i] = 0;
//		words_counted_2[i] = 0;
//		words_counted_3[i] = 0;
//		words_counted_4[i] = 0;
//	}
	
//*****  Offset to read each channel from a flat mem_in_* array*****//
//	unsigned offset_l_0[N_OUTPUT_CHANNELS];
//	unsigned offset_l_1[N_OUTPUT_CHANNELS];
//	unsigned offset_l_2[N_OUTPUT_CHANNELS];
//	unsigned offset_l_3[N_OUTPUT_CHANNELS];
//	unsigned offset_l_4[N_OUTPUT_CHANNELS];
//	for (int ch = 0; ch < N_OUTPUT_CHANNELS; ch++) {
//	#pragma HLS PIPELINE II=1
//	    offset_l_0[ch] = channel_offset_0[ch];
//	    offset_l_1[ch] = channel_offset_1[ch];
//	    offset_l_2[ch] = channel_offset_2[ch];
//	    offset_l_3[ch] = channel_offset_3[ch];
//	    offset_l_4[ch] = channel_offset_4[ch];
//	}

//*****  Main Core  *****//
	if(ddr){
    		for(int ev=0; ev<nevents; ev++){
//#pragma HLS pipeline
//			ddr_loop(words_counted_0, offset_l_0, mem_in_0, output_channels_PDUs_0);
//			ddr_loop(words_counted_1, offset_l_1, mem_in_1, output_channels_PDUs_1);
//			ddr_loop(words_counted_2, offset_l_2, mem_in_2, output_channels_PDUs_2);
//			ddr_loop(words_counted_3, offset_l_3, mem_in_3, output_channels_PDUs_3);
//			ddr_loop(words_counted_4, offset_l_4, mem_in_4, output_channels_PDUs_4);
//			
//			GTU_message_send(GTU_link_out, message_data_in);
		}		
 	}

	else{
    		for(int ev=0; ev<nevents; ev++){
		#pragma HLS DATAFLOW
			for(int ch=0; ch<N_OUTPUT_CHANNELS; ch++){
			#pragma HLS UNROLL
    				bram_loop(output_channels_PDUs_0[ch]);
    				bram_loop(output_channels_PDUs_1[ch]);
    				bram_loop(output_channels_PDUs_2[ch]);
    				bram_loop(output_channels_PDUs_3[ch]);
    				bram_loop(output_channels_PDUs_4[ch]);	
			}

			GTU_message_send(GTU_link_out, message_data_in);
		}

 	}
	
}
}

// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2
#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "xx.h"
#include "yy.h"




#define IMAGE_SIZE 16
#define MAX_WORD 8
#define MAX_HIT_PER_WORD 8

#define N_OUTPUT_CHANNELS 1
#define N_INPUT_CHANNELS 1

#include "ape_hls/hapecom.hpp"
//#define N_PORTS 2
//#define N_BOARDS 4

extern "C" {
typedef ap_uint<128> word_t;
void image_sender(unsigned int nports, unsigned int nboards, message_stream_t message_data_out[N_INPUT_CHANNELS], message_stream_t message_data_in[N_OUTPUT_CHANNELS]) {
//#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE axis port=message_data_out	 


	unsigned ch_id = 0;
	//message_stream_t input_stream;
	//#pragma HLS STREAM variable=input_stream
	while(true){
		 
		 //ch_id = (ch_id + 1) % N_OUTPUT_CHANNELS;
		 word_t word;
		 word_t buff_out[5];
	 	 ap_uint<IMAGE_SIZE*IMAGE_SIZE> image = 0;
	 	 size_t size=0;
	 	 size_t tmp=0;
	 	
	 	
		 for (int i=0; i<MAX_WORD; i++) {
        	 	if(size==0){
        	 		//auto hd = message_data_in[ch_id].read();
        	 		//size = hd.range(79,66)/sizeof(word_t);
        	 		//buff_out[0] = message_data_in[ch_id].read();
        	 		//size--;
		 		//size = receive_streaming(ch_id, input_stream, message_data_in);
		 		//auto hd = input_stream.read();
		 		size = receive_streaming(ch_id, word, message_data_in);
		 		size--;
		 		buff_out[0] = word;

        		}
      
        		if(size>0){
				//word = message_data_in[ch_id].read();
				tmp = receive_streaming(ch_id, word, message_data_in);
				//word = input_stream.read();
			}
        
       	 		for (int j=0; j<MAX_HIT_PER_WORD; j++) {
//#pragma HLS pipeline
           		if (size==0) continue;
            		unsigned short pmt = word.range((j+1)*16-1, j*16);
            		if (pmt==0) continue;
            		auto x = x_bin[pmt];
            		auto y = y_bin[pmt];
            		if (x>=0 && y>=0)
                		image.set(x+IMAGE_SIZE*y);
        		}	
        		if(size>0) size--;
       
        		if (size==0){
    //   		auto ftr = message_data_in[ch_id].read();
    		   		break;
       			} 
    		}
    	
    			if(size>0){
       				while(size>0){
       				word_t flush;
				tmp = receive_streaming(ch_id, flush, message_data_in);
       				//auto flush = message_data_in[ch_id].read();
				//auto flush = input_stream.read();
       				size--;
       			}
       	//auto ftr = message_data_in[ch_id].read();
      		}
		
		 
		buff_out[1] = image.range(127,0);
		buff_out[2] = image.range(255,128);
			
		static unsigned task_id = 2;
		static unsigned dest_coord = 0;	
		send(buff_out, 3*sizeof(word_t), dest_coord, task_id, ch_id, message_data_out);

			
		ch_id = (ch_id + 1) % N_OUTPUT_CHANNELS;
//		if(ch_id >= N_OUTPUT_CHANNELS-1) task_id++;
		//dest_coord++;
			 
			
//		if(task_id >= nports){
//			task_id = 0;
//			dest_coord++;
//			if(dest_coord >= nboards) dest_coord=1;
//		}
	}
}

}




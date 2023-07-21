#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "methods.hpp"


#define N_OUTPUT_CHANNELS 4
#define N_INPUT_CHANNELS 4
#define MAX_WORDS 5120

extern "C" {
typedef ap_uint<128> word_t;

void krnl_sr(int coord, int task_id, int npackets_send, int npackets_recv, int packet_size, word_t *mem_in, word_t *mem_out, int mem_usage_test, int setup,
message_stream_t message_data_out[N_INPUT_CHANNELS], message_stream_t message_data_in[N_OUTPUT_CHANNELS]) {
#pragma HLS INTERFACE m_axi port = mem_in bundle = gmem0
#pragma HLS INTERFACE m_axi port = mem_out bundle = gmem0
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE axis port=message_data_out

	int size = 0;
	int stride = 0;
	int nword = packet_size / sizeof(word_t);
	unsigned ch_id = 0;
	//unsigned task_id = 0;
	
	//1by1 LATENCY MODE
	if(setup==0){
		int npackets = npackets_send;
		if(mem_usage_test==0){ //DRAM
			for (int i=0; i<npackets; ++i){
			//Communication library APIs
			 ch_id = i % N_INPUT_CHANNELS;
			 //task_id = i % N_INPUT_CHANNELS;
			 send(mem_in, packet_size, coord, task_id, ch_id, message_data_out); 
			 size = receive(ch_id, mem_out+stride, message_data_in);
			 stride += size/sizeof(word_t);
			}
		}
		else{ //BRAM
			//Local buffers for BRAM test
			word_t local_send[MAX_WORDS]; 
			word_t local_receive[MAX_WORDS];
			//Send buffer initialization
			for(int j=0; j<nword; j++) {
				local_send[j]=j;
			}
			
			for (int i=0; i<npackets; ++i){
					ch_id = i % N_INPUT_CHANNELS;
					//task_id = i % N_INPUT_CHANNELS;
			//Communication library APIs
					send(local_send, packet_size, coord, task_id, ch_id, message_data_out);
					size = receive(ch_id, local_receive+stride, message_data_in);
					stride += size/sizeof(word_t);
			}
		}
	}

	 if(setup==1){
                int npackets = npackets_send;
                if(mem_usage_test==0){ //DRAM
                        for (int i=0; i<npackets; ++i){
                        //Communication library APIs
                         ch_id = i % N_OUTPUT_CHANNELS;
                         //task_id = i % N_OUTPUT_CHANNELS;
                         size = receive(ch_id, mem_out+stride, message_data_in);
                         stride += size/sizeof(word_t);
			 send(mem_in, packet_size, coord, task_id, ch_id, message_data_out);
                        }
                }
                else{ //BRAM
                        //Local buffers for BRAM test
                        word_t local_send[MAX_WORDS];
                        word_t local_receive[MAX_WORDS];
                        //Send buffer initialization
                        for(int j=0; j<nword; j++) {
                                local_send[j]=j;
                        }

                        for (int i=0; i<npackets; ++i){
                                        ch_id = i % N_OUTPUT_CHANNELS;
                                        //task_id = i % N_OUTPUT_CHANNELS;
                        //Communication library APIs
                                        size = receive(ch_id, local_receive+stride, message_data_in);
                                        stride += size/sizeof(word_t);
					  								 if(size>0) send(local_send, packet_size, coord, task_id, ch_id, message_data_out);
                        }
                }
        }

	
	//ONLY SENDER MODE
	if(setup==2){
		if(mem_usage_test==0){ //DRAM
			for (int i=0; i<npackets_send; ++i){
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
				send(mem_in, packet_size, coord, task_id, ch_id, message_data_out); 
				}
		}
		else{ //BRAM
			//Local buffers for BRAM test
			word_t local_send[MAX_WORDS]; 
			//Send buffer initialization
			for(int j=0; j<nword; j++){
			 	local_send[j]=j;
			 }
			for (int i=0; i<npackets_send; ++i) {
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
				send(local_send, packet_size, coord, task_id, ch_id, message_data_out);
				}
		}
	}
	//ONLY RECEIVER MODE
	
	if(setup==3){
		if(mem_usage_test==0){ //DRAM
			for (int i=0; i<npackets_recv; ++i){
			 ch_id = i%4;
			//Communication library APIs
			 size = receive(ch_id, mem_out+stride, message_data_in);
			 stride += size/sizeof(word_t);
			}
		}
		else{ //BRAM
			//Local buffers for BRAM test 
			word_t local_receive[MAX_WORDS];
			for (int i=0; i<npackets_recv; ++i){
					ch_id = i % N_OUTPUT_CHANNELS;
			//Communication library APIs
					size = receive(ch_id, local_receive+stride, message_data_in);
					stride += size/sizeof(word_t);
			}
		}
	}
	
	//BANDWIDTH SENDER/RECEIVER MODE
	if(setup==4){
		if(mem_usage_test==0){ //DRAM
			for (int i=0; i<npackets_send; ++i){
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
				send(mem_in, packet_size, coord, task_id, ch_id, message_data_out);
			}
			for (int i=0; i<npackets_recv; ++i){
				ch_id = i % N_OUTPUT_CHANNELS;
			 	size = receive(ch_id, mem_out+stride, message_data_in);
			 	stride += size/sizeof(word_t);
			}
		}
		else{ //BRAM
			//Local buffers for BRAM test
			word_t local_send[MAX_WORDS]; 
			word_t local_receive[MAX_WORDS];
			//Send buffer initialization
			for(int j=0; j<=nword; j++){
				local_send[j]=j;
			}
			
			for (int i=0; i<npackets_send; ++i){
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
				if(local_send[nword]==nword) send(local_send, packet_size, coord, task_id, ch_id, message_data_out);
			}
			for (int i=0; i<npackets_recv; ++i){
					ch_id = i % N_OUTPUT_CHANNELS;
					size = receive(ch_id, local_receive+stride, message_data_in);
					stride += size/sizeof(word_t);
			}
		}
	}
	
	//BANDWIDTH RECEIVER/SENDER MODE
	if(setup==5){
		if(mem_usage_test==0){ //DRAM
			for (int i=0; i<npackets_recv; ++i){ 
				ch_id = i % N_OUTPUT_CHANNELS;
			 	size = receive(ch_id, mem_out+stride, message_data_in);
			 	stride += size/sizeof(word_t);
			}
			for (int i=0; i<npackets_send; ++i){
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
			 	send(mem_in, packet_size, coord, task_id, ch_id, message_data_out);
			}
		}
		else{ //BRAM
			//Local buffers for BRAM test
			word_t local_send[MAX_WORDS]; 
			word_t local_receive[MAX_WORDS];
			//Send buffer initialization
			for(int j=0; j<nword; j++){
				local_send[j]=j;
			}
			for (int i=0; i<npackets_recv; ++i){
					ch_id = i % N_OUTPUT_CHANNELS;
					size = receive(ch_id, local_receive+stride, message_data_in);
					stride += size/sizeof(word_t);
			}
			for (int i=0; i<npackets_send; ++i){
				ch_id = i % N_INPUT_CHANNELS;
				//task_id = i % N_INPUT_CHANNELS;
				send(local_send, packet_size, coord, task_id, ch_id, message_data_out);
				}
		}
	}
	
	
	
	
	
}
}

#include <stdint.h>
#include <hls_stream.h>
#include <hls_print.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_INPUT_CHANNELS 1
#define N_OUTPUT_CHANNELS 1
#define MAX_WORDS 5120

#define N_BINS 16
#define BINWIDTH 2.54*4/N_BINS

#define BITWIDTH_BIN 128/N_BINS

constexpr int partitions = 6;

constexpr int NWORDS=32<<20;

extern "C" {
typedef ap_uint<128> word_t;

void krnl_sr(int npackets_recv, int packet_size, 
		word_t *mem_out_0,
		word_t *mem_out_1,
		word_t *mem_out_2,
		word_t *mem_out_3,
		word_t *mem_out_4,
		word_t *mem_out_5,
	       	int mem_usage_test, int setup, bool eth, float threshold,
//#if N_OUTPUT_CHANNELS>0
	//	message_stream_t message_data_out[N_OUTPUT_CHANNELS], 
//#endif
		message_stream_t message_data_in[N_INPUT_CHANNELS]) {
#pragma HLS INTERFACE m_axi port=mem_out_0  depth=NWORDS bundle=gmem0
#pragma HLS INTERFACE m_axi port=mem_out_1  depth=NWORDS bundle=gmem1
#pragma HLS INTERFACE m_axi port=mem_out_2  depth=NWORDS bundle=gmem2
#pragma HLS INTERFACE m_axi port=mem_out_3  depth=NWORDS bundle=gmem3
#pragma HLS INTERFACE m_axi port=mem_out_4  depth=NWORDS bundle=gmem4
#pragma HLS INTERFACE m_axi port=mem_out_5  depth=NWORDS bundle=gmem5
//#pragma HLS ARRAY_PARTITION block variable=mem_out factor=partitions

#pragma HLS INTERFACE axis port=message_data_in
//#pragma HLS INTERFACE axis port=message_data_out

	int size = 0;
	int stride[N_INPUT_CHANNELS];
	#pragma HLS ARRAY_PARTITION block variable=stride factor=partitions	
	word_t mem_pointer[N_INPUT_CHANNELS];
	#pragma HLS ARRAY_PARTITION block variable=mem_pointer factor=partitions
	//#pragma HLS array_reshape variable=stride complete dim=0
	for(int i=0; i<N_INPUT_CHANNELS; i++) stride[i] = 0;
       /*	mem_pointer[0]=mem_out_0;
	mem_pointer[1]=mem_out_1;
	mem_pointer[2]=mem_out_2;
	mem_pointer[3]=mem_out_3;
	mem_pointer[4]=mem_out_4;
	mem_pointer[5]=mem_out_5;
	*/
	int nword = (packet_size & (sizeof(word_t)-1)) ? (packet_size/sizeof(word_t)+1) : packet_size/sizeof(word_t);
	unsigned ch_id = 0;
	
	if(setup==3){
		if(mem_usage_test==0){ //DRAM
			for(ch_id=0; ch_id<N_INPUT_CHANNELS; ch_id++){
			#pragma HLS unroll
					for (int i=0; i<npackets_recv; ++i){
					#pragma HLS pipeline
					size = receive(ch_id, &mem_pointer[ch_id]+stride[ch_id], message_data_in);
			 		stride[ch_id] += nword;
				}
			}
		}
		else{ //BRAM
			word_t local_receive[N_INPUT_CHANNELS];
			#pragma HLS ARRAY_PARTITION block variable=local_receive factor=partitions
			//#pragma HLS array_reshape variable=stride complete dim=0
			for(ch_id=0; ch_id<N_INPUT_CHANNELS; ch_id++){	
			#pragma HLS unroll
					for (int i=0; i<npackets_recv; ++i){
					#pragma HLS pipeline
					size = receive(ch_id, &local_receive[ch_id], message_data_in);
				}
			}
		}
	}

	
	if(setup==69){
		if(mem_usage_test==0){ //DRAM
			for(int i=0; i<npackets_recv; ++i){
			#pragma HLS pipeline
				ap_uint<BITWIDTH_BIN> time_histo[N_BINS];
				#pragma HLS array_partition variable=time_histo type=complete dim=0	
				word_t histo_sum_arr[N_INPUT_CHANNELS];
				#pragma HLS array_partition variable=histo_sum_arr type=complete dim=0

				word_t result = 0;	
				for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
				#pragma HLS unroll
					word_t word = message_data_in[ch].read();
					histo_sum_arr[ch] = word;
				}
				word_t histo_sum = histo_sum_arr[0]; //+ histo_sum_arr[1] + histo_sum_arr[2] + histo_sum_arr[3] + histo_sum_arr[4] + histo_sum_arr[5];


				for(unsigned bin=0; bin<N_BINS; bin++){
				#pragma HLS unroll
					time_histo[bin].range() = histo_sum.range(BITWIDTH_BIN*(bin+1)-1,BITWIDTH_BIN*bin);
					if(time_histo[bin]>threshold) result++;
				}

				mem_out_0[i] = result; 
			}
	
		}
		else{ //BRAM
			for(int i=0; i<npackets_recv; ++i){
			#pragma HLS pipeline
				ap_uint<BITWIDTH_BIN> time_histo[N_BINS];
				#pragma HLS array_partition variable=time_histo type=complete dim=0
				word_t histo_sum_arr[N_INPUT_CHANNELS];
				#pragma HLS array_partition variable=histo_sum_arr type=complete dim=0

				word_t result = 0;	
				for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
				#pragma HLS unroll
					word_t word = message_data_in[ch].read();
					histo_sum_arr[ch] = word;
				}
				word_t histo_sum = histo_sum_arr[0]; //+ histo_sum_arr[1] + histo_sum_arr[2] + histo_sum_arr[3] + histo_sum_arr[4] + histo_sum_arr[5];


				for(unsigned bin=0; bin<N_BINS; bin++){
				#pragma HLS unroll
					time_histo[bin].range() = histo_sum.range(BITWIDTH_BIN*(bin+1)-1,BITWIDTH_BIN*bin);
					hls::print("bin %d",(int)bin);
					hls::print("time histo %d\n",(int)time_histo[bin]);
					if(time_histo[bin]>threshold) result++;
				}
			}
		}
	}

	if(setup==96){
		for(int i=0; i<npackets_recv; ++i){
			#pragma HLS pipeline
				for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
				#pragma HLS unroll
					word_t word = message_data_in[ch].read();
					//word_t *buff = (&mem_pointer[ch]+stride[ch]);
					mem_out_0[i] = word;
				}
		}
	}

	if(setup==99){
flush_loop: for(unsigned ch_id=0; ch_id<N_OUTPUT_CHANNELS; ch_id++){
			#pragma HLS unroll
	flush_loop_pipe: for(unsigned j=0; j<npackets_recv;j++){
			#pragma HLS pipeline

			word_t flush[N_INPUT_CHANNELS];
			//#pragma HLS array_reshape variable=flush complete dim=0
			#pragma HLS ARRAY_PARTITION block variable=flush factor=partitions

					flush[ch_id] = message_data_in[ch_id].read();
			}
		}
	
	}
	
/*#if N_OUTPUT_CHANNELS>0
	if(setup<0){
out_loop_pipe: for(unsigned j=0; j<100;j++){
			#pragma HLS pipeline

			word_t flush[N_OUTPUT_CHANNELS];
			#pragma HLS array_reshape variable=flush complete dim=0

		out_loop: for(unsigned ch_id=0; ch_id<N_OUTPUT_CHANNELS; ch_id++){
				#pragma HLS unroll
					message_data_out[ch_id].write(0xfffff);
			}
		}
	}	
#endif
*/

}
}



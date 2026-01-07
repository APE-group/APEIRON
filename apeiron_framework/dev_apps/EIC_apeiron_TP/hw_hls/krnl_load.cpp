#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_OUTPUT_CHANNELS 40

extern "C"{
	 void ddr_loop(unsigned nevent, float *mem_in, hls::stream<ap_fixed<16,8>> output_channels_PDUs[40]){
	  ap_fixed<16,8> out_array[N_OUTPUT_CHANNELS];
#pragma HLS array partition  variable=out_array type=complete
//ddr_loop: for(int n=0; n<nevents; ++n){
  //                #pragma HLS LOOP_TRIPCOUNT min=1
    //              #pragma HLS pipeline II=1
                  ddr_identity_instantiation: for(int i=0; i<N_OUTPUT_CHANNELS; ++i){
                          #pragma HLS unroll
                                  out_array[i] = mem_in[i+40*nevent];
                                  //if(i%1==0) out_array[i].data = 1;
                          }
                  ddr_output_streaming: for(int k=0; k<N_OUTPUT_CHANNELS; ++k){
                          #pragma HLS unroll
                                  output_channels_PDUs[k].write(out_array[k]);
                          }
                  }
         // }

}

extern "C"{
	 void bram_loop(unsigned nevents, hls::stream<ap_fixed<16,8>> output_channels_PDUs[40]){
	 ap_fixed<16,8> out_array[N_OUTPUT_CHANNELS];
#pragma HLS array partition  variable=out_array type=complete
//bram_loop: for(int n=0; n<nevents; ++n){
  //                #pragma HLS LOOP_TRIPCOUNT min=1
    //              #pragma HLS pipeline II=1
                  bram_identity_instantiation: for(int i=0; i<N_OUTPUT_CHANNELS; ++i){
                          #pragma HLS unroll
                                  //out_array[i].data = mem_in[i+42*n];
                                  out_array[i] = 1;
                          }
                  bram_output_streaming: for(int k=0; k<N_OUTPUT_CHANNELS; ++k){
                          #pragma HLS unroll
                                  output_channels_PDUs[k].write(out_array[k]);
                          }
                  }
          //}

}


extern "C" {
typedef ap_uint<128> word_t;

void krnl_load(unsigned nevents, 
		float *mem_in_0,
		float *mem_in_1,
		float *mem_in_2,
		float *mem_in_3,
		float *mem_in_4,
		float *mem_in_5,	
		bool ddr, 
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_0[N_OUTPUT_CHANNELS],
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_1[N_OUTPUT_CHANNELS],
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_2[N_OUTPUT_CHANNELS],
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_3[N_OUTPUT_CHANNELS],
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_4[N_OUTPUT_CHANNELS],
		hls::stream<ap_fixed<16,8>> output_channels_PDUs_5[N_OUTPUT_CHANNELS])
 {
#pragma HLS INTERFACE axis port=output_channels_PDUs_0
#pragma HLS INTERFACE axis port=output_channels_PDUs_1
#pragma HLS INTERFACE axis port=output_channels_PDUs_2
#pragma HLS INTERFACE axis port=output_channels_PDUs_3
#pragma HLS INTERFACE axis port=output_channels_PDUs_4
#pragma HLS INTERFACE axis port=output_channels_PDUs_5
#pragma HLS INTERFACE m_axi port = mem_in_0 bundle = gmem0
#pragma HLS INTERFACE m_axi port = mem_in_1 bundle = gmem1
#pragma HLS INTERFACE m_axi port = mem_in_2 bundle = gmem2
#pragma HLS INTERFACE m_axi port = mem_in_3 bundle = gmem3
#pragma HLS INTERFACE m_axi port = mem_in_4 bundle = gmem4
#pragma HLS INTERFACE m_axi port = mem_in_5 bundle = gmem5

#pragma HLS DATAFLOW

	if(ddr){
		for(int ev=0; ev<nevents; ev++){
		#pragma HLS pipeline
			for(int i = 0; i<6; i++){
			#pragma HLS unroll
				if(i==0) ddr_loop(ev, mem_in_0, output_channels_PDUs_0);
				if(i==1) ddr_loop(ev, mem_in_1, output_channels_PDUs_1);
				if(i==2) ddr_loop(ev, mem_in_2, output_channels_PDUs_2);
				if(i==3) ddr_loop(ev, mem_in_3, output_channels_PDUs_3);
				if(i==4) ddr_loop(ev, mem_in_4, output_channels_PDUs_4);
				if(i==5) ddr_loop(ev, mem_in_5, output_channels_PDUs_5);
			}
		}
 	}

	else{
		for(int ev=0; ev<nevents; ev++){
		#pragma HLS pipeline
			for(int i = 0; i<6; i++){
			#pragma HLS unroll
				if(i==0) bram_loop(nevents, output_channels_PDUs_0);
				if(i==1) bram_loop(nevents, output_channels_PDUs_1);
				if(i==2) bram_loop(nevents, output_channels_PDUs_2);
				if(i==3) bram_loop(nevents, output_channels_PDUs_3);
				if(i==4) bram_loop(nevents, output_channels_PDUs_4);
				if(i==5) bram_loop(nevents, output_channels_PDUs_5);
			}
		}
 	}
	
}
}

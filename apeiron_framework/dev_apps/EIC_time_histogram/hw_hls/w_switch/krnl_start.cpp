#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"
#include <hls_print.h>

#define N_DAMS 15
#define N_SUBSECTORS 3
#define N_WORDS 2

//typedef ap_uint<32> RDO_word_t;
typedef ap_axiu<32,0,0,0> RDO_word_t;
typedef ap_uint<128> APE_word_t;


extern "C" {
void krnl_start(hls::stream<bool> start_signal_out[N_DAMS])
 {
#pragma HLS INTERFACE axis port=start_signal_out

	for(unsigned dam=0; dam<N_DAMS;dam++){
	#pragma HLS UNROLL
		start_signal_out[dam].write(1);
	}
	
}
}

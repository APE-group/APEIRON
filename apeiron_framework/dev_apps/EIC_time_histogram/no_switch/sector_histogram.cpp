#include <stdint.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_INPUT_CHANNELS 5
#define N_OUTPUT_CHANNELS 1

typedef ap_uint<32> RDO_word_t;
typedef ap_uint<128> APE_word_t;

void APE_reading(hls::stream<APE_word_t> histo_sum[N_INPUT_CHANNELS], hls::stream<APE_word_t> message_data_in[N_INPUT_CHANNELS]){
#pragma HLS inline off

	write_ch_loop: for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
		#pragma HLS unroll
			APE_word_t input_word = message_data_in[ch].read();
			histo_sum[ch].write(input_word); 
	}
}
//}

void APE_sending(hls::stream<APE_word_t> fifo_histo_sum[N_INPUT_CHANNELS], hls::stream<APE_word_t> &message_data_out){
#pragma HLS inline off

		APE_word_t histo_sum[N_INPUT_CHANNELS];
#pragma HLS array_partition variable=histo_sum type=complete
write_ch_loop: for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
		#pragma HLS unroll
			histo_sum[ch] = fifo_histo_sum[ch].read();
		}

		APE_word_t apeword = histo_sum[0] + histo_sum[1] + histo_sum[2] + histo_sum[3] + histo_sum[4];
		message_data_out.write(apeword);	
}
//}

extern "C" {

void sector_histogram(unsigned nevents, 
		hls::stream<APE_word_t> message_data_in[N_INPUT_CHANNELS],
		hls::stream<APE_word_t> message_data_out[N_OUTPUT_CHANNELS])
 {
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=message_data_in
#pragma HLS INTERFACE axis port=message_data_out


	 hls::stream<APE_word_t> fifo_histo_sum[N_INPUT_CHANNELS];	
    	#pragma HLS STREAM variable=fifo_histo_sum depth=100
	//#pragma HLS array_partition variable=histo_sum type=complete dim=0

//event_loop: for(unsigned ev=0; ev<nevents; ev++){
#pragma HLS dataflow
	APE_reading(fifo_histo_sum,message_data_in);	
	APE_sending(fifo_histo_sum, message_data_out[0]);

//	}

}
}

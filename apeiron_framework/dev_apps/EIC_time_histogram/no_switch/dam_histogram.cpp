#include <stdint.h>
#include <hls_stream.h>
#include <hls_print.h>
#include "ap_axi_sdata.h"
#include "ape_hls/hapecom.hpp"


#define N_INPUT_CHANNELS 42
#define N_OUTPUT_CHANNELS 1

#define N_BINS 16
#define TIME_SCALE 100  //fixed_point scale
#define BINWIDTH_INT 63 //((int)(2.54*4*TIME_SCALE/N_BINS))  // scaled

#define BITWIDTH_BIN 8  //128/N_BINS

#define FINE_MULT 3
#define COARSE_MULT 254

typedef ap_uint<32> RDO_word_t;
typedef ap_uint<128> APE_word_t;
extern "C" void fifo_reading(hls::stream<ap_uint<BITWIDTH_BIN*N_BINS>> &fifo_time_histo, hls::stream<RDO_word_t> &dam_channel_PDU){
	//bool event_done = false;
	ap_uint<BITWIDTH_BIN*N_BINS> time_histo=0;

	word_loop: while(true){
	#pragma HLS pipeline
		//RDO_word_t input_word = dam_channels_PDUs[ch].read();
		RDO_word_t input_word = dam_channel_PDU.read();
		if(input_word == 0xffffffff){
		       	//event_done[ch] = true;	
		        //event_done = true;	
			//fifo_time_histo[ch].write(time_histo[ch]);
			fifo_time_histo.write(time_histo);
			break;
		}
		else{
			ap_uint<2> tdc_id = input_word.range(25,24);
			ap_uint<3> pix_id = input_word.range(28,26);
			ap_uint<3> col_id = input_word.range(31,29);

			unsigned sipm_id = tdc_id*64 + (col_id*8 + pix_id);

		//float coarse_counter = input_word.range(23,9);
		//float fine_counter = input_word.range(8,0);

		//float word_time = coarse_counter * 2.54 + fine_counter * 0.030;
			ap_uint<15> coarse_counter = input_word.range(23,9);
			ap_uint<9> fine_counter = input_word.range(8,0);

			ap_uint<24> word_time = coarse_counter * COARSE_MULT + fine_counter * FINE_MULT;
			//hls::print("DAM word_time %d \n", (int)word_time);
			unsigned bit_up = (unsigned)((unsigned)word_time/BINWIDTH_INT)*BITWIDTH_BIN;	
			hls::print("bit up %d \n", (int)bit_up);
			time_histo += (ap_uint<BITWIDTH_BIN*N_BINS>(1) << bit_up);
			//hls::print("time_histo[ch] %d \n", (int)time_histo);

		}

	}
}

extern "C" void APE_sending(hls::stream<ap_uint<BITWIDTH_BIN*N_BINS>> fifo_time_histo[N_INPUT_CHANNELS], hls::stream<APE_word_t> &message_data_out){
//#pragma HLS inline off
	APE_word_t apeword_arr[N_INPUT_CHANNELS];
#pragma HLS array_partition variable= apeword_arr type=complete dim=0
ch_loop: for(unsigned ch=0; ch<N_INPUT_CHANNELS; ch++){
		#pragma HLS unroll
			apeword_arr[ch] = fifo_time_histo[ch].read();
		}
		APE_word_t apeword = apeword_arr[0] + apeword_arr[1] + apeword_arr[2] + apeword_arr[3] + apeword_arr[4] + apeword_arr[5] +
				     apeword_arr[6] + apeword_arr[7] + apeword_arr[8] + apeword_arr[9] + apeword_arr[10] + apeword_arr[11] +
				     apeword_arr[12] + apeword_arr[13] + apeword_arr[14] + apeword_arr[15] + apeword_arr[16] + apeword_arr[17] +
				     apeword_arr[18] + apeword_arr[19] + apeword_arr[20] + apeword_arr[21] + apeword_arr[22] + apeword_arr[23] +
				     apeword_arr[24] + apeword_arr[25] + apeword_arr[26] + apeword_arr[27] + apeword_arr[28] + apeword_arr[29] +
				     apeword_arr[30] + apeword_arr[31] + apeword_arr[32] + apeword_arr[33] + apeword_arr[34] + apeword_arr[35] +
				     apeword_arr[36] + apeword_arr[37] + apeword_arr[38] + apeword_arr[39] + apeword_arr[40] + apeword_arr[41];
		message_data_out.write(apeword);	
}
//}

extern "C" {

void dam_histogram(hls::stream<RDO_word_t> dam_channels_PDUs[N_INPUT_CHANNELS],
		hls::stream<APE_word_t> message_data_out[N_OUTPUT_CHANNELS])
 {
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=dam_channels_PDUs
#pragma HLS INTERFACE axis port=message_data_out


	hls::stream<ap_uint<BITWIDTH_BIN*N_BINS>> fifo_time_histo[N_INPUT_CHANNELS];
	#pragma HLS STREAM variable=fifo_time_histo depth=16

#pragma HLS dataflow
	//**** Reading incoming data from RDO stream *****//
	for (int ch = 0; ch < N_INPUT_CHANNELS; ch++) {
	#pragma HLS unroll
    		fifo_reading(fifo_time_histo[ch], dam_channels_PDUs[ch]);
	}
	//**** Histogram packing into APEword *****//
	APE_sending(fifo_time_histo, message_data_out[0]);

}
}

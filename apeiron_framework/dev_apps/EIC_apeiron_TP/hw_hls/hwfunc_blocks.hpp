
#define N_INPUT_DAMs 40
#define N_INPUT_GRID 42

extern "C" void preprocessing_block( 
		hls::stream<ap_fixed<16,8>> input_channels[N_INPUT_DAMs], 
		hls::stream<input_t> &grid_out ) 
{

	#pragma HLS pipeline rewind
	input_t input_grid_array;//[N_INPUT_PDUs];
	//#pragma HLS ARRAY_RESHAPE variable=input_grid_array complete dim=0
    	#pragma HLS array partition variable=input_grid_array complete 

read_input_channels_loop:
	for(int ch_i=0; ch_i<N_INPUT_DAMs; ++ch_i){
	#pragma HLS unroll
		input_grid_array[ch_i] = input_channels[ch_i].read();//.data;
	}

	grid_out.write(input_grid_array);
}

#define N_OUTPUT_FEATUREs 4

extern "C" void feature_sender(int coord, unsigned ch_id, unsigned task_id, hls::stream<result_t> &feature_out, message_stream_t &message_data_out)
{

#pragma HLS pipeline rewind
	auto result = feature_out.read();
	word_t output_word_feature[N_OUTPUT_FEATUREs];
	word_t tmp[N_OUTPUT_FEATUREs];
    	word_t data_out;
	data_out=0;
#pragma HLS array partition variable=tmp complete
	
read_output_features_loop:
	for(unsigned i=0; i<N_OUTPUT_FEATUREs; ++i){
//#pragma HLS pipeline
#pragma HLS unroll
		tmp[i].range() = result[i].range();
		data_out.range(16*(i+1),16*i) = tmp[i].range();//.data;
		//data_out &= tmp[i] << 16*i;
	}
    size_t size = sizeof(word_t);
    send(&data_out, size, coord, task_id, ch_id, message_data_out);
    //message_data_out.write(data_out);
}


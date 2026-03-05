
// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include "host.h"
#define N_SUBSECTORS 5
#define N_PDUS 42

int main(int argc, char** argv)
{
	char *bitstream = NULL;
	pthread_t thread_registers; //Thread definition
	unsigned npackets = 0;
	unsigned local_coord = 0;

	while (1) {
		int option_index;
		int c = getopt_long(argc, argv, "b:n:c:h", test_options, &option_index);

		if (c == -1) break;

		switch (c) {
			case 'b':
				bitstream = optarg;
				break;
			case 'n':
				npackets = atoi(optarg);
				break;
			case 'c':
				local_coord = atoi(optarg);
				break;
			case 'h':
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (!bitstream) {
		std::cerr << "missing bitstream option\n";
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	
	xrt::device device(0);

	std::cout << "Device name: " << device.get_info<xrt::info::device::name>() << std::endl;
	std::cout << "Device bdf: " << device.get_info<xrt::info::device::bdf>() << std::endl;
	std::cout << "Device max freq: " << device.get_info<xrt::info::device::max_clock_frequency_mhz>() << std::endl;
	std::cout << "Device m2m: " << device.get_info<xrt::info::device::m2m>() << std::endl;
	std::cout << "Device nodma: " << device.get_info<xrt::info::device::nodma>() << std::endl;
	std::cout << "Device kdma: " << device.get_info<xrt::info::device::kdma>() << "\n\n";

	std::printf("Bitstream_loading \n");
	xrt::uuid uuid = device.load_xclbin(bitstream);
	std::printf("Bitstream_loaded \n");

	xrt::kernel ksr(device, uuid, "krnl_sr:{krnl_sr_1}");
	//xrt::kernel kpipe(device, uuid, "krnl_pipe:{krnl_pipe_1}");
	xrt::kernel kload(device, uuid, "krnl_load:{krnl_load_1}");
	//xrt::kernel ksector_histogram1(device, uuid, "sector_histogram:{sector_histogram_1}");
	std::ofstream outfile("data.dat");

	int n_events = npackets;
	int n_inputs = 40;
	int n_outputs = 4;
	bool ddr=true;
	auto inbuf_size = n_events*n_inputs*sizeof(unsigned int);
	auto outbuf_size = n_events*sizeof(stream_data_t);
	std::vector<unsigned int> input_events[N_SUBSECTORS];
	std::vector<float> output_events[6];
	std::vector<float> y_events;
	std::vector<unsigned int> event_word_count[N_SUBSECTORS];
	if(n_events<=180000){
		for(int j=0; j<N_SUBSECTORS; j++){
			event_word_count[j].push_back(0);
			for(int k=0; k<N_PDUS; k++){
				unsigned inp_count = 0;		
			
				std::string str = "/apotto/home1/media/ePIC/signal_input_histogram/input_subsector_" + std::to_string(j) + "/input_PDU_" + std::to_string(k) + ".txt";
				//std::string str = "/apotto/home1/media/ePIC/noise_input_histogram/input_subsector_" + std::to_string(j) + "/input_PDU_" + std::to_string(k) + ".txt";
				std::printf("Input buffer knrl_load \n");
				std::printf(str.c_str());
				std::printf("\n");
				inp_count += read_input_file(str.c_str(), input_events[j], n_events);
				event_word_count[j].push_back(inp_count);
				ddr=true;


				std::printf("Input buffer Subsector %d PDU %d: size=%lu event_word_counted=%lu\n", j, k, input_events[j].size(), inp_count);
				}
			}
		
	}
	else{
	inbuf_size = 2000;
	outbuf_size = 2000;
		ddr=false;
	}

	/*for(int i=0; i<input_events[1].size(); ++i){
		std::printf("input:0x%.8lx \n",input_events[1].at(i));
	}
	for(int i=0; i<event_word_count[1].size(); ++i){
		std::printf("offset:%u \n",event_word_count[1].at(i));
	}*/

	xrt::bo send_buffer[6];
	xrt::bo send_offset_buffer[6];
	unsigned int* send_buffer_map[6];
	unsigned int* send_offset_buffer_map[6];
	xrt::bo recv_buffer[6];
	stream_data_t* recv_buffer_map[6];
	for(int subs=0; subs<5; subs++){
		//std::printf("Input buffer (FPGA mapping) subsector n°%d\n",subs);
		send_buffer[subs] = xrt::bo(device, input_events[subs].size()*sizeof(unsigned int), kload.group_id(subs+1));
		send_offset_buffer[subs] = xrt::bo(device, event_word_count[subs].size()*sizeof(unsigned int), kload.group_id(subs+5+1));
		send_buffer_map[subs] = send_buffer[subs].map<unsigned int*>();
		send_offset_buffer_map[subs] = send_offset_buffer[subs].map<unsigned int*>();
		if(n_events<=50000){
 			memset(send_buffer_map[subs], 0, input_events[subs].size()*sizeof(unsigned int));
 			memset(send_offset_buffer_map[subs], 0, event_word_count[subs].size()*sizeof(unsigned int));
			std::printf("SEND BUFFER\n");
			for (int i=0; i < input_events[subs].size(); ++i) {
  				send_buffer_map[subs][i]=input_events[subs].at(i);
				//std::printf("%u : 0x%.8lx \n", i, send_buffer_map[subs][i]);
  			}
			std::printf("SEND OFFSET BUFFER\n");
			for (int j=0; j < event_word_count[subs].size(); ++j) {
  				send_offset_buffer_map[subs][j]=event_word_count[subs].at(j);
				//std::printf("%u : 0x%.8lx \n", j, send_offset_buffer_map[subs][j]);
  			}

			send_buffer[subs].sync(XCL_BO_SYNC_BO_TO_DEVICE);
			send_offset_buffer[subs].sync(XCL_BO_SYNC_BO_TO_DEVICE);
		}
	}
	for(int sec=0; sec<6; sec++){		
		std::printf("Output buffer (FPGA mapping) sector n°%d arg=%d\n",sec,sec+2);
		recv_buffer[sec] = xrt::bo(device, outbuf_size, ksr.group_id(sec+2));
		recv_buffer_map[sec] = recv_buffer[sec].map<stream_data_t*>();
		if(n_events<=100000) memset(recv_buffer_map[sec],0,outbuf_size);
		std::printf("Output buffer (FPGA mapping) sector n°%d END\n",sec);
	}
	

	std::chrono::duration<double,std::nano> deltat;
	std::printf("Starting kernel_receiver ...:\n");
	int status = 96;
	sleep(1);
        bool ksr_bram = false;
	bool kload_ddr = true;

	//std::printf("Starting ksector_histogram ...\n");
	//xrt::run ksector_histogram1_run = ksector_histogram1(n_events);

	//xrt::run kpipe_run = kpipe(n_events);
	//n_events = n_events*1000;
	xrt::run ksr_run = ksr(n_events,
		        16, 
			recv_buffer[0], recv_buffer[1], recv_buffer[2],
			recv_buffer[3], recv_buffer[4], recv_buffer[5],
		       	ksr_bram, status, false, 50);
		       	//!ddr, status, false);
	std::printf("Starting kernel_loader ...\n");
	auto tstart = std::chrono::high_resolution_clock::now();
	xrt::run kload_run = kload(n_events, 
			send_buffer[0], send_buffer[1], send_buffer[2],
		       	send_buffer[3], send_buffer[4], 
			send_offset_buffer[0], send_offset_buffer[1], send_offset_buffer[2],
		       	send_offset_buffer[3], send_offset_buffer[4], 
			//ddr);
			kload_ddr);

	//auto tstart = std::chrono::high_resolution_clock::now();
	std::printf("waiting  kernel_loader to complete ...\n");
	kload_run.wait();	
	//std::printf("waiting for ksector_histogram1 to complete ...\n");
	//ksector_histogram1_run.wait();	
	std::printf("waiting for kernel_receiver to complete ...\n");
	ksr_run.wait();
	//kpipe_run.wait();

	std::printf("end of process\n");

	auto tend = std::chrono::high_resolution_clock::now();
	if(n_events<=180000){
		if(!ksr_bram) for(int sec=0; sec<6; sec++) recv_buffer[sec].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	}
	deltat = tend - tstart;

	unsigned event_count[16]={0};
	unsigned signal_predict = 0;
	unsigned noise_predict =0;
	float threshold = 50;
	if(n_events<=180000 && status==96 && !ksr_bram){
		long unsigned int count[16] = {0};
		unsigned predict;
		for (int i=0; (i < n_events) & (n_events<=180000); ++i) { 	
			//std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
			//recv_buffer_map[0][i].high0, recv_buffer_map[0][i].low0);
			std::printf("BIN 0: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*0) & 0xFF);
			std::printf("BIN 1: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*1) & 0xFF);
			std::printf("BIN 2: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*2) & 0xFF);
			std::printf("BIN 3: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*3) & 0xFF);
			std::printf("BIN 4: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*4) & 0xFF);
			std::printf("BIN 5: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*5) & 0xFF);
			std::printf("BIN 6: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*6) & 0xFF);
			std::printf("BIN 7: %lu\n", 	(recv_buffer_map[0][i].low0 >> 8*7) & 0xFF);
			std::printf("BIN 8: %lu\n", 	(recv_buffer_map[0][i].high0 >> 8*0) & 0xFF);
			std::printf("BIN 9: %lu\n", 	(recv_buffer_map[0][i].high0 >> 8*1) & 0xFF);
			std::printf("BIN 10: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*2) & 0xFF);
			std::printf("BIN 11: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*3) & 0xFF);
			std::printf("BIN 12: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*4) & 0xFF);
			std::printf("BIN 13: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*5) & 0xFF);
			std::printf("BIN 14: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*6) & 0xFF);
			std::printf("BIN 15: %lu\n",	(recv_buffer_map[0][i].high0 >> 8*7) & 0xFF);
			
			count[0] = (recv_buffer_map[0][i].low0 >> 8*0) & 0xFF;
			count[1] = (recv_buffer_map[0][i].low0 >> 8*1) & 0xFF;
			count[2] = (recv_buffer_map[0][i].low0 >> 8*2) & 0xFF;
			count[3] = (recv_buffer_map[0][i].low0 >> 8*3) & 0xFF;
			count[4] = (recv_buffer_map[0][i].low0 >> 8*4) & 0xFF;
			count[5] = (recv_buffer_map[0][i].low0 >> 8*5) & 0xFF;
			count[6] = (recv_buffer_map[0][i].low0 >> 8*6) & 0xFF;
			count[7] = (recv_buffer_map[0][i].low0 >> 8*7) & 0xFF;
			count[8] = (recv_buffer_map[0][i].high0 >> 8*0) & 0xFF;
			count[9] = (recv_buffer_map[0][i].high0 >> 8*1) & 0xFF;
			count[10] = (recv_buffer_map[0][i].high0 >> 8*2) & 0xFF;
			count[11] = (recv_buffer_map[0][i].high0 >> 8*3) & 0xFF;
			count[12] = (recv_buffer_map[0][i].high0 >> 8*4) & 0xFF;
			count[13] = (recv_buffer_map[0][i].high0 >> 8*5) & 0xFF;
			count[14] = (recv_buffer_map[0][i].high0 >> 8*6) & 0xFF;
			count[15] = (recv_buffer_map[0][i].high0 >> 8*7) & 0xFF;
			for(int k=0; k<16; k++){
				event_count[k] += count[k];
				if(count[k]>threshold) predict=1;
			}
			if(predict==1) signal_predict++;
			else noise_predict++;	

		}
		std::printf("\n /**********COUNTING HITS**************/ \n"); 
		for(int j=0; j<16;j++){
			std::printf("BIN %d: %lu ==> media=%f\n", j, event_count[j], (float)event_count[j]/n_events);
		}
	}

	if(n_events<=180000 && status==69 && !ksr_bram){
		for (int i=0; (i < n_events) & (n_events<=180000); ++i) { 	
			//std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
			//	recv_buffer_map[0][i].high0, recv_buffer_map[0][i].low0);
			if(recv_buffer_map[0][i].low0 == 0) noise_predict++;
			else signal_predict++;
		}
	}

	std::printf("\n Noise events predicted: %u (%f %) \t Signal events predicted: %u (%f %) \n",noise_predict, (float)noise_predict/n_events*100, signal_predict, (float)signal_predict/n_events*100);


	std::printf("FINISH \n");
	std::printf("Time elapsed: %lf ns, Throughput: %lf MHz\n", deltat.count(), n_events/(deltat.count()/1000));

	
	return 0;
}

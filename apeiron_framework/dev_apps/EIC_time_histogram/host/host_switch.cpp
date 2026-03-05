
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
	xrt::ip kswitch(device, uuid, "TextaRossa_switch");
	kswitch_thread = kswitch; //ip handle for parallel thread 
	std::printf("Switch kernel \n");

	xrt::kernel kstart(device, uuid, "krnl_start:{krnl_start_1}");
	xrt::kernel kresult_r_GTU_s(device, uuid, "result_recv_GTU_send:{result_recv_GTU_send_1}");
	xrt::kernel kload[15];
	kload[0] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_1}");
	kload[1] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_2}");
	kload[2] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_3}");
	kload[3] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_4}");
	kload[4] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_5}");
	kload[5] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_6}");
	kload[6] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_7}");
	kload[7] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_8}");
	kload[8] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_9}");
	kload[9] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_10}");
	kload[10] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_11}");
	kload[11] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_12}");
	kload[12] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_13}");
	kload[13] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_14}");
	kload[14] = xrt::kernel(device, uuid, "krnl_load:{krnl_load_15}");
	
	
	//xrt::kernel ksector_histogram1(device, uuid, "sector_histogram:{sector_histogram_1}");
	std::ofstream outfile("data.dat");

	int n_events = npackets;
	int n_inputs = 40;
	int n_outputs = 4;
	bool ddr=true;
	auto inbuf_size = n_events*n_inputs*sizeof(unsigned int);
	auto outbuf_size = n_events*3*sizeof(stream_data_t);
	std::vector<unsigned int> input_events[N_SUBSECTORS];
	std::vector<float> output_events[6];
	std::vector<float> y_events;
	std::vector<unsigned int> event_word_count[N_SUBSECTORS];
	if(n_events<=5000){
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


				std::printf("Input buffer Subsector %d PDU %d: size=%lu event_word_counted=%u\n", j, k, input_events[j].size(), inp_count);
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

	xrt::bo send_buffer[3][5];
	xrt::bo send_offset_buffer[3][5];
	unsigned int* send_buffer_map[3][5];
	unsigned int* send_offset_buffer_map[3][5];
	xrt::bo recv_buffer;
	stream_data_t* recv_buffer_map;
	for(int sec=0; sec<3; sec++){
		for(int subs=0; subs<5; subs++){
		//std::printf("Input buffer (FPGA mapping) subsector n°%d\n",subs);
			send_buffer[sec][subs] = xrt::bo(device, input_events[subs].size()*sizeof(unsigned int), kload[sec*5+subs].group_id(1));
			send_offset_buffer[sec][subs] = xrt::bo(device, event_word_count[subs].size()*sizeof(unsigned int), kload[sec*5+subs].group_id(2));
			send_buffer_map[sec][subs] = send_buffer[sec][subs].map<unsigned int*>();
			send_offset_buffer_map[sec][subs] = send_offset_buffer[sec][subs].map<unsigned int*>();
			if(n_events<=50000){
 				memset(send_buffer_map[sec][subs], 0, input_events[subs].size()*sizeof(unsigned int));
 				memset(send_offset_buffer_map[sec][subs], 0, event_word_count[subs].size()*sizeof(unsigned int));
				std::printf("SEND BUFFER\n");
				for (int i=0; i < (int)input_events[subs].size(); ++i) {
  					send_buffer_map[sec][subs][i]=input_events[subs].at(i);
					//std::printf("%u : 0x%.8lx \n", i, send_buffer_map[subs][i]);
  				}
				std::printf("SEND OFFSET BUFFER\n");
				for (int j=0; j < (int)event_word_count[subs].size(); ++j) {
  					send_offset_buffer_map[sec][subs][j]=event_word_count[subs].at(j);
					//std::printf("%u : 0x%.8lx \n", j, send_offset_buffer_map[subs][j]);
  				}

				send_buffer[sec][subs].sync(XCL_BO_SYNC_BO_TO_DEVICE);
				send_offset_buffer[sec][subs].sync(XCL_BO_SYNC_BO_TO_DEVICE);
			}
		}
	}


	std::printf("Output buffer (FPGA mapping) sector\n");
	recv_buffer = xrt::bo(device, outbuf_size, kresult_r_GTU_s.group_id(2));
	recv_buffer_map = recv_buffer.map<stream_data_t*>();
	if(n_events<=5000) memset(recv_buffer_map,0,outbuf_size);
	std::printf("Output buffer (FPGA mapping) sector END\n");
	
	
	std::printf("Resetting switch\n");
	kswitch.write_register(4*4, 0x1); // auto-toggle reset
	sleep(1);
	kswitch.write_register(6*4, local_coord); // X coordinate
	//kswitch.write_register(68*4, 0x00030000); // overwrite destination
	kswitch.write_register(69*4, 0x01800060); // threshold
	kswitch.write_register(70*4, 0x0000ff40); // new credit cycle
	std::printf("CHANNEL_UP: %x\n", kswitch.read_register(67*4));
	
	int s = pthread_create(&thread_registers, NULL, thr_func, NULL);
	if(s){
		std::cout<<"Error in thread creation"<<std::endl;
		return EXIT_FAILURE;
	}

	std::chrono::duration<double,std::nano> deltat;
	std::printf("Starting kernel_receiver ...:\n");
	int status = 96;
	sleep(1);
        bool ksr_bram = false;
	bool kload_ddr = true;
	float threshold = 300;//270.37;
	//std::printf("Starting ksector_histogram ...\n");
	//xrt::run ksector_histogram1_run = ksector_histogram1(n_events);

	
	//xrt::run kpipe_run = kpipe(n_events);
	//n_events=n_events*1000;
	xrt::run ksr_run = kresult_r_GTU_s(n_events,//*3*3,
		        16, 
			recv_buffer,
		       	ksr_bram, status, false, threshold);
		       	//!ddr, status, false);
	std::printf("Starting kernel_loader ...\n");

	xrt::run kload_run_0 = kload[0](n_events, send_buffer[0][0], send_offset_buffer[0][0], kload_ddr);
	xrt::run kload_run_1 = kload[1](n_events, send_buffer[0][1], send_offset_buffer[0][1], kload_ddr);
	xrt::run kload_run_2 = kload[2](n_events, send_buffer[0][2], send_offset_buffer[0][2], kload_ddr);
	xrt::run kload_run_3 = kload[3](n_events, send_buffer[0][3], send_offset_buffer[0][3], kload_ddr);
	xrt::run kload_run_4 = kload[4](n_events, send_buffer[0][4], send_offset_buffer[0][4], kload_ddr);
	xrt::run kload_run_5 = kload[5](n_events, send_buffer[1][0], send_offset_buffer[1][0], kload_ddr);
	xrt::run kload_run_6 = kload[6](n_events, send_buffer[1][1], send_offset_buffer[1][1], kload_ddr);
	xrt::run kload_run_7 = kload[7](n_events, send_buffer[1][2], send_offset_buffer[1][2], kload_ddr);
	xrt::run kload_run_8 = kload[8](n_events, send_buffer[1][3], send_offset_buffer[1][3], kload_ddr);
	xrt::run kload_run_9 = kload[9](n_events, send_buffer[1][4], send_offset_buffer[1][4], kload_ddr);
	xrt::run kload_run_10 = kload[10](n_events, send_buffer[2][0], send_offset_buffer[2][0], kload_ddr);
	xrt::run kload_run_11 = kload[11](n_events, send_buffer[2][1], send_offset_buffer[2][1], kload_ddr);
	xrt::run kload_run_12 = kload[12](n_events, send_buffer[2][2], send_offset_buffer[2][2], kload_ddr);
	xrt::run kload_run_13 = kload[13](n_events, send_buffer[2][3], send_offset_buffer[2][3], kload_ddr);
	xrt::run kload_run_14 = kload[14](n_events, send_buffer[2][4], send_offset_buffer[2][4], kload_ddr);
/*	
	xrt::run kload_run_1 = kload[1](n_events, send_buffer[1][0], send_buffer[1][1], send_buffer[1][2],
		       				   send_buffer[1][3], send_buffer[1][4], 
						   send_offset_buffer[1][0], send_offset_buffer[1][1], send_offset_buffer[1][2],
		       				   send_offset_buffer[1][3], send_offset_buffer[1][4],
						   kload_ddr);
*/	

	//auto tstart = std::chrono::high_resolution_clock::now(i);

	sleep(20);
	std::printf("waiting  kernel_loader to complete ... krnl_start signal up\n");
	auto tstart = std::chrono::high_resolution_clock::now();
	xrt::run kstart_run = kstart(NULL);

	kload_run_0.wait();	
	kload_run_1.wait();	
	kload_run_2.wait();	
	kload_run_3.wait();	
	kload_run_4.wait();	
	kload_run_5.wait();	
	kload_run_6.wait();	
	kload_run_7.wait();	
	kload_run_8.wait();	
	kload_run_9.wait();	
	kload_run_10.wait();	
	kload_run_11.wait();	
	kload_run_12.wait();	
	kload_run_13.wait();	
	kload_run_14.wait();	
	//std::printf("waiting for ksector_histogram1 to complete ...\n");
	//ksector_histogram1_run.wait();	
						   
	ksr_run.wait();
	//kpipe_run.wait();

	std::printf("end of process\n");

	auto tend = std::chrono::high_resolution_clock::now();
	if(n_events<=180000){
		if(!ksr_bram) recv_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	}
	deltat = tend - tstart;

	unsigned event_count[16]={0};
	unsigned signal_predict = 0;
	unsigned noise_predict =0;
	if(n_events<=5000 && status==96 && !ksr_bram){
		long unsigned int count[16] = {0};
		unsigned predict;
		for (int i=0; (i < n_events) & (n_events<=180000); ++i){
			count[0]=0;
			count[1]=0;
			count[2]=0;
			count[3]=0;
			for(int sec=0; sec<3; sec++){	
			
			std::printf("\nEVENT: %d, SECTOR: %d\n", 	i,sec);
			std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
			recv_buffer_map[i*3+sec].high0, recv_buffer_map[i*3+sec].low0);
			std::printf("BIN 0: %lu\n", 	(recv_buffer_map[i*3+sec].low0 >> 8*0*4) & 0xFFFFFFFF);
			std::printf("BIN 1: %lu\n", 	(recv_buffer_map[i*3+sec].low0 >> 8*1*4) & 0xFFFFFFFF);
			std::printf("BIN 2: %lu\n", 	(recv_buffer_map[i*3+sec].high0 >> 8*0*4) & 0xFFFFFFFF);
			std::printf("BIN 3: %lu\n", 	(recv_buffer_map[i*3+sec].high0 >> 8*1*4) & 0xFFFFFFFF);
	/*		std::printf("BIN 4: %lu\n", 	(recv_buffer_map[i].low0 >> 8*4) & 0xFF);
			std::printf("BIN 5: %lu\n", 	(recv_buffer_map[i].low0 >> 8*5) & 0xFF);
			std::printf("BIN 6: %lu\n", 	(recv_buffer_map[i].low0 >> 8*6) & 0xFF);
			std::printf("BIN 7: %lu\n", 	(recv_buffer_map[i].low0 >> 8*7) & 0xFF);
			std::printf("BIN 8: %lu\n", 	(recv_buffer_map[i].high0 >> 8*0) & 0xFF);
			std::printf("BIN 9: %lu\n", 	(recv_buffer_map[i].high0 >> 8*1) & 0xFF);
			std::printf("BIN 10: %lu\n",	(recv_buffer_map[i].high0 >> 8*2) & 0xFF);
			std::printf("BIN 11: %lu\n",	(recv_buffer_map[i].high0 >> 8*3) & 0xFF);
			std::printf("BIN 12: %lu\n",	(recv_buffer_map[i].high0 >> 8*4) & 0xFF);
			std::printf("BIN 13: %lu\n",	(recv_buffer_map[i].high0 >> 8*5) & 0xFF);
			std::printf("BIN 14: %lu\n",	(recv_buffer_map[i].high0 >> 8*6) & 0xFF);
			std::printf("BIN 15: %lu\n",	(recv_buffer_map[i].high0 >> 8*7) & 0xFF);
	*/		
			count[0] += (recv_buffer_map[i*3+sec].low0 >> 8*0*4) & 0xFFFFFFFF;
			count[1] += (recv_buffer_map[i*3+sec].low0 >> 8*1*4) & 0xFFFFFFFF;
			count[2] += (recv_buffer_map[i*3+sec].high0 >> 8*0*4) & 0xFFFFFFFF;
			count[3] += (recv_buffer_map[i*3+sec].high0 >> 8*1*4) & 0xFFFFFFFF;
	/*		count[4] = (recv_buffer_map[i].low0 >> 8*4) & 0xFF;
			count[5] = (recv_buffer_map[i].low0 >> 8*5) & 0xFF;
			count[6] = (recv_buffer_map[i].low0 >> 8*6) & 0xFF;
			count[7] = (recv_buffer_map[i].low0 >> 8*7) & 0xFF;
			count[8] = (recv_buffer_map[i].high0 >> 8*0) & 0xFF;
			count[9] = (recv_buffer_map[i].high0 >> 8*1) & 0xFF;
			count[10] = (recv_buffer_map[i].high0 >> 8*2) & 0xFF;
			count[11] = (recv_buffer_map[i].high0 >> 8*3) & 0xFF;
			count[12] = (recv_buffer_map[i].high0 >> 8*4) & 0xFF;
			count[13] = (recv_buffer_map[i].high0 >> 8*5) & 0xFF;
			count[14] = (recv_buffer_map[i].high0 >> 8*6) & 0xFF;
			count[15] = (recv_buffer_map[i].high0 >> 8*7) & 0xFF;
	*/		
			}
			for(int k=0; k<16/4; k++){
				event_count[k] += count[k];
				if(count[k]>threshold) predict=1;
			}
			if(predict==1) signal_predict++;
			else noise_predict++;	
			predict=0;
		}
		std::printf("\n /**********COUNTING HITS**************/ \n"); 
		for(int j=0; j<16/4;j++){
			std::printf("BIN %d: %u ==> media=%f\n", j, event_count[j], (float)event_count[j]/n_events);
		}
	}

	if(n_events<=5000 && status==69 && !ksr_bram){
		for (int i=0; (i < n_events) & (n_events<=180000); ++i) { 	
			//std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
			//	recv_buffer_map[i].high0, recv_buffer_map[i].low0);
			if(recv_buffer_map[i].low0 == 0) noise_predict++;
			else signal_predict++;
		}
	}

	std::printf("\n Noise events predicted: %u (%f %) \t Signal events predicted: %u (%f %) \n",noise_predict, (float)noise_predict/n_events*100, signal_predict, (float)signal_predict/n_events*100);


	std::printf("FINISH \n");
	std::printf("Time elapsed: %lf ns, Throughput: %lf MHz\n", deltat.count(), n_events/(deltat.count()/1000));

	while(!kill){
			if(nreg >= 20) kill = true;
	}
   	pthread_join(thread_registers, NULL);
	
	
	kstart_run.wait();
	return 0;
}

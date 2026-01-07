
// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include "host.h"

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

	xrt::kernel ksr(device, uuid, "krnl_sr:{krnl_sr_1}");
	xrt::kernel kload(device, uuid, "krnl_load:{krnl_load_1}");
	std::ofstream outfile("data.dat");

	int n_events = npackets;
	int n_inputs = 40;
	int n_outputs = 4;
	bool ddr=true;
	auto inbuf_size = n_events*n_inputs*sizeof(float);
	auto outbuf_size = n_events*sizeof(stream_data_t); 
	std::vector<float> input_events[6];
	std::vector<float> output_events[6];
	std::vector<float> y_events;
	unsigned inp_count = 0;

	if(n_events<=180000){
		std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_100kHz/y_train.txt";
		//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_200kHz/y_train.txt";
		//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_300kHz/y_train.txt";
		//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile/y_test.txt";
		std::printf(str.c_str());
		std::printf("\n");
		inp_count += read_input_file_y(str.c_str(), y_events, n_events); 
		std::printf("Y buffer size=%lu\n", y_events.size());
		for(int i=0; i<6; i++){
			inp_count = 0;		
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile/sector_input_test_" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile/hls4ml_sector_input_train" + std::to_string(i) + ".txt";
			std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_100kHz/hls4ml_sector_input_train" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_200kHz/hls4ml_sector_input_train" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_300kHz/hls4ml_sector_input_train" + std::to_string(i) + ".txt";
			std::printf("Input buffer sectorNN \n");
			std::printf(str.c_str());
			std::printf("\n");
			inp_count += read_input_file_sector(str.c_str(), input_events[i], n_events, n_inputs);
			ddr=true;
			std::printf("Input buffer %d size=%lu\n", i, input_events[i].size());
		}
		for(int i=0; i<6; i++){
			inp_count = 0;		
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile/sector_output_test_" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile/hls4ml_sector_output_" + std::to_string(i) + ".txt";
			std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_100kHz/hls4ml_sector_output_train" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_200kHz/hls4ml_sector_output_train" + std::to_string(i) + ".txt";
			//std::string str = "/apotto/home1/aliens/rossi/APEIRON/apeiron_framework/dev_apps/EIC_apeiron_TP/host/infile_300kHz/hls4ml_sector_output_train" + std::to_string(i) + ".txt";
			std::printf("Input buffer sectorNN \n");
			std::printf(str.c_str());
			std::printf("\n");
			inp_count += read_input_file_sector(str.c_str(), output_events[i], n_events, n_outputs);
			ddr=true;
			std::printf("Input buffer %d size=%lu\n", i, output_events[i].size());
		}
	}
	else{
		inbuf_size = 2000;
		outbuf_size = 2000;
		ddr=false;
	}



	xrt::bo send_buffer[6];
	float* send_buffer_map[6];
	xrt::bo recv_buffer[6];
	stream_data_t* recv_buffer_map[6];
	for(int subs=0; subs<6; subs++){
		std::printf("Input buffer (FPGA mapping) subsector n°%d\n",subs);
		send_buffer[subs] = xrt::bo(device, inbuf_size, kload.group_id(subs+1));
		send_buffer_map[subs] = send_buffer[subs].map<float*>();
		if(n_events<=180000){
 			memset(send_buffer_map[subs], 0, inbuf_size);
			for (int i=0; i < n_events*n_inputs; ++i) {
  				send_buffer_map[subs][i]=input_events[subs][i];
				//std::printf("%u : %f \n", i, send_buffer_map[subs][i]);
  			}
			send_buffer[subs].sync(XCL_BO_SYNC_BO_TO_DEVICE);
		}
	}
	for(int sec=0; sec<6; sec++){		
		std::printf("Output buffer (FPGA mapping) sector n°%d arg=%d\n",sec,sec+2);
		recv_buffer[sec] = xrt::bo(device, outbuf_size, ksr.group_id(sec+2));
		recv_buffer_map[sec] = recv_buffer[sec].map<stream_data_t*>();
		if(n_events<=100000) memset(recv_buffer_map[sec],0,outbuf_size);
		std::printf("Output buffer (FPGA mapping) sector n°%d END\n",sec);
	}

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
	int status = 69;
	sleep(1);
        bool ksr_bram = false;
	bool kload_ddr = true;

	xrt::run ksr_run = ksr(n_events, 16, 
			recv_buffer[0], recv_buffer[1], recv_buffer[2],
			recv_buffer[3], recv_buffer[4], recv_buffer[5],
		       	ksr_bram, status, false);
		       	//!ddr, status, false);
	std::printf("Starting kernel_loader ...\n");
	xrt::run kload_run = kload(n_events, 
			send_buffer[0], send_buffer[1], send_buffer[2],
		       	send_buffer[3], send_buffer[4], send_buffer[5], 
			//ddr);
			kload_ddr);

	auto tstart = std::chrono::high_resolution_clock::now();
	std::printf("Waiting  kernel_loader to complete ...\n");
	kload_run.wait();
	std::printf("Waiting for kernel_receiver to complete ...\n");
	ksr_run.wait();
	std::printf("END of Process\n");

	auto tend = std::chrono::high_resolution_clock::now();
	if(n_events<=180000){
		if(!ksr_bram) for(int sec=0; sec<6; sec++) recv_buffer[sec].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	}
	deltat = tend - tstart;

	if(n_events<=180000 && status!=69){
		for (int i=0; (i < n_events-1) & (n_events<=180000); ++i) {
		for (unsigned j=0; j<6; j++){
			std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
				recv_buffer_map[j][i].high0, recv_buffer_map[j][i].low0);
			uint64_t value[4];
			for(unsigned k=0; k<4; k++){
				if(k>0) value[k] = (recv_buffer_map[j][i].low0  >> (16*k)) & 0xFFFF;     // Returns 6000
				else value[k] = (recv_buffer_map[j][i].low0 & 0xFFFF ) ;  	
			
				std::printf("In %u : 0x%.4lx  = %lu ==> %f \n", k, value[k], value[k], value[k]*0.00390625);
			}	
			for(unsigned k=0; k<4; k++){
				std::printf("OUTFILE ev %d sec %u feat %u := %f \n", i, j, k+4, output_events[j][i*4+k]);
			}

	   	}
		}
	}
	if(n_events<=180000 && status==69){
		unsigned int cf_matrix[4];
		cf_matrix[0]=0;
		cf_matrix[1]=0;
		cf_matrix[2]=0;
		cf_matrix[3]=0;

		for (int i=0; (i < n_events) & (n_events<=180000); ++i) { 	
			std::printf("0x%.8lx: 0x%.16lx%.16lx \n", i*sizeof(stream_data_t), 
				recv_buffer_map[0][i].high0, recv_buffer_map[0][i].low0);
			auto y_pred = recv_buffer_map[0][i].low0;
			if(y_events[i]==0 && y_pred==0) cf_matrix[0]++;
			if(y_events[i]==1 && y_pred==0) cf_matrix[1]++;
			if(y_events[i]==0 && y_pred==1) cf_matrix[2]++;
			if(y_events[i]==1 && y_pred==1) cf_matrix[3]++;
		}

		std::printf("CONFUSION MATRIX (FPGA 6 SECTOR MLP + AGGREGATE MLP IMPLEMENTATION)\n ");
		std::printf("\t %u \t %u \n", cf_matrix[0], cf_matrix[1]);
		std::printf("True. \n");
		std::printf("\t %u \t %u \n", cf_matrix[2], cf_matrix[3]);
		std::printf("\t \t Pred.\n");
	}



	std::printf("FINISH \n");
	std::printf("Time elapsed: %lf ns, Throughput: %lf MHz\n", deltat.count(), n_events/(deltat.count()/1000));

	kill = true;
   	pthread_join(thread_registers, NULL);
	
	return 0;
}

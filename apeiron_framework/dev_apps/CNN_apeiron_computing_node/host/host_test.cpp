
// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <chrono>
#include <getopt.h>

#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_kernel.h>
#include <experimental/xrt_ip.h>


//Thread able to print Switch's registers during the execution (useful in case of main thread crash)
xrt::ip kswitch_thread;
bool kill = false;
unsigned nreg = 0;

void *thr_func(void *arg) {
  while(!kill){
  std::printf("Thread started ==> Press a key to print registers\n");
	std::getchar();
	nreg++;
  for (int i=4;i<130;i++){
  		 if(i==4) std::printf("****************** FROM THREAD *********************\n");
       std::cout << "reg: "<< std::dec << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  (kswitch_thread).read_register(i*4) <<" \t 0x"<< std::hex <<(kswitch_thread).read_register(i*4)<< std::dec << "\n";
   }
   
   }
  pthread_exit(NULL);
}

static struct option test_options[] = {
	{"bitstream", required_argument, 0, 'b'},
	{"input", required_argument, 0, 'i'},
	{"npackets", required_argument, 0, 'n'},
	{"local_coord", required_argument, 0, 'c'},
	{"ndevices", required_argument, 0, 'd'},
	{"xdest", required_argument, 0, 'x'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

struct stream_data_t {
	uint64_t low;
	uint64_t high;
};

void print_usage(char *name) {
	std::cout << "Usage: " << name << " -b bitstream.xclbin\n\n";
	std::cout << "  -b, --bitstream <FILE>\tbitstream used to flash FPGA\n";
	std::cout << "  -i, --input <FILE>\tM2EGP input file\n";
	std::cout << "  -n, --npackets <NUM>\t\tnumber of packets to send/receive\n";
	std::cout << "  -d, --set number of devices in the system \n";
	std::cout << "  -c, --set local coord \n";
	std::cout << "  -x, --set destination in x \n";
	std::cout << "  -h, --help\t\t\tprint this help\n";
}

unsigned read_m2egp_file(const char *m2egp_file, std::vector<stream_data_t> &m2egp, unsigned npackets)
{
	std::string line;
	std::ifstream infile(m2egp_file);
	unsigned counter = 0;
	unsigned stride = 0;
	stream_data_t event = {0};


	while(std::getline(infile, line)) {
		uint64_t d = std::stoul(line, nullptr, 16);
		if ((d >> 16) == 0xffff) counter++;
		
		if(stride==0) event.low=d;
		if(stride==1) event.low+=d<<32;
		if(stride==2) event.high=d;
		
		
		if(stride==3){
			event.high+=d<<32;
			m2egp.push_back(event);
			stride=0; 
			if (counter > npackets) {
				m2egp.pop_back();
				//m2egp.pop_back();
				counter--;
				break;
			}
		}
		else stride++;
	}	
	return counter;
}

int main(int argc, char** argv)
{
	char *bitstream = NULL;
	pthread_t thread_registers; //Thread definition
	char *m2egp_file = NULL;
	unsigned npackets = 0;
	unsigned ndevices = 1;
	unsigned xdest = 0;
	unsigned local_coord = 0;

	while (1) {
		int option_index;
		int c = getopt_long(argc, argv, "b:i:n:d:x:c:h", test_options, &option_index);

		if (c == -1) break;

		switch (c) {
			case 'b':
				bitstream = optarg;
				break;
			case 'i':
				m2egp_file = optarg;
				break;
			case 'n':
				npackets = atoi(optarg);
				break;
			case 'd':
				ndevices = atoi(optarg);
				break;
			case 'x':
				xdest = atoi(optarg);
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

	xrt::uuid uuid = device.load_xclbin(bitstream);
	xrt::ip kswitch(device, uuid, "TextaRossa_switch");
	kswitch_thread = kswitch; //ip handle for parallel thread 
	
	if(local_coord == 0){
		xrt::kernel kreceiver(device, uuid, "krnl_receiver:{krnl_receiver_1}");
		xrt::kernel ksender(device, uuid, "krnl_sender:{krnl_sender_1}");
		std::ofstream outfile("data.dat");
	
		std::vector<stream_data_t> m2egp;
		unsigned m2egp_count = 0;
		//for(int p=0; p < npackets; p++){
		//m2egp_count += read_m2egp_file("DATA/clop_flow/cloppone_last.dat", m2egp, npackets);
		m2egp_count += read_m2egp_file("DATA/clop_flow/cloppone_val.dat", m2egp, npackets);
		//}
		std::cout << m2egp_count << " M2EGPs (" << m2egp.size() << " words)" << std::endl;
		if (!m2egp_count) exit(EXIT_FAILURE);

		size_t outbuf_size = m2egp_count * sizeof(stream_data_t);
		size_t inbuf_size = m2egp.size() * sizeof(m2egp[0]);

		std::cout << "inbuf " << inbuf_size << ", outbuf " << outbuf_size << std::endl;
	
		xrt::bo recv_buffer(device, outbuf_size, kreceiver.group_id(0));
		stream_data_t *recv_buffer_map = recv_buffer.map<stream_data_t*>();
		memset(recv_buffer_map, 0, outbuf_size);
	
		xrt::bo send_buffer(device, inbuf_size, ksender.group_id(2));
		stream_data_t *send_buffer_map = send_buffer.map<stream_data_t*>();
 		memset(send_buffer_map, 0, inbuf_size);

		for (unsigned i=0; i < m2egp.size(); ++i) {
  			send_buffer_map[i].high = m2egp[i].high;
  			send_buffer_map[i].low = m2egp[i].low;
  		}
  		for (unsigned i=0; i < inbuf_size/sizeof(stream_data_t); ++i) {
			//std::printf("0x%.8lx: 0x%.16lx%.16lx\n", i*sizeof(stream_data_t), send_buffer_map[i].high, 		send_buffer_map[i].low);
			//std::printf("\"0x%.16lx%.16lx\",\n", send_buffer_map[i].high, send_buffer_map[i].low);
		}
		//std::cout<<"LOAD DATA ON FPGA"<<std::endl;
		send_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

	//	std::printf("Press a key to continue\n");
  //      std::getchar();

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
		

		std::chrono::duration<double, std::nano> deltat;
	
		std::printf("Running receiver kernel ...\n");
		xrt::run kreceiver_run = kreceiver(recv_buffer, m2egp_count);

		std::printf("Starting sender kernel ...\n");
	
	
		xrt::run ksender_run = ksender(ndevices, npackets, send_buffer, 1);
		auto tstart = std::chrono::high_resolution_clock::now();
		//std::printf("Waiting for sender kernel to complete ...\n");
		//ksender_run.wait();
	
		//std::printf("Waiting for receiver kernel to complete ...\n");
		kreceiver_run.wait();

		auto tend = std::chrono::high_resolution_clock::now();
		recv_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	
		deltat = tend - tstart;

		float *rings;
		float *evts;
		rings = (float*)calloc(20,sizeof(float));
		evts = (float*)calloc(5,sizeof(float));
		for (unsigned i=0; i < m2egp_count; ++i) {
			//std::printf("0x%.8lx: 0x%.16lx%.16lx\n", i*sizeof(stream_data_t), recv_buffer_map[i].high, recv_buffer_map[i].low);
			outfile<<std::hex<<(recv_buffer_map[i].high>>48)<<recv_buffer_map[i].low<<"\n";
			auto ring_RECO= (recv_buffer_map[i].high >> 48) & 0xFF;
			//auto ring_RECO = recv_buffer_map[i].high >> 56;
			if(ring_RECO>=3) ring_RECO=3;
			evts[ring_RECO]++;
			auto ring_NN= recv_buffer_map[i].low & 0xFF;
			outfile<<ring_RECO<<" \t "<<ring_NN<<" \n";
		
			rings[4*ring_RECO+ring_NN]++;
		
		}
		std::printf("\n");
	
	
		std::cout << m2egp_count /** 2*/<< "m2egp in " << deltat.count()/(1e9) << "s -> " << deltat.count()/	(m2egp_count /** 2*/)<< "ns/evt" << std::endl;

		std::cout<< m2egp_count  << " \t "<< deltat.count()/(m2egp_count ) << "\n";
	
		
	
		std::cout <<"EFFICIENCY CONFUSION MATRIX (N_RINGS) \n";
		std::cout <<"0:\t"<<rings[0]/evts[0]<<" \t "<<rings[1]/evts[0]<<"\t"<<rings[2]/evts[0]<<"\t"<<rings[3]/	evts[0]<<"\t"<<"\n";
		std::cout <<"1:\t"<<rings[4]/evts[1]<<"\t"<<rings[5]/evts[1]<<"\t"<<rings[6]/evts[1]<<"\t"<<rings[7]/	evts[1]<<"\t"<<"\n";
		std::cout <<"2:\t"<<rings[8]/evts[2]<<"\t"<<rings[9]/evts[2]<<"\t"<<rings[10]/evts[2]<<"\t"<<rings[11]/	evts[2]<<"\t"<<"\n";
		std::cout <<"3+:\t"<<rings[12]/evts[3]<<"\t"<<rings[13]/evts[3]<<"\t"<<rings[14]/evts[3]<<"\t"<<rings[15]/evts[3]<<"\t"<<"\n";
	
		std::cout <<"PURITY CONFUSION MATRIX (N_RINGS)\n";
                std::cout <<"0:\t"<<rings[0]/(rings[0]+rings[4]+rings[8]+rings[12])<<" \t "<<rings[1]/(rings[1]+rings[5]+rings[9]+rings[13])<<"\t"<<rings[2]/(rings[2]+rings[6]+rings[10]+rings[14])<<"\t"<<rings[3]/(rings[3]+rings[7]+rings[11]+rings[15])<<"\t"<<"\n";
                std::cout <<"1:\t"<<rings[4]/(rings[0]+rings[4]+rings[8]+rings[12])<<"\t"<<rings[5]/(rings[1]+rings[5]+rings[9]+rings[13])<<"\t"<<rings[6]/(rings[2]+rings[6]+rings[10]+rings[14])<<"\t"<<rings[7]/(rings[3]+rings[7]+rings[11]+rings[15])<<"\t"<<"\n";
                std::cout <<"2:\t"<<rings[8]/(rings[0]+rings[4]+rings[8]+rings[12])<<"\t"<<rings[9]/(rings[1]+rings[5]+rings[9]+rings[13])<<"\t"<<rings[10]/(rings[2]+rings[6]+rings[10]+rings[14])<<"\t"<<rings[11]/(rings[3]+rings[7]+rings[11]+rings[15])<<"\t"<<"\n";
                std::cout <<"3+:\t"<<rings[12]/(rings[0]+rings[4]+rings[8]+rings[12])<<"\t"<<rings[13]/(rings[1]+rings[5]+rings[9]+rings[13])<<"\t"<<rings[14]/(rings[2]+rings[6]+rings[10]+rings[14])<<"\t"<<rings[15]/(rings[3]+rings[7]+rings[11]+rings[15])<<"\t"<<"\n";

	
		outfile.close();
	 	kill = true;
   	pthread_join(thread_registers, NULL);
	
	//free(rings);
	}
	else{
		std::printf("Resetting switch\n");
		
		//xrt::kernel kreceiver(device, uuid, "krnl_receiver:{krnl_receiver_1}");
		
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
		
	//	xrt::bo recv_buffer(device, npackets/3*sizeof(stream_data_t), kreceiver.group_id(0));
	//	stream_data_t *recv_buffer_map = recv_buffer.map<stream_data_t*>();
	//	memset(recv_buffer_map, 0,  npackets/3*sizeof(stream_data_t));
		
		//xrt::run kreceiver_run = kreceiver(NULL, npackets/3);
		//kreceiver_run.wait();
	//	recv_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
		
	/*	for (unsigned i=0; i < npackets/3; ++i) {
			std::printf("0x%.8lx: 0x%.16lx%.16lx\n", i*sizeof(stream_data_t), recv_buffer_map[i].high, recv_buffer_map[i].low);
		}
	*/	
		while(!kill){
			if(nreg >= 10) kill = true;
		}
   	pthread_join(thread_registers, NULL);
	}
	
	


	return 0;
}

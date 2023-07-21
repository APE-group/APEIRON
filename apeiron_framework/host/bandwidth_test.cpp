// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#include <iostream>
#include <cstdio>
#include <cstring>

#include <getopt.h>

#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_kernel.h>
#include <experimental/xrt_ip.h>
#include "read_registers.hpp"

#include <pthread.h>
#define INTERNAL_GENERATOR 0
#define CABLE_LOOPBACK 0
#define CONSUMER 0
/*
xrt::ip kswitch_thread;
bool kill = false;


//Thread able to print Switch's registers during the execution (useful in case of main thread crash)
void *thr_func(void *arg) {
  //xrt::ip krnl_handler(device, uuid_thread, "TextaRossa_switch");
  while(!kill){
  std::printf("Thread started ==> Press a key to print registers\n");
        std::getchar();
  for (int i=4;i<134;i++){
                 if(i==4) std::printf("****************** FROM THREAD *********************\n");
      std::cout << "reg: " << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  (kswitch_thread).read_register(i*4) << std::hex <<" \t 0x" << (kswitch_thread).read_register(i*4)<<std::dec << "\n";
   }
   }
   
  pthread_exit(NULL);
}
*/


// Elapsed Time function 
timeval startTime, endTime;
double elapsed(timeval ti, timeval te)
{
	return (te.tv_sec - ti.tv_sec)*1000000.0 + (te.tv_usec - ti.tv_usec);
}

//Default values
const unsigned PACKET_SIZE = 0x80;
const unsigned NPACKETS = 8;
const unsigned BUF_SIZE = (PACKET_SIZE + 32) * NPACKETS;

//Buffer data type (128bit = 1 word in HAPECOM)
struct stream_data_t {
	uint64_t low;
	uint64_t high;
};

//Flag definition
static struct option test_options[] = {
	{"bitstream", required_argument, 0, 'b'},
	{"length", required_argument, 0, 'l'},
	{"npackets", required_argument, 0, 'n'},
	{"xdest", required_argument, 0, 'x'},
	{"start_port", required_argument, 0, 's'},
	{"local_coord", no_argument, 0, 'c'},
	{"bram", no_argument, 0, 'm'},
	{"dest_task_id", required_argument, 0, 'i'},
	{"quiet", no_argument, 0, 'q'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

//Flag description from --help
void print_usage(char *name) {
	std::cout << "Usage: " << name << " -b bitstream.xclbin\n\n";
	std::cout << "  -b, --bitstream <FILE>\tbitstream used to flash FPGA\n";
	std::cout << "  -l, --length <SIZE>\t\tpacket length in byte\n";
	std::cout << "  -n, --npackets <NUM>\t\tnumber of packets to send/receive\n";
	std::cout << "  -q, --quiet useless print for data taking\n";
	std::cout << "  -m, --enable use of BRAM instead of DDR\n";
	std::cout << "  -i, --set intratile destination\n";
	std::cout << "  -s, --set krnl_sr status: 0) 1by1 send_receive; 1) only send 2) only receive 3) sender_receiver full \n";
	std::cout << "  -c, --set local coord \n";
	std::cout << "  -h, --help\t\t\tprint this help\n";
}


//Main Thread
int main(int argc, char** argv)
{

	pthread_t thread_registers; //Thread definition
	
	char *bitstream = NULL;
	bool quiet = false;
	unsigned bram = 0;
	unsigned packet_size = PACKET_SIZE;
	unsigned npackets = NPACKETS;
	unsigned start_port = 0;
	unsigned dest_task_id = 0;
	unsigned xdest = 0;
	unsigned status = 4;
	unsigned local_coord = 0;

	//Flags getting values
	while (1) {
		int option_index;
		int c = getopt_long(argc, argv, "b:l:n:s:qh:mx:i:c", test_options, &option_index);

		if (c == -1) break;

		switch (c) {
			case 'b':
				bitstream = optarg;
				break;
			case 'l':
				packet_size = atoi(optarg);
				break;
			case 'n':
				npackets = atoi(optarg);
				break;
			case 'q':
				quiet = true;
				break;
			case 'm':
				bram = 1;
				break;
			case 'x':
				xdest =  atoi(optarg);
				break;
			case 'i':
				dest_task_id = atoi(optarg);
				break;
			case 's':
				start_port = atoi(optarg);
				break;
			case 'c':
        			local_coord = 1;
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

//	size_t buf_size =  (npackets*((local_coord)%2)+1*(local_coord+1%2)) * (packet_size ); //buffer size def

	size_t buf_size =  npackets * packet_size; //buffer size def
	xrt::device device(0); //device initialization
	
	if(!quiet){
		std::cout << "Device name: " << device.get_info<xrt::info::device::name>() << std::endl;
		std::cout << "Device bdf: " << device.get_info<xrt::info::device::bdf>() << std::endl;
		std::cout << "Device max freq: " << device.get_info<xrt::info::device::max_clock_frequency_mhz>() << std::endl;
		std::cout << "Device m2m: " << device.get_info<xrt::info::device::m2m>() << std::endl;
		std::cout << "Device nodma: " << device.get_info<xrt::info::device::nodma>() << std::endl;
		std::cout << "Device kdma: " << device.get_info<xrt::info::device::kdma>() << "\n\n";
	}
	
	xrt::uuid uuid = device.load_xclbin(bitstream); //bitstream flashed
	
	xrt::ip kswitch(device, uuid, "TextaRossa_switch"); //ip Switch handle 
	kswitch_thread = kswitch; //ip handle for parallel thread 
	
	xrt::kernel krnl_sender_receiver; //kernel handle
	xrt::kernel krnl_sender_receiver2;

  if(start_port==0) krnl_sender_receiver = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_1}");
	else if(start_port==1) krnl_sender_receiver = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_2}");
	else if(start_port==2) krnl_sender_receiver = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_3}");
	else if(start_port==3) krnl_sender_receiver = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_4}");

  if(local_coord==1 || (xdest==0 && start_port!=dest_task_id)){
  		if(dest_task_id==0) krnl_sender_receiver2 = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_1}");
  		else if(dest_task_id==1) krnl_sender_receiver2 = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_2}");
  		else if(dest_task_id==2) krnl_sender_receiver2 = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_3}");
  		else if(dest_task_id==3) krnl_sender_receiver2 = xrt::kernel(device, uuid, "krnl_sr:{krnl_sr_4}");
  }
	
	//Receive buffer intialization
	
	xrt::bo recv_buffer(device, buf_size, krnl_sender_receiver.group_id(6));
	stream_data_t *recv_buffer_map = recv_buffer.map<stream_data_t*>();
	memset(recv_buffer_map, 0, buf_size);

	//Send buffer initialization
	xrt::bo send_buffer(device, packet_size, krnl_sender_receiver.group_id(5));
	stream_data_t *send_buffer_map = send_buffer.map<stream_data_t*>();
 	memset(send_buffer_map, 0, packet_size);
	for (unsigned i=0; i < packet_size/sizeof(stream_data_t); ++i) {
  		send_buffer_map[i].low=i;
  		send_buffer_map[i].high=i+1;
  	}

	if(bram==0 && status!=3) send_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE); //HOST-DDR SYNC (disabled for BRAM tests)
	

	if(!quiet){
		for (int i=26;i<134;i++){
			std::cout << "reg: " << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  kswitch.read_register(i*4) << std::dec << "\n";
		}
		std::printf("Press a key to continue\n");
		std::getchar();
	}

	if(!quiet) std::printf("Resetting switch\n");
	kswitch.write_register(4*4, 0x1); // auto-toggle reset
	sleep(1);
	
	if(!quiet){
		for (int i=26;i<36;i++){
        	std::cout << "reg: " << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  kswitch.read_register(i*4) << std::dec << "\n";
      }
	}
	
	kswitch.write_register(6*4, local_coord); //3D coordinate
	

	if(CABLE_LOOPBACK) kswitch.write_register(68*4, 0x00030000); // overwrite destination (needed for cable loopback)
	kswitch.write_register(69*4, 0x01800060); // threshold
	kswitch.write_register(70*4, 0x0000ff40); // new credit cycle
	if(!quiet) std::printf("CHANNEL_UP: %x\n", kswitch.read_register(67*4));

	//Register Thread start
	if(!quiet){
		int s = pthread_create(&thread_registers, NULL, thr_func, NULL);
		if(s){
			std::cout<<"Error in thread creation"<<std::endl;
			return EXIT_FAILURE;
		}
	}
	
	if(!quiet) std::printf("Running receiver kernel ...\n");
	gettimeofday(&startTime,NULL);
	if(bram==0 && status!=3) send_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE); //HOST-DDR SYNC (disabled for BRAM tests)
	
	xrt::run krnl_sender_receiver_run2;
	xrt::run krnl_sender_receiver_run;


	
	if (INTERNAL_GENERATOR && local_coord == 0) {
               unsigned tmp = npackets + (packet_size << 16);
               kswitch.write_register(14*4, tmp);

               if(CABLE_LOOPBACK) kswitch.write_register(16*4, 0x00000001);
               std::printf("Starting packet generator ...\n");

               kswitch.write_register(12*4, 0x00000101);
	       		 //kswitch.write_register(16*4, 0x00000001);

               sleep(1);
               //std::printf("Test ok: %x\n",kswitch.read_register(20*4)); // Tet_ok bi
               //std::printf("Clock cycle: %u\n", kswitch.read_register(22*4));
               //exit(0);
	 
	 						krnl_sender_receiver_run = krnl_sender_receiver(xdest, dest_task_id, npackets, npackets, packet_size, send_buffer, recv_buffer, bram, status);
	 						
    	}
    	
	else{
		if(local_coord==1 || (xdest==0 && start_port!=dest_task_id)) krnl_sender_receiver_run2 = krnl_sender_receiver2(xdest, start_port, 1, npackets, packet_size, NULL, NULL, 1, 5);
		if(local_coord==0 || xdest==1) krnl_sender_receiver_run = krnl_sender_receiver(xdest, dest_task_id, npackets*((local_coord+1)%2)+1*(local_coord%2), npackets*(local_coord%2)+1*(local_coord+1)%2, packet_size, send_buffer, recv_buffer, bram, status);
	}

	if(!quiet) std::printf("Waiting for receiver kernel to complete ...\n");
	if(local_coord==1 || (xdest==0 && start_port!=dest_task_id)) krnl_sender_receiver_run2.wait();
	if(local_coord==0 || xdest==1){
		krnl_sender_receiver_run.wait();
		if(bram==0 && status!=2) recv_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	}
	gettimeofday(&endTime,NULL);
	if(!quiet){
		std::printf("Printing output:\n");
		for (unsigned i=0; i < buf_size/sizeof(stream_data_t); ++i) {
			//std::printf("0x%.8lx: 0x%.16lx%.16lx\n", i*sizeof(stream_data_t), recv_buffer_map[i].high, recv_buffer_map[i].low);
		}
	}
	double elapsedTime = elapsed(startTime,endTime);
	if(!quiet){
  	std::cout<<"Bandwidth: "<<((npackets+4)*packet_size)/elapsedTime<<" MB/s"<<std::endl;
		std::cout<<"Latency: "<<elapsedTime/(2*npackets)<<" us"<<std::endl;
		std::cout<<"Elapsed time: "<<elapsedTime<<" us"<<std::endl;
	}
	else{
		std::cout<<packet_size<<" \t "<<((npackets+4)*packet_size)/elapsedTime<<std::endl;
	}
	
	if(!quiet){
   kill = true;
   pthread_join(thread_registers, NULL);
	}
	return 0;
}

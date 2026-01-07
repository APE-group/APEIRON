#ifndef HOST_H
#define HOST_H

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

//Parallel CPU thread for switch register polling
void *thr_func(void *arg) {
  while(!kill){
  std::printf("Thread started ==> Press a key to print registers\n");
	std::getchar();
	nreg++;
  for (int i=4;i<60;i++){
  		 if(i==4) std::printf("****************** FROM THREAD *********************\n");
       std::cout << "reg: "<< std::dec << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  (kswitch_thread).read_register(i*4) <<" \t 0x"<< std::hex <<(kswitch_thread).read_register(i*4)<< std::dec << "\n";
   }
   
   }
  pthread_exit(NULL);
}

static struct option test_options[] = {
	{"bitstream", required_argument, 0, 'b'},
	{"npackets", required_argument, 0, 'n'},
	{"local_coord", required_argument, 0, 'c'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

struct stream_data_t {
	uint64_t low0;	
	//uint64_t low1;
	uint64_t high0;	
	//uint64_t high1;
};

//Function for file reading ==> True prediction (tensorflow)
unsigned read_input_file_y(const char *input_file, std::vector<float> &input, unsigned npackets)
{
	std::string line;
	std::ifstream infile(input_file);
	unsigned int counter = 0;
	unsigned int events = 0;

	while(std::getline(infile, line)) {
		//std::cout<<line<<std::endl;
		counter++;
		if(counter%2==0) {
			events++;
			if(events==npackets) break;
			else continue;	
		}
		//std::printf("subs=%d\n",subsector);
		float value = std::stof(line,nullptr);
		float out = static_cast<float>(value);
		input.push_back(out);		
	}	
	return counter;
}


//Function for file reading ==> intermediate input/output (sector mlp, from tensorflow)
unsigned read_input_file_sector(const char *input_file, std::vector<float> &input, unsigned npackets, unsigned eachevent)
{
	std::string line;
	std::ifstream infile(input_file);
	unsigned int counter = 0;
	unsigned int events = 0;

	while(std::getline(infile, line)) {
		//std::cout<<line<<std::endl;
		counter++;
		if(counter%(eachevent+1)==0) {
			events++;
			if(events==npackets) break;
			else continue;	
		}
		//std::printf("subs=%d\n",subsector);
		float value = std::stof(line,nullptr);
		float out = static_cast<float>(value);
		input.push_back(out);		
	}	
	return counter;
}

void print_usage(char *name) {
	std::cout << "Usage: " << name << " -b bitstream.xclbin\n\n";
	std::cout << "  -b, --bitstream <FILE>\tbitstream used to flash FPGA\n";
	std::cout << "  -n, --npackets <NUM>\t\tnumber of packets to send/receive\n";
	std::cout << "  -c, --set local coord \n";
	std::cout << "  -h, --help\t\t\tprint this help\n";
}

#endif

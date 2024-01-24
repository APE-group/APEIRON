#include <iostream>
#include <string>
#include <fstream>
#include <vector>

std::vector<std::string> read_registers(){
	
	std::string reg_line;
	std::vector<std::string> registers;
	unsigned counter = 0;
	
	std::ifstream infile("host/reg_labels.txt");
	if ( infile.is_open() ) {
		while(counter<4){
			counter++;
			registers.push_back("");
		}
		counter--;
		while (infile) {
			std::string tmp_line;
			std::getline (infile, tmp_line);
			//std::cout << tmp_line << std::endl;
			if(tmp_line != "######") reg_line = reg_line + tmp_line + "\t \t \t \t \n";
			else {
				registers.push_back(reg_line);
				reg_line.clear();
				counter++;
			}
			if(counter==6 || counter==8 || counter==14 || counter==16 ||counter==47 || counter==71 || counter==99){
				while(counter<8-1){
					counter++;
					registers.push_back("");
				}
				while(counter>8-1 && counter<12-1){
					counter++;
					registers.push_back("");
				}
				while(counter>14-1 && counter<15){
					counter++;
					registers.push_back("");
				}
				while(counter>=16 && counter<20-1){
					counter++;
					registers.push_back("");
				}
				while(counter>20-1 && counter<65-1){
					counter++;
					registers.push_back("");
				}
				
				while(counter>71-1 && counter<80-1){
					counter++;
					registers.push_back("");
				}
				
				
				while(counter>99-1 && counter<110-1){
					counter++;
					registers.push_back("");
				}
				
			}
			
		}
	}
	
	//for (std::vector<std::string>::iterator it = registers.begin() ; it != registers.end(); ++it)
   //std::cout << *it << std::endl;
	
	
	return registers;
}


xrt::ip kswitch_thread;
bool kill = false;

void *thr_func(void *arg) {
  //xrt::ip krnl_handler(device, uuid_thread, "TextaRossa_switch");
  std::vector<std::string> regs = read_registers();
  while(!kill){
  std::printf("Thread started ==> Press a key to print registers\n");
	std::getchar();
  for (int i=0;i<152;i++){
  		 if(i==0) std::printf("****************** FROM THREAD *********************\n");
       std::cout << "REGISTER: "<< std::dec << i << " [0x" << std::hex << i*4 << "] = " << std::dec <<  (kswitch_thread).read_register(i*4) <<" \t 0x"<< std::hex <<(kswitch_thread).read_register(i*4)<< std::dec <<"\t"<< /*regs.at(i) <<*/ "\n";
   }
   }
   
  pthread_exit(NULL);
}

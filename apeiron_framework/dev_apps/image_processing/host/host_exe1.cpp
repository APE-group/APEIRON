
#include "host.h"
#include "read_registers.hpp"

//#include "xcl2.hpp"
#include <random>
#include <algorithm>
#include <cstdio>
#include <vector>
#include <experimental/xrt_system.h>

using std::default_random_engine;
using std::generate;
using std::uniform_int_distribution;
using std::vector;

//#include <algorithm>
//#include <vector>

#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <zlib.h>

#define _HW_RUN_ 
#define _SAVE_OUTPUT_FILES_
//#define _TWO_KERNELS_

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#define _DO_PCIE_TRANSFERS_

#define WORD_WIDTH 512  // width in bit of the interfaces
#define ELEMENTS_PER_PIXEL 3 // 1 Y image

// Settings for HW Emulation
#define NB_REPLICAS_X 1
#define NB_REPLICAS_Y 1

#ifdef _HW_RUN_
#define NB_IMAGES 100
#define IMAGE_ROWS 4096
#define IMAGE_COLS 4096
   const char * name[] = {
		   "Lamborghini_4096x4096_8bpp",
//		   "BurningRose_4096x4096_8bpp"
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
		   "Lamborghini_4096x4096_8bpp",
//	   		"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//	   		"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//			"Lamborghini_4096x4096_24bpp",
//			"Alps_4096x4096",
//			"BurningRose_4096x4096",
//			"ChildAndWolf_4096x4096",
//			"girl-woman-moon-beach-sunset-4096x4096",
//			"half-moon-stars-mountains-night-4096x4096",
//			"lake_4096x4096",
//			"nature_4096x4096",
    };
    unsigned short int NB_ROWS[] = {
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    };
    unsigned short int NB_COLS[] = {
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
    		4096,
	   };

#else
#define NB_IMAGES 2
#define IMAGE_ROWS 512
#define IMAGE_COLS 512
    const char * name[] = {
//    		"Chessboard_1024x1024_8bpp",
//    		"Rectangles_Lines_Circle_512x512_8bpp",
//    		"floor_512x512_8bpp",
//			"DoorAndFloor_512x512_8bpp",
//    		"Lenna_with_lines_512x512_8bpp",
//    		"lenna_512_512_8bpp",
//    		"lenna_512_512_8bpp",
//    		"lenna_512_512_8bpp",
     		"Lenna_512x512_24bpp",
     		"Lenna_512x512_24bpp",
     		"Lenna_512x512_24bpp",
     		"Lenna_512x512_24bpp",
// 			"Lenna_512x512_24bpp",
//     		"Lenna_512x512_24bpp",
// 			"Lenna_512x512_24bpp",
     };
     unsigned short int NB_ROWS[] = {
//     		1024,
 			512,
     		512,
    		512,
 			512,
     };
     unsigned short int NB_COLS[] = {
//      		1024,
  			512,
    		512,
    		512,
    		512,
//      		512,
//  			512,
 	   };

#endif


//    const char * name[] = {
//    		"Lenna_512x512_24bpp",
//			"Lenna_512x512_24bpp",
//    		"Lenna_512x512_24bpp",
//			"Lenna_512x512_24bpp",

//    		"Lamborghini_4096x4096_24bpp",
//			"Alps_4096x4096",
//			"BurningRose_4096x4096",
//			"ChildAndWolf_4096x4096",
//			"girl-woman-moon-beach-sunset-4096x4096",
//			"half-moon-stars-mountains-night-4096x4096",
//			"lake_4096x4096",
//			"nature_4096x4096",

//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"Lamborghini_4096x4096_24bpp",
//    		"airplain_512x512",
//    		"annie-spratt-3600x3600",
//    		"antonino-visalli-4096x2730",
////    		"Baboon_512x512",
////    		"barbara_720x576",
////    		"boats_784x574",
//    		"fons-heijnsbroek-4096x3072",
//    		"frymire_1120x1107",
////    		"goldhill_720x576",
//    		"jupiter_1392x1081",
//    		"Lamborghini_4096x4096_24bpp",
//    		"lazaro-rodriguez-2160x2700",
////    		"Lenna_512x512_24bpp",
//    		"Malmute_1616x1080",
////    		"Monarch_768x512",
////     		"peppers_512x512",
//    		"rainier_1920x1080",
//    		"sample_1920×1280",
//    		"saturn_1504x1203",
////    		"Serrano_640x608",
////    		"shubham-pawar-2488x2664",
//    		"sun_2096x2030",
//    		"sunrise_6000x2908",
////    		"tulips_768x512",
//    };
//    unsigned short int NB_ROWS[] = {
//    		512,
//			512,
//    		512,
//			512,
//
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,

//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		512,
//    		3600,
//    		2730,
////    		512,
////    		576,
////    		574,
//    		3072,
//    		1107,
////    		576,
//    		1081,
//    		4096,
//    		2700,
////    		512,
//    		1080,
////    		512,
////    		512,
//    		1080,
//    		1280,
//    		1203,
////    		608,
////    		2664,
//    		2030,
//    		2908,
////    		512,
//    };
//    unsigned short int NB_COLS[] = {
//    		512,
//			512,
//    		512,
//			512,
//
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,

//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		4096,
//    		512,
//    		3600,
//    		4096,
////    		512,
////    		720,
////    		784,
//    		4096,
//    		1120,
////    		720,
//    		1392,
//    		4096,
//    		2160,
////    		512,
//    		1616,
////    		768,
////    		512,
//    		1920,
//    		1920,
//    		1504,
////    		640,
//    		//2488,
//    		2096,
//    		6000,
////    		768,
//	   };

    unsigned int NbImages = (sizeof (name) / sizeof (const char *));

#define DATA_SIZE (IMAGE_ROWS*IMAGE_COLS*NB_REPLICAS_X*NB_REPLICAS_Y) // Number of pixels in the image
#define NB_PROCESSING_IMAGE 1  // number of images to be processed

#define Fck 100000000.0


timeval ts_global,te_global;
timeval ts_comp,te_comp,ts_write, te_write, ts_read, te_read;

size_t image_size_bytes = DATA_SIZE * ELEMENTS_PER_PIXEL;
size_t in_vector_size_bytes = sizeof(unsigned char) * ELEMENTS_PER_PIXEL * DATA_SIZE;
size_t out_vector_size_bytes = sizeof(unsigned char) * ELEMENTS_PER_PIXEL * DATA_SIZE;  //16 bit/component  out color image
size_t buffer_size_bytes = 4096*4096*3;
//size_t out_vector_size_bytes = DATA_SIZE*2;  //16 bit/component  out grey-level image
//size_t out_vector_size_bytes = ELEMENTS_PER_PIXEL*DATA_SIZE/2*3;  //12 bit/component color image
//size_t out_vector_size_bytes = DATA_SIZE/2*3;  //12 bit/component grey-level image

//std::vector<unsigned char, aligned_allocator<unsigned char> > source_in1(in_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > hw_results1(out_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > source_in2(in_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > hw_results2(out_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > source_in3(in_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > hw_results3(out_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > source_in4(in_vector_size_bytes);
//std::vector<unsigned char, aligned_allocator<unsigned char> > hw_results4(out_vector_size_bytes);

void outMsg(char msg[255])
{
	printf("%s\n",msg);
	fflush(stdout);
}
//La funzione restituisce l'header compatibile con zlib
ZlibHeader ZlibManager::GetZlibHeader()
{
    ZlibHeader h;

    h.cmf_cm=Z_DEFLATED;    //deflate
    h.cmf_cinfo=7;          //(CINFO=7 indicates a 32K window size)

    h.flg_fcheck=1;
    h.flg_fdict=0;          // no preset dictionary
    h.flg_flevel=COMPRESSION_LEVEL__FASTEST;    //fastest algorithm)

    return h;
}

//La funzione restituisce l'Adler32 del buffer passato
//uint32_t ZlibManager::GetAdler32(unsigned char *UncompressedBuf, uint64_t UncompressedBufferSize)
//{
//    //richiama la funzione adler32 della libreria zlib
//    uint64_t adler = adler32(0L, NULL, 0);
//
//    return adler32(adler, UncompressedBuf, UncompressedBufferSize);
//}

//La funzione salva su file i dati compressi aggiungendo l'header compatibile con zlib
bool ZlibManager::CreateZlibFile(std::string OutputFilePath, unsigned char *CompressedBuf, uint64_t CompressedBufferSize, uint32_t adler)
{
    ZlibHeader h;

    //calcola l'adler32
//    uint32_t adler = GetAdler32(UncompressedBuf, UncompressedBufferSize);


//    std::cout << "adler: " << adler << std::endl;

    //crea l'header
    h = GetZlibHeader();

//    uint32_t *ph=(uint16_t)&h;
//    std::cout << "header: " << *ph << std::endl;

    FILE *pFile;
    uint64_t result;
    bool res=true;

    //apre il file
    pFile = fopen ( OutputFilePath.c_str() , "wb" );
    if (pFile==NULL)
    {
        std::cout << "Error opening the Output file" << std::endl;
        res=false;
    }
    else
    {
//        std::cout << "file aperto" << std::endl;
        //scrive l'header
        result = fwrite((unsigned char*)&h , sizeof(char), sizeof(ZlibHeader), pFile);
        if (result != sizeof(ZlibHeader))
        {
            std::cout << "Writing header error" << std::endl;
            res=false;
        }
        else
        {
            //scrive i dati compressi
            result = fwrite(CompressedBuf , sizeof(char), CompressedBufferSize, pFile);
            if (result != CompressedBufferSize)
            {
                std::cout << "Writing data error" << std::endl;
                res=false;
            }
            else
            {
                //scrive l'adler32
                result = fwrite((unsigned char*)&adler , sizeof(char), sizeof(adler), pFile);
                if (result != sizeof(adler))
                {
                    std::cout << "Writing adler32 error" << std::endl;
                    res=false;
                }
            }
        }
        //chiude il file
        fclose(pFile);
    }
    return res;

}

double computeElapsed(timeval ts, timeval te)
{
     double retVal = 0.0;
     retVal = (te.tv_sec-ts.tv_sec)*1000000.0+(te.tv_usec-ts.tv_usec);
     return retVal;
}

/************ MAIN *****************/

int main(int argc, char** argv) {
	FILE *f;
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " NB_IMAGE_REPETITIONS <XCLBIN File> NODE_COORD" << std::endl;
        return EXIT_FAILURE;
    }
    unsigned int NumberOfImageRepetitions = atoi(argv[1]);
    unsigned int coord = atoi(argv[3]);
    unsigned int packet_size = 256;

    in_vector_size_bytes = NumberOfImageRepetitions*image_size_bytes;
    out_vector_size_bytes = NumberOfImageRepetitions*image_size_bytes;

    std::string xclbinFilename = argv[2];
    
    
    //unsigned int ImgSize = DATA_SIZE;
    unsigned short int ImgRows, ImgCols;

    pthread_t thread_registers;
    
   unsigned ndevices = xrt::system::enumerate_devices();
	if (ndevices == 0) {
		std::cerr << "no device found\n";
		exit(EXIT_FAILURE);
	}

	xrt::device device;
	bool device_found = false;
	for (unsigned d=0; d<ndevices; ++d) {
		device = xrt::device(d);
		if (device.get_info<xrt::info::device::name>() == "xilinx_u200_gen3x16_xdma_base_2") { // TODO: board name as command line argument
			device_found = true;
			break;
		}
	}

	if (!device_found) {
		std::cerr << "Device xilinx_u200_gen3x16_xdma_base_2 not found\n";
		exit(EXIT_FAILURE);
	}
    
    
    
    
    //    xrt::device device(0);
    std::cout << "Device name: " << device.get_info<xrt::info::device::name>() << std::endl;
    std::cout << "Device bdf: " << device.get_info<xrt::info::device::bdf>() << std::endl;
    std::cout << "Device max freq: " << device.get_info<xrt::info::device::max_clock_frequency_mhz>() << std::endl;
    std::cout << "Device m2m: " << device.get_info<xrt::info::device::m2m>() << std::endl;
    std::cout << "Device nodma: " << device.get_info<xrt::info::device::nodma>() << std::endl;
    std::cout << "Device kdma: " << device.get_info<xrt::info::device::kdma>() << "\n\n";

    xrt::uuid uuid = device.load_xclbin(xclbinFilename); //bitstream flashed

    /*std::vector<cl::Device> devices;
    cl_int err;
    cl::Context context;
    cl::CommandQueue q;
    cl::Program program;
    std::vector<cl::Platform> platforms;
    bool found_device = false;*/

    
    std::cout<<xclbinFilename<<std::endl;
    //xrt::kernel krnl_Memory2RGB_1 = xrt::kernel(device, uuid, "Memory2RGB:{Memory2RGB_1}");
    xrt::kernel krnl_ImgProc_1 = xrt::kernel(device, uuid, "ImgProc:{ImgProc_1}");
    //xrt::kernel krnl_ImgProc_2 = xrt::kernel(device, uuid, "ImgProc:{ImgProc_2}");
    //xrt::kernel krnl_ImgProc_3 = xrt::kernel(device, uuid, "ImgProc:{ImgProc_3}");
    //xrt::kernel krnl_RGB2Memory_1 = xrt::kernel(device, uuid, "RGB2Memory:{RGB2Memory_1}");
    
    xrt::ip kswitch(device,uuid,"TextaRossa_switch");
    kswitch_thread = kswitch; //ip handle for parallel thread 
    
    kswitch.write_register(4*4, 0x1);
    if(coord!=2) kswitch.write_register(8*4,0xfffffc4);
    printf("Resetting switch\n");
    sleep(1);
    kswitch.write_register(6*4,coord);
    kswitch.write_register(69*4, 0x01800060);
    kswitch.write_register(70*4, 0x0000ff40);
    
    double elapsed_global, elapsed_computation, elapsed_read, elapsed_write;


  
    ImgRows = NB_ROWS[0];
    ImgCols = NB_COLS[0];
    unsigned ImgSize = ImgRows*ImgCols;
	
	    
/*	xrt::bo buffer_in_0(device, in_vector_size_bytes, krnl_Memory2RGB_1.group_id(0));
	void *buffer_in_0_map = buffer_in_0.map<void *>();
	memset(buffer_in_0_map, 0, in_vector_size_bytes);
	
	xrt::bo buffer_out_0(device, out_vector_size_bytes, krnl_RGB2Memory_1.group_id(0));
	void *buffer_out_0_map = buffer_out_0.map<void*>();
	memset(buffer_out_0_map, 0, out_vector_size_bytes);	
	
	xrt::bo buffer_in_1(device, in_vector_size_bytes, krnl_Memory2RGB_1.group_id(0));
	void *buffer_in_1_map = buffer_in_1.map<void *>();
	memset(buffer_in_1_map, 0, in_vector_size_bytes);
	
	xrt::bo buffer_out_1(device, out_vector_size_bytes, krnl_RGB2Memory_1.group_id(0));
	void *buffer_out_1_map = buffer_out_1.map<void*>();
	memset(buffer_out_1_map, 0, out_vector_size_bytes);	

	//	for (unsigned  int i = 0; i<NbImages; i++)
		for (unsigned  int i = 0; i<1; i++)
		{
			sprintf(filename,"%s%s.%s",path, name[i],extension);
			f = fopen((const char *)filename,"rb");
			if (f==NULL)
			{
				printf("error opening input file %s\n", filename);
				exit(-1);
			}
			fseek(f, 0L, SEEK_END);
			image_size_bytes = ftell(f);
			rewind(f);

			if (i==0)
				fread(buffer_in_0_map,1,image_size_bytes,f);
			else
				fread(buffer_in_1_map,1,image_size_bytes,f);
			fclose(f);
			printf("Read %s\n",name[i]);
		}

		for (unsigned int j=1; j<NumberOfImageRepetitions; j++)
			memcpy(buffer_in_0_map+(j*image_size_bytes),buffer_in_0_map,image_size_bytes);
		//memcpy(buffer_in_1_map,buffer_in_0_map,in_vector_size_bytes);
*/

    //ImgSize = image_size_bytes;
    

	elapsed_global = 0.0;
	elapsed_computation = 0.0;
	elapsed_read = 0.0;
	elapsed_write = 0.0;

	double NOps = 0.0;
	double globalInputSize = 0;
	unsigned int globalOutputSize = 0;
	int flag = 1;

	int mul_factor = 1;

    




 	//xrt::run krnl_Memory2RGB_1_run = xrt::run(krnl_Memory2RGB_1);
	xrt::run krnl_ImgProc_1_run = xrt::run(krnl_ImgProc_1);
	//xrt::run krnl_ImgProc_2_run = xrt::run(krnl_ImgProc_2);
	//xrt::run krnl_ImgProc_3_run = xrt::run(krnl_ImgProc_3);
	//xrt::run krnl_RGB2Memory_1_run = xrt::run(krnl_RGB2Memory_1);


	krnl_ImgProc_1_run.set_arg(6, ImgSize);
	krnl_ImgProc_1_run.set_arg(7, NumberOfImageRepetitions);
	krnl_ImgProc_1_run.set_arg(8, ImgRows);
	krnl_ImgProc_1_run.set_arg(9, ImgCols);
	krnl_ImgProc_1_run.set_arg(10, coord-1);
	krnl_ImgProc_1_run.set_arg(11, packet_size);
	
	/*krnl_ImgProc_2_run.set_arg(6, ImgSize);
	krnl_ImgProc_2_run.set_arg(7, NumberOfImageRepetitions);
	krnl_ImgProc_2_run.set_arg(8, ImgRows);
	krnl_ImgProc_2_run.set_arg(9, ImgCols);
	krnl_ImgProc_2_run.set_arg(10, coord-1);
*/
	
	std::cout<<"CHANNEL RGB: "<<coord-1<<std::endl;
	

////////////////////////////////////////////////////////////////////////////////////


	printf("press any key to continue...");
	getchar();
	printf("done\n");
	
	int s = pthread_create(&thread_registers, NULL, thr_func, NULL);
		if(s){
			std::cout<<"Error in thread creation"<<std::endl;
			return EXIT_FAILURE;
		}
	
 //   gettimeofday(&ts_global,NULL);

// pipeline preamble - first stage  (flag = 0)
  //  outMsg("pipe 1st stage");
gettimeofday(&ts_global,NULL);
			//buffer_in_0.sync(XCL_BO_SYNC_BO_TO_DEVICE);
														
			//outMsg("migrate buf 0 - end 1st stage");
			globalInputSize += image_size_bytes*NumberOfImageRepetitions;

// pipeline preamble - second stage  (flag = 1)
			//outMsg("pipe 2nd stage");
														
			//outMsg("migrate buf 1");

			//krnl_Memory2RGB_1_run.set_arg(0, buffer_in_0);
			//krnl_RGB2Memory_1_run.set_arg(0, buffer_out_0);
			//outMsg("set kernel arguments - 0");

//			std::vector<cl::Event> waitList;
//			waitList.push_back(write_event[0]);  //before start processing, wait for the end of the previous in communication
			
			//krnl_RGB2Memory_1_run.start();
			krnl_ImgProc_1_run.start();
			//krnl_ImgProc_2_run.start();
			//krnl_ImgProc_3_run.start();
			//krnl_Memory2RGB_1_run.start();
			

			
			
			//outMsg("kernel started");

		NOps += (NumberOfImageRepetitions * ((488*(double)((ImgRows)*(ImgCols))-2494*(double)(ImgRows+ImgCols)+14296)))*ELEMENTS_PER_PIXEL*mul_factor;  // processing two images

			//krnl_Memory2RGB_1_run.wait();
			krnl_ImgProc_1_run.wait();
			//krnl_ImgProc_2_run.wait();
			//krnl_ImgProc_3_run.wait();
			//krnl_RGB2Memory_1_run.wait();
			
			
		//outMsg("second stage ended");
		globalInputSize += image_size_bytes*NumberOfImageRepetitions;

// pipeline steady state



	//NOps += (NumberOfImageRepetitions * ((488*(double)((ImgRows)*(ImgCols))-2494*(double)(ImgRows+ImgCols)+14296)))*ELEMENTS_PER_PIXEL*mul_factor;  // processing two images

	//outMsg("postamble 1st stage - ended");

	// pipeline postamble - last stage
gettimeofday(&te_global,NULL);
		// read results from output buffers
		//buffer_out_0.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
		//outMsg("read results - 0");
   
//gettimeofday(&te_global,NULL);
	
//	save one of the processed images to file

    printf("computation ended\n");
    
  
	// median filter 5x5 filter : 138*(ImgRows-4)*(ImgCols-4) ops (comparisons)
	// doNegative : (ImgRows*ImgCols)
	// sum : 2*(ImgRows*ImgCols)  (1 add and 1 compare -> diff)

	elapsed_global = computeElapsed(ts_global, te_global);

	printf("press any key to continue...");
	getchar();
	printf("done\n");

	printf("Processed %d (%dx%d) images \n",NumberOfImageRepetitions*NbImages,ImgRows, ImgCols);
    printf("Image Size = %d pixels\n",ImgRows*ImgCols);
    printf("Overall processing elapsed time : %lf us\n",elapsed_global);
    printf("Write BW : %lf MB/sec\n",((double)globalInputSize)/elapsed_global*(1e6/(1024.0*1024.0)));
//    printf("Read BW : %lf MB/sec\n",((double)globalOutputSize)/elapsed_global*(1e6/(1024.0*1024.0)));
    printf("Nb clock cycles to process the images : %f\n",(elapsed_global)/1000000.0*Fck);
    printf("Nb pixels/cycle processed : %f\n",(((float)globalInputSize))/((elapsed_global)/1000000.0*Fck));
    printf("Number of operations performed: %f\n",NOps);
    printf("Number of operations/cycle performed: %f\n",NOps/((elapsed_global)/1000000.0*Fck));
    printf("sustained speed : %f GOp/s\n",(NOps/(elapsed_global/1.0e6)/(1024.0*1024.0*1024.0)));

//    free(InputImage);
//    free(tmpImage);

    std::cout << "PROCESSING IMAGE FINISHED" << std::endl;
    
    kill = true;
    pthread_join(thread_registers, NULL);
    
    return (0);
}

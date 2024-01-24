#include "type_definitions.h"

#define SIZE (8)  //size of one image component (in bit)
#define SIZEx2 (SIZE*2)
#define SIZEx4 (SIZE*4)
#define MAX_OUT_VALUE (255)  // max value that can be represented with the bits used by the output image
#define MAX_Y 256 //corresponds to 8 bit/component
#define F_MAX_Y 256.0f //corresponds to 8 bit/component

#define N_OUTPUT_CHANNELS 3

namespace memory2RGB {
#include "utility.cpp"
//#include "setCoefficients.h"
#include "streams.cpp"
//#include "filters.h"
//#include "colorSpaceConversion.h"
//#include "pixelBasedImageOperations.h"
#include "manageRGBchannels.cpp"
//#include "morphologicalOperators.h"
//#include "medianFilter.h"
//#include "contrastManagement.h"
//#include "resampleImage.h"
#include "buffering.cpp"
//#include "harris.h"
//#include "rotateImage.h"
//#include "masking.h"
#include "dataMover.cpp"
}



void Memory2RGB_setRGBImgSize(unsigned int ImgSize, unsigned int &RGBImgSize)
{
	RGBImgSize = 3*ImgSize;
}
/*
void buffOut(hls::stream<dt16> &sR, hls::stream<dt16> &sG, hls::stream<dt16> &sB, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages){

	using namespace memory2RGB;
#pragma HLS unroll
	bufferedCommunication<dt16,8,32>(sR, message_data_out,  ImgSize, NbImages, 0);
	bufferedCommunication<dt16,2,32>(sG, message_data_out,  ImgSize, NbImages, 1);
	bufferedCommunication<dt16,2,32>(sB, message_data_out,  ImgSize, NbImages, 2);

}
*/
extern "C" {

void Memory2RGB(dt16* in,
		 //  hls::stream<io_stream_16B>  &osR,
		 //  hls::stream<io_stream_16B>  &osG,
		 //  hls::stream<io_stream_16B>  &osB,
		 message_stream_t message_data_out[N_OUTPUT_CHANNELS],
		   unsigned int ImgSize,
		   unsigned int NbImages,
		   unsigned int mode,
		   unsigned int dest_coord,
		   unsigned int packet_size)
{
#pragma HLS INTERFACE m_axi port = in bundle = gmem0
#pragma HLS INTERFACE axis port=message_data_out



    static hls::stream<dt16> s1("s1");
    static hls::stream<dt16> s2("s2");
    static hls::stream<dt16> s3("s3");
    static hls::stream<dt16> s4("s4");

#pragma HLS STREAM variable=s1 depth=2048 type=fifo
#pragma HLS STREAM variable=s2 depth=2048 type=fifo
#pragma HLS STREAM variable=s3 depth=2048 type=fifo
#pragma HLS STREAM variable=s4 depth=2048 type=fifo



    unsigned int RGBImgSize;

    using namespace memory2RGB;

	unsigned int id;
#pragma HLS dataflow

	setStreamId(id);
	Memory2RGB_setRGBImgSize(ImgSize, RGBImgSize);
    memory2Stream<dt16,SIZE,Nx2>(in,s1,RGBImgSize, NbImages);
    
    splitRGBChannels(s1, s2, s3, s4, ImgSize, NbImages);

//    buffOut(s2, s3, s4, message_data_out, ImgSize, NbImages);
    
/*    bufferedCommunication<dt16,2,32>(s2, message_data_out, ImgSize, NbImages, 0);
    bufferedCommunication<dt16,2,32>(s3, message_data_out, ImgSize, NbImages, 1);
    bufferedCommunication<dt16,2,32>(s4, message_data_out, ImgSize, NbImages, 2);
*/
	if(mode==0) bufferedCommunicationAll<dt16, 8, 128>(s2, s3, s4, message_data_out, ImgSize, NbImages, packet_size);
	else if(mode==1) bufferedCommunicationAll_multiDevice<dt16, 8, 128>(s2, s3, s4, message_data_out, ImgSize, NbImages, packet_size);
	else bufferedCommunicationAll_multiDevice<dt16, 8, 128>(s2, s3, s4, message_data_out, ImgSize, NbImages, dest_coord, packet_size);
//    convertToAxiStream(s2, osR, ImgSize, NbImages);4
//    convertToAxiStream(s3, osG, ImgSize, NbImages);
//    convertToAxiStream(s4, osB, ImgSize, NbImages);

}
}

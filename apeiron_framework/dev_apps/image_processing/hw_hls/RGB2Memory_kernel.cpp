#include "type_definitions.h"

#define SIZE (8)  //size of one image component (in bit)
#define SIZEx2 (SIZE*2)
#define SIZEx4 (SIZE*4)
#define MAX_OUT_VALUE (255)  // max value that can be represented with the bits used by the output image
#define MAX_Y 256 //corresponds to 8 bit/component
#define F_MAX_Y 256.0f //corresponds to 8 bit/component

#define N_INPUT_CHANNELS 3

namespace rgb2Memory {
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

void RGB2Memory_setRGBImgSize(unsigned int ImgSize, unsigned int &RGBImgSize)
{
	RGBImgSize = 3*ImgSize;
}
/*
void buffIn(message_stream_t message_data_in[N_INPUT_CHANNELS], hls::stream<dt16> &sR, hls::stream<dt16> &sG, hls::stream<dt16> &sB, unsigned int ImgSize, unsigned int NbImages){

	using namespace rgb2Memory;
#pragma HLS unroll
	bufferedCommunication<dt16,2,32>(message_data_in, sR, ImgSize, NbImages, 0);
	bufferedCommunication<dt16,2,32>(message_data_in, sG, ImgSize, NbImages, 1);
	bufferedCommunication<dt16,2,32>(message_data_in, sB, ImgSize, NbImages, 2);

}
*/

extern "C" {




void RGB2Memory(dt16* out,
		//   hls::stream<io_stream_16B>  &isR,
		//   hls::stream<io_stream_16B>  &isG,
		//   hls::stream<io_stream_16B>  &isB,
		message_stream_t message_data_in[N_INPUT_CHANNELS],
		   unsigned int ImgSize,
		   unsigned int NbImages,
		   unsigned int packet_size)
{
#pragma HLS INTERFACE m_axi port = out bundle = gmem1
#pragma HLS INTERFACE axis port=message_data_in




	static hls::stream<dt16> sout("sout");
	static hls::stream<dt16> sR("sR");
	static hls::stream<dt16> sG("sG");
	static hls::stream<dt16> sB("sB");

#pragma HLS STREAM variable=sout depth=4096 type=fifo
#pragma HLS STREAM variable=sR depth=4096 type=fifo
#pragma HLS STREAM variable=sG depth=4096 type=fifo
#pragma HLS STREAM variable=sB depth=4096 type=fifo

	unsigned int RGBImgSize;

	using namespace rgb2Memory;
#pragma HLS dataflow
	RGB2Memory_setRGBImgSize(ImgSize, RGBImgSize);
//	convertFromAxiStream(isR, sR, ImgSize, NbImages);
//	convertFromAxiStream(isG, sG, ImgSize, NbImages);
//	convertFromAxiStream(isB, sB, ImgSize, NbImages);

//	buffIn(message_data_in, sR, sG, sB, ImgSize, NbImages);
/*	bufferedCommunication<dt16,8,128>(message_data_in, sR, ImgSize, NbImages, 0);
	bufferedCommunication<dt16,8,128>(message_data_in, sG, ImgSize, NbImages, 1);
	bufferedCommunication<dt16,8,128>(message_data_in, sB, ImgSize, NbImages, 2);
*/

	bufferedCommunicationAll<dt16,8,128>(message_data_in, sR, sG, sB, ImgSize, NbImages, packet_size);


	mergeRGBChannels(sR, sG, sB, sout, ImgSize, NbImages);
	stream2Memory<dt16,SIZE,Nx2>(out, sout, RGBImgSize, NbImages);
}

}

/*
 * In this design we instantiate one kernel which, in a dataflow modality, executes two times the edge detect section and
 * feeding two different instances of the enhance contrast function (due to its store&forward behaviour, this kernel lasts
 * twice the time of the edge detect and filtering previous functions
 */

// Includes
#include "type_definitions.h"

#define N_INPUT_CHANNELS 3
#define N_OUTPUT_CHANNELS 3

namespace imgProc{

#include "utility.cpp"
#include "setCoefficients.cpp"
#include "streams.cpp"
#include "filters.cpp"
//#include "colorSpaceConversion.h"
#include "pixelBasedImageOperations.cpp"
//#include "manageRGBchannels.h"
//#include "morphologicalOperators.h"
#include "medianFilter.cpp"
//#include "contrastManagement.h"
//#include "resampleImage.h"
#include "buffering.cpp"
//#include "harris.h"
//#include "rotateImage.h"
//#include "masking.h"
//#include "dataMover.h"
}

void convertImgSize(unsigned int ImgSize, unsigned short int ImgRows, unsigned short int ImgCols, unsigned int &RGBImgSize, unsigned int &ImgSizex2, unsigned int &upsampledImgSize, unsigned int &upsampledRGBImgSize, unsigned short int &upsampledImgRows, unsigned short int &upsampledImgCols, unsigned short int r, unsigned short int &r2, unsigned short int &r4, unsigned short int x0, unsigned short int &vx0, unsigned short int y0, unsigned short int &vy0, unsigned int &bufferSize)
{
	RGBImgSize = 3*ImgSize;
	ImgSizex2 = 2*ImgSize;
	upsampledImgSize = ImgSize*4;
	upsampledRGBImgSize = 3 *upsampledImgSize;
	upsampledImgRows = ImgRows*2;
	upsampledImgCols = ImgCols*2;
	r2 = r*2;
	r4 = r*4;
	vx0 = x0-r;
	vy0 = y0-r;
	bufferSize = ImgSize-2*ImgCols;
}

void setNbImages(unsigned int n, unsigned int &nA, unsigned int &nB)
{
	if ((n&1) == 0)  //n is even
	{
		nA = n>>1;   // the two "long" functions perform the same number of iterations
		nB = n>>1;
	}
	else // n is odd
	{
		nA = (n>>1)+1;  // the first of the two "long" functions performs one additional iteration
		nB = n>>1;
	}
}
/*
 * ImgSize is the number of components in the image. If the image has Nrow rows, Ncolumn columns and Ncomponent components/pixels,
 * ImgSize = Nrow x Ncolumn x Ncomponent
 * In a RGB image ImgSize = Nrow x Ncolumn x 3
 * In a GreyLevel image ImgSize = Nrow x Ncolumn
 */



extern "C" {

void ImgProc(//hls::stream<io_stream_16B>  &s_in,
		//	 hls::stream<io_stream_16B>  &s_out,
		message_stream_t message_data_in[N_INPUT_CHANNELS],
		message_stream_t message_data_out[N_OUTPUT_CHANNELS],
			 unsigned int ImgSize,
			 unsigned int NbImages,
			 unsigned short int ImgRows,
			 unsigned short int ImgCols,
			 unsigned int ch_id,
			 unsigned int packet_size)
{
#pragma HLS INTERFACE axis  port=message_data_in
#pragma HLS INTERFACE axis  port=message_data_out

	static hls::stream<dt16> s1a("s1a");
#pragma HLS STREAM variable=s1a depth=1024 type=fifo


	
	static hls::stream<dt8> s1("s1");
	static hls::stream<dt8> s2("s2");
	static hls::stream<dt8> s3("s3");
	static hls::stream<dt8> s4("s4");
	static hls::stream<dt8> s5("s5");
	static hls::stream<dt8> s6("s6");
	static hls::stream<dt8> s5a("s5a");
	static hls::stream<dt8> s6a("s6a");
	static hls::stream<dt8> s7("s7");
	static hls::stream<dt8> s8("s8");
	static hls::stream<dt8> s9("s9");
	static hls::stream<dt8> s10("s10");
	static hls::stream<dt16> s10a("s10a");
	
	
#pragma HLS STREAM variable=s1 depth=512 type=fifo
#pragma HLS STREAM variable=s2 depth=512 type=fifo


#pragma HLS STREAM variable = s3 depth = (4096/8 * 1024) type=fifo

//#pragma HLS STREAM variable=s3 depth=512 type=fifo
#pragma HLS STREAM variable=s4 depth=512 type=fifo
#pragma HLS STREAM variable=s5 depth=512 type=fifo
#pragma HLS STREAM variable=s6 depth=512 type=fifo
#pragma HLS STREAM variable=s7 depth=512 type=fifo
#pragma HLS STREAM variable=s8 depth=512 type=fifo
#pragma HLS STREAM variable=s9 depth=512 type=fifo
#pragma HLS STREAM variable=s10 depth=512 type=fifo

#pragma HLS STREAM variable=s10a depth=1024 type=fifo


//#pragma HLS STREAM variable = s3 depth = (4096/8 * 16)

	short int c00, c01, c02, c03, c04, c05, c06, c10, c11, c12, c13, c14, c15, c16, c20, c21, c22, c23, c24, c25, c26, c30, c31, c32, c33, c34, c35, c36, c40, c41, c42, c43, c44, c45, c46, c50, c51, c52, c53, c54, c55, c56, c60, c61, c62, c63, c64, c65, c66;
	short int c00_1, c01_1, c02_1, c03_1, c04_1, c10_1, c11_1, c12_1, c13_1, c14_1, c20_1, c21_1, c22_1, c23_1, c24_1, c30_1, c31_1, c32_1, c33_1, c34_1, c40_1, c41_1, c42_1, c43_1, c44_1;

	unsigned int RGBImgSize;
	using namespace imgProc;
	unsigned int id;
#pragma HLS dataflow
	set7x7GaussianFilterWeights_s_eq_2(c00, c01, c02, c03, c04, c05, c06, c10, c11, c12, c13, c14, c15, c16, c20, c21, c22, c23, c24, c25, c26, c30, c31, c32, c33, c34, c35, c36, c40, c41, c42, c43, c44, c45, c46, c50, c51, c52, c53, c54, c55, c56, c60, c61, c62, c63, c64, c65, c66);
	set5x5GaussianFilterWeights_s_eq_2(c00_1, c01_1, c02_1, c03_1, c04_1, c10_1, c11_1, c12_1, c13_1, c14_1, c20_1, c21_1, c22_1, c23_1, c24_1, c30_1, c31_1, c32_1, c33_1, c34_1, c40_1, c41_1, c42_1, c43_1, c44_1);


	bufferedCommunication<dt16,8,128>(message_data_in, s1a, ImgSize, NbImages, ch_id, packet_size);
	halveStreamWidth(s1a, s1, ImgSize, NbImages);

//	PassThrough<dt8, SIZE, N>(s1, s2, ImgSize, NbImages);
    do_5x5_median_filtering<dt8, SIZE, N>(s1, s2, ImgRows, ImgCols, NbImages);
    streamCopy<dt8, SIZE, N>(s2, s3, s4, ImgSize, NbImages);
   //streamCopy<dt8, SIZE, N>(s1, s3, s4, ImgSize, NbImages);
   
   
    do_7x7_filtering<dt8, SIZE, N>(s4, s5, ImgRows, ImgCols, c00, c01, c02, c03, c04, c05, c06, c10, c11, c12, c13, c14, c15, c16, c20, c21, c22, c23, c24, c25, c26, c30, c31, c32, c33, c34, c35, c36, c40, c41, c42, c43, c44, c45, c46, c50, c51, c52, c53, c54, c55, c56, c60, c61, c62, c63, c64, c65, c66, true, NbImages);
    PassThrough<dt8, SIZE, N>(s5, s5a, ImgSize, NbImages);
    do_7x7_filtering<dt8, SIZE, N>(s5a, s6, ImgRows, ImgCols, c00, c01, c02, c03, c04, c05, c06, c10, c11, c12, c13, c14, c15, c16, c20, c21, c22, c23, c24, c25, c26, c30, c31, c32, c33, c34, c35, c36, c40, c41, c42, c43, c44, c45, c46, c50, c51, c52, c53, c54, c55, c56, c60, c61, c62, c63, c64, c65, c66, true, NbImages);
   
    PassThrough<dt8, SIZE, N>(s6, s6a, ImgSize, NbImages);
    
//     PassThrough<dt8, SIZE, N>(s1, s10, ImgSize, NbImages);
  
    do_7x7_filtering<dt8, SIZE, N>(s6a, s7, ImgRows, ImgCols, c00, c01, c02, c03, c04, c05, c06, c10, c11, c12, c13, c14, c15, c16, c20, c21, c22, c23, c24, c25, c26, c30, c31, c32, c33, c34, c35, c36, c40, c41, c42, c43, c44, c45, c46, c50, c51, c52, c53, c54, c55, c56, c60, c61, c62, c63, c64, c65, c66, true, NbImages);
    diffImages<dt8, SIZE, N>(s7, s3, s8, ImgSize, NbImages);
    doNegative<dt8, SIZE, N>(s8, s9, ImgSize, NbImages);
    do_5x5_filtering<dt8, SIZE, N>(s9, s10, ImgRows, ImgCols, c00_1, c01_1, c02_1, c03_1, c04_1, c10_1, c11_1, c12_1, c13_1, c14_1, c20_1, c21_1, c22_1, c23_1, c24_1, c30_1, c31_1, c32_1, c33_1, c34_1, c40_1, c41_1, c42_1, c43_1, c44_1, true, NbImages);

	doubleStreamWidth(s10, s10a, ImgSize, NbImages);
	bufferedCommunication<dt16,8,128>(s10a, message_data_out, ImgSize, NbImages, 0, ch_id, packet_size);
}
}

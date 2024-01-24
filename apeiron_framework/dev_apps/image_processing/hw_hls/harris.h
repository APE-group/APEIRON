/*
 * the mulAndSquareImages function takes as input two images I1 and I2 and produces I1^2, I2^2 and I1*I2
 * all the image product are considered element by element
 * in the current implementation, Tout has twice the size of T
 */
template <typename T, typename Tout, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void mulAndSquareImages(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<Tout>& out_I1square, hls::stream<Tout>& out_I1I2, hls::stream<Tout>& out_I2square,  unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2;
	Tout res1, res12, res2;
	short int d1, d2, r;
	int aux1, aux2, aux12;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();
			tmp2 = in_stream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				aux1 = d1*d1;
				aux12 = d1*d2;
				aux2 = d2*d2;
				if (aux1>(256*256-1))
					aux1 = (256*256-1);
				res1.range(2*S*(j+1)-1,2*S*j) = aux1;

				if (aux2>(256*256-1))
					aux2 = (256*256-1);
				res2.range(2*S*(j+1)-1,2*S*j) = aux2;

				if (aux12>(256*256-1))
					aux12 = (256*256-1);
				res12.range(2*S*(j+1)-1,2*S*j) = aux12;
			 }
			out_I1square.write(res1);
			out_I2square.write(res2);
			out_I1I2.write(res12);
		}
	}
}

/*
 * the function receives as input a streamed sequence of triplets <I1square, I1I2, I2square> that form the matrix
 *
 *     | I1square   I1I2     |
 * M = |                     |
 *     | I1I2       I2square |
 *
 * with
 *       Det(M) = D = I1square*I2square-I1I2*I1I2
 *       trace(M) = T = I1square+I2square
 *
 * It is compute the corner response coefficient R = D -k*T^2, being k = 0.04 - 0.06
 *
 * R < -10000 => the corresponding pixel of the input image is  and edge pixel . Assign to it the value out = 64;
 * -10000 < R < 10000  =>  the corresponding pixel of the input image is neither and edge nor a corner pixel . Assign to it the value out = 0;
 * R > 10000 =>  the corresponding pixel of the input image is a corner pixel . Assign to it the value out = 255;
 */

// the function as been thought with T size S = 16 and Tout size = 8 bit
template <typename T, typename Tout, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void determineCornerAndEdges(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream12, hls::stream<T>& in_stream2,  hls::stream<Tout>& out_Img, int RThreshold, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2, tmp12;
	Tout res;
	ap_uint<S/2> outVal;
	int d1, d2, d12, r, aux1, aux2, aux12;
	int Det, Trace;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();  //components of I1square
			tmp2 = in_stream2.read();  //components of I2square
			tmp12 = in_stream12.read();  //components of I1I2

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				d12 = (tmp12.range(S * (j + 1) - 1, S * j).to_int());

				Det = d1*d2-d12*d12;
				Trace = d1 +d2;
//				r = Det - ((Trace*Trace)>>5) - ((Trace*Trace)>>6) ;// Trace^2/32 + Trace^2/64 is nearly equal to Trace^2 * 0.047
				r = Det - (Trace*Trace)/20;

//				if (r> RThreshold)
//					outVal = 255;
//				else
//					outVal = 0;
				if (r < 0)
					outVal = 64;
				else if (r> RThreshold)
					outVal = 255;
				else
					outVal = 0;
				res.range((S/2)*(j+1)-1,(S/2)*j) = outVal;
			 }
			out_Img.write(res);
		}
	}
}

// the function as been thought with T size S = 16 and Tout size = 32 bit
template <typename T, int S, int W, typename Tout, int Sout>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void computeR_HarrisCornerDetection(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<T>& in_stream12,  hls::stream<Tout>& out_R, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2, tmp12;
	Tout res;
	int d1, d2, d12, r, aux1, aux2, aux12;
	unsigned int Det, Trace;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();  //components of I1square
			tmp2 = in_stream2.read();  //components of I2square
			tmp12 = in_stream12.read();  //components of I1I2

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				d12 = (tmp12.range(S * (j + 1) - 1, S * j).to_int());

				Det = d1*d2-d12*d12;
				Trace = d1 +d2;
				r = Det - ((Trace*Trace)>>5) - ((Trace*Trace)>>6) ;// Trace^2/32 + Trace^2/64 is nearly equal to Trace^2 * 0.047
				res.range((Sout)*(j+1)-1,(Sout)*j) = r;
			 }
			out_R.write(res);
		}
	}
}


template <typename T>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
T findMax(T d1, T d2, T d3, T d4, T d5, T d6, T d7, T d8, T d9)
{

#pragma HLS pipeline
	return maxVal<T>(maxVal<T>( maxVal<T>(maxVal<T>(d1,d2),maxVal<T>(d3,d4)), maxVal<T>(maxVal<T>(d5,d6),maxVal<T>(d7,d8))) , d9);
}

// for the time being, S=32  - read R values (int). Element i of outline is the max of the 3x3 neighborood centered in i
template <typename T, int S, int W, typename Tout, int Sout, int Wout>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void findMax_one_line_3x3(T *inline1, T *inline2, T *inline3, Tout *outline,
						int RThreshold,
						unsigned short int size)
{
	T din1, din2, din3, dout;
	int di00, di01, di02, di10, di11, di12, di20, di21, di22;
	int int_dout;
	int i,j;
	ap_int<Sout> aux;

	ap_uint<S> max_value;

	for (i=0; i<size; i++)

	{
#pragma HLS pipeline

		din1 = inline1[i];
		din2 = inline2[i];
		din3 = inline3[i];

		if (i==0)
		{
			di00 = (din1.range(S-1, 0).to_int());
			di10 = (din2.range(S-1, 0).to_int());
			di20 = (din3.range(S-1, 0).to_int());

			di01 = (din1.range(2*S-1, S).to_int());
			di11 = (din2.range(2*S-1, S).to_int());
			di21 = (din3.range(2*S-1, S).to_int());

//			dout.range(Sout-1,0) = din2.range(Sout-1,0);
			dout.range(Sout-1,0) = 0;

			for (int j=2; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di02 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di12 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di22 = (din3.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = findMax<int>(di00, di01, di02, di10, di11, di12, di20, di21, di22); //the result is divided by 1024
				if (int_dout < 0)
				{
					aux = 64;
				}
				else if (int_dout > RThreshold)
					aux = 255;
				else
					aux = 0;
				dout.range(Sout*j-1,Sout*(j-1)) = aux;
				di00 = di01;
				di10 = di11;
				di20 = di21;

				di01 = di02;
				di11 = di12;
				di21 = di22;

			 } // processed the PIXEL_IN_INPUT_WORD leftmost input pixels
		}
		else
		{
			// processing the last output pixel of previous word
			di02 = (din1.range(S-1,0).to_int());
			di12 = (din2.range(S-1,0).to_int());
			di22 = (din3.range(S-1,0).to_int());

			int_dout = findMax<int>(di00, di01, di02, di10, di11, di12, di20, di21, di22); //the result is divided by 1024
			if (int_dout < 0)
			{
				aux = 64;
			}
			else if (int_dout > RThreshold)
				aux = 255;
			else
				aux = 0;

			dout.range(Wout-1, Wout-Sout) = aux; //ap_uint < 16 > ((int)fdout);
			outline[i-1] = dout;

			di00 = di01;
			di10 = di11;
			di20 = di21;

			di01 = di02;
			di11 = di12;
			di21 = di22;

			for (int j=1; j<(W/S); j++)
		     {
		#pragma HLS unroll

				di02 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di12 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di22 = (din3.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = findMax<int>(di00, di01, di02, di10, di11, di12, di20, di21, di22); //the result is divided by 1024
				if (int_dout < 0)
				{
					aux = 64;
				}
				else if (int_dout > RThreshold)
					aux = 255;
				else
					aux = 0;
				dout.range(Sout*j-1,Sout*(j-1)) = aux;
				di00 = di01;
				di10 = di11;
				di20 = di21;

				di01 = di02;
				di11 = di12;
				di21 = di22;

		     }
		}  //end else (i==0)
	}  // end for i, the one scanning the entire line
	// the last output pixel of the row is equal to the input pixel
	dout.range(Wout-1, Wout-Sout) = din2.range(Wout-1, Wout-Sout);
	outline[i-1] = dout;
}

//findCornerAndEdges_3x3<dt64, SIZEx4, Nx4, dt16, SIZE, N>(s12, s13, ImgRows, ImgCols, RThreshold, NbImages);

template <typename T, int S, int W, typename Tout, int Sout, int Wout>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void findCornerAndEdges_3x3(hls::stream<T>& inStream,hls::stream<Tout>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  int RThreshold,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32
	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	Tout lineout1[MAX_COMPONENT_LINE_SIZE];
	Tout lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 1;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		writeEmptyline<Tout>(out_stream, NbParallelInputWords); // the first line is written without being processed

// first pipeline stage
		readline<T>(inStream,line3,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line4,NbParallelInputWords);
		findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line1, line2, line3, lineout1, RThreshold, NbParallelInputWords);

		for (i=4; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line2, line3, line4, lineout2, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line2, line3, line4, lineout1, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line3, line4, line1, lineout2, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line3, line4, line1, lineout1, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line4, line1, line2, lineout2, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line4, line1, line2, lineout1, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line1, line2, line3, lineout2, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line1, line2, line3, lineout1, RThreshold, NbParallelInputWords);
				writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line2, line3, line4, lineout2, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line2, line3, line4, lineout1, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line3, line4, line1, lineout2, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line3, line4, line1, lineout1, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line4, line1, line2, lineout2, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line4, line1, line2, lineout1, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line1, line2, line3, lineout2, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			findMax_one_line_3x3<T,S,W, Tout, Sout, Wout>(line1, line2, line3, lineout1, RThreshold, NbParallelInputWords);
			writeline<Tout>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}

		// second stage of the postamble - We do not read anymore new images and we do not do more processing
		if (output_line == 1)
			writeline<Tout>(out_stream, lineout1, NbParallelInputWords);
		else
			writeline<Tout>(out_stream, lineout2, NbParallelInputWords);

		// last stage of the postamble - Write a line of 0s
			writeEmptyline<Tout>(out_stream, NbParallelInputWords);


	}  // end for k, on the different images
}


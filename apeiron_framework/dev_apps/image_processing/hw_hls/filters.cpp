template <typename T>
void readline(hls::stream<T>& inStream, T *line, unsigned short int size)
{
	for (unsigned short int i=0; i<size; i++)
		line[i] = inStream.read();
}

template <typename T>
void writeline(hls::stream<T>& outStream, T *line, unsigned short int size)
{
	for (unsigned short int i=0; i<size; i++)
		outStream.write(line[i]);
}

template <typename T>
void writeEmptyline(hls::stream<T>& outStream, unsigned short int size)
{
	T aux = 0;
	for (unsigned short int i=0; i<size; i++)
		outStream.write(aux);
}

// for the time being, S=8
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void filter_one_line_3x3(T *inline1, T *inline2, T *inline3, T *outline,
					 short int c00, short int c01, short int c02,
					 short int c10, short int c11, short int c12,
					 short int c20, short int c21, short int c22,
					 unsigned short int size,
					 bool absVal)
{
	T din1, din2, din3, dout;
	int di00, di01, di02, di10, di11, di12, di20, di21, di22;
	int int_dout;
	int i,j;
	ap_uint<S> aux;

	ap_uint<S> max_value;

	if (S==8)
		max_value.range(S-1,0) = 0xFF;
	else if (S==16)
		max_value.range(S-1,0) = 0xFFFF;


	for (i=0; i<size; i++)

	{
#pragma HLS pipeline
#pragma HLS ALLOCATION instances=mul limit=(9*PIXEL_PROCESSED_IN_PARALLEL)
#pragma HLS ALLOCATION instances=add limit=(8*PIXEL_PROCESSED_IN_PARALLEL)

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

			dout.range(S-1,0) = din2.range(S-1,0);

			for (int j=2; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di02 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di12 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di22 = (din3.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = (c00*di00+c01*di01+c02*di02+c10*di10+c11*di11+c12*di12+c20*di20+c21*di21+c22*di22)>>10; //the result is divided by 1024
				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int) max_value)
					int_dout = (int) max_value;
				aux = ap_uint <S> (int_dout);
				dout.range(S*j-1,S*(j-1)) = aux;
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

			int_dout = (c00*di00+c01*di01+c02*di02+c10*di10+c11*di11+c12*di12+c20*di20+c21*di21+c22*di22)>>10;
			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int)max_value)
				int_dout = (int)max_value;
			aux = ap_uint < S > (int_dout);
			dout.range(W-1, W-S) = aux; //ap_uint < 16 > ((int)fdout);
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

				int_dout = (c00*di00+c01*di01+c02*di02+c10*di10+c11*di11+c12*di12+c20*di20+c21*di21+c22*di22)>>10;
				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int)max_value)
					int_dout = (int)max_value;
				aux = ap_uint < S > (int_dout);
				dout.range(S*j-1,S*(j-1)) = aux;
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
	dout.range(W-1, W-S) = din2.range(W-1, W-S);
	outline[i-1] = dout;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_3x3_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  short int c00, short int c01, short int c02,
				  short int c10, short int c11, short int c12,
				  short int c20, short int c21, short int c22,
				  bool absVal,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32
	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T lineout1[MAX_COMPONENT_LINE_SIZE];
	T lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 1;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed

// first pipeline stage
		readline<T>(inStream,line3,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line4,NbParallelInputWords);
		filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);

		for (i=4; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}

		// second stage of the postamble - We do not read anymore new images and we do not do more processing
		if (output_line == 1)
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
		else
			writeline<T>(out_stream, lineout2, NbParallelInputWords);

		// last stage of the postamble - Write unchanged the last read line
		if ((new_read_line == 1))
		{
			writeline<T>(out_stream, line4, NbParallelInputWords);
		}
		else if (new_read_line == 2)
		{
			writeline<T>(out_stream, line1, NbParallelInputWords);
		}
		else if (new_read_line == 3)
		{
			writeline<T>(out_stream, line2, NbParallelInputWords);
		}
		else if (new_read_line == 4)
		{
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}


	}  // end for k, on the different images
}

// 3x3 max/min filtering

template <typename T>
T computeMaxMin(T in1, T in2, bool computeMax)
{
	T res;
	if (computeMax)
	{
		if (in1 > in2)
			res = in1;
		else
			res = in2;
	}
	else
	{
		if (in1 > in2)
			res = in2;
		else
			res = in1;
	}

	return res;
}

// for the time being, S=8
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void max_min_filter_one_line_3x3(T *inline1, T *inline2, T *inline3, T *outline,
					 unsigned short int size,
					 bool computeMax)
{
	T din1, din2, din3, dout;
	ap_uint<S> di00, di01, di02, di10, di11, di12, di20, di21, di22;
	int int_dout;
	int i,j;
	ap_int<S> aux, aux1, aux2, aux3;

	ap_uint<S> max_value;

	if (S==8)
		max_value.range(S-1,0) = 0xFF;
	else if (S==16)
		max_value.range(S-1,0) = 0xFFFF;


	for (i=0; i<size; i++)

	{
#pragma HLS pipeline

		din1 = inline1[i];
		din2 = inline2[i];
		din3 = inline3[i];

		if (i==0)
		{
			di00 = (din1.range(S-1, 0));
			di10 = (din2.range(S-1, 0));
			di20 = (din3.range(S-1, 0));

			di01 = (din1.range(2*S-1, S));
			di11 = (din2.range(2*S-1, S));
			di21 = (din3.range(2*S-1, S));

			dout.range(S-1,0) = din2.range(S-1,0);

			for (int j=2; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di02 = (din1.range(S * (j + 1) - 1, S * j));
				di12 = (din2.range(S * (j + 1) - 1, S * j));
				di22 = (din3.range(S * (j + 1) - 1, S * j));

				aux1 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di00, di01,computeMax),computeMaxMin<ap_uint<S>>(di02, di10,computeMax),computeMax);
				aux2 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di11, di12,computeMax),computeMaxMin<ap_uint<S>>(di20, di21,computeMax),computeMax);
				aux3 = computeMaxMin<ap_uint<S>>(aux1,aux2,computeMax);
				aux = computeMaxMin<ap_uint<S>>(aux3,di22, computeMax);

				dout.range(S*j-1,S*(j-1)) = aux;
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
			aux1 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di00, di01,computeMax),computeMaxMin<ap_uint<S>>(di02, di10,computeMax),computeMax);
			aux2 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di11, di12,computeMax),computeMaxMin<ap_uint<S>>(di20, di21,computeMax),computeMax);
			aux3 = computeMaxMin<ap_uint<S>>(aux1,aux2,computeMax);
			aux = computeMaxMin<ap_uint<S>>(aux3,di22, computeMax);

			dout.range(W-1, W-S) = aux; //ap_uint < 16 > ((int)fdout);
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

				aux1 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di00, di01,computeMax),computeMaxMin<ap_uint<S>>(di02, di10,computeMax),computeMax);
				aux2 = computeMaxMin<ap_uint<S>>(computeMaxMin<ap_uint<S>>(di11, di12,computeMax),computeMaxMin<ap_uint<S>>(di20, di21,computeMax),computeMax);
				aux3 = computeMaxMin<ap_uint<S>>(aux1,aux2,computeMax);
				aux = computeMaxMin<ap_uint<S>>(aux3,di22, computeMax);

				dout.range(S*j-1,S*(j-1)) = aux;
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
	dout.range(W-1, W-S) = din2.range(W-1, W-S);
	outline[i-1] = dout;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_3x3_max_min_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  bool computeMax,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32
	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T lineout1[MAX_COMPONENT_LINE_SIZE];
	T lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 1;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed

// first pipeline stage
		readline<T>(inStream,line3,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line4,NbParallelInputWords);
		max_min_filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, NbParallelInputWords, computeMax);

		for (i=4; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				max_min_filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, NbParallelInputWords, computeMax);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			max_min_filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			max_min_filter_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			max_min_filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			max_min_filter_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			max_min_filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			max_min_filter_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			max_min_filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			max_min_filter_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, NbParallelInputWords, computeMax);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}

		// second stage of the postamble - We do not read anymore new images and we do not do more processing
		if (output_line == 1)
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
		else
			writeline<T>(out_stream, lineout2, NbParallelInputWords);

		// last stage of the postamble - Write unchanged the last read line
		if ((new_read_line == 1))
		{
			writeline<T>(out_stream, line4, NbParallelInputWords);
		}
		else if (new_read_line == 2)
		{
			writeline<T>(out_stream, line1, NbParallelInputWords);
		}
		else if (new_read_line == 3)
		{
			writeline<T>(out_stream, line2, NbParallelInputWords);
		}
		else if (new_read_line == 4)
		{
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}

	}  // end for k, on the different images
}


// end 3x3 max/min filtering




template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_3x3_filtering_withUF(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  short int c00, short int c01, short int c02,
				  short int c10, short int c11, short int c12,
				  short int c20, short int c21, short int c22,
				  bool absVal,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32

	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T lineoutA[MAX_COMPONENT_LINE_SIZE];
	T lineoutB[MAX_COMPONENT_LINE_SIZE];
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed
		readline<T>(inStream,line2,NbParallelInputWords);
		readline<T>(inStream,line3,NbParallelInputWords);
		for (i=3; i<ImgRows-(4*UF-1); i+=(4*UF))
		{
			for (int j=0; j<UF; j++)
			{
	#pragma HLS unroll
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line1, line2, line3, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line2, line3, line4, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);

				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line3, line4, line1, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_3x3<T,S,W>(line4, line1, line2, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
			}
		}
		for (; i<ImgRows-3; i+=4)
		{
			readline<T>(inStream,line4,NbParallelInputWords);
			filter_one_line_3x3<T,S,W>(line1, line2, line3, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line1,NbParallelInputWords);
			filter_one_line_3x3<T,S,W>(line2, line3, line4, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);

			readline<T>(inStream,line2,NbParallelInputWords);
			filter_one_line_3x3<T,S,W>(line3, line4, line1, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line3,NbParallelInputWords);
			filter_one_line_3x3<T,S,W>(line4, line1, line2, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);
		}

		if (i<ImgRows)
		{  // I can have still 3 lines to read
			i++;
			filter_one_line_3x3<T,S,W>(line1, line2, line3, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			readline<T>(inStream, line4, NbParallelInputWords);
			if (i<ImgRows)
			{  // I can have still 2 lines to read
				i++;
				filter_one_line_3x3<T,S,W>(line2, line3, line4, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				readline<T>(inStream, line1, NbParallelInputWords);
				if (i<ImgRows)
				{  // I can have still 1 line to read
					i++;
					filter_one_line_3x3<T,S,W>(line3, line4, line1, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					readline<T>(inStream, line2, NbParallelInputWords); // line2 is the last line of the input image
					filter_one_line_3x3<T,S,W>(line4, line1, line2, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
					writeline<T>(out_stream, lineoutB, NbParallelInputWords);
					writeline<T>(out_stream, line2, NbParallelInputWords);
				}
				else // line1 is the last line of the input image
				{
					filter_one_line_3x3<T,S,W>(line3, line4, line1, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, line1, NbParallelInputWords);
				}
			}
			else // line4 is the last line of the input image
			{
				filter_one_line_3x3<T,S,W>(line2, line3, line4, lineoutB, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, line4, NbParallelInputWords);
			}

		}
		else //line3 is the last line of the input image
		{
			filter_one_line_3x3<T,S,W>(line1, line2, line3, lineoutA, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}
	}
}




// for the time being, S=8
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void filter_one_line_5x5( T *inline1, T *inline2, T *inline3, T *inline4, T *inline5, T *outline,
						  short int c00, short int c01, short int c02, short int c03, short int c04,
						  short int c10, short int c11, short int c12, short int c13, short int c14,
						  short int c20, short int c21, short int c22, short int c23, short int c24,
						  short int c30, short int c31, short int c32, short int c33, short int c34,
						  short int c40, short int c41, short int c42, short int c43, short int c44,
						  unsigned short int size,
						  bool absVal)
{
	T din1, din2, din3, din4, din5, dout;
	int di00, di01, di02, di03, di04;
	int di10, di11, di12, di13, di14;
	int di20, di21, di22, di23, di24;
	int di30, di31, di32, di33, di34;
	int di40, di41, di42, di43, di44;
	int int_dout;
	int i,j;
	ap_int<16> aux;

	ap_uint<S> max_value;

	if (S==8)
		max_value.range(S-1,0) = 0xFF;
	else if (S==16)
		max_value.range(S-1,0) = 0xFFFF;


	for (i=0; i<size; i++)

	{
#pragma HLS pipeline
#pragma HLS ALLOCATION instances=mul limit=(9*PIXEL_PROCESSED_IN_PARALLEL)
#pragma HLS ALLOCATION instances=add limit=(8*PIXEL_PROCESSED_IN_PARALLEL)

		din1 = inline1[i];
		din2 = inline2[i];
		din3 = inline3[i];
		din4 = inline4[i];
		din5 = inline5[i];

		if (i==0)
		{
			di00 = (din1.range(S-1, 0).to_int());
			di10 = (din2.range(S-1, 0).to_int());
			di20 = (din3.range(S-1, 0).to_int());
			di30 = (din4.range(S-1, 0).to_int());
			di40 = (din5.range(S-1, 0).to_int());

			di01 = (din1.range(2*S-1, S).to_int());
			di11 = (din2.range(2*S-1, S).to_int());
			di21 = (din3.range(2*S-1, S).to_int());
			di31 = (din4.range(2*S-1, S).to_int());
			di41 = (din5.range(2*S-1, S).to_int());

			di02 = (din1.range(3*S-1, 2*S).to_int());
			di12 = (din2.range(3*S-1, 2*S).to_int());
			di22 = (din3.range(3*S-1, 2*S).to_int());
			di32 = (din4.range(3*S-1, 2*S).to_int());
			di42 = (din5.range(3*S-1, 2*S).to_int());

			di03 = (din1.range(4*S-1, 3*S).to_int());
			di13 = (din2.range(4*S-1, 3*S).to_int());
			di23 = (din3.range(4*S-1, 3*S).to_int());
			di33 = (din4.range(4*S-1, 3*S).to_int());
			di43 = (din5.range(3*S-1, 3*S).to_int());

			dout.range(S-1,0) = din3.range(S-1,0);
			dout.range(2*S-1,S) = din3.range(2*S-1,S);

			for (int j=4; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di04 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di14 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di24 = (din3.range(S * (j + 1) - 1, S * j).to_int());
				di34 = (din4.range(S * (j + 1) - 1, S * j).to_int());
				di44 = (din5.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+
							c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+
							c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+
							c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+
							c40*di40+c41*di41+c42*di42+c43*di43+c44*di44)>>10; //the result is divided by 1024
				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int) max_value)
					int_dout = (int) max_value;
				aux = ap_int < SIZE > (int_dout);

				dout.range(S*(j-1)-1,S*(j-2)) = aux;

				di00 = di01;
				di10 = di11;
				di20 = di21;
				di30 = di31;
				di40 = di41;

				di01 = di02;
				di11 = di12;
				di21 = di22;
				di31 = di32;
				di41 = di42;

				di02 = di03;
				di12 = di13;
				di22 = di23;
				di32 = di33;
				di42 = di43;

				di03 = di04;
				di13 = di14;
				di23 = di24;
				di33 = di34;
				di43 = di44;

			 } // processed the PIXEL_IN_INPUT_WORD leftmost input pixels
		}
		else
		{
			// processing the last two output pixels of previous word
			// 1st pixel
			di04 = (din1.range(S - 1,0).to_int());
			di14 = (din2.range(S - 1, 0).to_int());
			di24 = (din3.range(S - 1, 0).to_int());
			di34 = (din4.range(S - 1, 0).to_int());
			di44 = (din5.range(S - 1, 0).to_int());

			int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+
						c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+
						c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+
						c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+
						c40*di40+c41*di41+c42*di42+c43*di43+c44*di44)>>10; //the result is divided by 1024
			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int) max_value)
				int_dout = (int) max_value;
			aux = ap_int < SIZE > (int_dout);

			dout.range(W-S-1, W-2*S) = aux;

			// second pixel
			di00 = di01;
			di10 = di11;
			di20 = di21;
			di30 = di31;
			di40 = di41;

			di01 = di02;
			di11 = di12;
			di21 = di22;
			di31 = di32;
			di41 = di42;

			di02 = di03;
			di12 = di13;
			di22 = di23;
			di32 = di33;
			di42 = di43;

			di03 = di04;
			di13 = di14;
			di23 = di24;
			di33 = di34;
			di43 = di44;

			di04 = (din1.range(2*S - 1, S).to_int());
			di14 = (din2.range(2*S - 1, S).to_int());
			di24 = (din3.range(2*S - 1, S).to_int());
			di34 = (din4.range(2*S - 1, S).to_int());
			di44 = (din5.range(2*S - 1, S).to_int());

			int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+
						c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+
						c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+
						c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+
						c40*di40+c41*di41+c42*di42+c43*di43+c44*di44)>>10; //the result is divided by 1024
			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int) max_value)
				int_dout = (int) max_value;
			aux = ap_int < SIZE > (int_dout);

			dout.range(W-1, W-S) = aux;
			outline[i-1] = dout;

			di00 = di01;
			di10 = di11;
			di20 = di21;
			di30 = di31;
			di40 = di41;

			di01 = di02;
			di11 = di12;
			di21 = di22;
			di31 = di32;
			di41 = di42;

			di02 = di03;
			di12 = di13;
			di22 = di23;
			di32 = di33;
			di42 = di43;

			di03 = di04;
			di13 = di14;
			di23 = di24;
			di33 = di34;
			di43 = di44;

			for (int j=2; j<(W/S); j++)
		     {
		#pragma HLS unroll

				di04 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di14 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di24 = (din3.range(S * (j + 1) - 1, S * j).to_int());
				di34 = (din4.range(S * (j + 1) - 1, S * j).to_int());
				di44 = (din5.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+
							c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+
							c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+
							c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+
							c40*di40+c41*di41+c42*di42+c43*di43+c44*di44)>>10; //the result is divided by 1024
				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int) max_value)
					int_dout = (int) max_value;
				aux = ap_int < SIZE > (int_dout);

				dout.range(S*(j-1)-1,S*(j-2)) = aux;

				di00 = di01;
				di10 = di11;
				di20 = di21;
				di30 = di31;
				di40 = di41;

				di01 = di02;
				di11 = di12;
				di21 = di22;
				di31 = di32;
				di41 = di42;

				di02 = di03;
				di12 = di13;
				di22 = di23;
				di32 = di33;
				di42 = di43;

				di03 = di04;
				di13 = di14;
				di23 = di24;
				di33 = di34;
				di43 = di44;

		     }
		}  //end else (i==0)
	}  // end for i, the one scanning the entire line
	// the last output pixel of the row is equal to the input pixel
	dout.range(W-S-1, W-2*S) = din3.range(W-S-1, W-2*S);
	dout.range(W-1, W-S) = din3.range(W-1, W-S);
	outline[i-1] = dout;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_5x5_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  short int c00, short int c01, short int c02, short int c03, short int c04,
				  short int c10, short int c11, short int c12, short int c13, short int c14,
				  short int c20, short int c21, short int c22, short int c23, short int c24,
				  short int c30, short int c31, short int c32, short int c33, short int c34,
				  short int c40, short int c41, short int c42, short int c43, short int c44,
				  bool absVal,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32

	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T line5[MAX_COMPONENT_LINE_SIZE];
	T line6[MAX_COMPONENT_LINE_SIZE];
	T lineout1[MAX_COMPONENT_LINE_SIZE];
	T lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 1;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		readline<T>(inStream,line3,NbParallelInputWords);
		readline<T>(inStream,line4,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed
		writeline<T>(out_stream, line2, NbParallelInputWords); // the first line is written without being processed

// first pipeline stage
		readline<T>(inStream,line5,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line6,NbParallelInputWords);
		filter_one_line_5x5<T,S,W>(line1, line2, line3, line4, line5, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);

		for (i=6; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line2, line3, line4, line5, line6, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line2, line3, line4, line5, line6, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line3, line4, line5, line6, line1, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line3, line4, line5, line6, line1, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line4, line5, line6, line1, line2, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line4, line5, line6, line1, line2, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line5, line6, line1, line2, line3, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 5;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line5, line6, line1, line2, line3, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 5;
				output_line = 1;
			}
			else if ((new_read_line == 5) && (output_line == 1))
			{
				readline<T>(inStream,line5,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line6, line1, line2, line3, line4, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 6;
				output_line = 2;
			}
			else if ((new_read_line == 5) && (output_line == 2))
			{
				readline<T>(inStream,line5,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line6, line1, line2, line3, line4, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 6;
				output_line = 1;
			}
			else if ((new_read_line == 6) && (output_line == 1))
			{
				readline<T>(inStream,line6,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line1, line2, line3, line4, line5, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 6) && (output_line == 2))
			{
				readline<T>(inStream,line6,NbParallelInputWords);
				filter_one_line_5x5<T,S,W>(line1, line2, line3, line4, line5, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line2, line3, line4, line5, line6, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line2, line3, line4, line5, line6, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line3, line4, line5, line6, line1, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line3, line4, line5, line6, line1, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line4, line5, line6, line1, line2, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line4, line5, line6, line1, line2, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line5, line6, line1, line2, line3, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line5, line6, line1, line2, line3, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 5) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line6, line1, line2, line3, line4, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 5) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line6, line1, line2, line3, line4, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 6) && (output_line == 1))
		{
			filter_one_line_5x5<T,S,W>(line1, line2, line3, line4, line5, lineout2, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 6) && (output_line == 2))
		{
			filter_one_line_5x5<T,S,W>(line1, line2, line3, line4, line5, lineout1, c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23, c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44, NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}

		// second stage of the postamble - We do not read anymore new images and we do not do more processing
		if (output_line == 1)
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
		else
			writeline<T>(out_stream, lineout2, NbParallelInputWords);

		// last stage of the postamble - Write unchanged the last two read lines
		if ((new_read_line == 1))
		{
			writeline<T>(out_stream, line5, NbParallelInputWords);
			writeline<T>(out_stream, line6, NbParallelInputWords);
		}
		else if (new_read_line == 2)
		{
			writeline<T>(out_stream, line6, NbParallelInputWords);
			writeline<T>(out_stream, line1, NbParallelInputWords);
		}
		else if (new_read_line == 3)
		{
			writeline<T>(out_stream, line1, NbParallelInputWords);
			writeline<T>(out_stream, line2, NbParallelInputWords);
		}
		else if (new_read_line == 4)
		{
			writeline<T>(out_stream, line2, NbParallelInputWords);
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}
		else if (new_read_line == 5)
		{
			writeline<T>(out_stream, line3, NbParallelInputWords);
			writeline<T>(out_stream, line4, NbParallelInputWords);
		}
		else if (new_read_line == 6)
		{
			writeline<T>(out_stream, line4, NbParallelInputWords);
			writeline<T>(out_stream, line5, NbParallelInputWords);
		}


	}  // end for k, on the different images
}


// for the time being, S=8
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void filter_one_line_7x7( T *inline1, T *inline2, T *inline3, T *inline4, T *inline5,  T *inline6,  T *inline7, T *outline,
						  short int c00, short int c01, short int c02, short int c03, short int c04, short int c05, short int c06,
						  short int c10, short int c11, short int c12, short int c13, short int c14, short int c15, short int c16,
						  short int c20, short int c21, short int c22, short int c23, short int c24, short int c25, short int c26,
						  short int c30, short int c31, short int c32, short int c33, short int c34, short int c35, short int c36,
						  short int c40, short int c41, short int c42, short int c43, short int c44, short int c45, short int c46,
						  short int c50, short int c51, short int c52, short int c53, short int c54, short int c55, short int c56,
						  short int c60, short int c61, short int c62, short int c63, short int c64, short int c65, short int c66,
						  unsigned short int size,
						  bool absVal)
{
	T din1, din2, din3, din4, din5, din6, din7, dout;
	int di00, di01, di02, di03, di04, di05, di06;
	int di10, di11, di12, di13, di14, di15, di16;
	int di20, di21, di22, di23, di24, di25, di26;
	int di30, di31, di32, di33, di34, di35, di36;
	int di40, di41, di42, di43, di44, di45, di46;
	int di50, di51, di52, di53, di54, di55, di56;
	int di60, di61, di62, di63, di64, di65, di66;
	int int_dout;
	int i,j;
	ap_int<16> aux;

	ap_uint<S> max_value;

	if (S==8)
		max_value.range(S-1,0) = 0xFF;
	else if (S==16)
		max_value.range(S-1,0) = 0xFFFF;


	for (i=0; i<size; i++)

	{
#pragma HLS pipeline
#pragma HLS ALLOCATION instances=mul limit=(9*PIXEL_PROCESSED_IN_PARALLEL)
#pragma HLS ALLOCATION instances=add limit=(8*PIXEL_PROCESSED_IN_PARALLEL)

		din1 = inline1[i];
		din2 = inline2[i];
		din3 = inline3[i];
		din4 = inline4[i];
		din5 = inline5[i];
		din6 = inline6[i];
		din7 = inline7[i];

		if (i==0)
		{
			di00 = (din1.range(S-1, 0).to_int());
			di10 = (din2.range(S-1, 0).to_int());
			di20 = (din3.range(S-1, 0).to_int());
			di30 = (din4.range(S-1, 0).to_int());
			di40 = (din5.range(S-1, 0).to_int());
			di50 = (din6.range(S-1, 0).to_int());
			di60 = (din7.range(S-1, 0).to_int());

			di01 = (din1.range(2*S-1, S).to_int());
			di11 = (din2.range(2*S-1, S).to_int());
			di21 = (din3.range(2*S-1, S).to_int());
			di31 = (din4.range(2*S-1, S).to_int());
			di41 = (din5.range(2*S-1, S).to_int());
			di51 = (din6.range(2*S-1, S).to_int());
			di61 = (din7.range(2*S-1, S).to_int());

			di02 = (din1.range(3*S-1, 2*S).to_int());
			di12 = (din2.range(3*S-1, 2*S).to_int());
			di22 = (din3.range(3*S-1, 2*S).to_int());
			di32 = (din4.range(3*S-1, 2*S).to_int());
			di42 = (din5.range(3*S-1, 2*S).to_int());
			di52 = (din6.range(3*S-1, 2*S).to_int());
			di62 = (din7.range(3*S-1, 2*S).to_int());

			di03 = (din1.range(4*S-1, 3*S).to_int());
			di13 = (din2.range(4*S-1, 3*S).to_int());
			di23 = (din3.range(4*S-1, 3*S).to_int());
			di33 = (din4.range(4*S-1, 3*S).to_int());
			di43 = (din5.range(4*S-1, 3*S).to_int());
			di53 = (din6.range(4*S-1, 3*S).to_int());
			di63 = (din7.range(4*S-1, 3*S).to_int());

			di04 = (din1.range(5*S-1, 4*S).to_int());
			di14 = (din2.range(5*S-1, 4*S).to_int());
			di24 = (din3.range(5*S-1, 4*S).to_int());
			di34 = (din4.range(5*S-1, 4*S).to_int());
			di44 = (din5.range(5*S-1, 4*S).to_int());
			di54 = (din6.range(5*S-1, 4*S).to_int());
			di64 = (din7.range(5*S-1, 4*S).to_int());

			di05 = (din1.range(6*S-1, 5*S).to_int());
			di15 = (din2.range(6*S-1, 5*S).to_int());
			di25 = (din3.range(6*S-1, 5*S).to_int());
			di35 = (din4.range(6*S-1, 5*S).to_int());
			di45 = (din5.range(6*S-1, 5*S).to_int());
			di55 = (din6.range(6*S-1, 5*S).to_int());
			di65 = (din7.range(6*S-1, 5*S).to_int());

			dout.range(S-1,0) = din4.range(S-1,0);
			dout.range(2*S-1,S) = din4.range(2*S-1,S);
			dout.range(3*S-1,2*S) = din4.range(3*S-1,2*S);

			for (int j=6; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di06 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di16 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di26 = (din3.range(S * (j + 1) - 1, S * j).to_int());
				di36 = (din4.range(S * (j + 1) - 1, S * j).to_int());
				di46 = (din5.range(S * (j + 1) - 1, S * j).to_int());
				di56 = (din6.range(S * (j + 1) - 1, S * j).to_int());
				di66 = (din7.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+c05*di05+c06*di06+
							c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+c15*di15+c16*di16+
							c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+c25*di25+c26*di26+
							c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+c35*di35+c36*di36+
							c40*di40+c41*di41+c42*di42+c43*di43+c44*di44+c45*di45+c46*di46+
							c50*di50+c51*di51+c52*di52+c53*di53+c54*di54+c55*di55+c56*di56+
							c60*di60+c61*di61+c62*di62+c63*di63+c64*di64+c65*di65+c66*di66)>>10; //the result is divided by 1024
				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int) max_value)
					int_dout = (int) max_value;
				aux = ap_int < SIZE > (int_dout);

				dout.range(S*(j-2)-1,S*(j-3)) = aux;

				di00 = di01;
				di10 = di11;
				di20 = di21;
				di30 = di31;
				di40 = di41;
				di50 = di51;
				di60 = di61;

				di01 = di02;
				di11 = di12;
				di21 = di22;
				di31 = di32;
				di41 = di42;
				di51 = di52;
				di61 = di62;

				di02 = di03;
				di12 = di13;
				di22 = di23;
				di32 = di33;
				di42 = di43;
				di52 = di53;
				di62 = di63;

				di03 = di04;
				di13 = di14;
				di23 = di24;
				di33 = di34;
				di43 = di44;
				di53 = di54;
				di63 = di64;

				di04 = di05;
				di14 = di15;
				di24 = di25;
				di34 = di35;
				di44 = di45;
				di54 = di55;
				di64 = di65;

				di05 = di06;
				di15 = di16;
				di25 = di26;
				di35 = di36;
				di45 = di46;
				di55 = di56;
				di65 = di66;

			 } // processed the PIXEL_IN_INPUT_WORD leftmost input pixels
		}
		else
		{
			// processing the last three output pixels of previous word
			// 1st pixel
			di06 = (din1.range(S - 1,0).to_int());
			di16 = (din2.range(S - 1, 0).to_int());
			di26 = (din3.range(S - 1, 0).to_int());
			di36 = (din4.range(S - 1, 0).to_int());
			di46 = (din5.range(S - 1, 0).to_int());
			di56 = (din6.range(S - 1, 0).to_int());
			di66 = (din7.range(S - 1, 0).to_int());

			int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+c05*di05+c06*di06+
						c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+c15*di15+c16*di16+
						c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+c25*di25+c26*di26+
						c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+c35*di35+c36*di36+
						c40*di40+c41*di41+c42*di42+c43*di43+c44*di44+c45*di45+c46*di46+
						c50*di50+c51*di51+c52*di52+c53*di53+c54*di54+c55*di55+c56*di56+
						c60*di60+c61*di61+c62*di62+c63*di63+c64*di64+c65*di65+c66*di66)>>10; //the result is divided by 1024
			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int) max_value)
				int_dout = (int) max_value;
			aux = ap_int < SIZE > (int_dout);

			dout.range(W-2*S-1, W-3*S) = aux;

			// second pixel
			di00 = di01;
			di10 = di11;
			di20 = di21;
			di30 = di31;
			di40 = di41;
			di50 = di51;
			di60 = di61;

			di01 = di02;
			di11 = di12;
			di21 = di22;
			di31 = di32;
			di41 = di42;
			di51 = di52;
			di61 = di62;

			di02 = di03;
			di12 = di13;
			di22 = di23;
			di32 = di33;
			di42 = di43;
			di52 = di53;
			di62 = di63;

			di03 = di04;
			di13 = di14;
			di23 = di24;
			di33 = di34;
			di43 = di44;
			di53 = di54;
			di63 = di64;

			di04 = di05;
			di14 = di15;
			di24 = di25;
			di34 = di35;
			di44 = di45;
			di54 = di55;
			di64 = di65;

			di05 = di06;
			di15 = di16;
			di25 = di26;
			di35 = di36;
			di45 = di46;
			di55 = di56;
			di65 = di66;

			di06 = (din1.range(2*S - 1, S).to_int());
			di16 = (din2.range(2*S - 1, S).to_int());
			di26 = (din3.range(2*S - 1, S).to_int());
			di36 = (din4.range(2*S - 1, S).to_int());
			di46 = (din5.range(2*S - 1, S).to_int());
			di56 = (din6.range(2*S - 1, S).to_int());
			di66 = (din7.range(2*S - 1, S).to_int());

			int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+c05*di05+c06*di06+
						c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+c15*di15+c16*di16+
						c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+c25*di25+c26*di26+
						c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+c35*di35+c36*di36+
						c40*di40+c41*di41+c42*di42+c43*di43+c44*di44+c45*di45+c46*di46+
						c50*di50+c51*di51+c52*di52+c53*di53+c54*di54+c55*di55+c56*di56+
						c60*di60+c61*di61+c62*di62+c63*di63+c64*di64+c65*di65+c66*di66)>>10; //the result is divided by 1024
			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int) max_value)
				int_dout = (int) max_value;
			aux = ap_int < SIZE > (int_dout);

			dout.range(W-S-1, W-2*S) = aux;

			// third pixel
			di00 = di01;
			di10 = di11;
			di20 = di21;
			di30 = di31;
			di40 = di41;
			di50 = di51;
			di60 = di61;

			di01 = di02;
			di11 = di12;
			di21 = di22;
			di31 = di32;
			di41 = di42;
			di51 = di52;
			di61 = di62;

			di02 = di03;
			di12 = di13;
			di22 = di23;
			di32 = di33;
			di42 = di43;
			di52 = di53;
			di62 = di63;

			di03 = di04;
			di13 = di14;
			di23 = di24;
			di33 = di34;
			di43 = di44;
			di53 = di54;
			di63 = di64;

			di04 = di05;
			di14 = di15;
			di24 = di25;
			di34 = di35;
			di44 = di45;
			di54 = di55;
			di64 = di65;

			di05 = di06;
			di15 = di16;
			di25 = di26;
			di35 = di36;
			di45 = di46;
			di55 = di56;
			di65 = di66;

			di06 = (din1.range(3*S - 1, 2*S).to_int());
			di16 = (din2.range(3*S - 1, 2*S).to_int());
			di26 = (din3.range(3*S - 1, 2*S).to_int());
			di36 = (din4.range(3*S - 1, 2*S).to_int());
			di46 = (din5.range(3*S - 1, 2*S).to_int());
			di56 = (din6.range(3*S - 1, 2*S).to_int());
			di66 = (din7.range(3*S - 1, 2*S).to_int());

			int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+c05*di05+c06*di06+
						c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+c15*di15+c16*di16+
						c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+c25*di25+c26*di26+
						c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+c35*di35+c36*di36+
						c40*di40+c41*di41+c42*di42+c43*di43+c44*di44+c45*di45+c46*di46+
						c50*di50+c51*di51+c52*di52+c53*di53+c54*di54+c55*di55+c56*di56+
						c60*di60+c61*di61+c62*di62+c63*di63+c64*di64+c65*di65+c66*di66)>>10; //the result is divided by 1024

			if (int_dout < 0)
			{
				if (absVal)
					int_dout = -int_dout;
				else
					int_dout = 0;
			}
			else if (int_dout>(int) max_value)
				int_dout = (int) max_value;
			aux = ap_int < SIZE > (int_dout);

			dout.range(W-1, W-S) = aux;
			outline[i-1] = dout;

			di00 = di01;
			di10 = di11;
			di20 = di21;
			di30 = di31;
			di40 = di41;
			di50 = di51;
			di60 = di61;

			di01 = di02;
			di11 = di12;
			di21 = di22;
			di31 = di32;
			di41 = di42;
			di51 = di52;
			di61 = di62;

			di02 = di03;
			di12 = di13;
			di22 = di23;
			di32 = di33;
			di42 = di43;
			di52 = di53;
			di62 = di63;

			di03 = di04;
			di13 = di14;
			di23 = di24;
			di33 = di34;
			di43 = di44;
			di53 = di54;
			di63 = di64;

			di04 = di05;
			di14 = di15;
			di24 = di25;
			di34 = di35;
			di44 = di45;
			di54 = di55;
			di64 = di65;

			di05 = di06;
			di15 = di16;
			di25 = di26;
			di35 = di36;
			di45 = di46;
			di55 = di56;
			di65 = di66;


			for (int j=3; j<(W/S); j++)
		    {
			#pragma HLS unroll

				di06 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di16 = (din2.range(S * (j + 1) - 1, S * j).to_int());
				di26 = (din3.range(S * (j + 1) - 1, S * j).to_int());
				di36 = (din4.range(S * (j + 1) - 1, S * j).to_int());
				di46 = (din5.range(S * (j + 1) - 1, S * j).to_int());
				di56 = (din6.range(S * (j + 1) - 1, S * j).to_int());
				di66 = (din7.range(S * (j + 1) - 1, S * j).to_int());

				int_dout = (c00*di00+c01*di01+c02*di02+c03*di03+c04*di04+c05*di05+c06*di06+
							c10*di10+c11*di11+c12*di12+c13*di13+c14*di14+c15*di15+c16*di16+
							c20*di20+c21*di21+c22*di22+c23*di23+c24*di24+c25*di25+c26*di26+
							c30*di30+c31*di31+c32*di32+c33*di33+c34*di34+c35*di35+c36*di36+
							c40*di40+c41*di41+c42*di42+c43*di43+c44*di44+c45*di45+c46*di46+
							c50*di50+c51*di51+c52*di52+c53*di53+c54*di54+c55*di55+c56*di56+
							c60*di60+c61*di61+c62*di62+c63*di63+c64*di64+c65*di65+c66*di66)>>10; //the result is divided by 1024

				if (int_dout < 0)
				{
					if (absVal)
						int_dout = -int_dout;
					else
						int_dout = 0;
				}
				else if (int_dout>(int) max_value)
					int_dout = (int) max_value;
				aux = ap_int < SIZE > (int_dout);

				dout.range(S*(j-2)-1,S*(j-3)) = aux;

				di00 = di01;
				di10 = di11;
				di20 = di21;
				di30 = di31;
				di40 = di41;
				di50 = di51;
				di60 = di61;

				di01 = di02;
				di11 = di12;
				di21 = di22;
				di31 = di32;
				di41 = di42;
				di51 = di52;
				di61 = di62;

				di02 = di03;
				di12 = di13;
				di22 = di23;
				di32 = di33;
				di42 = di43;
				di52 = di53;
				di62 = di63;

				di03 = di04;
				di13 = di14;
				di23 = di24;
				di33 = di34;
				di43 = di44;
				di53 = di54;
				di63 = di64;

				di04 = di05;
				di14 = di15;
				di24 = di25;
				di34 = di35;
				di44 = di45;
				di54 = di55;
				di64 = di65;

				di05 = di06;
				di15 = di16;
				di25 = di26;
				di35 = di36;
				di45 = di46;
				di55 = di56;
				di65 = di66;
		     }

		}  //end else (i==0)
	}  // end for i, the one scanning the entire line
	// the last three output pixels of the row are equal to the input pixels
	dout.range(W-2*S-1, W-3*S) = din4.range(W-2*S-1, W-3*S);
	dout.range(W-S-1, W-2*S) = din4.range(W-S-1, W-2*S);
	dout.range(W-1, W-S) = din4.range(W-1, W-S);
	outline[i-1] = dout;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_7x7_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols,
				  short int c00, short int c01, short int c02, short int c03, short int c04, short int c05, short int c06,
				  short int c10, short int c11, short int c12, short int c13, short int c14, short int c15, short int c16,
				  short int c20, short int c21, short int c22, short int c23, short int c24, short int c25, short int c26,
				  short int c30, short int c31, short int c32, short int c33, short int c34, short int c35, short int c36,
				  short int c40, short int c41, short int c42, short int c43, short int c44, short int c45, short int c46,
				  short int c50, short int c51, short int c52, short int c53, short int c54, short int c55, short int c56,
				  short int c60, short int c61, short int c62, short int c63, short int c64, short int c65, short int c66,
				  bool absVal,
				  unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32

	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T line5[MAX_COMPONENT_LINE_SIZE];
	T line6[MAX_COMPONENT_LINE_SIZE];
	T line7[MAX_COMPONENT_LINE_SIZE];
	T line8[MAX_COMPONENT_LINE_SIZE];
	T lineout1[MAX_COMPONENT_LINE_SIZE];
	T lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 8;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		readline<T>(inStream,line3,NbParallelInputWords);
		readline<T>(inStream,line4,NbParallelInputWords);
		readline<T>(inStream,line5,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed
		writeline<T>(out_stream, line2, NbParallelInputWords); // the second line is written without being processed
		writeline<T>(out_stream, line3, NbParallelInputWords); // the third line is written without being processed

// first pipeline stage
		readline<T>(inStream,line6,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line7,NbParallelInputWords);
		filter_one_line_7x7<T,S,W>(line1, line2, line3, line4, line5, line6, line7, lineout1,
				c00, c01, c02, c03, c04, c05, c06,
				c10, c11, c12, c13, c14, c15, c16,
				c20, c21, c22, c23, c24, c25, c26,
				c30, c31, c32, c33, c34, c35, c36,
				c40, c41, c42, c43, c44, c45, c46,
				c50, c51, c52, c53, c54, c55, c56,
				c60, c61, c62, c63, c64, c65, c66,
				NbParallelInputWords, absVal);

		for (i=7; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line2, line3, line4, line5, line6, line7, line8, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line2, line3, line4, line5, line6, line7, line8, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line3, line4, line5, line6, line7, line8, line1, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line3, line4, line5, line6, line7, line8, line1, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line4, line5, line6, line7, line8, line1, line2, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line4, line5, line6, line7, line8, line1, line2, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line5, line6, line7, line8, line1, line2, line3, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 5;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line5, line6, line7, line8, line1, line2, line3, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 5;
				output_line = 1;
			}
			else if ((new_read_line == 5) && (output_line == 1))
			{
				readline<T>(inStream,line5,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line6, line7, line8, line1, line2, line3, line4, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 6;
				output_line = 2;
			}
			else if ((new_read_line == 5) && (output_line == 2))
			{
				readline<T>(inStream,line5,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line6, line7, line8, line1, line2, line3, line4, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 6;
				output_line = 1;
			}
			else if ((new_read_line == 6) && (output_line == 1))
			{
				readline<T>(inStream,line6,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line7, line8, line1, line2, line3, line4, line5, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 7;
				output_line = 2;
			}
			else if ((new_read_line == 6) && (output_line == 2))
			{
				readline<T>(inStream,line6,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line7, line8, line1, line2, line3, line4, line5, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 7;
				output_line = 1;
			}
			else if ((new_read_line == 7) && (output_line == 1))
			{
				readline<T>(inStream,line7,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line8, line1, line2, line3, line4, line5, line6, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 8;
				output_line = 2;
			}
			else if ((new_read_line == 7) && (output_line == 2))
			{
				readline<T>(inStream,line7,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line8, line1, line2, line3, line4, line5, line6, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 8;
				output_line = 1;
			}
			else if ((new_read_line == 8) && (output_line == 1))
			{
				readline<T>(inStream,line8,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line1, line2, line3, line4, line5, line6, line7, lineout2,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 8) && (output_line == 2))
			{
				readline<T>(inStream,line8,NbParallelInputWords);
				filter_one_line_7x7<T,S,W>(line1, line2, line3, line4, line5, line6, line7, lineout1,
						c00, c01, c02, c03, c04, c05, c06,
						c10, c11, c12, c13, c14, c15, c16,
						c20, c21, c22, c23, c24, c25, c26,
						c30, c31, c32, c33, c34, c35, c36,
						c40, c41, c42, c43, c44, c45, c46,
						c50, c51, c52, c53, c54, c55, c56,
						c60, c61, c62, c63, c64, c65, c66,
						NbParallelInputWords, absVal);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images

		if ((new_read_line == 1) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line2, line3, line4, line5, line6, line7, line8, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line2, line3, line4, line5, line6, line7, line8, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line3, line4, line5, line6, line7, line8, line1, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line3, line4, line5, line6, line7, line8, line1, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line4, line5, line6, line7, line8, line1, line2, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line4, line5, line6, line7, line8, line1, line2, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line5, line6, line7, line8, line1, line2, line3, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line5, line6, line7, line8, line1, line2, line3, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 5) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line6, line7, line8, line1, line2, line3, line4, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 5) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line6, line7, line8, line1, line2, line3, line4, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 6) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line7, line8, line1, line2, line3, line4, line5, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 6) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line7, line8, line1, line2, line3, line4, line5, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 7) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line8, line1, line2, line3, line4, line5, line6, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 7) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line8, line1, line2, line3, line4, line5, line6, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 8) && (output_line == 1))
		{
			filter_one_line_7x7<T,S,W>(line1, line2, line3, line4, line5, line6, line7, lineout2,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 8) && (output_line == 2))
		{
			filter_one_line_7x7<T,S,W>(line1, line2, line3, line4, line5, line6, line7, lineout1,
					c00, c01, c02, c03, c04, c05, c06,
					c10, c11, c12, c13, c14, c15, c16,
					c20, c21, c22, c23, c24, c25, c26,
					c30, c31, c32, c33, c34, c35, c36,
					c40, c41, c42, c43, c44, c45, c46,
					c50, c51, c52, c53, c54, c55, c56,
					c60, c61, c62, c63, c64, c65, c66,
					NbParallelInputWords, absVal);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		// second stage of the postamble - We do not read anymore new images and we do not do more processing
		if (output_line == 1)
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
		else
			writeline<T>(out_stream, lineout2, NbParallelInputWords);

		// last stage of the postamble - Write unchanged the last two read lines
		if ((new_read_line == 1))
		{
			writeline<T>(out_stream, line7, NbParallelInputWords);
			writeline<T>(out_stream, line8, NbParallelInputWords);
		}
		else if (new_read_line == 2)
		{
			writeline<T>(out_stream, line8, NbParallelInputWords);
			writeline<T>(out_stream, line1, NbParallelInputWords);
		}
		else if (new_read_line == 3)
		{
			writeline<T>(out_stream, line1, NbParallelInputWords);
			writeline<T>(out_stream, line2, NbParallelInputWords);
		}
		else if (new_read_line == 4)
		{
			writeline<T>(out_stream, line2, NbParallelInputWords);
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}
		else if (new_read_line == 5)
		{
			writeline<T>(out_stream, line3, NbParallelInputWords);
			writeline<T>(out_stream, line4, NbParallelInputWords);
		}
		else if (new_read_line == 6)
		{
			writeline<T>(out_stream, line4, NbParallelInputWords);
			writeline<T>(out_stream, line5, NbParallelInputWords);
		}
		else if (new_read_line == 7)
		{
			writeline<T>(out_stream, line5, NbParallelInputWords);
			writeline<T>(out_stream, line6, NbParallelInputWords);
		}
		else if (new_read_line == 8)
		{
			writeline<T>(out_stream, line6, NbParallelInputWords);
			writeline<T>(out_stream, line7, NbParallelInputWords);
		}

	}  // end for k, on the different images
}


// for the time being, S=8
// input and output images are supposed to be binary, i.e. Iij = 0 | 255;
// cij are the structuring element values. cij = 0 | 255
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void erode_one_line_2x2(T *inline1, T *inline2, T *outline,
					 short int c00, short int c01,
					 short int c10, short int c11,
					 unsigned short int size)
{
	T din1, din2, dout;
	short int di00, di01, di10, di11;
	int int_dout;
	int i,j;
	bool cond;
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

		if (i==0)
		{
			di00 = (din1.range(S-1, 0).to_int());
			di10 = (din2.range(S-1, 0).to_int());

//			dout.range(S-1,0) = din2.range(S-1,0);

			for (int j=1; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di01 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di11 = (din2.range(S * (j + 1) - 1, S * j).to_int());

				cond = ((c00==di00) && (c01==di01) && (c10==di10) && (c11==di11));

				if (cond)
					aux = ap_int < S > (di00);  // all the pixels match the corresponding values in the structuring element
				else
					aux = ap_int < S > (0);

				dout.range(S*j-1,S*(j-1)) = aux;
				di00 = di01;
				di10 = di11;

			 } // processed the PIXEL_IN_INPUT_WORD leftmost input pixels
		}
		else
		{
			// processing the last output pixel of previous word
			di01 = (din1.range(S-1,0).to_int());
			di11 = (din2.range(S-1,0).to_int());

			cond = ((c00==di00) && (c01==di01) && (c10==di10) && (c11==di11));

			if (cond)
				aux = ap_int < S > (di00);  // all the pixels match the corresponding values in the structuring element
			else
				aux = ap_int < S > (0);

			dout.range(W-1, W-S) = aux; //ap_uint < 16 > ((int)fdout);
			outline[i-1] = dout;

			di00 = di01;
			di10 = di11;

			for (int j=1; j<(W/S); j++)
		     {
		#pragma HLS unroll

				di01 = (din1.range(S * (j + 1) - 1, S * j).to_int());
				di11 = (din2.range(S * (j + 1) - 1, S * j).to_int());

				cond = ((c00==di00) && (c01==di01) && (c10==di10) && (c11==di11));

				if (cond)
					aux = ap_int < S > (di00);  // all the pixels match the corresponding values in the structuring element
				else
					aux = ap_int < S > (0);

				dout.range(S*j-1,S*(j-1)) = aux;
				di00 = di01;
				di10 = di11;
		     }
		}  //end else (i==0)
	}  // end for i, the one scanning the entire line
	// the last output pixel of the row is equal to the input pixel
	dout.range(W-1, W-S) = din1.range(W-1, W-S);
	outline[i-1] = dout;
}


/*
 * Erosion operator with 2x2 structuring element
 */

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void erosion_2x2(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				 unsigned short int ImgRows, unsigned short int ImgCols,
				 short int c00, short int c01,
				 short int c10,short int c11,
  				 unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32
	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T lineout1[MAX_COMPONENT_LINE_SIZE];
	T lineout2[MAX_COMPONENT_LINE_SIZE];
	short int new_read_line = 1;
	short int output_line = 1;
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
// first pipeline stage
		readline<T>(inStream,line1,NbParallelInputWords);

// second pipeline stage
		readline<T>(inStream,line2,NbParallelInputWords);

// third pipeline stage
		readline<T>(inStream,line3,NbParallelInputWords);
		erode_one_line_2x2<T,S,W>(line1, line2, lineout1, c00, c01, c10, c11, NbParallelInputWords);

		for (i=3; i<ImgRows; i++)
		{
			// fourth pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line2, line3, lineout2, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line2, line3, lineout1, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line3, line1, lineout2, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line3, line1, lineout1, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line1, line2, lineout2, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				erode_one_line_2x2<T,S,W>(line1, line2, lineout1, c00, c01, c10, c11, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			erode_one_line_2x2<T,S,W>(line2, line3, lineout2, c00, c01, c10, c11, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			erode_one_line_2x2<T,S,W>(line2, line3, lineout1, c00, c01, c10, c11, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			erode_one_line_2x2<T,S,W>(line3, line1, lineout2, c00, c01, c10, c11, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			erode_one_line_2x2<T,S,W>(line3, line1, lineout1, c00, c01, c10, c11, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			erode_one_line_2x2<T,S,W>(line1, line2, lineout2, c00, c01, c10, c11, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			erode_one_line_2x2<T,S,W>(line1, line2, lineout1, c00, c01, c10, c11, NbParallelInputWords);
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
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}
		else if (new_read_line == 2)
		{
			writeline<T>(out_stream, line1, NbParallelInputWords);
		}
		else if (new_read_line == 3)
		{
			writeline<T>(out_stream, line2, NbParallelInputWords);
		}

	}  // end for k, on the different images
}



// for the time being, S=8
// input and output images are supposed to be binary, i.e. Iij = 0 | 255;
// cij are the structuring element values. cij = 0 | 255
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void erode_one_line_3x3(T *inline1, T *inline2, T *inline3, T *outline,
					 short int c00, short int c01, short int c02,
					 short int c10, short int c11, short int c12,
					 short int c20, short int c21, short int c22,
					 unsigned short int size)
{
	T din1, din2, din3, dout;
	short int di00, di01, di02, di10, di11, di12, di20, di21, di22;
	int int_dout;
	int i,j;
	bool cond;
	ap_int<16> aux;

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

				cond = ((c00==di00) && (c01==di01) && (c02==di02) && (c10==di10) && (c11==di11) && (c12==di12) && (c20==di20) && (c21==di21) && (c22==di22));

				if (cond)
//					aux = ap_int < SIZE > (di11);  // all the pixels match the corresponding values in the structuring element
					aux = ap_int < SIZE > (255);
				else
					aux = ap_int < SIZE > (0);

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

			cond = ((c00==di00) && (c01==di01) && (c02==di02) && (c10==di10) && (c11==di11) && (c12==di12) && (c20==di20) && (c21==di21) && (c22==di22));

			if (cond)
//				aux = ap_int < SIZE > (di11);  // all the pixels match the corresponding values in the structuring element
				aux = ap_int < SIZE > (255);
			else
				aux = ap_int < SIZE > (0);

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

				cond = ((c00==di00) && (c01==di01) && (c02==di02) && (c10==di10) && (c11==di11) && (c12==di12) && (c20==di20) && (c21==di21) && (c22==di22));

				if (cond)
//					aux = ap_int < SIZE > (di11);  // all the pixels match the corresponding values in the structuring element
					aux = ap_int < SIZE > (255);
				else
					aux = ap_int < SIZE > (0);

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


/*
 * Erosion operator with 3x3 structuring element
 */

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void erosion_3x3(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				 unsigned short int ImgRows, unsigned short int ImgCols,
				 short int c00, short int c01, short int c02,
				 short int c10, short int c11, short int c12,
				 short int c20, short int c21, short int c22,
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
		erode_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);

		for (i=4; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				erode_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			erode_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			erode_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			erode_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			erode_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			erode_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			erode_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			erode_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			erode_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
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


/*
 * Dilation operator
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void dilate_one_line_3x3(T *inline1, T *inline2, T *inline3, T *outline,
					 short int c00, short int c01, short int c02,
					 short int c10, short int c11, short int c12,
					 short int c20, short int c21, short int c22,
					 unsigned short int size)
{
	T din1, din2, din3, dout;
	short int di00, di01, di02, di10, di11, di12, di20, di21, di22;
	int int_dout;
	int i,j;
	bool cond;
	ap_int<16> aux;

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

				cond = (((c00==di00)&&(di00==255)) ||
						((c01==di01)&&(di01==255)) ||
						((c02==di02)&&(di02==255)) ||
						((c10==di10)&&(di10==255)) ||
						((c11==di11)&&(di11==255)) ||
						((c12==di12)&&(di12==255)) ||
						((c20==di20)&&(di20==255)) ||
						((c21==di21)&&(di21==255)) ||
						((c22==di22)&&(di22==255)));

				if (cond)
					aux = ap_int < SIZE > (255);  // at least one pixel matches the corresponding 255 value in the structuring element
				else
					aux = ap_int < SIZE > (0);

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

			cond = (((c00==di00)&&(di00==255)) ||
					((c01==di01)&&(di01==255)) ||
					((c02==di02)&&(di02==255)) ||
					((c10==di10)&&(di10==255)) ||
					((c11==di11)&&(di11==255)) ||
					((c12==di12)&&(di12==255)) ||
					((c20==di20)&&(di20==255)) ||
					((c21==di21)&&(di21==255)) ||
					((c22==di22)&&(di22==255)));

			if (cond)
				aux = ap_int < SIZE > (255);  // at least one pixel matches the corresponding 255 value in the structuring element
			else
				aux = ap_int < SIZE > (0);

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

				cond = (((c00==di00)&&(di00==255)) ||
						((c01==di01)&&(di01==255)) ||
						((c02==di02)&&(di02==255)) ||
						((c10==di10)&&(di10==255)) ||
						((c11==di11)&&(di11==255)) ||
						((c12==di12)&&(di12==255)) ||
						((c20==di20)&&(di20==255)) ||
						((c21==di21)&&(di21==255)) ||
						((c22==di22)&&(di22==255)));

				if (cond)
					aux = ap_int < SIZE > (255);  // at least one pixel matches the corresponding 255 value in the structuring element
				else
					aux = ap_int < SIZE > (0);

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


/*
 * Dilation operator with 3x3 structuring element
 */

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void dilation_3x3(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				 unsigned short int ImgRows, unsigned short int ImgCols,
				 short int c00, short int c01, short int c02,
				 short int c10, short int c11, short int c12,
				 short int c20, short int c21, short int c22,
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
		dilate_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);

		for (i=4; i<ImgRows; i++)
		{
			// third pipeline stage - steady pipeline stage
			if ((new_read_line == 1) && (output_line == 1))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 2;
				output_line = 2;
			}
			else if ((new_read_line == 1) && (output_line == 2))
			{
				readline<T>(inStream,line1,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 2;
				output_line = 1;
			}
			else if ((new_read_line == 2) && (output_line == 1))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 3;
				output_line = 2;
			}
			else if ((new_read_line == 2) && (output_line == 2))
			{
				readline<T>(inStream,line2,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 3;
				output_line = 1;
			}
			else if ((new_read_line == 3) && (output_line == 1))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 4;
				output_line = 2;
			}
			else if ((new_read_line == 3) && (output_line == 2))
			{
				readline<T>(inStream,line3,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 4;
				output_line = 1;
			}
			else if ((new_read_line == 4) && (output_line == 1))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout1, NbParallelInputWords);
				new_read_line = 1;
				output_line = 2;
			}
			else if ((new_read_line == 4) && (output_line == 2))
			{
				readline<T>(inStream,line4,NbParallelInputWords);
				dilate_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
				writeline<T>(out_stream, lineout2, NbParallelInputWords);
				new_read_line = 1;
				output_line = 1;
			}
		}
		// first stage of the postamble - We do not read anymore new images
		if ((new_read_line == 1) && (output_line == 1))
		{
			dilate_one_line_3x3<T,S,W>(line2, line3, line4, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 1) && (output_line == 2))
		{
			dilate_one_line_3x3<T,S,W>(line2, line3, line4, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 2) && (output_line == 1))
		{
			dilate_one_line_3x3<T,S,W>(line3, line4, line1, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 2) && (output_line == 2))
		{
			dilate_one_line_3x3<T,S,W>(line3, line4, line1, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 3) && (output_line == 1))
		{
			dilate_one_line_3x3<T,S,W>(line4, line1, line2, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 3) && (output_line == 2))
		{
			dilate_one_line_3x3<T,S,W>(line4, line1, line2, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout2, NbParallelInputWords);
			output_line = 1;
		}
		else if ((new_read_line == 4) && (output_line == 1))
		{
			dilate_one_line_3x3<T,S,W>(line1, line2, line3, lineout2, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
			writeline<T>(out_stream, lineout1, NbParallelInputWords);
			output_line = 2;
		}
		else if ((new_read_line == 4) && (output_line == 2))
		{
			dilate_one_line_3x3<T,S,W>(line1, line2, line3, lineout1, c00, c01, c02, c10, c11, c12, c20, c21, c22, NbParallelInputWords);
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


/*
 * MEDIAN Filter
 */

void reorder(unsigned char i1, unsigned char i2, unsigned char &o1, unsigned char &o2)
{
#pragma HLS pipeline
	if (i1<i2)
	{
		o1 = i1;
		o2 = i2;
	}
	else
	{
		o1 = i2;
		o2 = i1;
	}
}

unsigned char Median3x3(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5, unsigned char c6, unsigned char c7, unsigned char c8)
{
#pragma HLS pipeline
	unsigned char c00, c01, c02, c03, c04, c05, c06, c07, c08;  //output first stage
	unsigned char c10, c11, c12, c13, c14, c15, c16, c17, c18;  //output second stage
	unsigned char c20, c21, c22, c23, c24, c25, c26, c27, c28;  //output third stage
	unsigned char c30, c31, c32, c33, c34, c35, c36, c37, c38;  //output fourth stage
	unsigned char c40, c41, c42, c43, c44, c45, c46, c47, c48;  //output fifth stage
	unsigned char c50, c51, c52, c53, c54, c55, c56, c57, c58;  //output sixth stage
	unsigned char c60, c61, c62, c63, c64, c65, c66, c67, c68;  //output seventh stage
	unsigned char c70, c71, c72, c73, c74, c75, c76, c77, c78;  //output eighth stage
	unsigned char c80, c81, c82, c83, c84, c85, c86, c87, c88;  //output nineth stage

	//first and second stages
	reorder(c0, c1, c00, c01);
	reorder(c2, c3, c02, c03);
	reorder(c4, c5, c04, c05);
	reorder(c6, c7, c06, c07);
	c08 = c8;
	c10 = c00;
	reorder(c01, c02, c11, c12);
	reorder(c03, c04, c13, c14);
	reorder(c05, c06, c15, c16);
	reorder(c07, c08, c17, c18);
	//3rd and 4th stages
	reorder(c10, c11, c20, c21);
	reorder(c12, c13, c22, c23);
	reorder(c14, c15, c24, c25);
	reorder(c16, c17, c26, c27);
	c28 = c18;
	c30 = c20;
	reorder(c21, c22, c31, c32);
	reorder(c23, c24, c33, c34);
	reorder(c25, c26, c35, c36);
	reorder(c27, c28, c37, c38);
	//5th and 6th stages
	reorder(c30, c31, c40, c41);
	reorder(c32, c33, c42, c43);
	reorder(c34, c35, c44, c45);
	reorder(c36, c37, c46, c47);
	c48 = c38;
	c50 = c40;
	reorder(c41, c42, c51, c52);
	reorder(c43, c44, c53, c54);
	reorder(c45, c46, c55, c56);
	reorder(c47, c48, c57, c58);
	//7th and 8th stages
	reorder(c52, c53, c62, c63);
	reorder(c54, c55, c64, c65);
	reorder(c56, c57, c66, c67);
	reorder(c63, c64, c73, c74);
	reorder(c65, c66, c75, c76);
	//9th stage
	reorder(c74, c75, c84, c85);

	return c84;

}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void median_3x3_filter_one_line(T *inline1, T *inline2, T *inline3, T *outline,
					 unsigned short int size)
{
	T din1, din2, din3, dout;
	unsigned char di00, di01, di02, di10, di11, di12, di20, di21, di22;
	unsigned char int_dout;
	int i,j;
	ap_int<S> aux;

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

				int_dout = Median3x3(di00, di01, di02, di10, di11, di12, di20, di21, di22);
				aux = ap_int < S > (int_dout);
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
			di02 = (din1.range(S-1,0));
			di12 = (din2.range(S-1,0));
			di22 = (din3.range(S-1,0));

			int_dout = Median3x3(di00, di01, di02, di10, di11, di12, di20, di21, di22);
			aux = ap_int < S > (int_dout);
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

				di02 = (din1.range(S * (j + 1) - 1, S * j));
				di12 = (din2.range(S * (j + 1) - 1, S * j));
				di22 = (din3.range(S * (j + 1) - 1, S * j));

				int_dout = Median3x3(di00, di01, di02, di10, di11, di12, di20, di21, di22);
				aux = ap_int < S > (int_dout);
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
void do_3x3_median_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
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
				median_3x3_filter_one_line<T,S,W>(line1, line2, line3, lineoutA, NbParallelInputWords);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line1,NbParallelInputWords);
				median_3x3_filter_one_line<T,S,W>(line2, line3, line4, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);

				readline<T>(inStream,line2,NbParallelInputWords);
				median_3x3_filter_one_line<T,S,W>(line3, line4, line1, lineoutA, NbParallelInputWords);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line3,NbParallelInputWords);
				median_3x3_filter_one_line<T,S,W>(line4, line1, line2, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
			}
		}
		for (; i<ImgRows-3; i+=4)
		{
			readline<T>(inStream,line4,NbParallelInputWords);
			median_3x3_filter_one_line<T,S,W>(line1, line2, line3, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line1,NbParallelInputWords);
			median_3x3_filter_one_line<T,S,W>(line2, line3, line4, lineoutB, NbParallelInputWords);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);

			readline<T>(inStream,line2,NbParallelInputWords);
			median_3x3_filter_one_line<T,S,W>(line3, line4, line1, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line3,NbParallelInputWords);
			median_3x3_filter_one_line<T,S,W>(line4, line1, line2, lineoutB, NbParallelInputWords);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);
		}

		if (i<ImgRows)
		{  // I can have still 3 lines to read
			i++;
			median_3x3_filter_one_line<T,S,W>(line1, line2, line3, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			readline<T>(inStream, line4, NbParallelInputWords);
			if (i<ImgRows)
			{  // I can have still 2 lines to read
				i++;
				median_3x3_filter_one_line<T,S,W>(line2, line3, line4, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				readline<T>(inStream, line1, NbParallelInputWords);
				if (i<ImgRows)
				{  // I can have still 1 line to read
					i++;
					median_3x3_filter_one_line<T,S,W>(line3, line4, line1, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					readline<T>(inStream, line2, NbParallelInputWords); // line2 is the last line of the input image
					median_3x3_filter_one_line<T,S,W>(line4, line1, line2, lineoutB, NbParallelInputWords);
					writeline<T>(out_stream, lineoutB, NbParallelInputWords);
					writeline<T>(out_stream, line2, NbParallelInputWords);
				}
				else // line1 is the last line of the input image
				{
					median_3x3_filter_one_line<T,S,W>(line3, line4, line1, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, line1, NbParallelInputWords);
				}
			}
			else // line4 is the last line of the input image
			{
				median_3x3_filter_one_line<T,S,W>(line2, line3, line4, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, line4, NbParallelInputWords);
			}

		}
		else //line3 is the last line of the input image
		{
			median_3x3_filter_one_line<T,S,W>(line1, line2, line3, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, line3, NbParallelInputWords);
		}
	}
}


unsigned char Median5x5_even_odd_sorting_network(unsigned char c0_0,
			unsigned char c0_1,
			unsigned char c0_2,
			unsigned char c0_3,
			unsigned char c0_4,
			unsigned char c0_5,
			unsigned char c0_6,
			unsigned char c0_7,
			unsigned char c0_8,
			unsigned char c0_9,
			unsigned char c0_10,
			unsigned char c0_11,
			unsigned char c0_12,
			unsigned char c0_13,
			unsigned char c0_14,
			unsigned char c0_15,
			unsigned char c0_16,
			unsigned char c0_17,
			unsigned char c0_18,
			unsigned char c0_19,
			unsigned char c0_20,
			unsigned char c0_21,
			unsigned char c0_22,
			unsigned char c0_23,
			unsigned char c0_24)

{
#pragma HLS pipeline
	unsigned char c1_0, c1_1, c1_2, c1_3, c1_4, c1_5, c1_6, c1_7, c1_8, c1_9, c1_10, c1_11, c1_12, c1_13, c1_14, c1_15, c1_16, c1_17, c1_18, c1_19, c1_20, c1_21, c1_22, c1_23, c1_24;
	unsigned char c2_0, c2_1, c2_2, c2_3, c2_4, c2_5, c2_6, c2_7, c2_8, c2_9, c2_10, c2_11, c2_12, c2_13, c2_14, c2_15, c2_16, c2_17, c2_18, c2_19, c2_20, c2_21, c2_22, c2_23, c2_24;
	unsigned char c3_0, c3_1, c3_2, c3_3, c3_4, c3_5, c3_6, c3_7, c3_8, c3_9, c3_10, c3_11, c3_12, c3_13, c3_14, c3_15, c3_16, c3_17, c3_18, c3_19, c3_20, c3_21, c3_22, c3_23, c3_24;
	unsigned char c4_0, c4_1, c4_2, c4_3, c4_4, c4_5, c4_6, c4_7, c4_8, c4_9, c4_10, c4_11, c4_12, c4_13, c4_14, c4_15, c4_16, c4_17, c4_18, c4_19, c4_20, c4_21, c4_22, c4_23, c4_24;
	unsigned char c5_0, c5_1, c5_2, c5_3, c5_4, c5_5, c5_6, c5_7, c5_8, c5_9, c5_10, c5_11, c5_12, c5_13, c5_14, c5_15, c5_16, c5_17, c5_18, c5_19, c5_20, c5_21, c5_22, c5_23, c5_24;
	unsigned char c6_0, c6_1, c6_2, c6_3, c6_4, c6_5, c6_6, c6_7, c6_8, c6_9, c6_10, c6_11, c6_12, c6_13, c6_14, c6_15, c6_16, c6_17, c6_18, c6_19, c6_20, c6_21, c6_22, c6_23, c6_24;
	unsigned char c7_0, c7_1, c7_2, c7_3, c7_4, c7_5, c7_6, c7_7, c7_8, c7_9, c7_10, c7_11, c7_12, c7_13, c7_14, c7_15, c7_16, c7_17, c7_18, c7_19, c7_20, c7_21, c7_22, c7_23, c7_24;
	unsigned char c8_0, c8_1, c8_2, c8_3, c8_4, c8_5, c8_6, c8_7, c8_8, c8_9, c8_10, c8_11, c8_12, c8_13, c8_14, c8_15, c8_16, c8_17, c8_18, c8_19, c8_20, c8_21, c8_22, c8_23, c8_24;
	unsigned char c9_0, c9_1, c9_2, c9_3, c9_4, c9_5, c9_6, c9_7, c9_8, c9_9, c9_10, c9_11, c9_12, c9_13, c9_14, c9_15, c9_16, c9_17, c9_18, c9_19, c9_20, c9_21, c9_22, c9_23, c9_24;
	unsigned char c10_0, c10_1, c10_2, c10_3, c10_4, c10_5, c10_6, c10_7, c10_8, c10_9, c10_10, c10_11, c10_12, c10_13, c10_14, c10_15, c10_16, c10_17, c10_18, c10_19, c10_20, c10_21, c10_22, c10_23, c10_24;
	unsigned char c11_0, c11_1, c11_2, c11_3, c11_4, c11_5, c11_6, c11_7, c11_8, c11_9, c11_10, c11_11, c11_12, c11_13, c11_14, c11_15, c11_16, c11_17, c11_18, c11_19, c11_20, c11_21, c11_22, c11_23, c11_24;
	unsigned char c12_0, c12_1, c12_2, c12_3, c12_4, c12_5, c12_6, c12_7, c12_8, c12_9, c12_10, c12_11, c12_12, c12_13, c12_14, c12_15, c12_16, c12_17, c12_18, c12_19, c12_20, c12_21, c12_22, c12_23, c12_24;
	unsigned char c13_0, c13_1, c13_2, c13_3, c13_4, c13_5, c13_6, c13_7, c13_8, c13_9, c13_10, c13_11, c13_12, c13_13, c13_14, c13_15, c13_16, c13_17, c13_18, c13_19, c13_20, c13_21, c13_22, c13_23, c13_24;
	unsigned char c14_0, c14_1, c14_2, c14_3, c14_4, c14_5, c14_6, c14_7, c14_8, c14_9, c14_10, c14_11, c14_12, c14_13, c14_14, c14_15, c14_16, c14_17, c14_18, c14_19, c14_20, c14_21, c14_22, c14_23, c14_24;
	unsigned char c15_0, c15_1, c15_2, c15_3, c15_4, c15_5, c15_6, c15_7, c15_8, c15_9, c15_10, c15_11, c15_12, c15_13, c15_14, c15_15, c15_16, c15_17, c15_18, c15_19, c15_20, c15_21, c15_22, c15_23, c15_24;
//	reordering stage number 1
	reorder(c0_0,c0_16,c1_0,c1_16);
	reorder(c0_1,c0_17,c1_1,c1_17);
	reorder(c0_2,c0_18,c1_2,c1_18);
	reorder(c0_3,c0_19,c1_3,c1_19);
	reorder(c0_4,c0_20,c1_4,c1_20);
	reorder(c0_5,c0_21,c1_5,c1_21);
	reorder(c0_6,c0_22,c1_6,c1_22);
	reorder(c0_7,c0_23,c1_7,c1_23);
	reorder(c0_8,c0_24,c1_8,c1_24);
	c1_9 = c0_9;
	c1_10 = c0_10;
	c1_11 = c0_11;
	c1_12 = c0_12;
	c1_13 = c0_13;
	c1_14 = c0_14;
	c1_15 = c0_15;
//	reordering stage number 2
	reorder(c1_0,c1_8,c2_0,c2_8);
	reorder(c1_1,c1_9,c2_1,c2_9);
	reorder(c1_2,c1_10,c2_2,c2_10);
	reorder(c1_3,c1_11,c2_3,c2_11);
	reorder(c1_4,c1_12,c2_4,c2_12);
	reorder(c1_5,c1_13,c2_5,c2_13);
	reorder(c1_6,c1_14,c2_6,c2_14);
	reorder(c1_7,c1_15,c2_7,c2_15);
	reorder(c1_16,c1_24,c2_16,c2_24);
	c2_17 = c1_17;
	c2_18 = c1_18;
	c2_19 = c1_19;
	c2_20 = c1_20;
	c2_21 = c1_21;
	c2_22 = c1_22;
	c2_23 = c1_23;
//	reordering stage number 3
	reorder(c2_8,c2_16,c3_8,c3_16);
	reorder(c2_9,c2_17,c3_9,c3_17);
	reorder(c2_10,c2_18,c3_10,c3_18);
	reorder(c2_11,c2_19,c3_11,c3_19);
	reorder(c2_12,c2_20,c3_12,c3_20);
	reorder(c2_13,c2_21,c3_13,c3_21);
	reorder(c2_14,c2_22,c3_14,c3_22);
	reorder(c2_15,c2_23,c3_15,c3_23);
	c3_0 = c2_0;
	c3_1 = c2_1;
	c3_2 = c2_2;
	c3_3 = c2_3;
	c3_4 = c2_4;
	c3_5 = c2_5;
	c3_6 = c2_6;
	c3_7 = c2_7;
	c3_24 = c2_24;
//	reordering stage number 4
	reorder(c3_0,c3_4,c4_0,c4_4);
	reorder(c3_1,c3_5,c4_1,c4_5);
	reorder(c3_2,c3_6,c4_2,c4_6);
	reorder(c3_3,c3_7,c4_3,c4_7);
	reorder(c3_8,c3_12,c4_8,c4_12);
	reorder(c3_9,c3_13,c4_9,c4_13);
	reorder(c3_10,c3_14,c4_10,c4_14);
	reorder(c3_11,c3_15,c4_11,c4_15);
	reorder(c3_16,c3_20,c4_16,c4_20);
	reorder(c3_17,c3_21,c4_17,c4_21);
	reorder(c3_18,c3_22,c4_18,c4_22);
	reorder(c3_19,c3_23,c4_19,c4_23);
	c4_24 = c3_24;
//	reordering stage number 5
	reorder(c4_4,c4_16,c5_4,c5_16);
	reorder(c4_5,c4_17,c5_5,c5_17);
	reorder(c4_6,c4_18,c5_6,c5_18);
	reorder(c4_7,c4_19,c5_7,c5_19);
	reorder(c4_12,c4_24,c5_12,c5_24);
	c5_0 = c4_0;
	c5_1 = c4_1;
	c5_2 = c4_2;
	c5_3 = c4_3;
	c5_8 = c4_8;
	c5_9 = c4_9;
	c5_10 = c4_10;
	c5_11 = c4_11;
	c5_13 = c4_13;
	c5_14 = c4_14;
	c5_15 = c4_15;
	c5_20 = c4_20;
	c5_21 = c4_21;
	c5_22 = c4_22;
	c5_23 = c4_23;
//	reordering stage number 6
	reorder(c5_4,c5_8,c6_4,c6_8);
	reorder(c5_5,c5_9,c6_5,c6_9);
	reorder(c5_6,c5_10,c6_6,c6_10);
	reorder(c5_7,c5_11,c6_7,c6_11);
	reorder(c5_12,c5_16,c6_12,c6_16);
	reorder(c5_13,c5_17,c6_13,c6_17);
	reorder(c5_14,c5_18,c6_14,c6_18);
	reorder(c5_15,c5_19,c6_15,c6_19);
	reorder(c5_20,c5_24,c6_20,c6_24);
	c6_0 = c5_0;
	c6_1 = c5_1;
	c6_2 = c5_2;
	c6_3 = c5_3;
	c6_21 = c5_21;
	c6_22 = c5_22;
	c6_23 = c5_23;
//	reordering stage number 7
	reorder(c6_0,c6_2,c7_0,c7_2);
	reorder(c6_1,c6_3,c7_1,c7_3);
	reorder(c6_4,c6_6,c7_4,c7_6);
	reorder(c6_5,c6_7,c7_5,c7_7);
	reorder(c6_8,c6_10,c7_8,c7_10);
	reorder(c6_9,c6_11,c7_9,c7_11);
	reorder(c6_12,c6_14,c7_12,c7_14);
	reorder(c6_13,c6_15,c7_13,c7_15);
	reorder(c6_16,c6_18,c7_16,c7_18);
	reorder(c6_17,c6_19,c7_17,c7_19);
	reorder(c6_20,c6_22,c7_20,c7_22);
	reorder(c6_21,c6_23,c7_21,c7_23);
	c7_24 = c6_24;
//	reordering stage number 8
	reorder(c7_2,c7_16,c8_2,c8_16);
	reorder(c7_3,c7_17,c8_3,c8_17);
	reorder(c7_6,c7_20,c8_6,c8_20);
	reorder(c7_7,c7_21,c8_7,c8_21);
	reorder(c7_10,c7_24,c8_10,c8_24);
	c8_0 = c7_0;
	c8_1 = c7_1;
	c8_4 = c7_4;
	c8_5 = c7_5;
	c8_8 = c7_8;
	c8_9 = c7_9;
	c8_11 = c7_11;
	c8_12 = c7_12;
	c8_13 = c7_13;
	c8_14 = c7_14;
	c8_15 = c7_15;
	c8_18 = c7_18;
	c8_19 = c7_19;
	c8_22 = c7_22;
	c8_23 = c7_23;
//	reordering stage number 9
	reorder(c8_2,c8_8,c9_2,c9_8);
	reorder(c8_3,c8_9,c9_3,c9_9);
	reorder(c8_6,c8_12,c9_6,c9_12);
	reorder(c8_7,c8_13,c9_7,c9_13);
	reorder(c8_10,c8_16,c9_10,c9_16);
	reorder(c8_11,c8_17,c9_11,c9_17);
	reorder(c8_14,c8_20,c9_14,c9_20);
	reorder(c8_15,c8_21,c9_15,c9_21);
	reorder(c8_18,c8_24,c9_18,c9_24);
	c9_0 = c8_0;
	c9_1 = c8_1;
	c9_4 = c8_4;
	c9_5 = c8_5;
	c9_19 = c8_19;
	c9_22 = c8_22;
	c9_23 = c8_23;
//	reordering stage number 10
	reorder(c9_2,c9_4,c10_2,c10_4);
	reorder(c9_3,c9_5,c10_3,c10_5);
	reorder(c9_6,c9_8,c10_6,c10_8);
	reorder(c9_7,c9_9,c10_7,c10_9);
	reorder(c9_10,c9_12,c10_10,c10_12);
	reorder(c9_11,c9_13,c10_11,c10_13);
	reorder(c9_14,c9_16,c10_14,c10_16);
	reorder(c9_15,c9_17,c10_15,c10_17);
	reorder(c9_18,c9_20,c10_18,c10_20);
	reorder(c9_19,c9_21,c10_19,c10_21);
	reorder(c9_22,c9_24,c10_22,c10_24);
	c10_0 = c9_0;
	c10_1 = c9_1;
	c10_23 = c9_23;
//	reordering stage number 11
	reorder(c10_0,c10_1,c11_0,c11_1);
	reorder(c10_2,c10_3,c11_2,c11_3);
	reorder(c10_4,c10_5,c11_4,c11_5);
	reorder(c10_6,c10_7,c11_6,c11_7);
	reorder(c10_8,c10_9,c11_8,c11_9);
	reorder(c10_10,c10_11,c11_10,c11_11);
	reorder(c10_12,c10_13,c11_12,c11_13);
	reorder(c10_14,c10_15,c11_14,c11_15);
	reorder(c10_16,c10_17,c11_16,c11_17);
	reorder(c10_18,c10_19,c11_18,c11_19);
	reorder(c10_20,c10_21,c11_20,c11_21);
	reorder(c10_22,c10_23,c11_22,c11_23);
	c11_24 = c10_24;
//	reordering stage number 12
	reorder(c11_1,c11_16,c12_1,c12_16);
	reorder(c11_3,c11_18,c12_3,c12_18);
	reorder(c11_5,c11_20,c12_5,c12_20);
	reorder(c11_7,c11_22,c12_7,c12_22);
	reorder(c11_9,c11_24,c12_9,c12_24);
	c12_0 = c11_0;
	c12_2 = c11_2;
	c12_4 = c11_4;
	c12_6 = c11_6;
	c12_8 = c11_8;
	c12_10 = c11_10;
	c12_11 = c11_11;
	c12_12 = c11_12;
	c12_13 = c11_13;
	c12_14 = c11_14;
	c12_15 = c11_15;
	c12_17 = c11_17;
	c12_19 = c11_19;
	c12_21 = c11_21;
	c12_23 = c11_23;
//	reordering stage number 13
	reorder(c12_1,c12_8,c13_1,c13_8);
	reorder(c12_3,c12_10,c13_3,c13_10);
	reorder(c12_5,c12_12,c13_5,c13_12);
	reorder(c12_7,c12_14,c13_7,c13_14);
	reorder(c12_9,c12_16,c13_9,c13_16);
	reorder(c12_11,c12_18,c13_11,c13_18);
	reorder(c12_13,c12_20,c13_13,c13_20);
	reorder(c12_15,c12_22,c13_15,c13_22);
	reorder(c12_17,c12_24,c13_17,c13_24);
	c13_0 = c12_0;
	c13_2 = c12_2;
	c13_4 = c12_4;
	c13_6 = c12_6;
	c13_19 = c12_19;
	c13_21 = c12_21;
	c13_23 = c12_23;
//	reordering stage number 14
	reorder(c13_1,c13_4,c14_1,c14_4);
	reorder(c13_3,c13_6,c14_3,c14_6);
	reorder(c13_5,c13_8,c14_5,c14_8);
	reorder(c13_7,c13_10,c14_7,c14_10);
	reorder(c13_9,c13_12,c14_9,c14_12);
	reorder(c13_11,c13_14,c14_11,c14_14);
	reorder(c13_13,c13_16,c14_13,c14_16);
	reorder(c13_15,c13_18,c14_15,c14_18);
	reorder(c13_17,c13_20,c14_17,c14_20);
	reorder(c13_19,c13_22,c14_19,c14_22);
	reorder(c13_21,c13_24,c14_21,c14_24);
	c14_0 = c13_0;
	c14_2 = c13_2;
	c14_23 = c13_23;
//	reordering stage number 15
	reorder(c14_1,c14_2,c15_1,c15_2);
	reorder(c14_3,c14_4,c15_3,c15_4);
	reorder(c14_5,c14_6,c15_5,c15_6);
	reorder(c14_7,c14_8,c15_7,c15_8);
	reorder(c14_9,c14_10,c15_9,c15_10);
	reorder(c14_11,c14_12,c15_11,c15_12);
	reorder(c14_13,c14_14,c15_13,c15_14);
	reorder(c14_15,c14_16,c15_15,c15_16);
	reorder(c14_17,c14_18,c15_17,c15_18);
	reorder(c14_19,c14_20,c15_19,c15_20);
	reorder(c14_21,c14_22,c15_21,c15_22);
	reorder(c14_23,c14_24,c15_23,c15_24);
	c15_0 = c14_0;
	return c15_12;
}
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void median_filter_5x5_one_line(T *inline1, T *inline2, T *inline3, T *inline4, T *inline5, T *outline, unsigned short int size)
{
	T  din1,  din2,  din3,  din4,  din5, dout;
	unsigned char di00, di01, di02, di03, di04;
	unsigned char di10, di11, di12, di13, di14;
	unsigned char di20, di21, di22, di23, di24;
	unsigned char di30, di31, di32, di33, di34;
	unsigned char di40, di41, di42, di43, di44;
	unsigned char int_dout;
	int i,j;
	ap_int<S> aux;

	for (i=0; i<size; i++)

	{
#pragma HLS pipeline

		din1 = inline1[i];
		din2 = inline2[i];
		din3 = inline3[i];
		din4 = inline4[i];
		din5 = inline5[i];

		if (i==0)
		{
			di00 = (din1.range(S-1, 0));
			di10 = (din2.range(S-1, 0));
			di20 = (din3.range(S-1, 0));
			di30 = (din4.range(S-1, 0));
			di40 = (din5.range(S-1, 0));
			di01 = (din1.range(2*S-1, S));
			di11 = (din2.range(2*S-1, S));
			di21 = (din3.range(2*S-1, S));
			di31 = (din4.range(2*S-1, S));
			di41 = (din5.range(2*S-1, S));
			di02 = (din1.range(3*S-1, 2*S));
			di12 = (din2.range(3*S-1, 2*S));
			di22 = (din3.range(3*S-1, 2*S));
			di32 = (din4.range(3*S-1, 2*S));
			di42 = (din5.range(3*S-1, 2*S));
			di03 = (din1.range(4*S-1, 3*S));
			di13 = (din2.range(4*S-1, 3*S));
			di23 = (din3.range(4*S-1, 3*S));
			di33 = (din4.range(4*S-1, 3*S));
			di43 = (din5.range(4*S-1, 3*S));

			dout.range(S-1,0) = din3.range(S-1,0);
			dout.range(2*S-1,S) = din3.range(2*S-1,S);

			for (int j=4; j<(W/S); j++)
			 {
		#pragma HLS unroll

				di04 = (din1.range(S * (j + 1) - 1, S * j));
				di14 = (din2.range(S * (j + 1) - 1, S * j));
				di24 = (din3.range(S * (j + 1) - 1, S * j));
				di34 = (din4.range(S * (j + 1) - 1, S * j));
				di44 = (din5.range(S * (j + 1) - 1, S * j));

				int_dout = Median5x5_even_odd_sorting_network(di00, di01, di02, di03, di04, di10, di11, di12, di13, di14, di20, di21, di22, di23, di24, di30, di31, di32, di33, di34, di40, di41, di42, di43, di44);
				aux = ap_int < S > (int_dout);
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
			di04 = (din1.range(S-1,0));
			di14 = (din2.range(S-1,0));
			di24 = (din3.range(S-1,0));
			di34 = (din4.range(S-1,0));
			di44 = (din5.range(S-1,0));

			int_dout = Median5x5_even_odd_sorting_network(di00, di01, di02, di03, di04, di10, di11, di12, di13, di14, di20, di21, di22, di23, di24, di30, di31, di32, di33, di34, di40, di41, di42, di43, di44);
			aux = ap_int < S > (int_dout);
			dout.range(W-S-1,W-2*S) = aux; //ap_uint < 16 > ((int)fdout);

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

			di04 = (din1.range(2*S-1,S));
			di14 = (din2.range(2*S-1,S));
			di24 = (din3.range(2*S-1,S));
			di34 = (din4.range(2*S-1,S));
			di44 = (din5.range(2*S-1,S));

			int_dout = Median5x5_even_odd_sorting_network(di00, di01, di02, di03, di04, di10, di11, di12, di13, di14, di20, di21, di22, di23, di24, di30, di31, di32, di33, di34, di40, di41, di42, di43, di44);
			aux = ap_int < S > (int_dout);
			dout.range(W-1, W-S) = aux; //ap_uint < 16 > ((int)fdout);
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

				di04 = (din1.range(S * (j + 1) - 1, S * j));
				di14 = (din2.range(S * (j + 1) - 1, S * j));
				di24 = (din3.range(S * (j + 1) - 1, S * j));
				di34 = (din4.range(S * (j + 1) - 1, S * j));
				di44 = (din5.range(S * (j + 1) - 1, S * j));

				int_dout = Median5x5_even_odd_sorting_network(di00, di01, di02, di03, di04, di10, di11, di12, di13, di14, di20, di21, di22, di23, di24, di30, di31, di32, di33, di34, di40, di41, di42, di43, di44);
				aux = ap_int < S > (int_dout);
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
	dout.range(W-S-1,W-2*S) = din3.range(W-S-1,W-2*S); //ap_uint < 16 > ((int)fdout);
	dout.range(W-1, W-S) = din3.range(W-1, W-S);
	outline[i-1] = dout;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void do_5x5_median_filtering(hls::stream<T>& inStream,hls::stream<T>& out_stream,
				  unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
{
	int i;
	// ImgCols*16/N is the number of dt type elements contained in an image row; for ImgCols = 1024, and N=512 bit, colSize = 32

	T line1[MAX_COMPONENT_LINE_SIZE];
	T line2[MAX_COMPONENT_LINE_SIZE];
	T line3[MAX_COMPONENT_LINE_SIZE];
	T line4[MAX_COMPONENT_LINE_SIZE];
	T line5[MAX_COMPONENT_LINE_SIZE];
	T line6[MAX_COMPONENT_LINE_SIZE];
	T lineoutA[MAX_COMPONENT_LINE_SIZE];
	T lineoutB[MAX_COMPONENT_LINE_SIZE];
	const unsigned short int NbParallelInputWords = ImgCols*S/W;
	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,line1,NbParallelInputWords);
		writeline<T>(out_stream, line1, NbParallelInputWords); // the first line is written without being processed
		readline<T>(inStream,line2,NbParallelInputWords);
		writeline<T>(out_stream, line2, NbParallelInputWords); // the first line is written without being processed
		readline<T>(inStream,line3,NbParallelInputWords);
		readline<T>(inStream,line4,NbParallelInputWords);
		readline<T>(inStream,line5,NbParallelInputWords);
		for (i=5; i<ImgRows-(6*UF-1); i+=(6*UF))
		{
			for (int j=0; j<UF; j++)
			{
			#pragma HLS unroll
				readline<T>(inStream,line6,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line1, line2, line3, line4, line5, lineoutA, NbParallelInputWords);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line1,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line2, line3, line4, line5, line6, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);

				readline<T>(inStream,line2,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line3, line4, line5, line6, line1, lineoutA, NbParallelInputWords);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line3,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line4, line5, line6, line1, line2, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);

				readline<T>(inStream,line4,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line5, line6, line1, line2, line3, lineoutA, NbParallelInputWords);
				writeline<T>(out_stream, lineoutA, NbParallelInputWords);

				readline<T>(inStream,line5,NbParallelInputWords);
				median_filter_5x5_one_line<T,S,W>(line6, line1, line2, line3, line4, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
			}
		}
		for (; i<ImgRows-5; i+=6)
		{
			readline<T>(inStream,line6,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line1, line2, line3, line4, line5, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line1,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line2, line3, line4, line5, line6, lineoutB, NbParallelInputWords);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);

			readline<T>(inStream,line2,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line3, line4, line5, line6, line1, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line3,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line4, line5, line6, line1, line2, lineoutB, NbParallelInputWords);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);

			readline<T>(inStream,line4,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line5, line6, line1, line2, line3, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);

			readline<T>(inStream,line5,NbParallelInputWords);
			median_filter_5x5_one_line<T,S,W>(line6, line1, line2, line3, line4, lineoutB, NbParallelInputWords);
			writeline<T>(out_stream, lineoutB, NbParallelInputWords);
		}

		if (i<ImgRows)
		{  // I can have still 5 lines to read
			i++;
			median_filter_5x5_one_line<T,S,W>(line1, line2, line3, line4, line5, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			readline<T>(inStream,line6,NbParallelInputWords);

			if (i<ImgRows)
			{  // I can have still 4 lines to read
				i++;
				median_filter_5x5_one_line<T,S,W>(line2, line3, line4, line5, line6, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				readline<T>(inStream,line1,NbParallelInputWords);
				if (i<ImgRows)
				{  // I can have still 3 lines to read
					i++;
					median_filter_5x5_one_line<T,S,W>(line3, line4, line5, line6, line1, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					readline<T>(inStream,line2,NbParallelInputWords);
					if (i<ImgRows)
					{  // I can have still 2 lines to read
						i++;
						median_filter_5x5_one_line<T,S,W>(line4, line5, line6, line1, line2, lineoutB, NbParallelInputWords);
						writeline<T>(out_stream, lineoutB, NbParallelInputWords);
						readline<T>(inStream,line3,NbParallelInputWords);
						if (i<ImgRows)
						{  // I can have still 1 lines to read
							i++;
							median_filter_5x5_one_line<T,S,W>(line5, line6, line1, line2, line3, lineoutA, NbParallelInputWords);
							writeline<T>(out_stream, lineoutA, NbParallelInputWords);
							readline<T>(inStream,line4,NbParallelInputWords);

							median_filter_5x5_one_line<T,S,W>(line6, line1, line2, line3, line4, lineoutB, NbParallelInputWords);

							writeline<T>(out_stream, lineoutB, NbParallelInputWords);
							writeline<T>(out_stream, line3, NbParallelInputWords);
							writeline<T>(out_stream, line4, NbParallelInputWords);

						}
						else // line3 is the last line of the input image
						{
							median_filter_5x5_one_line<T,S,W>(line5, line6, line1, line2, line3, lineoutA, NbParallelInputWords);
							writeline<T>(out_stream, lineoutA, NbParallelInputWords);
							writeline<T>(out_stream, line2, NbParallelInputWords);
							writeline<T>(out_stream, line3, NbParallelInputWords);
						}
					}
					else  // line2 is the last line of the input image
					{
						median_filter_5x5_one_line<T,S,W>(line4, line5, line6, line1, line2, lineoutB, NbParallelInputWords);
						writeline<T>(out_stream, lineoutB, NbParallelInputWords);
						writeline<T>(out_stream, line1, NbParallelInputWords);
						writeline<T>(out_stream, line2, NbParallelInputWords);

					}
				}
				else // line1 is the last line of the image
				{
					median_filter_5x5_one_line<T,S,W>(line3, line4, line5, line6, line1, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, lineoutA, NbParallelInputWords);
					writeline<T>(out_stream, line6, NbParallelInputWords);
					writeline<T>(out_stream, line1, NbParallelInputWords);

				}
			}
			else  // line6 is the last line of the input image
			{
				median_filter_5x5_one_line<T,S,W>(line2, line3, line4, line5, line6, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, lineoutB, NbParallelInputWords);
				writeline<T>(out_stream, line5, NbParallelInputWords);
				writeline<T>(out_stream, line6, NbParallelInputWords);
			}
		}
		else // line1 is the last line of the input image
		{
			median_filter_5x5_one_line<T,S,W>(line1, line2, line3, line4, line5, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, lineoutA, NbParallelInputWords);
			writeline<T>(out_stream, line4, NbParallelInputWords);
			writeline<T>(out_stream, line5, NbParallelInputWords);

		}
	}  // end for k<NbImages
}

